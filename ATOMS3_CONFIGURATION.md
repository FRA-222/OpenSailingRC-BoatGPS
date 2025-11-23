# Configuration AtomS3 Lite + GPS Atom v2 (AT6668)

## Vue d'ensemble

Ce document décrit la configuration pour utiliser le M5Stack AtomS3 Lite avec le module GPS Atom v2 (AT6668).

**Différences avec Atom Lite + GPS Base :**
- ✅ Pas de carte SD (stockage désactivé)
- ✅ Pins GPS différentes (GPIO1/GPIO2)
- ✅ LED RGB sur GPIO35
- ✅ ESP32-S3 au lieu d'ESP32 classique
- ✅ USB CDC natif pour Serial

## Configuration Matérielle

### M5Stack AtomS3 Lite
- **MCU :** ESP32-S3-PICO-1-N8R2 (8MB Flash, 2MB PSRAM)
- **LED RGB :** GPIO35 (SK6812)
- **Bouton :** GPIO41

### GPS Atom v2 (AT6668)
- **Module :** Airoha AT6668
- **Constellations :** GPS + Galileo + BeiDou
- **Connexion I2C :** Disponible mais non utilisée
- **Connexion UART :** GPIO1 (TX) / GPIO2 (RX)

### Connexion

**AtomS3 Lite → GPS Atom v2 :**
```
AtomS3 GPIO1 (TX) → GPS AT6668 RX
AtomS3 GPIO2 (RX) → GPS AT6668 TX
GND              → GND
5V               → 5V
```

**Baudrate :** 9600 bps (par défaut pour AT6668)

## Configuration PlatformIO

**Fichier :** `platformio.ini`

```ini
[env:m5stack-atoms3]
platform = espressif32@6.5.0
board = m5stack-atoms3
framework = arduino

; Build options
build_flags = 
    -DARDUINO_USB_CDC_ON_BOOT=1      # USB CDC pour Serial
    -DARDUINO_USB_MODE=1              # Mode USB natif
    -DCORE_DEBUG_LEVEL=3              # Debug logs
    -DDISABLE_SD_STORAGE=1            # Désactive stockage SD

; Monitor options
monitor_speed = 115200
monitor_filters = esp32_exception_decoder
monitor_rts = 0
monitor_dtr = 0

; Library dependencies
lib_deps = 
    m5stack/M5Unified@^0.1.16
    mikalhart/TinyGPSPlus@^1.0.3
    bblanchon/ArduinoJson@^7.0.4
    fastled/FastLED@^3.7.0

; Upload settings
upload_speed = 1500000
```

## Configuration Pins (Code)

**Fichier :** `src/main.cpp`

```cpp
#ifdef CONFIG_IDF_TARGET_ESP32S3
// AtomS3 Lite + GPS Atom v2 (AT6668)
const uint8_t GPS_RX_PIN = 2;    // GPIO2 <- GPS TX
const uint8_t GPS_TX_PIN = 1;    // GPIO1 -> GPS RX
const uint8_t LED_PIN = 35;      // RGB LED
#else
// Atom Lite + GPS Base
const uint8_t GPS_RX_PIN = 22;
const uint8_t GPS_TX_PIN = 19;
const uint8_t LED_PIN = 27;
#endif

// SD Storage désactivé pour AtomS3
#ifdef DISABLE_SD_STORAGE
const bool ENABLE_SD_STORAGE = false;
#else
const bool ENABLE_SD_STORAGE = true;
#endif
```

## Compilation et Upload

### Compilation
```bash
pio run --environment m5stack-atoms3
```

### Upload
```bash
pio run --target upload --environment m5stack-atoms3
```

### Monitor
```bash
pio device monitor --environment m5stack-atoms3
```

**Note :** Sur ESP32-S3 avec USB CDC, le port série apparaît comme `/dev/cu.usbmodem*`

## Fonctionnalités

### ✅ Activées
- GPS acquisition (AT6668)
- ESP-NOW broadcast
- Logs Serial (via USB CDC)
- LED RGB status
- Séquence de numérotation des paquets
- Configuration GPS (GPS + Galileo)

### ❌ Désactivées
- Stockage SD card
- Écriture fichiers JSON

## LED Status

| Couleur | État |
|---------|------|
| 🔵 Bleu | Initialisation |
| 🟡 Jaune | Attente fix GPS |
| 🟢 Vert | Fix GPS valide, broadcast actif |
| 🔴 Rouge (clignotant) | Erreur |

## Logs Serial

### Au Démarrage
```
==========================================
  OpenSailingRC - BoatGPS Tracker v1.0
==========================================

1. Initializing GPS...
✓ GPS: Initialized
  RX: GPIO2, TX: GPIO1, Baud: 9600
  Waiting for GPS data...
  GPS constellation configuration sent

2. Initializing ESP-NOW...
✓ ESP-NOW: MAC Address: XX:XX:XX:XX:XX:XX
✓ ESP-NOW: Initialized in broadcast mode
✓ ESP-NOW: Broadcast peer added

3. Initializing Logger...
✓ Logger: Initialized

4. Initializing Storage...
✓ SD storage disabled (AtomS3 Lite configuration)

==========================================
  System Ready - Waiting for GPS fix...
==========================================
```

### En Fonctionnement
```
[SEQ #1] Lat: 48.123456, Lon: -4.567890, Speed: 5.2 knots, Course: 180.0°, Sats: 12

📊 Status Report
  Valid packets:   1
  Invalid packets: 0
  Satellites:      12
  HDOP:           1.2
  Uptime:         10s
```

## Différences avec Atom Lite

| Caractéristique | Atom Lite | AtomS3 Lite |
|-----------------|-----------|-------------|
| MCU | ESP32 | ESP32-S3 |
| Flash | 4MB | 8MB |
| PSRAM | Non | 2MB |
| GPS Pins | GPIO22/19 | GPIO1/2 |
| LED Pin | GPIO27 | GPIO35 |
| Serial USB | UART → USB | USB CDC natif |
| SD Card | Oui (GPS Base) | Non |

## Dépannage

### GPS ne reçoit pas de données

**Vérifier les pins :**
```cpp
Serial.printf("GPS RX Pin: %d, TX Pin: %d\n", GPS_RX_PIN, GPS_TX_PIN);
```

**Activer debug NMEA :**
Dans `GPS.cpp`, décommenter :
```cpp
void GPS::update() {
    while (gpsSerial->available() > 0) {
        char c = gpsSerial->read();
        Serial.print(c);  // ← Décommenter
        gps.encode(c);
    }
}
```

### Port Serial non reconnu

**ESP32-S3 USB CDC :**
- Attendre 3 secondes après reset
- Vérifier les flags : `-DARDUINO_USB_CDC_ON_BOOT=1`
- Port apparaît comme `/dev/cu.usbmodem*` (pas `/dev/cu.usb*`)

### LED ne s'allume pas

**Vérifier la configuration FastLED :**
```cpp
// LED AtomS3 = SK6812 (pas WS2812)
FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_COUNT);
FastLED.setBrightness(50);
```

Si problème, tester avec :
```cpp
FastLED.addLeds<SK6812, LED_PIN, GRB>(leds, LED_COUNT);
```

## Performances

### Mémoire
- **RAM :** 14.4% (47136 bytes / 327680 bytes)
- **Flash :** 26.5% (885505 bytes / 3342336 bytes)

### Consommation
- **Idle (no GPS fix) :** ~80-100 mA
- **GPS fix + Broadcast :** ~120-150 mA
- **LED éteinte :** -10 mA

### Portée ESP-NOW
- **Puissance TX :** 21 dBm (maximum)
- **Portée typique :** 50-100m (dépend environnement)
- **Avec 4 retries :** Taux réception > 98%

## Compatibilité Display

Le Display (M5Stack Core2) reçoit les paquets de **tous les bateaux** (Atom Lite ou AtomS3) sans distinction.

**Structure de paquet identique :**
```cpp
struct GPSBroadcastPacket {
    int8_t messageType;      // 1 = Boat
    char name[18];           // MAC address
    int boatId;
    uint32_t sequenceNumber;
    uint32_t gpsTimestamp;
    float latitude;
    float longitude;
    float speed;
    float heading;
    uint8_t satellites;
};
```

## Avantages AtomS3 vs Atom Lite

### ✅ Avantages
- **Plus de Flash** : 8MB vs 4MB (plus d'espace pour futures fonctionnalités)
- **PSRAM** : 2MB disponible pour buffers
- **USB natif** : Plus stable pour Serial Monitor
- **Plus compact** : Sans GPS Base en dessous

### ⚠️ Inconvénients
- **Pas de SD** : Pas de stockage local (seulement sur Display)
- **Pins différentes** : Nécessite adaptation code

## Recommandations

### Pour Utilisation Simple (Tracking uniquement)
✅ **AtomS3 Lite + GPS Atom v2**
- Plus compact
- Pas besoin de stockage local
- Données reçues et stockées sur Display

### Pour Utilisation Avancée (Backup local)
✅ **Atom Lite + GPS Base**
- Stockage SD local
- Backup si Display hors portée
- Compatible code existant

## Fichiers Modifiés

1. **platformio.ini** :
   - Ajout flag `-DDISABLE_SD_STORAGE=1`
   - Configuration USB CDC

2. **src/main.cpp** :
   - Pins GPIO1/GPIO2 pour GPS
   - Désactivation conditionnelle SD
   - Logs adaptés

3. Aucune modification dans :
   - `GPS.cpp` / `GPS.h`
   - `Communication.cpp` / `Communication.h`
   - `Storage.cpp` / `Storage.h`

## Version

- **Firmware :** v1.0
- **Date :** 23 novembre 2025
- **Testé avec :** ESP32-S3, GPS AT6668
