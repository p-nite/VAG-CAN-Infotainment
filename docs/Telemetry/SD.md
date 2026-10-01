# Telemetry Data Flow (Powertrain CAN)
*Covers Features: Pi01 to Pi09, IB01, IB02*

This sequence diagram illustrates how real-time engine data is read from the vehicle, processed by the Raspberry Pi, and displayed on the dashboard smoothly.

```mermaid
sequenceDiagram
    actor Driver
    
    box Hardware Layer
    participant ECU as Powertrain CAN
    participant MCP as MCP2515 (SPI)
    end
    
    box Backend (Raspberry Pi)
    participant Decoder as CAN Decoder (Python)
    participant API as Local API (WebSocket)
    end
    
    box Frontend
    participant UI as HUDIY Dashboard
    end

    loop Continuous (10-50Hz)
        ECU->>MCP: Broadcasts 0x280 (RPM), 0x5A0 (Speed), etc.
        MCP->>Decoder: Sends raw frames via SPI (SocketCAN)
        Decoder->>Decoder: Parses bytes using DBC file
        Decoder->>API: Pushes updated values
        API-->>UI: WebSocket Broadcast (JSON payload)
        UI->>UI: GPU-accelerated CSS Transitions (60fps)
        UI-->>Driver: Visual needle movement on gauges
    end
```
