# Numéro de Séquence GPS - Détection de Perte de Paquets

## Principe

Chaque paquet GPS broadcast contient maintenant un champ `sequenceNumber` qui s'incrémente automatiquement à chaque envoi.

## Utilisation

### Côté Émetteur (BoatGPS)

Le compteur s'incrémente automatiquement à chaque appel de `broadcastGPSData()` :

```cpp
// Dans main.cpp
bool success = comm.broadcastGPSData(data);
uint32_t seqNum = comm.getSequenceNumber();  // Obtenir le numéro actuel
```

### Structure du Paquet

```cpp
struct GPSBroadcastPacket {
    int8_t messageType;       // 1 = Boat GPS data
    char name[18];            // MAC address as string
    int boatId;               // Numeric boat ID
    uint32_t sequenceNumber;  // ⭐ Nouveau champ
    uint32_t gpsTimestamp;    // GPS timestamp
    float latitude;           // Latitude
    float longitude;          // Longitude
    float speed;              // Speed in knots
    float heading;            // Heading in degrees
    uint8_t satellites;       // Number of satellites
};
```

## Détection des Pertes

### Sur le BoatGPS (Local)

Le numéro de séquence est enregistré dans :
- **Serial Monitor** : `[SEQ #123] GPS: ...`
- **Fichier SD** : Champ JSON `"sequenceNumber": 123`

Exemple de fichier JSON :
```json
{"timestamp":12345,"type":1,"boat":{"messageType":1,"sequenceNumber":123,"gpsTimestamp":12345,"latitude":48.123456,"longitude":-4.123456,"speed":5.2,"heading":45,"satellites":12}}
```

### Sur le Display (Réception)

Pour détecter les paquets perdus, le Display doit :

1. **Enregistrer le dernier `sequenceNumber` reçu** pour chaque bateau (basé sur MAC)
2. **Comparer** avec le nouveau `sequenceNumber` reçu
3. **Calculer les pertes** : 
   ```cpp
   uint32_t expectedSeq = lastSeq + 1;
   if (receivedSeq > expectedSeq) {
       uint32_t lostPackets = receivedSeq - expectedSeq;
       Serial.printf("⚠️  Lost %lu packet(s) from boat %s\n", lostPackets, mac);
   }
   ```

### Exemple de Code pour le Display

```cpp
// Structure pour tracker les bateaux
struct BoatTracker {
    uint8_t mac[6];
    uint32_t lastSequence;
    uint32_t receivedCount;
    uint32_t lostCount;
};

std::map<String, BoatTracker> boats;

void onBoatGPSReceived(const GPSBroadcastPacket& packet, const uint8_t* mac) {
    String macStr = formatMAC(mac);
    
    // Premier paquet de ce bateau
    if (boats.find(macStr) == boats.end()) {
        boats[macStr].lastSequence = packet.sequenceNumber;
        boats[macStr].receivedCount = 1;
        boats[macStr].lostCount = 0;
        memcpy(boats[macStr].mac, mac, 6);
        Serial.printf("✓ New boat registered: %s (seq #%lu)\n", 
                     macStr.c_str(), packet.sequenceNumber);
        return;
    }
    
    // Bateau connu - vérifier la séquence
    BoatTracker& tracker = boats[macStr];
    uint32_t expectedSeq = tracker.lastSequence + 1;
    
    if (packet.sequenceNumber == expectedSeq) {
        // Séquence correcte
        tracker.receivedCount++;
    } else if (packet.sequenceNumber > expectedSeq) {
        // Perte détectée
        uint32_t lost = packet.sequenceNumber - expectedSeq;
        tracker.lostCount += lost;
        tracker.receivedCount++;
        Serial.printf("⚠️  Boat %s: Lost %lu packet(s) (expected #%lu, got #%lu)\n",
                     macStr.c_str(), lost, expectedSeq, packet.sequenceNumber);
    } else {
        // Paquet en retard ou dupliqué
        Serial.printf("⚠️  Boat %s: Out-of-order packet (expected #%lu, got #%lu)\n",
                     macStr.c_str(), expectedSeq, packet.sequenceNumber);
    }
    
    tracker.lastSequence = packet.sequenceNumber;
}

void printBoatStatistics() {
    Serial.println("\n=== Boat Statistics ===");
    for (auto& [macStr, tracker] : boats) {
        float lossRate = 100.0f * tracker.lostCount / 
                        (tracker.receivedCount + tracker.lostCount);
        Serial.printf("Boat %s:\n", macStr.c_str());
        Serial.printf("  Received: %lu packets\n", tracker.receivedCount);
        Serial.printf("  Lost: %lu packets (%.1f%%)\n", 
                     tracker.lostCount, lossRate);
        Serial.printf("  Last seq: #%lu\n", tracker.lastSequence);
    }
    Serial.println("======================\n");
}
```

## Cas Particuliers

### Débordement du Compteur

Le compteur est un `uint32_t` (4 bytes) :
- Capacité : 0 à 4,294,967,295
- À 1 Hz : ~136 ans avant débordement
- À 10 Hz : ~13.6 ans avant débordement

Le débordement n'est donc pas un problème pratique. Si nécessaire, gérer avec :

```cpp
// Détection de débordement (wrap-around)
int32_t seqDiff = (int32_t)(receivedSeq - expectedSeq);
if (seqDiff > 0 && seqDiff < 1000) {
    // Perte normale
    lostCount += seqDiff;
} else if (seqDiff < -1000000) {
    // Probable débordement
    Serial.println("⚠️  Sequence counter wrapped around");
    lastSequence = receivedSeq;
}
```

### Redémarrage du BoatGPS

Quand le BoatGPS redémarre :
- Le compteur repart à 1
- Le Display doit détecter le "reset" : `receivedSeq < expectedSeq` avec grande différence
- Solution : Réinitialiser le tracker pour ce bateau

```cpp
if (packet.sequenceNumber < 100 && tracker.lastSequence > 1000000) {
    Serial.printf("✓ Boat %s rebooted (seq reset to #%lu)\n",
                 macStr.c_str(), packet.sequenceNumber);
    tracker.lastSequence = packet.sequenceNumber;
    tracker.receivedCount = 1;
    // NE PAS réinitialiser lostCount pour garder l'historique
}
```

## Logs Exemple

### BoatGPS (Émetteur)
```
→ Broadcast #1: 48.123456,-4.123456 (5.2kts, 45°, 12 sats)
[SEQ #1] [12345] GPS: 48.123456,-4.123456 | 5.2kts 45° | 12 sats | MAC: AA:BB:CC:DD:EE:FF

→ Broadcast #2: 48.123457,-4.123457 (5.3kts, 46°, 12 sats)
[SEQ #2] [13345] GPS: 48.123457,-4.123457 | 5.3kts 46° | 12 sats | MAC: AA:BB:CC:DD:EE:FF
```

### Display (Récepteur)
```
✓ Received from AA:BB:CC:DD:EE:FF [SEQ #1]
✓ Received from AA:BB:CC:DD:EE:FF [SEQ #2]
⚠️  Boat AA:BB:CC:DD:EE:FF: Lost 2 packet(s) (expected #3, got #5)
✓ Received from AA:BB:CC:DD:EE:FF [SEQ #5]
```

## Analyse des Performances

Pour analyser les pertes sur une session :

```python
# Script Python pour analyser les logs JSON
import json

def analyze_gps_log(filename):
    sequences = []
    
    with open(filename, 'r') as f:
        for line in f:
            data = json.loads(line)
            sequences.append(data['boat']['sequenceNumber'])
    
    sequences.sort()
    expected = list(range(sequences[0], sequences[-1] + 1))
    lost = set(expected) - set(sequences)
    
    print(f"Total envoyés: {sequences[-1]}")
    print(f"Total reçus: {len(sequences)}")
    print(f"Total perdus: {len(lost)}")
    print(f"Taux de perte: {100 * len(lost) / sequences[-1]:.2f}%")
    
    if lost:
        print(f"Paquets perdus: {sorted(lost)}")
```

## Avantages

✅ **Détection immédiate** des pertes de paquets  
✅ **Quantification précise** du nombre de paquets perdus  
✅ **Statistiques** par bateau sur le Display  
✅ **Debug** facilité des problèmes de communication  
✅ **Validation** que le stockage SD est complet  

## Modifications Nécessaires sur le Display

1. **Ajouter le champ dans la structure** (déjà fait si aligné sur `GPSBroadcastPacket`)
2. **Implémenter le tracker** de séquences par bateau
3. **Afficher les statistiques** de perte
4. **Logger** dans les fichiers JSON de replay

## Prochaines Étapes

- [ ] Modifier le code du Display pour tracker les séquences
- [ ] Ajouter un écran de statistiques sur le Display
- [ ] Tester en conditions réelles (portée, interférences)
- [ ] Analyser les logs pour optimiser la fréquence d'envoi
