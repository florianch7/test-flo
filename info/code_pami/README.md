# 🤖 Code pour les PAMIs - by Jules & Flo & Antoine (2026)

## État du projet

### ✅ Fonctionnalités implémentées
- 🚨 Détection d'obstacles (IR et ultrason)
- ⚙️ Asservissement proportionnel en continu avec support de listes
- 🔧 Fonction diagnostique
- 📊 Logs structurées
- 🏎️ Déplacement simple sans asservissement (naïf)
- 🎯 Asservissement sequentiel (avancer/tourner tout droit)

### 📋 À faire
- 🔄 PID (actuellement proportionnel uniquement)
- 📍 Mesure de position absolue fonctionnelle
  - Vérifier la calibration de (x, y, θ) avec une règle
  - Définir l'origine et les axes du terrain
- 📈 Rampes d'accélération et de freinage
- 📋 Tableau de configuration pour positions de départ/arrivée

---

## 🚀 Démarrage rapide

### Configuration obligatoire dans `define.h`
- 📌 **Pins** : Configurer les pins de chaque composant avec l'électronique
- ⚙️ **Asservissement** : Régler les gains (K, K_angle)
- 📍 **Positions** : Modifier les valeurs de départ/arrivée pour chaque PAMI

> 💡 **Fichier à modifier** : Uniquement `main.cpp` pour écrire la stratégie. Les classes `Pami`, `Moteur`, etc. contiennent la logique.

---

## ⚙️ Concept clé : Code non bloquant séquentiel

Le code doit s'exécuter à haute fréquence (au moins de manière continue) et **ne jamais bloquer**. Voici pourquoi :

### ⏱️ Problème : Les fonctions bloquantes

```cpp
// ❌ MAUVAIS - Bloque tout pendant 2 secondes
delay(2000);
moteur.set_speed(255, 255);  // Cette ligne attend 2s !

while (condition) {
    action
}
// La suite du code attend la fin du while...ca peut être long
```

Pendant un `delay()`, **plus rien ne se passe** : pas de détection d'obstacles, pas d'asservissement, pas de logs.
- Si le match se termine (100s), votre code attend toujours → le servomoteur ne se lance pas

### ✅ Solution : Testez constamment dans `loop()`

```cpp
// ✅ BON - S'exécute en continu, ne bloque jamais
if (millis() - pami.m_time_match >= END_TIME) {
    pami.stop();  // Le match est fini
}

if ((millis() - pami.m_time_match) > START_TIME) {
    pami.asserv_list(mouvements, nb_mouvements);  // Avancer si le match a commencé
}
```

Chaque appel à `loop()` teste les conditions et met à jour l'état : pas d'attente, pas de blocage.

---

## 🧩 Architecture : Trois types de fonctions

### 1️⃣ Fonctions **naïves** (pas d'asservissement)
Vous spécifiez la distance/angle → la PAMI bouge sans correction.

**Exemples :**
```cpp
pami.avancer(150);      // Avancer 150 cm
pami.tourner(90);       // Tourner 90° sens trigonométrique
```

---

### 2️⃣ Fonctions **séquentielles** avec numéro d'appel
Asservissement actif uniquement pendant le mouvement et plus aucun asservissement lorsque l'objectif est atteint.
Nécessite de passer un numéro d'étape qui sera comparé à `etape_globale`.

**Utilisation :**
- Avancer tout droit ou tourner sur soi même
- Enchaîner des actions sans brancher

**Fonctionnement :**
```cpp
int etape_globale = 0;  // Variable de suivi d'étape globale

// Dans loop() :
avancer_asservi(0, 150, 1000);   // Étape 0 : avancer 150 mm
tourner_asservi(1, 90, 1000);    // Étape 1 : tourner 90°
avancer_asservi(2, 100, 1000);   // Étape 2 : avancer 100 mm
```

La fonction teste `if (etape_globale == mon_numero)` pour savoir si c'est son tour. Une fois finie, elle incrémente `etape_globale` pour passer à la suivante.

**Exemple complet :**
```cpp
void loop() {
    if ((millis() - pami.m_time_match) < END_TIME) {
        avancer_asservi(0, 150, 1000);  // Avancer 15 cm
        tourner_asservi(1, 90, 1000);  // Ne sera éxecuté que lorsque etape_globale == 1 i.e lorsque avancer de 150 mm sera fini
    }
}
```

---

### 3️⃣ Fonctions **continues** avec asservissement permanent
Vous donner une consigne (distance + angle) → la PAMI la maintient indéfiniment.

**Utilisation :**
- Trajectoires complexes curviligne
- Maintenir une position en attendant

**Fonctionnement :**
- `asservi(distance, angle)` : une consigne simple
- `asservi_list(mouvements, nb)` : liste de consignes

La PAMI passe automatiquement à la consigne suivante quand la précédente est atteinte.

**Exemple avec liste :**
```cpp
float mouvements[][2] = {
    {150, 0},    // Avancer 150 mm tout droit
    {100, 45},   // Avancer 100 mm à 45°
    {50, -90},   // Avancer 50 mm à -90°
};
int nb_mouvements = sizeof(mouvements) / sizeof(mouvements[0]);

void loop() {
    pami.asserv_list(mouvements, nb_mouvements);    // Chaque mouvement sera exécuté l'un après l'autre
}
```

---

## 📘 Architecture du code

Après ce long tunnel sur le fonctionnement des fonctions, voilà l'architecture globale ainsi que les fonctions utiles. Le code est décomposé en plusieurs fichiers **(une classe = un composant/une fonctionnalité)** :

| Classe | Rôle |
|--------|------|
| 🔢 **Encodeur** | Récupère les ticks des encodeurs |
| 🏎️ **Moteur** | Envoie les ordres aux moteurs (futur : avec rampe) |
| 🔴 **Irsensor** | Capteur ToF (VL53L5CX) - distance à l'obstacle le plus proche |
| 🔊 **Ultrason** | Capteur HC-SR04 - distance à l'obstacle le plus proche |
| 📐 **Serv** | Contrôle le servomoteur |
| 📺 **Screen** | Écran LCD |
| 🗺️ **Mesure_pos** | Position absolue du PAMI sur le terrain (À faire) |
| 🤖 **Pami** | Classe principale - combine les autres composants |
| 💻 **main** | Programme à adapter - définit la stratégie |

---

## 📚 Référence des fonctions

### 🤖 Classe `Pami` (les plus utiles)

#### Informations
```cpp
pami.print_log();                    // Affiche : temps, distance IR, étape globale
pami.print_encodeur();               // Affiche : position de chaque encodeur (ticks & cm)
pami.print_infos_interrupteur();     // Affiche : numéro PAMI, tirette, équipe
```

#### Mouvements naïfs (sans asservissement)
```cpp
pami.avancer(distance_cm);           // Avancer d'une distance (cm). Recule si < 0
pami.tourner(angle_deg);             // Tourner d'un angle (degrés). > 0 = trigo, < 0 = horaire
```

#### Mouvements séquentiels (asservi uniquement pendant le mouvement)
```cpp
avancer_asservi(num_etape, distance_mm, temps_ms);
tourner_asservi(num_etape, angle_deg, temps_ms);
```

#### Mouvements continus (asservi permanent)
```cpp
pami.asservi(distance_mm, angle_deg);              // Une seule consigne
pami.asservi_list(tableau_2d, nb_mouvements);     // Liste de consignes
```

#### Moteurs & servo
```cpp
pami.set_speed(vitesse_droite, vitesse_gauche);   // 0-255
pami.stop();                                        // ⚠️ Bloquant - utiliser hors du match
pami.blink_servo(theta1, theta2, temps_ms);        // Balancer le servo
```

#### Capteurs
```cpp
float distance = pami.get_IR_distance();           // Distance à l'obstacle (cm)
```

---

## 💡 Exemples d'utilisation

### Exemple 1️⃣ : Asservissement avec liste de mouvements (main.cpp actuel)

```cpp
// Définir la liste des mouvements : {distance en mm, angle en degrés}
float mouvements[][2] = {
    {150, 0},    // Étape 0 : avancer 150 mm tout droit
    {100, 90},   // Étape 1 : avancer 100 mm à 90°
};
int nb_mouvements = sizeof(mouvements) / sizeof(mouvements[0]);

void loop() {
    pami.update_position();

    // Arrêt d'urgence à 100s
    if (millis() - pami.m_time_match >= END_TIME) {
        pami.stop();
        Serial.println("Match terminé!");
        while (true) pami.blink_servo(0, 90);
    }

    // Lancer les mouvements après START_TIME
    if ((millis() - pami.m_time_match) > START_TIME &&
        (millis() - pami.m_time_match) < END_TIME) {
        pami.asserv_list(mouvements, nb_mouvements);  // Lance la stratégie
    }

    // Logs toutes les secondes
    if (millis() - pami.m_time_log >= PERIODE_LOG) {
        pami.print_log();
        pami.m_time_log = millis();
    }
}
```

**Explication :**
1. La PAMI attend START_TIME (ex: 3s)
2. Elle exécute `asserv_list()` qui suit automatiquement la liste
3. À 100s, elle s'arrête d'urgence et balance le servo

---

### Exemple 2️⃣ : Détection d'obstacle et arrêt d'urgence

```cpp
void loop() {
    pami.update_position();

    // ⚠️ Vérifier les obstacles en permanence
    float dist = pami.get_IR_distance();
    if (dist < DISTANCE_MIN && dist > 0.5) {
        Serial.println("⚠️ Obstacle détecté ! Arrêt.");
        pami.stop();
        return;  // Quitter loop() jusqu'au prochain cycle
    }

    // ... reste de la stratégie ...
}
```

---

### Exemple 3️⃣ : Mouvements naïfs (sans asservissement)

```cpp
void loop() {
    // Avancer 50 cm sans correction
    pami.avancer(50);
    delay(500);

    // Tourner 90° sans correction
    pami.tourner(90);
    delay(500);
}
```

⚠️ **Attention :** Cet exemple utilise `delay()` qui bloque ! À utiliser uniquement pour du test.

---

### Exemple 4️⃣ : Mouvements séquentiels avec étapes

```cpp
int etape_globale = 0;

void loop() {
    pami.update_position();

    if (millis() - pami.m_time_match < END_TIME) {
        avancer_asservi(0, 150);
        tourner_asservi(1, 90);
        avancer_asservi(2, 100);
    }
}
```

**Fonctionnement :**
1. Loop exécute les 3 fonctions à chaque cycle
2. `avancer_asservi(0, ...)` vérifie `if (etape_globale == 0)` en interne
3. Si oui, elle exécute et incrémente `etape_globale` quand finie
4. Les fonctions suivantes attendent leur tour

---

## ⚡ Conseils pratiques

| ✅ À faire | ❌ À éviter |
|-----------|-----------|
| Mettre `delay()` uniquement dans `setup()` | `delay()` dans `loop()` |
| Utiliser des `if` pour les conditions | Utiliser des `while` bloquantes |
| Tester constamment dans `loop()` | Espérer que les événements se produisent |
| Paramétrer dans `define.h` | Hardcoder les valeurs dans le code |
| Lancer des fonctions continues (`asserv_list`) | Tout faire en séquentiel avec étapes |

---

*Voilà le code qu'on a réussi à pondre à la coupe en 2026. J'espère que c'est plus clair ! N'hésitez pas à demander aux auteurs (Jules, Flo, Antoine) si vous avez des doutes ! 🤖*