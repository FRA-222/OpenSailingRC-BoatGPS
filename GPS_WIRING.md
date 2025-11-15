# OpenSailingRC-BoatGPS - Câblage GPS

## Configuration Matérielle

### Module GPS
- **Type** : NEO-6M ou compatible
- **Protocole** : NMEA 0183
- **Vitesse** : 9600 bauds
- **Constellations** : GPS + Galileo (configuré automatiquement au démarrage)

### Câblage AtomS3 Lite

| GPS Pin | AtomS3 Pin | Description |
|---------|------------|-------------|
| VCC     | 5V         | Alimentation GPS (3.3V ou 5V selon module) |
| GND     | GND        | Masse |
| TX      | GPIO5      | Transmission GPS → Réception ESP32 (RX) |
| RX      | GPIO6      | Réception GPS ← Transmission ESP32 (TX) |

⚠️ **Important** : 
- Le GPS TX se connecte au RX de l'ESP32 (GPIO5) et vice versa !
- L'ESP32-S3 a des restrictions sur les GPIO UART, d'où l'utilisation de GPIO5/6

### LED de Statut
- **GPIO35** : LED RGB WS2812 intégrée au bouton de l'AtomS3 Lite

## Codes Couleur LED

| Couleur | État |
|---------|------|
| 🔵 Bleu | Initialisation système |
| 🟡 Jaune | En attente du fix GPS (< 4 satellites) |
| 🟢 Vert | GPS valide et transmission active |
| 🔴 Rouge | Erreur critique |

## Configuration GPS

Au démarrage, le système envoie automatiquement une commande UBX pour activer :
- **GPS (USA)** : Système de positionnement américain
- **Galileo (EU)** : Système de positionnement européen

Les autres constellations (GLONASS, BeiDou, SBAS) sont désactivées pour optimiser les performances.

## Diagnostic

### Vérification du câblage
1. Téléverser le firmware
2. Ouvrir le moniteur série (115200 bauds)
3. Observer les messages d'initialisation :
   ```
   ✓ GPS: Initialized
     RX: GPIO22, TX: GPIO21, Baud: 9600
     Waiting for GPS data...
     Configuring GPS + Galileo...
     GPS constellation configuration sent
   ```

### Pas de signal GPS ?
- Vérifier que le module GPS a vue sur le ciel (pas en intérieur)
- Attendre 1-2 minutes pour le premier fix (cold start)
- Vérifier le câblage (TX/RX inversés ?)
- Vérifier l'alimentation du module GPS (LED rouge allumée sur le module)

### Messages GPS
Pour voir les trames NMEA brutes, décommenter dans `GPS.cpp` :
```cpp
// Serial.print(c);
```

## Carte SD (M5Stack Atom GPS Base)

Le module **Atom GPS Base** dispose d'un slot microSD intégré.

### Configuration SPI (AtomS3 + Atom GPS Base)
- **CS** : GPIO9
- **MOSI** : GPIO8
- **MISO** : GPIO7
- **SCK** : GPIO17

### Utilisation
- **Format** : FAT32
- **Fichiers** : `/gps_XXX.json` (auto-incrémenté)
- **Rotation** : Nouveau fichier tous les 10MB ou 10 000 enregistrements
- **Structure JSON** : Compatible avec OpenSailingRC-Display

### Préparation
1. Formater la carte SD en FAT32
2. Insérer la carte dans le slot du module Atom GPS
3. Le système créera automatiquement les fichiers de log

## Performance

- **Fréquence de mise à jour GPS** : 1 Hz (1 fois par seconde)
- **Broadcast ESP-NOW** : Toutes les 1 secondes
- **Précision typique** : 2-5 mètres avec GPS+Galileo
- **Temps au premier fix** : 30-60 secondes (cold start)
