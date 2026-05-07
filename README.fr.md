# OpenSailingRC-BoatGPS

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32%20%2F%20ESP32--S3-green.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Hardware: M5Stack AtomS3 / Atom Lite](https://img.shields.io/badge/Hardware-M5Stack%20AtomS3%20%2F%20Atom%20Lite-orange.svg)](https://shop.m5stack.com/)
[![Version](https://img.shields.io/badge/Version-1.0.6-brightgreen.svg)](releases/)

> **[English version](README.md)**

Traceur GPS pour bateaux de voile radiocommandés avec broadcast ESP-NOW. Transmet la position GPS en temps réel à tous les appareils à l'écoute (afficheurs, enregistreurs) et journalise optionnellement les données sur carte SD.

## Fonctionnalités

- **Suivi GPS** : Position, vitesse et cap en temps réel à 1 Hz
- **Nom personnalisé** : Configurable via les préférences M5Burner (NVS), repli sur l'adresse MAC
- **Broadcast ESP-NOW** : Transmission sans fil à tous les appareils voisins (100–200 m en visibilité directe)
- **Anti-collision** : Jitter aléatoire de ±100 ms pour éviter les collisions entre bateaux
- **Numéro de séquence** : Compteur incrémental pour détecter la perte de paquets
- **Journalisation SD** : Enregistrement CSV optionnel (Atom Lite + GPS Base uniquement)
- **Validation des données** : Transmission uniquement avec un fix GPS valide (minimum 4 satellites)
- **LED RGB** : Retour visuel sur l'état du système
- **Compatible** : Format de paquet compatible avec OpenSailingRC-Display v1.0.x

## Matériel supporté

| Configuration | Carte | Module GPS | Carte SD |
|---|---|---|---|
| **AtomS3 Lite** | M5Stack AtomS3 Lite | GPS Atom v2 (AT6668) | Non |
| **Atom Lite** | M5Stack Atom Lite | GPS Base (NEO-6M) | Oui |

## Câblage

### AtomS3 Lite + GPS Atom v2

Le module GPS se connecte via le port Grove (automatique) :
- GPS RX → GPIO 5
- GPS TX → GPIO 6

### Atom Lite + GPS Base

- GPS RX → GPIO 22
- GPS TX → GPIO 19
- Carte SD incluse sur la GPS Base

## Installation

### Option 1 – Flasher le firmware pré-compilé (recommandé)

Téléchargez le dernier fichier `_MERGED.bin` dans le dossier [releases/](releases/) et flashez-le avec [M5Burner](https://docs.m5stack.com/en/tool/m5burner) ou `esptool` :

```bash
# AtomS3 Lite
esptool.py --chip esp32s3 write_flash 0x0 OpenSailingRC_BoatGPS_v1.0.6_AtomS3_MERGED.bin

# Atom Lite
esptool.py --chip esp32 write_flash 0x0 OpenSailingRC_BoatGPS_v1.0.6_AtomLite_MERGED.bin
```

### Option 2 – Compiler depuis les sources

1. Ouvrir ce projet dans PlatformIO
2. Connecter la carte
3. Sélectionner l'environnement et uploader :

```bash
# AtomS3 Lite
pio run -e m5stack-atoms3 -t upload

# Atom Lite
pio run -e m5stack-atom -t upload
```

## Configuration du nom de bateau

Le nom est stocké dans la NVS (mémoire non volatile) et peut être défini sans recompiler :

1. Ouvrir **M5Burner**
2. Charger le firmware `.bin` correspondant à votre matériel
3. Cliquer sur **Configure** et renseigner le champ `boat_name` (17 caractères max, ex : `BOAT1`, `FRA999`)
4. Flasher le firmware — le nom est conservé après redémarrage

Si le champ est vide, l'appareil utilise son adresse MAC comme identifiant.

## Indicateurs LED

| Couleur | Signification |
|---|---|
| Bleu | Initialisation du système |
| Jaune | En attente d'un fix GPS valide |
| Vert | Fix GPS valide, broadcast en cours |
| Rouge (clignotant) | Erreur d'initialisation |

## Format des données

### Paquet broadcast ESP-NOW

```cpp
struct GPSBroadcastPacket {
    int8_t   messageType;      // 1 = Bateau
    char     name[18];         // Nom du bateau ou adresse MAC
    uint32_t sequenceNumber;   // Compteur incrémental
    uint32_t gpsTimestamp;     // Horodatage GPS (ms)
    float    latitude;         // Latitude (degrés)
    float    longitude;        // Longitude (degrés)
    float    speed;            // Vitesse (nœuds)
    float    heading;          // Cap (degrés, 0=N)
    uint8_t  satellites;       // Nombre de satellites
    uint8_t  ttl;              // Time-To-Live (support relais)
};
```

### Format CSV carte SD (Atom Lite uniquement)

```
timestamp,latitude,longitude,speed,heading,satellites,name
1234567,48.123456,-4.567890,5.2,180.5,8,FRA999
```

Les fichiers sont nommés `gps_001.csv`, `gps_002.csv`, … et pivotent à 10 Mo ou 10 000 enregistrements.

## Moniteur série

```bash
pio device monitor   # 115200 bauds
```

## Architecture

| Module | Responsabilité |
|---|---|
| `GPS` | Communication avec le module GPS et validation des données |
| `Communication` | Broadcast ESP-NOW avec logique de retry |
| `Logger` | Journalisation série et carte SD |
| `Storage` | Préférences NVS (nom du bateau) |
| `main` | Logique applicative et coordination |

## Compatibilité

- **OpenSailingRC-Display** : Format de paquet entièrement compatible avec v1.0.x
- **Protocole** : Broadcast ESP-NOW standard — aucun appairage nécessaire

## Licence

GNU General Public License v3.0 — voir [LICENSE.md](LICENSE.md)

## Version

**1.0.6** (mai 2026)
