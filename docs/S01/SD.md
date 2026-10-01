# Keyless Entry — Touch to Lock/Unlock Sequence Diagram

```mermaid
sequenceDiagram
    actor Driver
    activate Driver
    
    box Hardware Layer
    participant TTP as TTP223 Sensor
    participant BLE_HW as BLE Antenna
    participant CAN_HW as CAN Transceiver
    end
    
    box Services Layer
    participant Auth as AuthService
    participant CAN_Srv as canManager
    end
    
    box Business Logic (Core)
    participant LockSys as LockSystem
    end

    %% Início do fluxo
    Driver->>TTP: Touch the glass (Sensor)
    activate TTP
    TTP->>LockSys: Interrupt (Wake up)
    deactivate TTP
    
    activate LockSys
    %% Autenticação
    LockSys->>Auth: isOwnerNearby()
    

    activate Auth
    Auth->>BLE_HW: isDevicePresent(OWNER_MAC, Timeout: 3s)
    activate BLE_HW

    alt Phone responds
        BLE_HW-->>Auth: True (Remote Name Received)
        deactivate BLE_HW
        Auth-->>LockSys: True (Authorized)
        deactivate Auth
        
        %% Decisão de negócio (Trancar ou Destrancar?)
        LockSys->>CAN_Srv: getCarStatus()
        CAN_Srv-->>LockSys: State: UNLOCKED
        
        %% Ação
        LockSys->>CAN_Srv: commandLock()
        activate CAN_Srv
        CAN_Srv->>CAN_Srv: Prepares CAN Frame (ID 0x291, Data: 0x01)
        CAN_Srv->>CAN_HW: sendFrame()
        deactivate CAN_Srv
        activate CAN_HW
        CAN_HW-->>Driver: Car locks
        deactivate CAN_HW
        
        
    else Phone does not respond
        BLE_HW-->>Auth: Timeout (No response)
        activate Auth
        Auth-->>LockSys: False (Not authorized)
        deactivate Auth
    end
    
    LockSys->>LockSys: Return to Deep Sleep
    deactivate LockSys
    deactivate Driver