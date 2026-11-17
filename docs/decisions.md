# Architecture Decision Records (ADRs)

> This document captures all key architectural and technical decisions made during the design phase of the VAG CAN Infotainment project, along with the reasoning behind each choice.
>
> Decisions are never deleted. When a later finding invalidates one, it is marked as superseded and the new decision explains why. ADR-001 to ADR-017 describe the original CAN-based design (tag `v0-can-design`); ADR-018 onwards record the move to K-line after the vehicle was found to have no Powertrain CAN.
>
> **Status legend:** Accepted · Amended (still valid, partly changed by a later ADR) · Superseded (replaced by a later ADR) · On hold (waiting on a pending verification)

---

## ADR-001: Dual-Controller Architecture (Pi 4 + ESP32)

**Status:** Amended by ADR-019 — the Pi now reads the engine ECU over K-line instead of the Powertrain CAN; the split between Pi and ESP32 still holds.

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

**Status:** Superseded by ADR-019 — there is no Powertrain CAN to connect to (ADR-018).

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

**Status:** On hold — depends on confirming that the vehicle has a Comfort CAN (ADR-018).

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

**Status:** On hold — depends on confirming that the vehicle has a Comfort CAN (ADR-018). The context below assumed the OBD2 port exposes the Powertrain CAN, which turned out to be false for this vehicle.

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

**Status:** Amended by ADR-019 — Python stays; `python-obd` replaces `python-can`/SocketCAN.

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

**Status:** Accepted

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

**Status:** Accepted

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

**Status:** Accepted

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

**Status:** Amended by ADR-020 — the CSS approach stays, but data now arrives at a few Hz instead of 10–20 Hz.

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

**Status:** Accepted

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

**Status:** Accepted

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

**Status:** Accepted

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

**Status:** Accepted

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

**Status:** Accepted

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

**Status:** Accepted

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

**Status:** On hold — no longer applies to the powertrain (ADR-018); only relevant again if a Comfort CAN is confirmed.

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

## ADR-017: SMS Cost Optimization — Silence as Error Pattern

**Status:** Accepted

**Context:**
The SIM800L module uses a prepaid IoT SIM (1NCE) with a limited budget of 250 SMS over 10 years. Every outgoing SMS costs credits. The system needs to confirm successful commands but also handle invalid requests (wrong token, unknown commands, spam).

**Decision:**
Adopt a **"Silence as Error"** strategy:
- The ESP32 only sends an SMS reply when a command is **successfully executed** (e.g., `OK:LOCKED`).
- Invalid auth tokens, unknown commands, and malformed messages are **silently dropped** — logged to Serial for debugging but no SMS is sent.
- The **Mobile App** implements a **response timeout** (e.g., 15 seconds): if no confirmation SMS arrives after sending a command, the app displays a warning: _"No confirmation received. Check vehicle status."_

**Rationale:**
- Prevents wasting SMS credits on spam, typos, or unauthorized access attempts
- Silence itself becomes the error signal — no SMS needed to communicate failure
- Moves the error-handling UX responsibility to the app (which is free) instead of the SIM (which costs money)
- Same pattern used by banking apps (MBWay), ride-sharing (Uber), and home alarm systems

**Consequences:**
- The user only sees feedback for successful operations (clean UX)
- Failure scenarios require the app to infer the error from the absence of a reply
- If the app is not installed or the user sends SMS manually, there is no feedback on failures — acceptable trade-off for a personal project

---

## ADR-018: The Vehicle Has No Powertrain CAN — Engine ECU Is K-line Only

**Status:** Accepted

**Context:**
The original design (ADR-001 to ADR-004) assumed two CAN networks: a Powertrain CAN at 500 kbps, reachable on OBD2 pins 6/14, and a Comfort CAN at 100 kbps behind the dashboard. The first vehicle-side capture contradicted this:
- The UCAN adapter (candleLight) connected to OBD2 pins 6 (CAN-H) and 14 (CAN-L) received **zero frames**.
- The engine ECU was identified from its label as a **Siemens Simos 3PE** — VAG part `03E906033D`, Siemens `5WP40121 04`, built January 2002, fitted to the 1.2 petrol engine.
- This ECU generation has no CAN interface. It only communicates through the **K-line** (OBD2 pin 7) using ISO 9141-2 / KWP2000, a slow request/response serial protocol.
- On this trim the instrument cluster is driven by dedicated wires (pulses and analog senders), not by a shared bus.

**Decision:**
Remove the Powertrain CAN from the design. Engine data is obtained by querying the ECU over its K-line diagnostic interface (ADR-019).

The existence of a Comfort CAN is **not yet confirmed**. It will be checked by scanning the Central Locking (address 35) and Comfort System (address 46) modules with a KKL cable and VCDS-Lite, which speak the VAG KW1281 protocol that an ELM327 does not support. Until then, every Comfort CAN decision (ADR-003, ADR-004, ADR-016) is on hold.

**Rationale:**
- Measurements on the vehicle take precedence over assumptions taken from the platform in general: later 6L facelifts do have CAN, this one does not
- Keeping the original decisions as superseded, rather than deleting them, documents how the design was validated against the real car

**Consequences:**
- The MCP2515 + TJA1050 interface and the SavvyCAN/DBC reverse-engineering workflow no longer apply to the powertrain
- Engine data is polled (request → response) instead of broadcast, which limits the update rate (ADR-020)
- Open issue: no generic OBD2 scanner (ELM327 or standalone) could connect to the ECU yet. The suspected cause is the aftermarket radio pulling the K-line to ground. The next step is to measure pin 7 against pin 4 with the ignition on (≈12 V expected) and to retest with the radio unplugged

---

## ADR-019: ELM327 Bluetooth + python-obd for RPM and Speed

**Status:** Accepted — pending the K-line connection fix (ADR-018, open issue)

**Context:**
With only a K-line available (ADR-018), three ways of getting RPM and speed were considered:

| Option | Effort | Update rate | Electronics needed |
|---|---|---|---|
| Custom K-line transceiver (e.g. L9637D) on an ESP32 UART, implementing KWP2000 by hand | High | Few Hz | Transceiver circuit + protocol init timing |
| Tap the tachometer and vehicle speed sensor (VSS) pulse wires directly | Medium | Real time | Locating wires + 12 V → 3.3 V protection circuits |
| Off-the-shelf ELM327 adapter | Low | Few Hz | None |

The project's focus is software; designing and installing protection circuits is outside the author's experience.

**Decision:**
Plug an **ELM327 v1.5 Bluetooth adapter** permanently into the OBD2 port and connect to it from the **Raspberry Pi 4's built-in Bluetooth** over RFCOMM (serial port profile). The Python backend uses `python-obd` to poll:
- `010C` — engine RPM
- `010D` — vehicle speed
- Mode `03` — stored fault codes (DTCs), for the check engine alert

The OBD protocol is pinned with `AT SP` (ISO 9141-2 or ISO 14230-4 KWP at 10.4 kbaud, whichever the ECU answers) instead of relying on auto-detection, and adaptive timing (`AT AT2`) is enabled to maximise the polling rate.

Coolant temperature and fuel level are **out of scope**: the stock instrument cluster already shows them, so duplicating them does not justify the extra work.

**Rationale:**
- The ELM327 encapsulates the K-line physical layer and the protocol initialisation — no custom electronics
- The Pi 4 already has Bluetooth, so the ESP32 is not involved in reading the engine
- The OBD2 port is a standard connector: the adapter can be removed at any time (e.g. for workshop diagnostics)
- Fault code reading comes for free, which was previously a backlog item (B02)

**Consequences:**
- Update rate limited to a few Hz (to be measured on the car) — handled in the dashboard by ADR-020
- PID `010D` may be unsupported if the speed sensor is wired straight to the cluster instead of the ECU; to be checked with PID `0100` (supported PIDs)
- Cheap ELM327 v1.5 clones can be unreliable with K-line protocols; a different adapter may be needed
- The adapter stays powered from OBD2 pin 16; battery drain must be checked if the car is parked for long periods
- If the refresh rate turns out to be insufficient, tapping the tachometer/VSS pulses (second option above) remains the upgrade path for those two signals

---

## ADR-020: Gauge Animation with Low-Rate Polled Data

**Status:** Accepted — amends ADR-009

**Context:**
ADR-009 assumed CAN data arriving at 10–20 Hz. Over K-line each value is a request/response round trip, so RPM and speed arrive at only a few Hz. Driving the needle directly from each sample would make it visibly jump.

**Decision:**
- The OBD polling loop runs in its **own task**, decoupled from the WebSocket server and the UI. The UI never waits for the K-line; it always renders the last known value.
- The WebSocket server pushes a new value only when a new sample arrives.
- The CSS transition duration on the gauge needle is set to the **measured polling interval**, with linear easing, so the needle keeps moving towards the next sample instead of jumping and stopping.

**Rationale:**
- Rendering stays at 60fps regardless of how slow the ECU answers
- Interpolating between samples is how commercial digital clusters hide low sensor rates

**Consequences:**
- The needle trails the real value by roughly one polling interval — acceptable for display, not precise enough for a shift light
- The polling interval must be measured on the car and fed into the dashboard configuration

---

*Last updated: November 2026*
*ADR-001 to ADR-017: initial design and planning phase (CAN-based). ADR-018 onwards: redesign after vehicle validation (K-line).*
