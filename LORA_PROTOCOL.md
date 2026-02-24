# 📡 Protocole LoRa Request/Response pour Bouées GPS

## Vue d'ensemble

Pour éviter les **collisions radio** en LoRa, le système utilise un modèle **maître-esclave** :
- **Joystick = Maître** : Interroge séquentiellement chaque bouée
- **Bouées = Esclaves** : Répondent uniquement quand interrogées

Ce protocole garantit qu'une seule bouée transmet à la fois.

---

## Architecture de Communication

```
JOYSTICK (Maître)                    BOUÉE #1 (Esclave)
      |                                      |
      |------- REQUEST (ID=1) ------------->| 📥 Reçoit REQUEST
      |                                      | 🔍 Vérifie ID
      |                                      | 📤 Prépare état
      |<------ RESPONSE (État) --------------|
      | 📊 Traite données                    |
      | ✅ Bouée #1 connectée               |
      |                                      |
      |------- REQUEST (ID=2) ------------->| (ignore, pas pour elle)
                                             
BOUÉE #2 (Esclave)
      | 📥 Reçoit REQUEST
      | 🔍 Vérifie ID
      | 📤 Prépare état
      |<------ RESPONSE (État) --------------|
```

---

## Types de Messages

### 1. REQUEST (Joystick → Bouée)
Interrogation d'une bouée spécifique pour obtenir son état.

```cpp
enum class LoRaMessageType : uint8_t {
    REQUEST = 0x01,
    RESPONSE = 0x02,
    COMMAND = 0x03
};

struct RequestPacketLora {
    LoRaMessageType messageType;  // REQUEST (0x01)
    uint8_t targetBuoyId;         // ID de la bouée interrogée (0-5)
    uint32_t timestamp;           // Timestamp de la requête
};
```

**Taille** : 6 bytes

---

### 2. RESPONSE (Bouée → Joystick)
Réponse contenant l'état complet de la bouée.

```cpp
struct ResponsePacketLora {
    LoRaMessageType messageType;  // RESPONSE (0x02)
    BuoyStateLora state;          // État complet (voir ci-dessous)
};
```

**Taille** : 1 + sizeof(BuoyStateLora) ≈ 50-60 bytes

---

### 3. COMMAND (Joystick → Bouée)
Commande de pilotage (cap, vitesse, mode).

```cpp
struct CommandPacketLora {
    LoRaMessageType messageType;  // COMMAND (0x03)
    uint8_t targetBuoyId;         // ID de la bouée cible
    BuoyCommand command;          // Type de commande
    int16_t heading;              // Cap cible (degrés)
    int8_t throttle;              // Vitesse cible (-100 à +100%)
    uint32_t timestamp;           // Timestamp
};
```

**Taille** : ~12 bytes

---

## Structure BuoyStateLora

État complet de la bouée transmis dans RESPONSE :

```cpp
struct BuoyStateLora {
    uint8_t buoyId;                     // ID de la bouée (0-5)
    uint32_t timestamp;                 // Timestamp du message
    
    // État général
    tEtatsGeneral generalMode;          // Mode général (INIT, READY, NAV, etc.)
    tEtatsNav navigationMode;           // Mode de navigation
    
    // Statut capteurs
    bool gpsOk;                         // GPS fonctionnel
    bool headingOk;                     // Compas fonctionnel
    bool yawRateOk;                     // Gyroscope fonctionnel
    
    // Données environnementales
    float temperature;                  // Température (°C)
    
    // Batterie
    float remainingCapacity;            // Capacité restante (mAh)
    
    // Navigation
    float distanceToCons;               // Distance à la consigne (m)
    
    // Commandes autopilote
    int8_t autoPilotThrottleCmde;       // Throttle autopilote (-100 à +100%)
    float autoPilotTrueHeadingCmde;     // Cap autopilote (degrés)
    int8_t autoPilotRudderCmde;         // Gouvernail autopilote (-100 à +100%)
    
    // Commandes forcées
    int8_t forcedThrottleCmde;          // Throttle forcé
    bool forcedThrottleCmdeOk;          // Throttle forcé actif
    float forcedTrueHeadingCmde;        // Cap forcé
    bool forcedTrueHeadingCmdeOk;       // Cap forcé actif
    int8_t forcedRudderCmde;            // Gouvernail forcé
    bool forcedRudderCmdeOk;            // Gouvernail forcé actif
};
```

---

## Implémentation Côté Bouée

### 1. Configuration LoRa

Identique au joystick :

```cpp
#define LORA_CHANNEL 23             // Canal 23 = 920.6 MHz
#define LORA_ADDRESS_H 0x00         
#define LORA_ADDRESS_L 0x02         // Adresse bouée (0x02, 0x03, etc.)

loraConfig.own_address = (LORA_ADDRESS_H << 8) | LORA_ADDRESS_L;
loraConfig.baud_rate = BAUD_9600;
loraConfig.air_data_rate = BW125K_SF9;
loraConfig.transmitting_power = TX_POWER_13dBm;
loraConfig.own_channel = LORA_CHANNEL;
```

**⚠️ IMPORTANT** : Chaque bouée doit avoir une **adresse unique** (0x02, 0x03, 0x04, etc.)

---

### 2. Boucle Principale (loop)

```cpp
void loop() {
    // 1. Écouter les messages LoRa
    RecvFrame_t recvFrame;
    
    if (lora.RecieveFrame(&recvFrame) == 0) {
        // Message reçu
        processLoRaMessage(recvFrame.recv_data, recvFrame.recv_data_len);
    }
    
    // 2. Continuer tâches normales (GPS, pilotage, etc.)
    // ...
}
```

---

### 3. Traitement des Messages

```cpp
void processLoRaMessage(const uint8_t* data, size_t len) {
    if (len < 1) return;
    
    // Lire le type de message
    LoRaMessageType msgType = *(LoRaMessageType*)data;
    
    switch (msgType) {
        case LoRaMessageType::REQUEST:
            handleRequest(data, len);
            break;
            
        case LoRaMessageType::COMMAND:
            handleCommand(data, len);
            break;
            
        case LoRaMessageType::RESPONSE:
            // Ignore (les bouées n'écoutent pas les RESPONSE)
            break;
    }
}
```

---

### 4. Gestion des REQUEST

```cpp
void handleRequest(const uint8_t* data, size_t len) {
    if (len != sizeof(RequestPacketLora)) {
        Logger::log("✗ LoRa: Taille REQUEST invalide");
        return;
    }
    
    RequestPacketLora* request = (RequestPacketLora*)data;
    
    // Vérifier si la requête nous concerne
    if (request->targetBuoyId != MY_BUOY_ID) {
        // Pas pour nous, ignorer
        return;
    }
    
    Logger::logf("📥 LoRa: REQUEST reçu du joystick");
    
    // Préparer la réponse avec notre état actuel
    ResponsePacketLora response;
    response.messageType = LoRaMessageType::RESPONSE;
    
    // Remplir l'état de la bouée
    response.state.buoyId = MY_BUOY_ID;
    response.state.timestamp = millis();
    response.state.generalMode = myGeneralMode;
    response.state.navigationMode = myNavigationMode;
    response.state.gpsOk = myGpsOk;
    response.state.headingOk = myHeadingOk;
    response.state.yawRateOk = myYawRateOk;
    response.state.temperature = myTemperature;
    response.state.remainingCapacity = myBatteryCapacity;
    response.state.distanceToCons = myDistanceToWaypoint;
    response.state.autoPilotThrottleCmde = myAutoPilotThrottle;
    response.state.autoPilotTrueHeadingCmde = myAutoPilotHeading;
    response.state.autoPilotRudderCmde = myAutoPilotRudder;
    response.state.forcedThrottleCmde = myForcedThrottle;
    response.state.forcedThrottleCmdeOk = myForcedThrottleActive;
    response.state.forcedTrueHeadingCmde = myForcedHeading;
    response.state.forcedTrueHeadingCmdeOk = myForcedHeadingActive;
    response.state.forcedRudderCmde = myForcedRudder;
    response.state.forcedRudderCmdeOk = myForcedRudderActive;
    
    // Envoyer la réponse
    loraConfig.target_address = 0xFFFF;  // Broadcast
    loraConfig.target_channel = LORA_CHANNEL;
    
    int result = lora.SendFrame(loraConfig, (uint8_t*)&response, sizeof(response));
    
    if (result == 0) {
        Logger::log("✓ LoRa: RESPONSE envoyé");
    } else {
        Logger::log("✗ LoRa: Échec envoi RESPONSE");
    }
}
```

---

### 5. Gestion des COMMAND

```cpp
void handleCommand(const uint8_t* data, size_t len) {
    if (len != sizeof(CommandPacketLora)) {
        Logger::log("✗ LoRa: Taille COMMAND invalide");
        return;
    }
    
    CommandPacketLora* command = (CommandPacketLora*)data;
    
    // Vérifier si la commande nous concerne
    if (command->targetBuoyId != MY_BUOY_ID) {
        return;
    }
    
    Logger::logf("📥 LoRa: COMMAND reçu (type=%d)", command->command);
    
    // Traiter la commande selon le type
    switch (command->command) {
        case BuoyCommand::HEADING:
            setForcedHeading(command->heading);
            break;
            
        case BuoyCommand::THROTTLE:
            setForcedThrottle(command->throttle);
            break;
            
        case BuoyCommand::STOP:
            stopBuoy();
            break;
            
        // ... autres commandes
    }
}
```

---

## Timing et Performance

### Cycle de Polling (Joystick)

- **1 bouée** : Interrogation toutes les **1 seconde**
- **6 bouées** : Chaque bouée interrogée toutes les **6 secondes**
- **Timeout** : 500ms maximum par requête

```
Temps total cycle complet = nb_bouées × (pollInterval + timeout_moyen)
                          = 6 × (1000ms + 100ms) = 6.6 secondes
```

### Délais de Réponse

| Étape | Durée |
|-------|-------|
| Envoi REQUEST | ~50ms |
| Traitement bouée | ~20ms |
| Envoi RESPONSE | ~50ms |
| **Total** | **~120ms** |

---

## Avantages du Protocole

✅ **Pas de collisions** : Une seule bouée transmet à la fois  
✅ **Détection présence** : Timeout = bouée déconnectée  
✅ **Scalable** : Supporte facilement 10+ bouées  
✅ **Diagnostic** : RSSI/SNR par bouée  
✅ **Déterministe** : Ordre de polling prévisible  

---

## Limitations

⚠️ **Latence** : 6 bouées = 6 secondes pour un cycle complet  
⚠️ **Bande passante** : ~10 paquets/seconde max (vs 100+ en ESP-NOW)  
⚠️ **Pas de push** : Bouées ne peuvent pas signaler événements urgents spontanément  

---

## Comparaison ESP-NOW vs LoRa

| Caractéristique | ESP-NOW | LoRa Request/Response |
|-----------------|---------|------------------------|
| **Portée** | 50-200m | 1-5 km |
| **Latence** | 10-50ms | 100-500ms |
| **Collisions** | Possibles | Évitées |
| **Scalabilité** | 10-20 bouées | 50+ bouées |
| **Consommation** | Moyenne | Basse |
| **Complexité** | Simple | Modérée |

---

## Migration depuis ESP-NOW

Pour migrer une bouée d'ESP-NOW vers LoRa :

1. ✅ Remplacer `#include <esp_now.h>` par `#include <M5_LoRa_E220_JP.h>`
2. ✅ Ajouter `LoRa_E220_JP lora;` et `LoRaConfigItem_t loraConfig;`
3. ✅ Supprimer callbacks ESP-NOW (`onDataRecv`, `onDataSent`)
4. ✅ Implémenter `handleRequest()` et `handleCommand()`
5. ✅ Appeler `lora.RecieveFrame()` dans `loop()`

---

## Fichiers à Créer/Modifier

### Bouée (Autonomous-GPS-Buoy)

```
include/
  LoRaCommunication.h         ← CRÉER
src/
  LoRaCommunication.cpp       ← CRÉER
  main.cpp                    ← MODIFIER (ajouter polling LoRa)

platformio.ini                ← MODIFIER (ajouter lib M5_LoRa920)
```

---

## Configuration Matérielle

### Joystick
- **M5Stack AtomS3** + **Unit LoRaE220-920**
- **Adresse** : 0x0001
- **ID** : N/A (maître)

### Bouée #1
- **M5Stack Core2** + **Unit LoRaE220-920**
- **Adresse** : 0x0002
- **ID** : 1

### Bouée #2
- **M5Stack Core2** + **Unit LoRaE220-920**
- **Adresse** : 0x0003
- **ID** : 2

---

## Exemple de Log Côté Joystick

```
✓ LoRa: Prêt à recevoir
🔄 LoRa: Interrogation Bouée #1...
📡 LoRa: Reçu 58 bytes, RSSI=-45 dBm
📊 Bouée #1: Mode=NAV, GPS=OK, Batt=4523 mAh
✓ LoRa: Bouée #1 a répondu

🔄 LoRa: Interrogation Bouée #2...
⏱️  LoRa: Bouée #2 timeout (déconnectée)

🔄 LoRa: Interrogation Bouée #3...
📡 LoRa: Reçu 58 bytes, RSSI=-67 dBm
📊 Bouée #3: Mode=READY, GPS=OK, Batt=3821 mAh
✓ LoRa: Bouée #3 a répondu
```

---

## Exemple de Log Côté Bouée

```
✓ LoRa: Module E220-JP configuré
✓ LoRa: Prêt à recevoir (ID=1, Addr=0x0002)

📥 LoRa: REQUEST reçu du joystick
📊 État: GPS=OK, Cap=285°, Batt=4523mAh
✓ LoRa: RESPONSE envoyé

📥 LoRa: COMMAND reçu (type=HEADING)
🎯 Nouveau cap forcé: 315°
✓ Commande appliquée
```

---

## Support et Debug

Pour tester la communication :

```cpp
// Côté bouée - Mode debug
#define LORA_DEBUG_MODE 1

void loop() {
    if (Serial.available()) {
        char cmd = Serial.read();
        if (cmd == 't') {
            // Test manual response
            sendTestResponse();
        }
    }
}
```

---

## Auteur

Philippe Hubert - OpenSailingRC Contributors  
Date : Décembre 2025  
Version : 1.0
