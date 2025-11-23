# Troubleshooting GPS AT6668 - Pas de Fix après 10 minutes

## Symptômes
- ✅ Caractères GPS reçus sur UART (GPIO5/6)
- ✅ Phrases NMEA détectées
- ❌ Pas de fix GPS après 10+ minutes

## Causes Possibles

### 1. Antenne GPS
- **Problème** : Antenne mal connectée ou positionnée
- **Vérification** : 
  - L'antenne est-elle bien connectée au module GPS Atom ?
  - L'antenne a-t-elle une vue dégagée du ciel ?
  - Êtes-vous à l'intérieur ou à l'extérieur ?
- **Solution** : Placer l'appareil à l'extérieur avec vue dégagée du ciel

### 2. Phrases NMEA Vides
- **Problème** : Le GPS reçoit des données mais pas de satellites
- **Vérification dans les phrases NMEA** :
  ```
  $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
  ```
  - Position 6 (fix quality) : `0` = pas de fix, `1` = fix GPS
  - Position 7 (nombre de satellites) : doit être >= 4
  
- **Si les phrases sont** :
  ```
  $GPGGA,,,,,,0,00,,,M,,M,,*XX
  ```
  C'est que le GPS ne reçoit aucun satellite

### 3. AT6668 vs U-blox
- **Problème** : Commandes UBX envoyées à un module non-compatible
- **Status** : ✅ Déjà corrigé - commandes UBX désactivées

### 4. Baudrate Incorrect
- **Problème** : Mauvaise vitesse UART
- **Vérification** : 
  - AT6668 par défaut = 9600 bps
  - Code actuel = 9600 bps ✅
  - Si caractères reçus mais phrases échouées = mauvais baudrate

### 5. Cold Start Delay
- **Problème** : Premier démarrage du GPS
- **Durée normale** :
  - Cold start : 26-60 secondes
  - Warm start : 25 secondes  
  - Hot start : 1 seconde
- **Si > 10 minutes** : Ce n'est PAS un cold start normal

## Diagnostic à Effectuer

### Étape 1 : Vérifier les Phrases NMEA
Regardez dans le moniteur série les phrases qui commencent par `$GP` ou `$GN` :

```
$GPGGA - Position GPS
$GPRMC - Données minimales recommandées
$GPGSV - Satellites en vue
$GPGSA - DOP et satellites actifs
```

**Questions** :
1. Voyez-vous ces phrases complètes ou vides ?
2. Quel est le chiffre après les virgules dans `$GPGGA` ?
3. Combien de satellites sont listés dans `$GPGSV` ?

### Étape 2 : Vérifier les Statistiques
Dans les logs toutes les 5 secondes :
```
[GPS] Chars: XXXX, Sentences: XX, Failed: X, Sats: X, HDOP: X.X
```

**Valeurs attendues** :
- `Chars` : 300-600 par 5 secondes (normal)
- `Sentences` : 5-15 par 5 secondes (normal)
- `Failed` : 0 (idéal) ou très faible
- `Sats` : 0 = **PROBLÈME**, 1-3 = insuffisant, 4+ = OK pour fix
- `HDOP` : <2.0 = excellent, 2-5 = bon, >5 = mauvais

### Étape 3 : Test de Compatibilité
Le AT6668 utilise le protocole **CASIC**, pas UBX. Il supporte :
- GPS (USA)
- BDS/Beidou (Chine) 
- GLONASS (Russie)
- GALILEO (Europe) - si version récente

**Test** : Le module démarre-t-il avec sa config par défaut ?

## Solutions selon le Diagnostic

### Si `Sats: 0` et phrases NMEA vides
➜ **Problème d'antenne ou environnement**
- Déplacer à l'extérieur
- Vérifier connexion antenne
- Attendre 2-3 minutes supplémentaires

### Si `Failed: XX` (élevé) et `Sentences: 0`
➜ **Problème de baudrate ou corruption UART**
- Vérifier les câbles GPIO5/6
- Tester avec 115200 bps

### Si `Sats: 1-3` constant
➜ **Manque de satellites visibles**
- Améliorer la vue du ciel
- Vérifier que l'antenne est horizontale (patch antenna)

### Si phrases NMEA lisibles mais TinyGPS++ ne parse pas
➜ **Format NMEA non standard**
- Le AT6668 peut envoyer des phrases CASIC propriétaires
- Vérifier la compatibilité TinyGPS++

## Prochaines Étapes

1. **Activez le debug NMEA** (déjà fait dans la dernière version)
2. **Copiez 10-20 lignes** de phrases NMEA du moniteur série
3. **Notez les statistiques** `[GPS]` affichées
4. **Précisez l'environnement** : intérieur/extérieur, étage, vue du ciel

## Code de Debug Actuel

### GPS.cpp - Logs détaillés toutes les 5s
```cpp
Serial.printf("[GPS] Chars: %u, Sentences: %u, Failed: %u, Sats: %d, HDOP: %.1f\n",
              charCount, gps.passedChecksum(), gps.failedChecksum(),
              gps.satellites.value(), gps.hdop.hdop());
```

### GPS.cpp - Affichage NMEA brut
```cpp
Serial.print(c);  // Affiche chaque caractère reçu du GPS
```

## Références

- [AT6668 Datasheet](https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/950/Multimode_satellite_navigation_receiver_cn.pdf)
- [TinyGPS++ Library](https://github.com/mikalhart/TinyGPSPlus)
- [NMEA Protocol](https://www.gpsinformation.org/dale/nmea.htm)
