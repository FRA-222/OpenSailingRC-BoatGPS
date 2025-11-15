# OpenSailingRC-BoatGPS

GPS tracker with ESP-NOW broadcast for sailing boats. Transmits real-time GPS position to all listening devices and logs data to SD card.

## Features

- **GPS Tracking**: Real-time position, speed, and course tracking
- **ESP-NOW Broadcast**: Wireless transmission of GPS data to all nearby devices
- **Boat Identification**: Each boat identified by its unique MAC address
- **SD Card Logging**: Optional data logging in CSV format
- **Data Validation**: Only transmits validated GPS data (minimum 4 satellites)
- **Status LEDs**: Visual feedback for system status
- **Compatible Format**: CSV format compatible with OpenSailingRC-Display

## Hardware Requirements

- **Board**: M5Stack Atom S3 (or compatible ESP32 board)
- **GPS Module**: Any NMEA-compatible GPS (UART interface)
- **Optional**: SD card for data logging

## Wiring

### GPS Module
- GPS RX → GPIO 1
- GPS TX → GPIO 2
- GPS VCC → 5V/3.3V
- GPS GND → GND

### SD Card (M5Stack Atom S3)
- SD CS → GPIO 4
- SD MOSI → GPIO 23
- SD MISO → GPIO 19
- SD SCK → GPIO 18

## Installation

1. Open this project in PlatformIO
2. Connect your M5Stack Atom S3
3. Build and upload:
   ```
   pio run -t upload
   ```

## Configuration

Edit the constants in `src/main.cpp`:

```cpp
const uint8_t GPS_RX_PIN = 1;              // GPS RX pin
const uint8_t GPS_TX_PIN = 2;              // GPS TX pin
const uint32_t BROADCAST_INTERVAL = 1000;  // Broadcast interval (ms)
const bool ENABLE_SD_LOGGING = true;       // Enable/disable SD logging
```

## LED Status Indicators

- **Blue**: System initializing
- **Yellow**: Waiting for valid GPS fix
- **Green**: GPS valid, broadcasting data
- **Red blinking**: Initialization error

## Data Format

### ESP-NOW Broadcast Packet

```cpp
struct GPSBroadcastPacket {
    uint8_t macAddress[6];   // Boat MAC address
    double latitude;         // Latitude (degrees)
    double longitude;        // Longitude (degrees)
    float speed;             // Speed (knots)
    float course;            // Course (degrees)
    uint32_t timestamp;      // Timestamp (ms)
    uint8_t satellites;      // Number of satellites
    uint16_t packetNumber;   // Sequential packet number
};
```

### SD Card CSV Format

```
timestamp,latitude,longitude,speed,course,satellites,mac_address
1234567,48.123456,-4.567890,5.2,180.5,8,AA:BB:CC:DD:EE:FF
```

## File Management

- Files are named: `gps_001.csv`, `gps_002.csv`, etc.
- Automatic file rotation at 10 MB or 10,000 records
- Files are automatically numbered sequentially

## Serial Monitor

Monitor output at 115200 baud:
```
pio device monitor
```

## Architecture

The project follows a clean modular architecture:

- **GPS**: Handles GPS module communication and data validation
- **Communication**: Manages ESP-NOW broadcast
- **Logger**: Handles serial and SD card logging
- **main**: Application logic and system coordination

## Compatibility

- **Display**: Data format compatible with OpenSailingRC-Display
- **Anemometer**: Similar architecture to OpenSailingRC-Anemometer-v2
- **Protocol**: Standard ESP-NOW broadcast (no pairing required)

## Author

OpenSailingRC Project

## License

MIT License

## Version

1.0.0 (2025)
