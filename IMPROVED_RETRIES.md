# Amélioration du Nombre de Retries ESP-NOW

## Modification Effectuée

**Date :** 22 novembre 2025

**Fichier :** `src/main.cpp` ligne 238

**Changement :**
```cpp
// AVANT
bool success = comm.broadcastGPSData(data);  // 2 retries par défaut

// APRÈS
bool success = comm.broadcastGPSData(data, 4);  // 4 retries = 5 tentatives totales
```

## Objectif

Améliorer la **fiabilité de réception** des paquets GPS par le Display en augmentant le nombre de tentatives d'envoi.

## Détails Techniques

### Configuration Actuelle

| Paramètre | Avant | Après |
|-----------|-------|-------|
| Retries | 2 | 4 |
| Tentatives totales | 3 | 5 |
| Délai entre retries | 10 ms | 10 ms |
| Temps maximum | 20 ms | 40 ms |

### Calcul du Temps

**Avant :**
- 1ère tentative : 0 ms
- 2ème tentative : +10 ms
- 3ème tentative : +10 ms
- **Total : 20 ms maximum**

**Après :**
- 1ère tentative : 0 ms
- 2ème tentative : +10 ms
- 3ème tentative : +10 ms
- 4ème tentative : +10 ms
- 5ème tentative : +10 ms
- **Total : 40 ms maximum**

### Impact sur le Système

✅ **Compatible avec l'intervalle de broadcast**
- Intervalle : 1000 ms (1 Hz)
- Temps de retry : 40 ms max
- Marge restante : 960 ms

✅ **Pas d'impact sur les performances**
- Charge CPU négligeable
- Consommation électrique légèrement accrue (insignifiant)

## Amélioration Attendue

### Probabilité de Réception

Hypothèse : Probabilité de succès par tentative = 70% (exemple)

**Avant (3 tentatives) :**
```
P(échec) = (1 - 0.70)³ = 0.30³ = 0.027 = 2.7%
P(succès) = 1 - 0.027 = 97.3%
```

**Après (5 tentatives) :**
```
P(échec) = (1 - 0.70)⁵ = 0.30⁵ = 0.00243 = 0.24%
P(succès) = 1 - 0.00243 = 99.76%
```

**Amélioration : +2.5%** de réussite

### Scénarios Réels

| Qualité Signal | P(succès/tentative) | Taux réussite (3 tentatives) | Taux réussite (5 tentatives) | Amélioration |
|----------------|---------------------|------------------------------|------------------------------|--------------|
| Excellent | 95% | 99.9% | 99.99% | +0.09% |
| Bon | 80% | 99.2% | 99.97% | +0.77% |
| Moyen | 70% | 97.3% | 99.76% | +2.46% |
| Faible | 50% | 87.5% | 96.88% | +9.38% |
| Très faible | 30% | 65.7% | 83.2% | +17.5% |

**Conclusion :** Plus le signal est faible, plus l'amélioration est significative.

## Tests de Validation

### Avant de Tester
1. ✅ Compiler le BoatGPS : `pio run --environment m5stack-atom`
2. 🔄 Flasher sur le M5Stack Atom
3. 🔄 Vérifier que le Display a le filtre de doublons actif

### Test de Comparaison

**Protocole :**
1. Enregistrer 5-10 minutes avec l'ancienne version
2. Analyser : `python3 analyze_packets.py boat1.json display1.json`
3. Noter le taux de perte
4. Flasher la nouvelle version
5. Enregistrer 5-10 minutes dans les mêmes conditions
6. Analyser : `python3 analyze_packets.py boat2.json display2.json`
7. Comparer les taux de perte

**Résultats Attendus :**
- ✅ Taux de perte réduit de 20-50% selon les conditions
- ✅ Particulièrement efficace à moyenne/longue distance

### Logs à Observer

**BoatGPS (pas de changement visible) :**
```
[SEQ #123] Lat: 48.1234, Lon: -4.5678, Speed: 5.2 knots
```

**Display :**
- Moins de messages `⚠️ Perte détectée!`
- Plus de séquences continues
- Statistiques améliorées : `📊 Stats: Reçus=95, Perdus=5 (5%)`

## Prochaines Améliorations Possibles

Si le taux de perte reste élevé après cette modification :

### 1. Augmenter encore les Retries
```cpp
comm.broadcastGPSData(data, 6);  // 7 tentatives (60 ms max)
comm.broadcastGPSData(data, 9);  // 10 tentatives (90 ms max)
```

**Limite :** ~90 ms reste acceptable avec un intervalle de 1000 ms

### 2. Optimiser le Canal WiFi
Choisir un canal moins encombré (voir `IMPROVING_RELIABILITY.md`)

### 3. Augmenter la Puissance d'Émission
Vérifier que la puissance est au maximum (21 dBm)

### 4. Implémenter un Mode Unicast
Envoyer directement à l'adresse MAC du Display (ACK disponible)

### 5. Ajouter un Monitoring RSSI
Mesurer la force du signal pour diagnostiquer

## Compilation

✅ **Build réussi**
- Environment : m5stack-atom
- RAM : 14.5% (47428 bytes)
- Flash : 75.0% (982577 bytes)
- Durée : 6.72 secondes

## Références

- Voir `IMPROVING_RELIABILITY.md` pour plus de détails
- Utiliser `analyze_packets.py` pour analyser les résultats
