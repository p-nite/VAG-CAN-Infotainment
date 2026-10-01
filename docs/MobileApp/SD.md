# Mobile App & Comfort Control
*Covers Features: CC01 to CC08, M01 to M08*

This sequence diagram illustrates how the Android app interacts with the ESP32 to send commands and receive live updates about the vehicle's state (e.g., doors open/closed).

```mermaid
sequenceDiagram
    actor Owner
    participant App as Mobile App (Flutter)
    
    box ESP32 Layer
    participant BLE as BleManager
    participant Core as LockSystem (Core)
    participant CAN_Srv as CanManager
    end
    
    box Vehicle
    participant Comfort as Comfort CAN Bus
    end
    
    Owner->>App: Opens App
    App->>BLE: Connects via Bluetooth Low Energy (GATT)
    
    %% Background Telemetry Flow
    opt Continuous Updates
        Comfort->>CAN_Srv: Broadcasts Door Status (e.g., ID 0x420)
        CAN_Srv->>Core: Updates Internal State
        Core-->>App: BLE Notify Characteristic (Doors Closed)
        App-->>Owner: UI updates to show doors closed
    end
    
    %% Command Flow
    Owner->>App: Taps "Unlock" Button
    App->>BLE: Writes to Command Characteristic
    BLE->>Core: Parses BLE Command
    Core->>CAN_Srv: commandUnlock()
    CAN_Srv->>Comfort: Injects Comfort Frame (0x291)
    
    Comfort-->>Owner: Car Unlocks (Physical Feedback)
    
    Comfort->>CAN_Srv: Broadcasts Door Status (Unlocked)
    CAN_Srv->>Core: Updates State
    Core-->>App: BLE Notify (State: Unlocked)
    App-->>Owner: UI changes button state to Unlocked
```
