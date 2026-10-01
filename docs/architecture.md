# Architecture (C4 Model)

This document describes the software architecture of the Ibiza Smart Car project using the C4 model. It is broken down into three levels of detail: System Context, Container, and Component.

---

## Level 1: System Context
Shows how the custom infotainment system interacts with the driver, their mobile phone, and the vehicle's existing networks.

```mermaid
graph TD
    User([Driver])
    Vehicle[SEAT Ibiza 6L \n CAN-BUS Networks]
    Phone([Android Application])
    System[Custom Infotainment System]
    
    User -->|Interacts with UI in the dashboard| System
    User -->|Interacts with mobile app| Phone
    Phone <-->|Sends commands and retrieves data| System
    System <-->|Reads and sends network signals| Vehicle

    classDef container fill:#1168bd,stroke:#0b4884,color:#ffffff
    classDef external fill:#999999,stroke:#666666,color:#ffffff
    classDef actor fill:#08427b,stroke:#052e56,color:#ffffff

    class System container
    class Vehicle,Phone external
    class User actor
```

---

## Level 2: Container Architecture
Zooms into the Custom Infotainment System to show the main processing units (Raspberry Pi and ESP32) and how they split the workload between the Powertrain and Comfort CAN buses.

```mermaid
graph TD
    User([Driver])
    Phone([Android Application\n«Dart / Flutter»])
    Powertrain[Powertrain CAN \n 500 kbps]
    Comfort[Comfort CAN \n 100 kbps]

    subgraph System [Custom Infotainment System]
        subgraph RPi [Raspberry Pi 4]
            UI[HUDIY Interface \n «Python / UI»]
            Backend[Telemetry Backend \n «Python / SocketCAN»]
            MCP[MCP2515 + TJA1050\n «Hardware, 5V, SPI»]
        end
        
        subgraph Micro [ESP32 Microcontroller]
            ESP[Controller Logic \n «C++»]
            SIM[SIM800L Module\n«Hardware, GSM/GPRS»]
            TTP[TTP223\n «Hardware, Touch Sensor»]
            SN[SN65HVD230\n «Hardware, 3.3V»]
        end
    end

    User -->|Interacts with dashboard| UI
    User -->|Interacts with mobile app| Phone
    User -->|Touch input| TTP
    Phone -->|SMS Commands & Alerts| SIM
    Phone -->|Retrieves telemetry reports «BLE»| Backend
    
    UI <-->|Telemetry data «Local API»| Backend
    SIM <-->|SMS processing «UART»| ESP

    ESP <-->|Door status & Alerts «Serial USB»| Backend
    TTP -->|Digital signal «GPIO»| ESP
    
    Backend -->|Reads powertrain telemetry «SPI»| MCP
    MCP -->|CAN Bus 500kbps| Powertrain

    ESP -->|Wake/Lock/Lights| SN
    SN <-->|CAN Bus 100kbps| Comfort

    classDef container fill:#1168bd,stroke:#0b4884,color:#ffffff
    classDef component fill:#85bbf0,stroke:#5b93c7,color:#000000
    classDef external fill:#999999,stroke:#666666,color:#ffffff
    classDef actor fill:#08427b,stroke:#052e56,color:#ffffff

    class RPi,Micro container
    class UI,Backend,ESP component
    class MCP,SIM,TTP,SN,Powertrain,Comfort,Phone external
    class User actor
```

---

## Level 3: Component Architecture (ESP32)
Zooms into the ESP32 Microcontroller container to show how the C++ firmware is structured using a layered architecture to separate hardware drivers from business logic.

```mermaid
graph TD
    Driver([Driver])
    Phone([Mobile App])
    Comfort[Comfort CAN Bus]
    TTP[TTP223 Touch Sensor]
    SIM[SIM800L Module]

    subgraph ESP32 [ESP32 Container «C++ / FreeRTOS»]
        
        %% Entry Point
        Main[Main / Task Scheduler\n«FreeRTOS»]

        %% Layer: Core (Business Logic)
        subgraph CoreLayer [Core / Business Logic]
            LockSys[Lock System\n«lockSystem.cpp»\nLock decision logic]
        end

        %% Layer: Services
        subgraph ServiceLayer [Services]
            AuthSrv[Auth Service\n«authService.cpp»\nMAC Whitelist validation]
            SmsSrv[SMS Service\n«smsService.cpp»\nSMS command parsing]
            CanSrv[CAN Service\n«canManager.cpp»\nCAN frame building]
        end

        %% Layer: Hardware Drivers
        subgraph HWLayer [Hardware Drivers]
            BleMgr[BLE Manager\n«bleManager.cpp»\nBT Classic radio control]
            CanMgr[CAN Manager\n«canManager.cpp»\nTWAI driver interface]
            GsmMgr[GSM Manager\n«gsmManager.cpp»\nUART / AT commands]
        end
    end

    %% External connections
    Driver -->|Touch input| TTP
    TTP -->|GPIO interrupt| Main
    Phone <-->|Bluetooth| BleMgr
    SIM <-->|UART| GsmMgr
    CanMgr <-->|CAN TX/RX| Comfort

    %% Internal dependencies (structural only)
    Main -->|Delegates to| LockSys
    Main -->|Schedules| CanMgr
    Main -->|Schedules| BleMgr
    Main -->|Schedules| GsmMgr

    LockSys -->|Uses| AuthSrv
    LockSys -->|Uses| CanSrv

    AuthSrv -->|Uses| BleMgr
    SmsSrv -->|Uses| CanSrv
    SmsSrv -->|Uses| GsmMgr
    CanSrv -->|Uses| CanMgr

    %% Estilos (Cores C4)
    classDef container fill:#1168bd,stroke:#0b4884,color:#ffffff
    classDef component fill:#85bbf0,stroke:#5b93c7,color:#000000
    classDef external fill:#999999,stroke:#666666,color:#ffffff
    classDef actor fill:#08427b,stroke:#052e56,color:#ffffff

    class ESP32 container
    class Main,LockSys,AuthSrv,SmsSrv,CanSrv,BleMgr,CanMgr,GsmMgr component
    class Comfort,TTP,SIM external
    class Driver,Phone actor
```

---

## Level 3: Component Architecture (Raspberry Pi)
Zooms into the Raspberry Pi 4 container to show how the Python backend is structured using the MVC pattern: the **Model** holds vehicle state and trip data (SQLite), the **Controller** reads CAN frames and ESP32 serial data, and the **View** is the HUDIY dashboard rendered in a fullscreen browser.

```mermaid
graph TD
    Driver([Driver])
    Phone([Mobile App])
    PowertrainBus[Powertrain CAN Bus]
    ESP32Link[ESP32 «Serial USB»]

    subgraph Pi [Raspberry Pi 4 «Python / MVC»]

        %% View Layer
        subgraph ViewLayer [View — HUDIY Dashboard]
            Gauges[Gauge Components\n«HTML / CSS / JS»\nRPM, Speed, Temp, Fuel]
            Alerts[Alert Overlay\n«HTML / JS»\nCheck Engine, Overheat]
            AndroidAuto[Android Auto\n«HUDIY»\nNav, Media, Calls]
        end

        %% Controller Layer
        subgraph ControllerLayer [Controller — Backend Python]
            CanCtrl[CAN Controller\n«can_controller.py»\nReads SocketCAN, decodes frames]
            EspCtrl[ESP Controller\n«esp_controller.py»\nReads Serial USB from ESP32]
            WsServer[WebSocket Server\n«ws_server.py / FastAPI»\nPushes JSON to View at 20Hz]
            BleCtrl[BLE Controller\n«ble_controller.py»\nSends reports to Mobile App]
        end

        %% Model Layer
        subgraph ModelLayer [Model — Data & State]
            VehicleState[Vehicle State\n«vehicle_state.py»\nIn-memory current values]
            DbcDecoder[DBC Decoder\n«dbc_decoder.py»\nParses CAN bytes → values]
            TripLogger[Trip Logger\n«trip_logger.py»\nDetects trips, aggregates stats]
            DB[(SQLite Database\n«trips.db»\ntrips, trip_stats,\ntrip_fuel, trip_temps)]
        end
    end

    %% External connections
    PowertrainBus -->|SPI / SocketCAN| CanCtrl
    ESP32Link -->|Serial USB| EspCtrl
    Driver -->|Views| Gauges
    Phone <-->|BLE| BleCtrl

    %% Controller → Model (dependencies)
    CanCtrl -->|Uses| DbcDecoder
    DbcDecoder -->|Updates| VehicleState
    EspCtrl -->|Updates| VehicleState
    VehicleState -->|Feeds| TripLogger
    TripLogger -->|Writes to| DB

    %% Model / Controller → View
    VehicleState -->|Feeds| WsServer
    WsServer -->|Uses «WebSocket»| Gauges
    WsServer -->|Uses «WebSocket»| Alerts

    %% Model → Controller (reports)
    DB -->|Read by| BleCtrl

    %% Estilos (Cores C4)
    classDef container fill:#1168bd,stroke:#0b4884,color:#ffffff
    classDef component fill:#85bbf0,stroke:#5b93c7,color:#000000
    classDef external fill:#999999,stroke:#666666,color:#ffffff
    classDef actor fill:#08427b,stroke:#052e56,color:#ffffff
    classDef database fill:#438dd5,stroke:#2e6295,color:#ffffff

    class Pi container
    class Gauges,Alerts,AndroidAuto,CanCtrl,EspCtrl,WsServer,BleCtrl,VehicleState,DbcDecoder,TripLogger component
    class DB database
    class PowertrainBus,ESP32Link external
    class Driver,Phone actor
```