# Installation OpenSailingRC-BoatGPS v1.0.4

## 📦 Contenu du package

- `OpenSailingRC_BoatGPS_v1.0.4_MERGED.bin` - Firmware combiné (952 KB)
- `SHA256SUMS.txt` - Checksums de vérification
- `RELEASE_NOTES_V1.0.4.md` - Notes de version complètes
- `M5BURNER_PREFERENCES.md` - Guide de configuration

## 🚀 Installation rapide avec M5Burner

### Prérequis
- M5Burner installé ([Télécharger ici](https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/software/M5Burner.zip))
- M5Stack AtomS3 Lite avec GPS Atom v2
- Câble USB-C

### Étapes d'installation

#### 1. Connecter l'AtomS3
- Brancher l'AtomS3 via USB-C
- Vérifier que le port USB apparaît (ex: `/dev/cu.usbmodem*` sur Mac)

#### 2. Lancer M5Burner
- Ouvrir M5Burner
- Sélectionner le port USB dans la liste déroulante

#### 3. Charger le firmware
- Cliquer sur **"Custom"** ou **"Load firmware"**
- Sélectionner `OpenSailingRC_BoatGPS_v1.0.4_MERGED.bin`
- **Adresse de flash : `0x0000`** (important !)

#### 4. Configurer le nom du bateau (optionnel mais recommandé)
- Cliquer sur l'icône **⚙️ Configuration** ou **Settings**
- Champ **"Boat Name"** : entrer un nom personnalisé
  - Exemples : `BOAT1`, `Starboard`, `Tribord`, `Skipper`
  - Maximum : 17 caractères
  - **Laisser vide** pour utiliser l'adresse MAC par défaut

#### 5. Flasher
- Cliquer sur **"Burn"** ou **"Start"**
- Attendre la fin du processus (environ 30 secondes)
- Message de confirmation : "Burn Successfully"

#### 6. Vérifier
- Ouvrir le **Serial Monitor** dans M5Burner (115200 bauds)
- Appuyer sur le bouton Reset de l'AtomS3
- Vérifier le message de démarrage :

```
==========================================
  OpenSailingRC - BoatGPS Tracker v1.0
==========================================

1. Initializing GPS...
2. Initializing ESP-NOW...
  Custom boat name: BOAT1          <- Votre nom personnalisé
  Boat ID (MAC): AA:BB:CC:DD:EE:FF
```

## 🔧 Installation avec PlatformIO

### Prérequis
- Visual Studio Code + PlatformIO
- Projet OpenSailingRC-BoatGPS cloné

### Étapes

```bash
# 1. Naviguer dans le projet
cd OpenSailingRC-BoatGPS

# 2. Compiler
platformio run --environment m5stack-atoms3

# 3. Uploader
platformio run --target upload --environment m5stack-atoms3

# 4. Moniteur série (optionnel)
platformio device monitor --baud 115200
```

### Configuration du nom via NVS Tools

Si vous utilisez PlatformIO, vous pouvez configurer le nom via le terminal série :

```cpp
// Code à ajouter temporairement dans setup() pour définir le nom
Preferences preferences;
preferences.begin("boatgps", false);
preferences.putString("boat_name", "BOAT1");
preferences.end();
```

## ⚙️ Configuration des préférences

### Paramètre : Boat Name

| Propriété | Valeur |
|-----------|--------|
| **Clé** | `boat_name` |
| **Namespace** | `boatgps` |
| **Type** | String |
| **Longueur max** | 17 caractères |
| **Par défaut** | "" (vide = utilise MAC) |

### Comportement

**Avec nom personnalisé :**
```
name = "BOAT1"
```

**Sans nom personnalisé :**
```
name = "AA:BB:CC:DD:EE:FF"
```

### Exemples de noms

✅ **Valides :**
- `BOAT1`, `BOAT2`, `BOAT3`
- `Starboard`, `Port`
- `Skipper`, `Crew`
- `Red`, `Blue`, `Green`

❌ **Invalides :**
- Plus de 17 caractères
- Caractères spéciaux non-ASCII

## 🧪 Test et validation

### 1. Vérifier le démarrage

**LED RGB de l'AtomS3 :**
- 🔵 **Bleu** : Initialisation en cours
- 🟢 **Vert** : GPS lock OK, transmission active
- 🟠 **Orange** : GPS en recherche de satellites
- 🔴 **Rouge clignotant** : Erreur critique (GPS ou ESP-NOW)

### 2. Serial Monitor

Connecter le Serial Monitor (115200 bauds) et vérifier :

```
OpenSailingRC - BoatGPS Tracker v1.0
==========================================

1. Initializing GPS...
✓ GPS initialized successfully
  Module: AT6668
  Baud rate: 115200

2. Initializing ESP-NOW...
✓ ESP-NOW initialized successfully
  Custom boat name: BOAT1           <- Vérifier le nom
  Boat ID (MAC): AA:BB:CC:DD:EE:FF
  Channel: 1

3. Waiting for GPS fix...
  Satellites: 0 → ... → 8
✓ GPS fix acquired!

Broadcasting GPS data (1 Hz)...
[001] LAT:43.123456 LON:2.654321 SPD:0.0kt HDG:0° SAT:8
```

### 3. Vérifier la réception sur le Display

- Allumer le Display OpenSailingRC v1.0.4
- Le nom du bateau devrait apparaître sur l'écran principal
- Vérifier que les données GPS sont affichées

### 4. Vérifier les logs SD (Display)

Si l'enregistrement est actif sur le Display, vérifier le fichier JSON :

```json
{
  "timestamp": 1234567890,
  "type": 1,
  "name": "BOAT1",                    <- Le nom personnalisé apparaît ici
  "sequenceNumber": 42,
  "gpsTimestamp": 1234567890,
  "latitude": 43.123456,
  "longitude": 2.654321,
  "speed": 4.5,
  "heading": 285.0,
  "satellites": 8
}
```

## 🔍 Dépannage

### Le nom personnalisé n'apparaît pas

**Cause :** Préférences non sauvegardées

**Solution :**
1. Reflasher avec M5Burner en configurant le nom
2. Vérifier le Serial Monitor : doit afficher "Custom boat name: XXX"
3. Si affiche "No custom boat name - using MAC address", la préférence n'est pas stockée

### LED rouge clignotante

**Cause :** GPS ou ESP-NOW non initialisé

**Solution :**
1. Vérifier le câblage GPS (TX → GPIO5, RX → GPIO6)
2. Vérifier le module GPS (AT6668) est bien alimenté
3. Consulter le Serial Monitor pour voir l'erreur exacte

### Pas de réception sur le Display

**Cause :** Incompatibilité de version ou distance trop grande

**Solution :**
1. Mettre à jour le Display en v1.0.4
2. Réduire la distance entre GPS et Display (<50m pour les tests)
3. Vérifier que le Display est en mode réception ESP-NOW

### Checksum incorrect

**Cause :** Fichier .bin corrompu lors du téléchargement

**Solution :**
1. Vérifier le SHA256 :
   ```bash
   shasum -a 256 OpenSailingRC_BoatGPS_v1.0.4_MERGED.bin
   ```
2. Comparer avec `SHA256SUMS.txt`
3. Re-télécharger si différent

## 📊 Spécifications techniques

### Matériel supporté
- M5Stack AtomS3 Lite
- GPS Atom v2 (AT6668)
- Protocoles GPS : GPS, BeiDou, GLONASS

### Communication
- **Protocole :** ESP-NOW broadcast
- **Fréquence :** 1 Hz (1 paquet/seconde)
- **Portée :** 100-200m en ligne de vue
- **Taille paquet :** 52 bytes (optimisé)

### Consommation
- **Mode actif :** ~100 mA
- **Autonomie :** 2-3h sur batterie 390mAh (Display)

### Performances
- **Temps fix GPS :** 30-120 secondes (cold start)
- **Précision GPS :** 2.5m CEP (avec bon signal)
- **Latence :** <100ms (GPS → Transmission)

## 📚 Documentation complète

- [RELEASE_NOTES_V1.0.4.md](RELEASE_NOTES_V1.0.4.md) - Notes de version
- [M5BURNER_PREFERENCES.md](../M5BURNER_PREFERENCES.md) - Guide préférences
- [Communication.h](../../include/Communication.h) - API ESP-NOW

## 🆘 Support

- **Issues GitHub :** [github.com/FRA-222/Boat-GPS-Display/issues](https://github.com/FRA-222/Boat-GPS-Display/issues)
- **Documentation :** [github.com/FRA-222/Boat-GPS-Display/wiki](https://github.com/FRA-222/Boat-GPS-Display/wiki)

---

**Version du guide :** 1.0.4  
**Dernière mise à jour :** 25 novembre 2025
