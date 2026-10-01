# Architecture Decision Records (ADRs)

> This document captures all key architectural and technical decisions made during the design phase of the VAG CAN Infotainment project, along with the reasoning behind each choice.

---

## ADR-001: Dual-Controller Architecture (Pi 4 + ESP32)

**Context:**
The project requires interacting with two separate CAN Bus networks in the SEAT Ibiza 6L — Powertrain (500 kbps) and Comfort (100 kbps) — with different responsibilities: reading telemetry data and sending control commands.

**Decision:**
Use a **Raspberry Pi 4** for the Powertrain network and an **ESP32** for the Comfort network.

**Rationale:**
- Clear **separation of concerns** — reading data vs. controlling actuators
- The Pi 4 is better suited for running a full Linux stack (HUDIY, Android Auto, Python backend)
- The ESP32 is ideal for real-time control (FreeRTOS), low power (deep sleep), and concurrent tasks (CAN + BLE + GSM)
- Failure isolation — a crash in the dashboard does not affect comfort controls

**Consequences:**
- Requires a communication bridge between Pi and ESP32 (Serial USB)
- Two separate codebases to maintain (C++ and Python)

---

## ADR-002: MCP2515 + TJA1050 for Raspberry Pi CAN Interface

**Context:**
The Raspberry Pi 4 does not have a native CAN controller, and the Powertrain CAN runs at 5V logic levels.

**Decision:**
Use **MCP2515** (SPI CAN controller) + **TJA1050** (5V CAN transceiver) connected via SPI.

**Rationale:**
- MCP2515 is the standard SPI CAN controller for Linux/Raspberry Pi — well supported by SocketCAN
- TJA1050 operates at 5V, matching the Powertrain CAN bus voltage
- Both components integrate natively with the Pi's `spi0` bus and the `mcp251x` kernel driver

**Consequences:**
- Requires SPI to be enabled in `raspi-config`
- Adds hardware dependency, but is cheap (~€2-3 combined)

---

## ADR-003: SN65HVD230 for ESP32 CAN Interface

**Context:**
The ESP32 has a native CAN controller (TWAI) but requires a physical transceiver to interface with the CAN bus. The Comfort CAN operates at 3.3V logic.

**Decision:**
Use **SN65HVD230** (3.3V CAN transceiver) connected to ESP32 GPIO 4 (RX) and GPIO 5 (TX).

**Rationale:**
- SN65HVD230 operates at 3.3V — directly compatible with ESP32 without level shifting
- Works natively with the ESP32's built-in TWAI (Two-Wire Automotive Interface) driver
- GPIO 4 and GPIO 5 are on the same side of the DevKit, allowing clean breadboard wiring

**Consequences:**
- GPIO 4 and 5 are reserved and cannot be used for other functions

---

## ADR-004: Comfort CAN Access via Scotch Locks (Direct Tap)

**Context:**
The OBD2 port only exposes the Powertrain CAN bus. The Comfort CAN (which controls locks, lights, windows, and windows) is not accessible via OBD2.

**Decision:**
Access the Comfort CAN bus via **Scotch Lock connectors** tapped directly into the BCM (J519) wiring harness behind the dashboard.

**Rationale:**
- The only way to access the Comfort CAN without modifying the vehicle's ECU or gateway
- Scotch locks allow non-destructive tapping — no cutting of wires
- Used by the automotive enthusiast community for similar VAG platform projects

**Consequences:**
- Requires locating and identifying the correct wires (orange/green for CAN-H, orange/brown for CAN-L)
- Installation is more involved than OBD2 plug-and-play

---

## ADR-005: Python for Raspberry Pi Backend

**Context:**
The Pi backend needs to read CAN Bus data via SocketCAN and expose it to the HUDIY dashboard via a local API. The initial idea was to use C++ for performance reasons.

**Decision:**
Use **Python** with the `python-can` library for the backend.

**Rationale:**
- The bottleneck is I/O (CAN bus at 500 kbps), not CPU — C++ provides no meaningful performance advantage
- Memory difference is negligible: Python uses ~50 MB vs C++ ~10 MB on a system with 4 GB RAM
- `python-can` provides a high-level API for SocketCAN in 3 lines of code vs 30+ lines of raw C socket code
- Significantly faster development — days instead of weeks
- Easier debugging (no compilation cycle)

**Consequences:**
- Slight memory overhead (~40 MB more than C++) — irrelevant given available RAM
- Python is already used in the CAN reverse engineering scripts, maintaining language consistency on the Pi

---

## ADR-006: C++ with FreeRTOS for ESP32 Firmware

**Context:**
The ESP32 must handle three concurrent tasks simultaneously: CAN Bus communication, Bluetooth Low Energy (BLE), and GSM (SIM800L via UART).

**Decision:**
Use **C++ with FreeRTOS** tasks for the ESP32 firmware, with one dedicated FreeRTOS task per communication channel.

**Rationale:**
- The ESP32 has 520 KB of RAM — resource constraints justify C++ over higher-level languages
- FreeRTOS is natively supported by the ESP32 Arduino core and ESP-IDF
- Task-based concurrency ensures each channel (CAN, BLE, GSM) gets guaranteed CPU time
- The user has prior experience with C and concurrent programming

**Consequences:**
- More complex codebase than a single-threaded approach
- Requires careful handling of shared state between tasks (mutexes/queues)

---

## ADR-007: HUDIY for Android Auto Integration

**Context:**
The project requires an Android Auto-capable head unit running on the Raspberry Pi 4. OpenAuto Pro (the previous community standard) is discontinued and its activation servers are offline.

**Decision:**
Use **HUDIY** as the Android Auto platform on the Raspberry Pi 4.

**Rationale:**
- HUDIY is actively maintained (updated through 2025-2026) — unlike OpenAuto Pro which was abandoned
- Supports Wireless Android Auto on Raspberry Pi 4 and 5
- Allows custom HTML/CSS/JavaScript widgets, enabling integration of the CAN dashboard alongside Android Auto
- Commercial but affordable (~€10), with active community support

**Consequences:**
- Small cost (~€10 license)
- Custom dashboard must be implemented as HTML/JS widgets within HUDIY's framework
- Android Auto dependency on phone for GPS and data connectivity

---

## ADR-008: Raspberry Pi 4 — 4 GB Variant

**Context:**
Choosing the correct RAM variant of the Pi 4 to ensure stable operation of HUDIY (Android Auto), the custom dashboard, and the Python backend simultaneously.

**Decision:**
Use the **Raspberry Pi 4 with 4 GB RAM**.

**Rationale:**
| Component | Estimated RAM |
|---|---|
| Operating System | ~400 MB |
| HUDIY + Android Auto | ~1500 MB |
| Dashboard HTML/JS widgets | ~200 MB |
| Python CAN backend | ~50 MB |
| **Total** | ~2150 MB |

The 4 GB variant leaves ~1.9 GB free, providing headroom for future features. The 2 GB variant would leave only ~200 MB free — too tight for stability.

**Consequences:**
- Higher cost than 2 GB variant (~€10-15 more)
- 8 GB variant is overkill — ~2 GB would never be used

---

## ADR-009: CSS Transitions for 60fps Gauge Animations

**Context:**
The dashboard gauges (RPM, speed, temperature) need smooth 60fps animations on the Pi 4, which has limited GPU power.

**Decision:**
Use **CSS transitions with `will-change: transform`** for gauge needle animations instead of redrawing on every frame in JavaScript.

**Rationale:**
- CAN data updates at 10-20 Hz (every 50-100ms) — there is no benefit in driving JavaScript at 60fps
- CSS transitions delegate animation to the GPU, freeing CPU for the HUDIY rendering pipeline
- `will-change: transform` hints to the browser to promote the element to its own GPU layer
- JavaScript only updates the CSS value when new CAN data arrives; the GPU interpolates smoothly between values

**Consequences:**
- Gauges are visually smooth at 60fps with minimal CPU usage
- Avoids heavy SVG or Canvas redraw loops that would degrade performance on the Pi 4

---

## ADR-010: Android Auto at 30fps, Dashboard Gauges at 60fps

**Context:**
Running both Android Auto and custom gauges at 60fps simultaneously may cause performance issues on the Pi 4.

**Decision:**
Configure **HUDIY to render Android Auto at 30fps** and allow the CSS-driven gauges to run at 60fps.

**Rationale:**
- Android Auto video decode is the most GPU/CPU-intensive workload
- 30fps is visually sufficient for navigation and media (imperceptible difference for maps)
- Reducing Android Auto to 30fps frees rendering resources for the 60fps gauge animations
- HUDIY v1.2+ supports configurable FPS (30 or 60) for the projection

**Consequences:**
- Android Auto is slightly less smooth than a dedicated head unit but fully functional
- Gauges remain fluid and responsive

---

## ADR-011: Dart/Flutter for Android Mobile App

**Context:**
A companion Android app is needed for remote control (lock/unlock, lights) and telemetry reports. The user has no prior Android development experience, and the app is not the primary focus of the project.

**Decision:**
Use **Flutter (Dart)** for the mobile application.

**Rationale:**
- Significantly lower learning curve than Kotlin + Jetpack Compose
- `flutter_blue_plus` and `flutter_sms` plugins cover BLE and SMS requirements in minimal code
- Hot reload dramatically speeds up UI iteration
- Cross-platform bonus: same codebase works on iOS if needed in the future
- Dart syntax is familiar to anyone with C/C++ background

**Consequences:**
- App is not native Android — minor performance overhead, irrelevant for this use case
- Adds Dart as a fifth language, but demonstrates polyglot capability in the portfolio

---

## ADR-012: TTP223 Capacitive Touch Sensor for Keyless Entry

**Context:**
A convenient and invisible entry mechanism is needed to trigger the lock/unlock flow without a physical button.

**Decision:**
Mount a **TTP223 capacitive touch sensor behind the car window glass**, wired to an ESP32 GPIO interrupt that wakes the microcontroller from deep sleep.

**Rationale:**
- Capacitive sensors work through glass — completely hidden, clean aesthetic
- GPIO interrupt wakes the ESP32 from deep sleep instantly, minimising power consumption when idle
- Simpler and more reliable than a PIR or proximity sensor for this specific use case
- Unique feature that demonstrates creative hardware integration

**Consequences:**
- Glass thickness affects sensitivity — may require calibration
- Must be positioned carefully to avoid false triggers from rain or vibration

---

## ADR-013: Bluetooth Classic Direct Paging for Keyless Entry Authentication

**Context:**
The keyless entry system triggered by the TTP223 sensor needs an authentication mechanism to prevent unauthorised access. Modern smartphones randomise their BLE MAC addresses and hide themselves from general Bluetooth scans for privacy.

**Decision:**
Authenticate using **Bluetooth Classic Direct Paging (Remote Name Request)**: after a touch event, the ESP32 directly pages the pre-registered static MAC address of the owner's phone to request its name. If the phone responds, it is nearby.

**Rationale:**
- Works reliably with the phone in the pocket (screen off).
- Bypasses Android's privacy limits that block general scans.
- Faster than a full scan (resolves in < 1 second if nearby).
- The phone's Bluetooth is always on — no user action required beyond touching the sensor.
- Uses near zero battery on the phone since it only responds to targeted requests.

**Consequences:**
- Requires pairing or knowing the static Bluetooth Classic MAC address beforehand.
- Paging timeout (typically 2 to 4 seconds) means a slight delay when an unauthorised person touches the car.

---

## ADR-014: SMS with Auth Token for Remote Control via SIM800L

**Context:**
The ESP32 needs to receive remote commands (lock, unlock, lights) from the owner's phone when out of BLE range.

**Decision:**
Use **SMS with an authentication token** (`CMD:LOCK:AUTH_TOKEN`) as the command format for the SIM800L.

**Rationale:**
- SMS works anywhere with GSM coverage — no internet required
- SIM800L is already included in the hardware bill for GSM connectivity
- Auth token prevents unauthorised users who know the SIM number from sending commands
- AT command interface of SIM800L is well-documented for ESP32 UART communication

**Consequences:**
- SMS has latency (typically 1-5 seconds) — acceptable for lock/unlock, not suitable for real-time control
- SIM card with data plan required
- Token must be hardcoded or stored securely in ESP32 flash

---

## ADR-015: C4 Model + Sequence Diagrams + ADRs for Documentation

**Context:**
The project needs clear documentation for a GitHub portfolio that communicates architectural decisions to both technical and non-technical audiences (recruiters, engineers).

**Decision:**
Use **C4 Model** (Levels 1-3) for structural architecture, **Mermaid sequence diagrams** for key interaction flows, and **ADRs** (this document) for decision rationale. All rendered in Mermaid directly in Markdown files.

**Rationale:**
- C4 is intuitive — no prior UML knowledge needed to understand Level 1 and Level 2 diagrams
- Mermaid renders natively in GitHub Markdown — no external tools required
- ADRs demonstrate engineering maturity: not just "what was built" but "why it was built this way"
- Sequence diagrams show edge case thinking (auth failure, BLE not found, timeout handling)

**Consequences:**
- Documentation stays co-located with code in the repository
- Diagrams are version-controlled alongside the code
- Recruiters see a professional, well-reasoned project from the first README visit

---

## ADR-016: Linux Live USB for CAN Capture, Windows + SavvyCAN for Analysis

**Context:**
CAN Bus data capture in the vehicle requires Linux (SocketCAN + can-utils), but the development machine runs Windows. WSL2 USB passthrough for CAN adapters is complex and unreliable.

**Decision:**
Use a **Ubuntu Linux Live USB** for all vehicle-side CAN captures, and **Windows with SavvyCAN** for post-capture analysis at the workbench.

**Rationale:**
- CAN adapters (UCAN with gs_usb/candleLight firmware) work plug-and-play on Linux without driver installation
- `candump`, `cansniffer`, and `canplayer` from `can-utils` are only available natively on Linux
- SavvyCAN is the best GUI tool for differential analysis and DBC editing, and runs well on Windows
- `.log` files from `candump` are plain text — trivially portable between Linux and Windows

**Consequences:**
- Two-environment workflow — minor inconvenience offset by reliability
- The UCAN keeps its original candleLight firmware (no re-flashing needed)

---

*Last updated: September 2026*
*All decisions made during initial design and planning phase.*
