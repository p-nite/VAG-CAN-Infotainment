# Features

> Status legend: 🔲 Planned · 🔄 In Progress · ✅ Done · ❌ Cancelled · 🔍 Under Research
>
> The car turned out to have no Powertrain CAN: the engine ECU only speaks K-line ([ADR-018](decisions.md#adr-018-the-vehicle-has-no-powertrain-can--engine-ecu-is-k-line-only)). Features that relied on it were either moved to K-line or cancelled. Each one keeps its original data source in the **Original (CAN design)** column, so the change stays visible.

---

## Dashboard & Telemetry (Raspberry Pi 4 — Engine ECU over K-line)

| ID | Feature | Description | Trigger | Hardware | Stack | Original (CAN design) | Status |
|----|---------|-------------|---------|----------|-------|-----------------------|--------|
| Pi01 | **Live RPM Gauge** | Displays engine RPM polled from the ECU with OBD PID `010C` | Ignition ON | Pi 4, ELM327 (Bluetooth) | Python, python-obd, HTML/JS | CAN ID `0x280` | 🔲 |
| Pi02 | **Live Speed Gauge** | Displays vehicle speed polled with OBD PID `010D` — support by this ECU still to be confirmed | Ignition ON | Pi 4, ELM327 (Bluetooth) | Python, python-obd, HTML/JS | CAN ID `0x5A0` | 🔍 |
| Pi03 | **Coolant Temperature Gauge** | Out of scope: the stock cluster already shows it | — | — | — | CAN ID `0x288` | ❌ |
| Pi04 | **Throttle Position Gauge** | Out of scope: each extra PID lowers the K-line polling rate of RPM and speed | — | — | — | CAN ID `0x380` | ❌ |
| Pi05 | **Fuel Level Indicator** | Out of scope: the stock cluster already shows it | — | — | — | Powertrain CAN | ❌ |
| Pi06 | **Engine Oil Temperature** | Out of scope: not reported by this ECU's generic OBD interface | — | — | — | Powertrain CAN | ❌ |
| Pi07 | **Check Engine Indicator** | Visual alert listing the stored fault codes, read with OBD mode `03` | DTCs present | Pi 4, ELM327 (Bluetooth) | Python, python-obd, HTML/JS | CAN ID `0x480` | 🔲 |
| Pi08 | **Android Auto Integration** | Full Android Auto on the 7" dashboard screen (navigation, media, calls) | Phone connects via WiFi | Pi 4, 7" touchscreen | HUDIY | — | 🔲 |
| Pi09 | **60fps Smooth Gauge Animations** | GPU-accelerated CSS transitions that interpolate between polled samples ([ADR-020](decisions.md#adr-020-gauge-animation-with-low-rate-polled-data)) | New sample (a few Hz) | Pi 4 GPU | HTML/CSS | Data at 10–20 Hz | 🔲 |

---

## 🔧 Comfort Control (ESP32 — Comfort CAN, unconfirmed)

> On hold until a scan of the Central Locking (35) and Comfort System (46) modules with a KKL cable and VCDS-Lite confirms whether the car has a Comfort CAN. If it does not, these features move to direct control of the lock actuators.

| ID | Feature | Description | Trigger | Hardware | Stack | Status |
|----|---------|-------------|---------|----------|-------|--------|
| CC01 | **Central Lock via CAN** | Send lock command to BCM (J519) via Comfort CAN | Software command | ESP32, SN65HVD230 | C++, TWAI | 🔍 |
| CC02 | **Central Unlock via CAN** | Send unlock command to BCM via Comfort CAN | Software command | ESP32, SN65HVD230 | C++, TWAI | 🔍 |
| CC03 | **Headlights Control** | Toggle low beam headlights via Comfort CAN | Software command | ESP32, SN65HVD230 | C++, TWAI | 🔍 |
| CC04 | **High Beam Control** | Toggle high beam via Comfort CAN | Software command | ESP32, SN65HVD230 | C++, TWAI | 🔍 |
| CC05 | **Hazard Lights** | Activate/deactivate hazard lights via Comfort CAN | Software command | ESP32, SN65HVD230 | C++, TWAI | 🔍 |
| CC06 | **Driver Window Control** | Send open/close command for driver window | Software command | ESP32, SN65HVD230 | C++, TWAI | 🔍 |
| CC07 | **Passenger Window Control** | Send open/close command for passenger window | Software command | ESP32, SN65HVD230 | C++, TWAI | 🔍 |
| CC08 | **Door Status Reading** | Read open/closed state of driver door, passenger door, and boot | CAN broadcast | ESP32, SN65HVD230 | C++, TWAI | 🔍 |

---

## Security & Keyless Entry (ESP32 — BLE + GPIO)

| ID | Feature | Description | Trigger | Hardware | Stack | Status |
|----|---------|-------------|---------|----------|-------|--------|
| S01 | **Keyless Entry — Touch to Lock/Unlock** | Touch sensor behind window glass wakes ESP32 → Direct Paging via BT Classic → authenticates phone → locks or unlocks | TTP223 touch | ESP32, TTP223, SN65HVD230 | C++, FreeRTOS, BT Classic | 🔲 |
| S02 | **BT Classic MAC Paging** | Only unlock if the pre-registered Bluetooth Classic MAC address responds to a remote name request | BT response | ESP32 | C++ | 🔲 |
| S03 | **Deep Sleep Between Events** | ESP32 enters deep sleep when idle, wakes on GPIO interrupt from TTP223 | GPIO interrupt | ESP32, TTP223 | C++, ESP-IDF | 🔲 |
| S04 | **Paging Timeout Safety** | If no registered phone responds within 3 seconds, abort — no action taken | BT paging timeout | ESP32 | C++ | 🔲 |

---

## Remote Control (ESP32 — SIM800L GSM)

| ID | Feature | Description | Trigger | Hardware | Stack | Status |
|----|---------|-------------|---------|----------|-------|--------|
| RC01 | **Remote Lock via SMS** | Send `CMD:LOCK:AUTH_TOKEN` via SMS → ESP32 validates token → locks car | SMS received | ESP32, SIM800L | C++, AT commands | 🔲 |
| RC02 | **Remote Unlock via SMS** | Send `CMD:UNLOCK:AUTH_TOKEN` via SMS → ESP32 validates → unlocks car | SMS received | ESP32, SIM800L | C++, AT commands | 🔲 |
| RC03 | **Remote Lights via SMS** | Toggle lights remotely via authenticated SMS command | SMS received | ESP32, SIM800L | C++, AT commands | 🔲 |
| RC04 | **SMS Auth Token Validation** | Reject any SMS command without a valid auth token — reply with `STATUS:AUTH_FAIL` | Invalid SMS | ESP32, SIM800L | C++ | 🔲 |
| RC05 | **Command Confirmation SMS** | After executing a command, reply with status (e.g. `STATUS:LOCKED:OK`) | Command executed | ESP32, SIM800L | C++, AT commands | 🔲 |

---

## 📱 Mobile App (Android — Flutter/Dart)

| ID | Feature | Description | Trigger | Hardware | Stack | Status |
|----|---------|-------------|---------|----------|-------|--------|
| M01 | **BLE Connection to ESP32** | App connects to ESP32 via Bluetooth Low Energy | App open + BLE scan | Phone, ESP32 | Dart, flutter_blue_plus | 🔲 |
| M02 | **Lock/Unlock Button (BLE)** | One-tap lock and unlock via BLE when in range | Button tap | Phone, ESP32 | Dart, BLE | 🔲 |
| M03 | **Lock/Unlock Button (SMS)** | Fallback to SMS command when outside BLE range | Button tap | Phone, SIM800L | Dart, flutter_sms | 🔲 |
| M04 | **Lights Control Button** | Toggle headlights / hazard via BLE or SMS | Button tap | Phone, ESP32 | Dart, BLE / SMS | 🔲 |
| M05 | **Live Telemetry View** | Display real-time RPM and speed fetched from Pi backend via BLE (temperature dropped with Pi03) | App open + connected | Phone, Pi 4 | Dart, BLE | 🔲 |
| M06 | **Telemetry Reports** | View trip history, distance and average/max speed (fuel consumption and temperature logs dropped with Pi03/Pi05) | App navigation | Phone, Pi 4 | Dart, SQLite | 🔲 |
| M07 | **Door Status Indicator** | Show current open/closed state of all doors in the app | BLE data push | Phone, ESP32 | Dart, BLE | 🔲 |
| M08 | **Connection Mode Indicator** | Show whether the app is communicating via BLE or SMS fallback | Automatic detection | Phone | Dart | 🔲 |

---

## Infrastructure & Backend (Raspberry Pi 4 — Python)

| ID | Feature | Description | Trigger | Hardware | Stack | Status |
|----|---------|-------------|---------|----------|-------|--------|
| IB01 | **CAN Frame Decoder** | Cancelled: no Powertrain CAN on this car — replaced by IB05 | — | — | Python, python-can, cantools | ❌ |
| IB02 | **Local Telemetry API** | Expose vehicle data to HUDIY dashboard and mobile app via local REST/WebSocket API | Continuous | Pi 4 | Python, FastAPI | 🔲 |
| IB03 | **Trip Logger** | Record trip start/end, distance and average speed for reports | Ignition ON/OFF | Pi 4 | Python, SQLite | 🔲 |
| IB04 | **CAN Signal Database (DBC)** | Cancelled: the Simos 3PE has no CAN to decode — replaced by standard OBD PIDs | — | — | SavvyCAN, cantools | ❌ |
| IB05 | **OBD Poller** | Connect to the ELM327 over Bluetooth RFCOMM, pin the K-line protocol (`AT SP`) and poll RPM/speed in a task decoupled from the UI ([ADR-019](decisions.md#adr-019-elm327-bluetooth--python-obd-for-rpm-and-speed)) | Ignition ON | Pi 4, ELM327 (Bluetooth) | Python, python-obd | 🔲 |
| IB06 | **K-line Connection Fix** | Restore OBD2 communication with the ECU (suspected aftermarket radio pulling the K-line to ground) — prerequisite for IB05 | Manual | Multimeter, ELM327 | — | 🔄 |

---

## 🗓️ Backlog (Ideas to Explore)

| ID | Idea | Notes |
|----|------|-------|
| B01 | **Offline navigation (Organic Maps)** | Pi 4 + USB GPS module — fully independent of phone |
| B02 | **OBD-II diagnostic reader** | Reading DTCs moved to Pi07. Still in backlog: clearing DTCs from the dashboard |
| B06 | **Pulse taps for RPM/speed** | Read the tachometer and VSS pulse wires directly if K-line polling is too slow for the gauges (ADR-019) |
| B03 | **Startup animation** | Custom boot screen on 7" display while Pi initialises |
| B04 | **Voice commands via BLE** | Use phone mic to trigger lock/unlock by voice |
| B05 | **Geo-fence alert** | SMS alert if car leaves a defined geographic area |

---

*Last updated: November 2026*
*Add new features below the relevant section. Update status as implementation progresses.*
