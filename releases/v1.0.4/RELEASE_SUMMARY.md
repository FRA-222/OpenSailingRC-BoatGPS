# Release v1.0.4 - OpenSailingRC-BoatGPS

**Date :** 25 novembre 2025  
**Type :** Feature + Optimisation

## 📦 Contenu de la release

### Fichiers disponibles

| Fichier | Taille | Description |
|---------|--------|-------------|
| `OpenSailingRC_BoatGPS_v1.0.4_MERGED.bin` | 932 KB | Firmware combiné prêt à flasher |
| `README.md` | 7 KB | Guide d'installation complet |
| `RELEASE_NOTES_V1.0.4.md` | 6 KB | Notes de version détaillées |
| `SHA256SUMS.txt` | 106 B | Checksums de vérification |

### Checksum SHA256

```
01849774ea99f3b7b441242779c6d1882e868ac51f60df9d4a38a847165e564b  OpenSailingRC_BoatGPS_v1.0.4_MERGED.bin
```

## 🎯 Principales nouveautés

### 1. Nom personnalisé du bateau (M5Burner)
- Configuration du nom via M5Burner lors du flash
- Champ `name` : nom personnalisé ou MAC address
- Identification facile des bateaux sur le Display
- Max 17 caractères, stocké en NVS

### 2. Optimisation ESP-NOW
- **-8 bytes par paquet** (timestamp + boatId supprimés)
- Paquet réduit de 60 → 52 bytes (-13%)
- Meilleure portée et fiabilité

## 🚀 Installation

### Via M5Burner (recommandé)

```
1. Connecter M5Stack AtomS3 Lite
2. Charger OpenSailingRC_BoatGPS_v1.0.4_MERGED.bin
3. Adresse: 0x0000
4. Configuration: Définir "Boat Name" (ex: BOAT1)
5. Cliquer "Burn"
```

### Via esptool

```bash
esptool.py --chip esp32s3 --port /dev/ttyUSB0 \
  write_flash 0x0000 OpenSailingRC_BoatGPS_v1.0.4_MERGED.bin
```

## ✅ Compatibilité

| Composant | Version min | Version recommandée | Status |
|-----------|-------------|---------------------|--------|
| Display | v1.0.3 | v1.0.4 | ✅ Compatible |
| Anemometer | v1.0.3 | v1.0.3 | ✅ Compatible |
| M5Stack AtomS3 | Hardware v1.0 | - | ✅ Testé |
| GPS Atom v2 | AT6668 | - | ✅ Testé |

## 📊 Tests effectués

### Matériel
- ✅ M5Stack AtomS3 Lite + GPS Atom v2
- ✅ M5Stack Core2 Display v1.0.4

### Fonctionnalités
- ✅ Flash avec nom personnalisé
- ✅ Flash sans nom (fallback MAC)
- ✅ Transmission ESP-NOW optimisée
- ✅ Réception sur Display v1.0.4
- ✅ Logs SD avec nom personnalisé
- ✅ Portée 90m (75% réception)

## 🔄 Migration depuis v1.0.3

### Changements incompatibles (breaking changes)

**Structure des paquets ESP-NOW modifiée :**
- ❌ `timestamp` supprimé (géré par Display)
- ❌ `boatId` supprimé (calculé par Display)

**Impact :**
- Display v1.0.3 : ⚠️ Peut avoir des problèmes de synchronisation
- Display v1.0.4 : ✅ Pleine compatibilité

**Action requise :**
- Mettre à jour le Display en v1.0.4 pour bénéficier des optimisations

## 📚 Documentation

### Guides disponibles
- `README.md` - Installation complète
- `RELEASE_NOTES_V1.0.4.md` - Notes de version
- `M5BURNER_PREFERENCES.md` - Configuration préférences

### Documentation technique
- Structure des paquets ESP-NOW
- API Communication
- Gestion des préférences NVS

## 🐛 Problèmes connus

Aucun problème connu dans cette version.

## 🔮 Roadmap v1.0.5

- Support protocoles GPS multiples (NMEA + UBX)
- Mode économie d'énergie configurable
- Intervalle broadcast configurable

## 📄 Licence

GNU General Public License v3.0

## 👥 Contributeurs

- Philippe Hubert (@FRA-222)
- OpenSailingRC Community

---

**Installation recommandée :** M5Burner avec configuration du nom personnalisé

**Compatibilité Display :** Mettre à jour en v1.0.4 pour optimisations complètes
