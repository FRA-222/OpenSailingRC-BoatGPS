# OpenSailingRC-BoatGPS v1.0.5

**Date de publication :** 24 février 2026  
**Type :** Maintenance et packaging

## 🎯 Contenu

### 📦 Deux firmwares pré-compilés

Cette release fournit des firmwares merged prêts à flasher pour les deux plateformes supportées :

| Fichier | Plateforme | Taille |
|---------|-----------|--------|
| `OpenSailingRC_BoatGPS_v1.0.5_AtomS3_MERGED.bin` | M5Stack AtomS3 Lite + GPS Atom v2 (AT6668) | 932 KB |
| `OpenSailingRC_BoatGPS_v1.0.5_AtomLite_MERGED.bin` | M5Stack Atom Lite + GPS Base (NEO-6M) | 1.0 MB |

### ✨ Rappel des fonctionnalités

- **Nom personnalisé du bateau** via M5Burner (Preferences NVS)
- **Broadcast ESP-NOW** optimisé (1 Hz + jitter anti-collision)
- **Numéro de séquence** pour détection de perte de paquets
- **Paquet compact** ~52 bytes (messageType, name, sequenceNumber, gpsTimestamp, lat, lon, speed, heading, satellites)
- **Compatible** avec Display v1.0.x

## 📋 Configuration du nom de bateau

### Via M5Burner (recommandé)

1. Ouvrir M5Burner
2. Charger le firmware `.bin` correspondant à votre hardware
3. Cliquer sur **⚙️ Configuration**
4. Remplir le champ **"Boat Name"** (max 17 caractères)
   - Exemples : `BOAT1`, `FRA999`, `Starboard`
   - Laisser vide → utilise l'adresse MAC automatiquement
5. Flasher à l'adresse **0x0000**

### Via PlatformIO (pour développeurs)

```cpp
// Ajouter temporairement dans setup() :
Preferences preferences;
preferences.begin("boatgps", false);
preferences.putString("boat_name", "BOAT1");
preferences.end();
```

## 🔧 Vérification après flash

Ouvrir le Serial Monitor (115200 bauds) et vérifier :

```
==========================================
  OpenSailingRC - BoatGPS Tracker v1.0
==========================================

1. Initializing GPS...
2. Initializing ESP-NOW...
  Custom boat name: BOAT1
  Boat ID (MAC): AA:BB:CC:DD:EE:FF
```

Si pas de nom configuré :
```
  No custom boat name - using MAC address
```

## 📦 Checksums SHA256

```
c732aa498d9bf3c07470d11fef26d9fd36d213284d104f05d6cbfe44e3bd3a0f  OpenSailingRC_BoatGPS_v1.0.5_AtomLite_MERGED.bin
5305846a44d2754ede1378851f59f8ff08c19b8d2e4678be67cfeb1f0a378cdd  OpenSailingRC_BoatGPS_v1.0.5_AtomS3_MERGED.bin
```

## ⚙️ Configuration M5Burner (m5burner.json)

Le fichier `m5burner.json` à la racine du projet définit les préférences configurables dans M5Burner :

```json
{
  "name": "OpenSailingRC-BoatGPS",
  "version": "1.0.5",
  "description": "GPS tracker for RC sailboats with ESP-NOW broadcast",
  "author": "OpenSailingRC Contributors",
  "license": "GPL-3.0",
  "platform": "ESP32-S3",
  "board": "M5Stack AtomS3",
  "preferences": [
    {
      "key": "boat_name",
      "label": "Boat Name",
      "type": "string",
      "default": "",
      "description": "Custom name for this boat (max 17 characters). Leave empty to use MAC address.",
      "maxLength": 17,
      "placeholder": "e.g. BOAT1, FRA999, etc."
    }
  ]
}
```

### Comment ça fonctionne

1. **M5Burner** lit `m5burner.json` et affiche un formulaire de configuration
2. L'utilisateur saisit le nom du bateau
3. M5Burner écrit la valeur dans la **NVS** (Non-Volatile Storage) de l'ESP32
4. Au démarrage, le firmware lit la NVS avec `preferences.getString("boat_name", "")`
5. Si la valeur est non-vide → utilisée comme identifiant du bateau
6. Si vide → l'adresse MAC est utilisée comme fallback

### Namespace NVS

| Propriété | Valeur |
|-----------|--------|
| **Namespace** | `boatgps` |
| **Clé** | `boat_name` |
| **Type** | String |
| **Max** | 17 caractères |
| **Défaut** | `""` (adresse MAC) |

## 🔄 Compatibilité

- ✅ Display OpenSailingRC v1.0.x
- ✅ Bouée Autonomous GPS Buoy
- ✅ Fonctionne avec ou sans nom personnalisé
- ✅ Rétrocompatible avec les versions précédentes du Display
