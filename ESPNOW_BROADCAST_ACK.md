# ESP-NOW Broadcast et Acquittement (ACK)

## Problème : Pas d'ACK en mode Broadcast

En mode broadcast ESP-NOW (adresse `FF:FF:FF:FF:FF:FF`), **il n'existe pas de mécanisme d'acquittement (ACK) de la part des récepteurs**.

### Ce que fait le callback `onDataSent()` :
```cpp
void onDataSent(const uint8_t* mac, esp_now_send_status_t status)
```

Ce callback indique uniquement si :
- ✅ Le paquet a été **transmis avec succès par la couche radio** (ESP_NOW_SEND_SUCCESS)
- ❌ La transmission a échoué au niveau de l'émetteur (ESP_NOW_SEND_FAIL)

### Ce que le callback NE fait PAS :
- ❌ Il n'indique **PAS** si un récepteur a effectivement reçu le message
- ❌ Il n'indique **PAS** combien de récepteurs ont reçu le message
- ❌ Il n'indique **PAS** quels récepteurs ont reçu le message

## Solutions Possibles

### Option 1 : Retry Automatique (Implémenté) ✅

**Principe :** Renvoyer le paquet plusieurs fois pour augmenter les chances de réception.

**Avantages :**
- Simple à implémenter
- Pas de modification du récepteur nécessaire
- Garde le mode broadcast (pas besoin de connaître les MAC des bouées)
- Augmente la probabilité de réception

**Inconvénients :**
- Pas de confirmation réelle de réception
- Augmente le trafic radio
- Ne résout pas les problèmes de portée/interférences

**Code :**
```cpp
// Envoie avec 2 retries automatiques
bool success = comm.broadcastGPSData(data, 2);
```

### Option 2 : Mode Unicast avec ACK Natif

**Principe :** Envoyer à chaque bouée individuellement (adresse MAC unique) au lieu du broadcast.

**Avantages :**
- ESP-NOW fournit un vrai ACK par récepteur
- On sait précisément qui a reçu le message
- Plus fiable

**Inconvénients :**
- Nécessite de maintenir une liste des MAC des bouées
- Il faut envoyer N paquets au lieu de 1 (N = nombre de bouées)
- Plus de code à gérer
- Complexité de découverte/enregistrement des bouées

**Exemple de code (à implémenter) :**
```cpp
// Liste des MAC des bouées connues
std::vector<uint8_t[6]> buoyMACs;

// Envoyer à chaque bouée individuellement
for (auto& mac : buoyMACs) {
    esp_err_t result = esp_now_send(mac, (uint8_t*)&packet, sizeof(packet));
    // Ici on a un vrai ACK si result == ESP_OK
}
```

### Option 3 : ACK Applicatif (Protocole Custom)

**Principe :** Les bouées envoient un petit paquet de confirmation quand elles reçoivent les données GPS.

**Avantages :**
- Garde le mode broadcast
- Confirmation réelle de réception
- Flexible (on peut ajouter des infos dans l'ACK)

**Inconvénients :**
- Nécessite de modifier le code de la bouée
- Double le trafic radio (données + ACK)
- Risque de collision si plusieurs bouées répondent en même temps

**Exemple de protocole :**
```cpp
// Émetteur (BoatGPS) :
struct GPSBroadcastPacket {
    uint32_t messageId;  // ID unique du message
    // ... autres données GPS
};

// Récepteur (Bouée) envoie un ACK :
struct GPSAckPacket {
    uint8_t messageType = 99;  // Type = ACK
    uint32_t messageId;        // ID du message reçu
    uint8_t buoyId;           // Quelle bouée répond
};

// L'émetteur attend les ACK pendant 100ms
// Si timeout, renvoie le message
```

## Recommandation

Pour votre cas d'usage (GPS de bateau → bouées) :

1. **Court terme :** Utiliser l'**Option 1** (retry automatique) ✅ **Implémentée**
   - Simple et efficace
   - Augmente significativement les chances de réception
   - Pas de modification des bouées

2. **Moyen terme :** Si vous constatez des pertes de paquets :
   - Implémenter l'**Option 3** (ACK applicatif)
   - Permet un vrai feedback
   - Plus robuste pour les courses

3. **Long terme :** Pour un système de production :
   - Combiner **Option 2 + Option 3**
   - Enregistrement automatique des bouées
   - Envoi unicast avec retry intelligent
   - ACK applicatif pour garantie de livraison

## État Actuel

✅ **Option 1 implémentée** : La méthode `broadcastGPSData()` supporte maintenant les retries automatiques (2 par défaut).

```cpp
// Usage dans main.cpp
bool success = comm.broadcastGPSData(data);  // 2 retries par défaut
// ou
bool success = comm.broadcastGPSData(data, 3);  // 3 retries
```

## Tests Recommandés

1. **Test de portée :** Vérifier la distance max de réception fiable
2. **Test d'interférences :** Tester avec plusieurs bateaux/bouées actifs
3. **Test de perte :** Simuler des obstacles (métal, eau) entre émetteur et récepteur
4. **Mesure de latence :** Vérifier le délai entre émission et réception

## Logs de Diagnostic

Le code affiche maintenant :
- `→ Broadcast: ...` : Succès du premier envoi
- `→ Broadcast: ... [retry N]` : Succès après N tentatives
- `✗ Broadcast attempt N failed` : Échec d'une tentative
- `✗ Broadcast failed after N attempts` : Échec total
