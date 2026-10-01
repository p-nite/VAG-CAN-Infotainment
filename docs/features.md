# Features

> Status legend: 🔲 Planned · 🔄 In Progress · ✅ Done · ❌ Cancelled · 🔍 Under Research

---

## Dashboard & Telemetry (Raspberry Pi 4 — Powertrain CAN)

| ID | Feature | Description | Trigger | Hardware | Stack | Status |
|----|---------|-------------|---------|----------|-------|--------|
| Pi01 | **Live RPM Gauge** | Displays real-time engine RPM from CAN ID `0x280` | Ignition ON | Pi 4, MCP2515+TJA1050 | Python, HTML/JS | 🔲 |
| Pi02 | **Live Speed Gauge** | Displays vehicle speed from CAN ID `0x5A0` | Ignition ON | Pi 4, MCP2515+TJA1050 | Python, HTML/JS | 🔲 |
| Pi03 | **Coolant Temperature Gauge** | Displays engine coolant temperature from CAN ID `0x288` | Ignition ON | Pi 4, MCP2515+TJA1050 | Python, HTML/JS | 🔲 |
| Pi04 | **Throttle Position Gauge** | Displays accelerator pedal position (0–100%) from CAN ID `0x380` | Ignition ON | Pi 4, MCP2515+TJA1050 | Python, HTML/JS | 🔲 |
| Pi05 | **Fuel Level Indicator** | Displays estimated fuel level from Powertrain CAN | Ignition ON | Pi 4, MCP2515+TJA1050 | Python, HTML/JS | 🔍 |
| Pi06 | **Engine Oil Temperature** | Displays engine oil temperature if available on CAN | Ignition ON | Pi 4, MCP2515+TJA1050 | Python, HTML/JS | 🔍 |
| Pi07 | **Check Engine Indicator** | Visual alert when ECU broadcasts engine fault status (CAN ID `0x480`) | Fault detected | Pi 4, MCP2515+TJA1050 | Python, HTML/JS | 🔲 |
| Pi08 | **Android Auto Integration** | Full Android Auto on the 7" dashboard screen (navigation, media, calls) | Phone connects via WiFi | Pi 4, 7" touchscreen | HUDIY | 🔲 |
| Pi09 | **60fps Smooth Gauge Animations** | GPU-accelerated CSS transitions for fluid gauge needle movement | Data update (10–20 Hz) | Pi 4 GPU | HTML/CSS | 🔲 |

---

## 🔧 Comfort Control (ESP32 — Comfort CAN)

| ID | Feature | Description | Trigger | Hardware | Stack | Status |
|----|---------|-------------|---------|----------|-------|--------|
| CC01 | **Central Lock via CAN** | Send lock command to BCM (J519) via Comfort CAN | Software command | ESP32, SN65HVD230 | C++, TWAI | 🔲 |
| CC02 | **Central Unlock via CAN** | Send unlock command to BCM via Comfort CAN | Software command | ESP32, SN65HVD230 | C++, TWAI | 🔲 |
| CC03 | **Headlights Control** | Toggle low beam headlights via Comfort CAN | Software command | ESP32, SN65HVD230 | C++, TWAI | 🔍 |
| CC04 | **High Beam Control** | Toggle high beam via Comfort CAN | Software command | ESP32, SN65HVD230 | C++, TWAI | 🔍 |
| CC05 | **Hazard Lights** | Activate/deactivate hazard lights via Comfort CAN | Software command | ESP32, SN65HVD230 | C++, TWAI | 🔍 |
| CC06 | **Driver Window Control** | Send open/close command for driver window | Software command | ESP32, SN65HVD230 | C++, TWAI | 🔍 |
| CC07 | **Passenger Window Control** | Send open/close command for passenger window | Software command | ESP32, SN65HVD230 | C++, TWAI | 🔍 |
| CC08 | **Door Status Reading** | Read open/closed state of driver door, passenger door, and boot | CAN broadcast | ESP32, SN65HVD230 | C++, TWAI | 🔲 |

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
| M05 | **Live Telemetry View** | Display real-time RPM, speed, temperature fetched from Pi backend via BLE | App open + connected | Phone, Pi 4 | Dart, BLE | 🔲 |
| M06 | **Telemetry Reports** | View trip history, average fuel consumption, max temperature logs | App navigation | Phone, Pi 4 | Dart, SQLite | 🔲 |
| M07 | **Door Status Indicator** | Show current open/closed state of all doors in the app | BLE data push | Phone, ESP32 | Dart, BLE | 🔲 |
| M08 | **Connection Mode Indicator** | Show whether the app is communicating via BLE or SMS fallback | Automatic detection | Phone | Dart | 🔲 |

---

## Infrastructure & Backend (Raspberry Pi 4 — Python)

| ID | Feature | Description | Trigger | Hardware | Stack | Status |
|----|---------|-------------|---------|----------|-------|--------|
| IB01 | **CAN Frame Decoder** | Decode raw CAN frames into human-readable values (RPM, speed, temp) using DBC | Continuous | Pi 4, MCP2515 | Python, python-can, cantools | 🔲 |
| IB02 | **Local Telemetry API** | Expose vehicle data to HUDIY dashboard and mobile app via local REST/WebSocket API | Continuous | Pi 4 | Python, FastAPI | 🔲 |
| IB03 | **Trip Logger** | Record trip start/end, distance, average speed, and temperature for reports | Ignition ON/OFF | Pi 4 | Python, SQLite | 🔲 |
| IB04 | **CAN Signal Database (DBC)** | Maintain a `.dbc` file with all decoded signals for the Ibiza 6L / Simos 3PE | Manual update | — | SavvyCAN, cantools | 🔄 |

---

## 🗓️ Backlog (Ideas to Explore)

| ID | Idea | Notes |
|----|------|-------|
| B01 | **Offline navigation (Organic Maps)** | Pi 4 + USB GPS module — fully independent of phone |
| B02 | **OBD-II diagnostic reader** | Read and clear DTCs, display fault codes on dashboard |
| B03 | **Startup animation** | Custom boot screen on 7" display while Pi initialises |
| B04 | **Voice commands via BLE** | Use phone mic to trigger lock/unlock by voice |
| B05 | **Geo-fence alert** | SMS alert if car leaves a defined geographic area |

---

*Last updated: September 2026*
*Add new features below the relevant section. Update status as implementation progresses.*
