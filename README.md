# OpenSailingRC-BoatGPS

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32%20%2F%20ESP32--S3-green.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Hardware: M5Stack AtomS3 / Atom Lite](https://img.shields.io/badge/Hardware-M5Stack%20AtomS3%20%2F%20Atom%20Lite-orange.svg)](https://shop.m5stack.com/)
[![Version](https://img.shields.io/badge/Version-1.0.6-brightgreen.svg)](releases/)

> **[Version française](README.fr.md)**

GPS tracker for RC sailing boats with ESP-NOW broadcast. Transmits real-time GPS position to all listening devices (displays, loggers) and optionally logs data to an SD card.

## Features

- **GPS Tracking**: Real-time position, speed, and heading at 1 Hz
- **Custom Boat Name**: Configurable via M5Burner preferences (NVS), falls back to MAC address
- **ESP-NOW Broadcast**: Wireless transmission to all nearby devices (100–200 m line of sight)
- **Anti-collision Jitter**: ±100 ms random delay avoids packet collisions between multiple boats
- **Sequence Number**: Incremental counter for packet loss detection
- **SD Card Logging**: Optional CSV data logging (Atom Lite + GPS Base only)
- **Data Validation**: Only transmits with a valid GPS fix (minimum 4 satellites)
- **RGB Status LED**: Visual feedback for system state
- **Compatible**: Packet format compatible with OpenSailingRC-Display v1.0.x

## Supported Hardware

| Configuration | Board | GPS Module | SD Card |
|---|---|---|---|
| **AtomS3 Lite** | M5Stack AtomS3 Lite | GPS Atom v2 (AT6668) | No |
| **Atom Lite** | M5Stack Atom Lite | GPS Base (NEO-6M) | Yes |

## Wiring

### AtomS3 Lite + GPS Atom v2

The GPS module connects via the Grove port (automatic):
- GPS RX → GPIO 5
- GPS TX → GPIO 6

### Atom Lite + GPS Base

- GPS RX → GPIO 22
- GPS TX → GPIO 19
- SD card included on GPS Base

## Installation

### Option 1 – Flash pre-compiled firmware (recommended)

Download the latest `_MERGED.bin` from the [Releases](releases/) folder and flash it with [M5Burner](https://docs.m5stack.com/en/tool/m5burner) or `esptool`:

```bash
# AtomS3 Lite
esptool.py --chip esp32s3 write_flash 0x0 OpenSailingRC_BoatGPS_v1.0.6_AtomS3_MERGED.bin

# Atom Lite
esptool.py --chip esp32 write_flash 0x0 OpenSailingRC_BoatGPS_v1.0.6_AtomLite_MERGED.bin
```

### Option 2 – Build from source

1. Open this project in PlatformIO
2. Connect your board
3. Select the correct environment and upload:

```bash
# AtomS3 Lite
pio run -e m5stack-atoms3 -t upload

# Atom Lite
pio run -e m5stack-atom -t upload
```

## Boat Name Configuration

The boat name is stored in NVS (non-volatile storage) and can be set without recompiling:

1. Open **M5Burner**
2. Load the firmware `.bin` for your hardware
3. Click **Configure** and set the `boat_name` field (max 17 characters, e.g. `BOAT1`, `FRA999`)
4. Flash the firmware — the name is preserved across reboots

If left empty, the device uses its MAC address as identifier.

## LED Status Indicators

| Color | Meaning |
|---|---|
| Blue | System initializing |
| Yellow | Waiting for valid GPS fix |
| Green | GPS valid, broadcasting |
| Red (blinking) | Initialization error |

## Data Format

### ESP-NOW Broadcast Packet

```cpp
struct GPSBroadcastPacket {
    int8_t   messageType;      // 1 = Boat
    char     name[18];         // Boat name or MAC address
    uint32_t sequenceNumber;   // Incremental counter
    uint32_t gpsTimestamp;     // GPS timestamp (ms)
    float    latitude;         // Latitude (degrees)
    float    longitude;        // Longitude (degrees)
    float    speed;            // Speed (knots)
    float    heading;          // Heading (degrees, 0=N)
    uint8_t  satellites;       // Number of satellites
    uint8_t  ttl;              // Time-To-Live (relay support)
};
```

### SD Card CSV Format (Atom Lite only)

```
timestamp,latitude,longitude,speed,heading,satellites,name
1234567,48.123456,-4.567890,5.2,180.5,8,FRA999
```

Files are named `gps_001.csv`, `gps_002.csv`, … and rotate at 10 MB or 10,000 records.

## Serial Monitor

```bash
pio device monitor   # 115200 baud
```

## Architecture

| Module | Responsibility |
|---|---|
| `GPS` | GPS module communication and data validation |
| `Communication` | ESP-NOW broadcast with retry logic |
| `Logger` | Serial and SD card logging |
| `Storage` | NVS preferences (boat name) |
| `main` | Application logic and coordination |

## Compatibility

- **OpenSailingRC-Display**: Packet format fully compatible with v1.0.x
- **Protocol**: Standard ESP-NOW broadcast — no pairing required

## License

GNU General Public License v3.0 — see [LICENSE.md](LICENSE.md)

## Version

**1.0.6** (May 2026)
