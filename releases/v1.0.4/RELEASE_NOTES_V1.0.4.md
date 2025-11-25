# OpenSailingRC-BoatGPS v1.0.4

**Date de publication :** 25 novembre 2025  
**Type :** Optimisation et personnalisation

## 🎯 Nouveautés

### ✨ Nom personnalisé du bateau via M5Burner
- Possibilité de définir un **nom personnalisé** pour chaque bateau lors du flash avec M5Burner
- Le champ `name` contient désormais :
  - Le **nom personnalisé** (ex: "BOAT1", "Starboard") si configuré dans M5Burner
  - L'**adresse MAC** (ex: "AA:BB:CC:DD:EE:FF") par défaut si pas de configuration
- Identification plus facile des bateaux sur le Display et dans les logs
- Configuration persistante stockée en mémoire flash (NVS)

**Configuration dans M5Burner :**
1. Sélectionner le firmware OpenSailingRC-BoatGPS v1.0.4
2. Cliquer sur "Configuration" (icône engrenage)
3. Entrer un nom dans le champ "Boat Name" (max 17 caractères)
4. Flasher le firmware avec la configuration

### 📦 Optimisation de la communication ESP-NOW

**Réduction de la taille des paquets :**
- ❌ Suppression de `timestamp` (4 bytes) - géré localement par le Display
- ❌ Suppression de `boatId` (4 bytes) - calculé par le Display à partir de la MAC
- **Économie totale : 8 bytes par paquet** (de ~60 à ~52 bytes)

**Bénéfices :**
- 🚀 Meilleure portée ESP-NOW
- 📶 Fiabilité accrue à longue distance (90m+)
- 📉 Réduction de la charge réseau de ~13%

## 🔧 Améliorations techniques

### Gestion des préférences
- Utilisation de la bibliothèque `Preferences` ESP32
- Namespace : `boatgps`
- Clé : `boat_name`
- Lecture au démarrage avec fallback automatique sur l'adresse MAC

### Compatibilité
- ✅ **Rétrocompatible** avec Display v1.0.3
- ✅ **Compatible** avec Display v1.0.4 (recommandé)
- ✅ Fonctionne avec ou sans nom personnalisé
- ✅ Les anciennes versions du Display afficheront la MAC ou le nom personnalisé

## 📋 Structure des paquets ESP-NOW

```cpp
struct GPSBroadcastPacket {
    int8_t messageType;           // 1 = Boat
    char name[18];                // Nom personnalisé ou MAC (17 chars + null)
    uint32_t sequenceNumber;      // Numéro de séquence
    uint32_t gpsTimestamp;        // Timestamp GPS
    float latitude;               // Latitude en degrés
    float longitude;              // Longitude en degrés
    float speed;                  // Vitesse en nœuds
    float heading;                // Cap en degrés
    uint8_t satellites;           // Nombre de satellites
};
```

**Taille du paquet : ~52 bytes** (optimisé)

## 📦 Installation

### Via M5Burner (recommandé)

1. **Télécharger le firmware :**
   - `OpenSailingRC_BoatGPS_v1.0.4_MERGED.bin`

2. **Configurer M5Burner :**
   - Port : Sélectionner le port USB de l'AtomS3
   - Firmware : Charger le fichier .bin
   - Adresse : `0x0000`
   - **Configuration** (optionnel) : Définir "Boat Name"

3. **Flasher :**
   - Cliquer sur "Burn"
   - Attendre la fin du flash
   - Redémarrer l'AtomS3

### Via PlatformIO

```bash
cd OpenSailingRC-BoatGPS
platformio run --target upload --environment m5stack-atoms3
```

**Note :** Le nom personnalisé doit être configuré via M5Burner ou directement dans le NVS

## 🧪 Tests effectués

### Matériel testé
- ✅ M5Stack AtomS3 Lite + GPS Atom v2 (AT6668)
- ✅ M5Stack Core2 Display v1.0.4

### Scénarios validés
- ✅ Flash avec nom personnalisé "BOAT1"
- ✅ Flash sans nom personnalisé (MAC utilisée)
- ✅ Lecture des préférences au démarrage
- ✅ Transmission ESP-NOW avec nom personnalisé
- ✅ Réception sur Display v1.0.4
- ✅ Affichage correct du nom sur le Display
- ✅ Enregistrement du nom dans les logs SD

### Performances
- Taille du firmware : **888 KB** (26.6% de la flash)
- Utilisation RAM : **47 KB** (14.4%)
- Portée ESP-NOW testée : **90m** avec 75% de réception

## 📝 Checksums (SHA256)

```
01849774ea99f3b7b441242779c6d1882e868ac51f60df9d4a38a847165e564b  OpenSailingRC_BoatGPS_v1.0.4_MERGED.bin
```

## 🔗 Compatibilité Display

| Fonction                    | Display v1.0.3 | Display v1.0.4 |
|----------------------------|----------------|----------------|
| Réception paquets optimisés | ⚠️ Partiel     | ✅ Complet     |
| Affichage nom personnalisé  | ✅ Oui         | ✅ Oui         |
| Logs SD avec nom           | ✅ Oui         | ✅ Oui         |
| Optimisation 8 bytes       | ⚠️ Non         | ✅ Oui         |

**Recommandation :** Mettre à jour le Display en v1.0.4 pour bénéficier de toutes les optimisations

## 📚 Documentation

- `M5BURNER_PREFERENCES.md` - Guide de configuration des préférences M5Burner
- `README.md` - Documentation complète du projet
- `Communication.h` - Documentation de l'API ESP-NOW

## 🐛 Corrections de bugs

Aucune correction dans cette version (optimisations uniquement)

## 🚀 Prochaines versions

### Prévu pour v1.0.5
- Support de plusieurs protocoles GPS (NMEA + UBX)
- Mode économie d'énergie configurable
- Intervalles de broadcast configurables via préférences

## 🔧 Développement

### Dépendances
- M5Unified v0.1.17
- TinyGPSPlus v1.1.0
- ArduinoJson v7.4.2
- FastLED v3.10.3
- Preferences v2.0.0 (ESP32)

### Environnement de build
- PlatformIO 6.1.x
- ESP32 Arduino Framework 2.0.14
- ESP-IDF 4.4.x

## 📄 Licence

GNU General Public License v3.0

Copyright (c) 2025 OpenSailingRC Contributors

## 👥 Contributeurs

- Philippe Hubert (@FRA-222)
- OpenSailingRC Community

## 🙏 Remerciements

Merci à la communauté OpenSailingRC pour les tests et les retours !

---

**Installation recommandée :** Utiliser M5Burner avec configuration du nom personnalisé pour une identification optimale des bateaux.
