# 🤖 Code pour les PAMIs - by Jules & Flo & Antoine (2026)

## État du projet

### ✅ Fonctionnalités implémentées
- 🚨 Détection d'obstacles (IR et ultrason)
- ⚙️ Asservissement **PD** (proportionnel + dérivateur) en continu avec support de listes
- ⚙️ Asservissement **PD** pour mouvements séquentiels (avancer/tourner)
- 🔧 Fonction diagnostique (9 modes de test)
- 📊 Logs structurées
- 🏎️ Déplacement simple sans asservissement (naïf)
- 🎯 Arrêt linéaire bloquant et non bloquant (rampe de freinage)
- 📍 Mesure de position absolue (cinématique différentielle)

### 📋 À faire
- 📍 Vérifier la calibration de (x, y, θ) avec une règle
- 📋 Tableau de configuration pour positions de départ/arrivée

---

## 🚀 Pour commencer

### Configuration obligatoire dans `define.h`
- 📌 **Pins** : Configurer les pins de chaque composant avec l'électronique
- 📍 **Positions** : Modifier les valeurs de départ/arrivée pour chaque PAMI
- ⚙️ **Asservissement** : Régler les gains (KP_DISTANCE, KP_ANGLE, KD_POSITION, KD_ANGLE)

> 💡 **Fichier à modifier** : Uniquement `main.cpp` pour écrire la stratégie & `define.h` pour la config. Les classes `Pami`, `Moteur`, etc. contiennent la logique.

---

## ⚙️ Paramètres existants dans `define.h`

### 📌 Tous les pins, les GPIO
Modifier les pins en fonction de la carte elec

### Tous les paramètres globaux
- **Periodes** : Les temps de tout (début & fin du match, fréquence d'asservissement, du servomoteur, de l'infrarouge, des logs, etc)
- **Géométrie** : L'empatement du robot, la vitesse moteur par défaut (0-255 pour PWM)
- **Tolérances** : Toutes les tolérances [déplacement en distance (mm) et angle (°), détection d'obstacle]

### 🚀 Gains pour mouvements naïfs (bloquants)
- **K_NAIF** : Facteur de temps pour avancer (test 8)
- **K_ANGLE_NAIF** : Facteur de temps pour tourner (test 9)

### 🎮 Gains PD pour asservissement
- **KP_DISTANCE / KD_DISTANCE** : Correction proportionnelle/dérivative pour avancer tout droit
- **KP_ANGLE / KD_ANGLE** : Correction proportionnelle/dérivative pour tourner sur place

*Le KP fait converger l'erreur ; le KD évite les dépassements en freinant la correction.*

### 📏 Gains de conversion encodeur
- **GAIN_MM_TO_TICKS** : Nombre de ticks par mm parcouru → calibré avec test 6
- **GAIN_ANGLE_TO_TICKS** : Nombre de ticks par degré tournés → calibré avec test 7

### 📍 Positions de départ/arrivée
Modifier dans `define.h` les positions initiales et cibles pour chaque PAMI :
- Ca fait peur mais c'est simple.
- Un tableau avec 4 entrées - une pour chaque pami (on peut en rajouter si plus de pami) <br>
- Pour chaque pami une équipe (Jaune ou Bleue)
- Pour chaque équipe une position (x, y) de départ et une d'arrivée
donc on a {.j = {{x_depart_j, y_depart_j}, {x_arrivee_j, y_arrivee_j}}, .b = {{x_depart_b, y_depart_b}, {x_arrivee_b, y_arrivee_b}}}
- Pour y accèder (voir exemple) : .j ou .b pour l'équipe puis .start ou .final pour départ/arrivée puis .x ou .y pour la coordonnée
- On a également une fonction .delta() qui calcul la distance à faire en x ou en y (voir exemple)

```cpp
inline const Robot pami_pos[4] = {
    {.j = {{0, 0}, {200, 70}}, .b = {{0, 0}, {200, 70}}},
    {.j = {{0, 0}, {120, 90}}, .b = {{0, 0}, {120, 90}}},
    {.j = {{0, 0}, {120, 10}}, .b = {{0, 0}, {120, 10}}},
    {.j = {{0, 0}, {0, 0}}, .b = {{0, 0}, {0, 0}}}};

int pos_jaune_x_start = pami_pos[2].j.start.x
int pos_bleue_y_final = pami_pos[2].b.final.y
int delta_pos_bleue_x = pami_pos[2].b.delta().x
```

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

## ⚙️ Asservissement : Fonctionnement des corrections PD

L'asservissement utilise une correction **proportionnelle-dérivée (PD)** pour corriger les erreurs :

### 📊 Principe du PD

```
correction = KP * erreur + KD * vitesse_erreur
```

- **KP** (gain proportionnel) : Plus l'erreur est grande, plus on corrige fort
- **KD** (gain dérivateur) : Évite les dépassements en freinant si l'erreur diminue trop vite

### 🔄 Fonctions d'asservissement disponibles

- ATTENTION : Toutes ces fonctions ne sont pas forcément compatibles entre-elles

#### 1️⃣ **asserv_list()** - Mouvements complexes curvilignes
```cpp
float mouvements[][2] = {
    {150, 0},    // Avancer 150 mm tout droit
    {100, 45},   // Avancer 100 mm à 45°
    {50, -90},   // Avancer 50 mm à -90°
};
pami.asserv_list(mouvements, 3);  // Chaque mouvement = distance + angle
```

**Asservit simultanément :**
- Distance : contrôle la vitesse moyenne (roues + et -)
- Angle : contrôle la rotation (roues en opposition)

#### 2️⃣ **asserv()** - Une consigne simple
```cpp
pami.asserv(150, 0);  // Avancer 150 mm tout droit
```
Même fonctionnement que `asserv_list()` mais avec une seule consigne.

#### 3️⃣ **avancer_asservi()** - Avancer en ligne droite uniquement
```cpp
avancer_asservi(0, 150);  // Numéro d'étape, distance en mm
```

**Spécialité :**
- Asservissement **simplifié** : corrige uniquement l'écart entre les roues
- Pas d'asservissement d'angle (on suppose que les roues avancent droit)
- Recule si distance < 0

#### 4️⃣ **tourner_asservi()** - Tourner sur place
```cpp
tourner_asservi(1, 90);  // Numéro d'étape, angle en degrés
```

**Spécialité :**
- Les roues tournent en sens **opposé**
- Asservit l'écart : `erreur = ticks_gauche + ticks_droit`
- Positif = trigo, négatif = horaire

---

## 🧩 Architecture : Trois types de fonctions

### 1️⃣ Fonctions **naïves** (pas d'asservissement)
Vous spécifiez la distance/angle → la PAMI bouge sans correction pendant le temps estimé.

**Exemples :**
```cpp
pami.avancer(150);      // Avancer 150 cm - bloquant !
pami.tourner(90);       // Tourner 90° sens trigonométrique - bloquant !
```

⚠️ **Attention :** Ces fonctions sont **bloquantes** et utilisent `delay()`. À utiliser uniquement en dehors du match ou pour du test. Le temps de mouvement est calculé par :
- `temps = K_NAIF * (distance / SPEED) * 1000 ms`



### 2️⃣ Fonctions **séquentielles** avec numéro d'appel
Asservissement **PD actif uniquement** pendant le mouvement, arrêt automatique quand l'objectif est atteint.
Nécessite de passer un numéro d'étape qui sera comparé à `etape_globale`.

**Utilisation :**
- Avancer tout droit ou tourner sur place
- Enchaîner des actions sans branchement
- Idéal pour les **mouvements basiques** (ligne droite uniquement)

**Fonctionnement :**
```cpp
int etape_globale = 0;  // Variable de suivi d'étape globale (existe déjà dans main.cpp)

// Dans loop() :
avancer_asservi(0, 150);   // Étape 0 : avancer 150 mm
tourner_asservi(1, 90);    // Étape 1 : tourner 90°
avancer_asservi(2, 100);   // Étape 2 : avancer 100 mm
```

**Comment ça marche :**
1. La fonction teste `if (etape_globale == mon_numero)`
2. Si oui, elle **asservit en continu** jusqu'à atteindre la consigne
3. Une fois finie, elle incrémente `etape_globale` pour passer à la suivante
4. Les encodeurs sont **réinitialisés** à zéro après chaque étape

**Correction PD appliquée :**
- **Avancer** : corrige l'écart latéral entre les roues
- **Tourner** : corrige le centre de rotation (sum des ticks)

**Exemple complet :**
```cpp
void loop() {
    if ((millis() - pami.m_time_match) < END_TIME) {
        avancer_asservi(0, 150);  // Avancer 15 cm
        tourner_asservi(1, 90);   // Ne s'exécute que quand étape 0 est finie
    }
}
```



### 3️⃣ Fonctions **continues** avec asservissement PD permanent
Vous donnez une consigne (distance + angle) → la PAMI la maintient indéfiniment jusqu'à l'arrêt manuel.

**Utilisation :**
- Trajectoires **complexes curvilignes** (courbes, diagonales)
- Mouvements **composites** (avancer + tourner simultanément)
- Maintenir une position en attendant

**Asservissement PD parallèle :**
```
commande_moteur_d = KP_D * erreur_distance + KD_D * vitesse  +  KP_A * erreur_angle + KD_A * vitesse_angulaire
commande_moteur_g = KP_D * erreur_distance + KD_D * vitesse  -  KP_A * erreur_angle + KD_A * vitesse_angulaire
```

Chaque roue reçoit une commande = (correction distance) ± (correction angle)

**Exemple avec une seule consigne :**
```cpp
pami.asserv(150, 0);   // Avancer 150 mm tout droit
pami.asserv(100, 45);  // Avancer 100 mm à 45°
pami.asserv(50, -90);  // Avancer 50 mm vers la droite
```

**Exemple avec liste :**
```cpp
float mouvements[][2] = {
    {150, 0},    // Avancer 150 mm tout droit
    {100, 45},   // Avancer 100 mm à 45°
    {50, -90},   // Avancer 50 mm à -90°
};
int nb_mouvements = sizeof(mouvements) / sizeof(mouvements[0]);

void loop() {
    pami.asserv_list(mouvements, nb_mouvements);    // Chaque mouvement = distance + angle
}
```

La PAMI passe automatiquement à la consigne suivante quand la précédente est atteinte (à l'intérieur des tolérances `ERREUR_DISTANCE` et `ERREUR_ANGLE`).



## 🔧 Modes de test diagnostique

La fonction `pami.test(mode)` permet de tester chaque système individuellement. Appellez-la dans `setup()` pour déboguer :

```cpp
pami.test(1);   // Lance le mode 1 (boucle infinie)
```

| Mode | Fonction | Utilité |
|------|----------|---------|
| 1 | Interrupteurs & Tirette | Vérifier que les I/O GPIO fonctionnent |
| 2 | Capteur IR (ToF) | Vérifier les distances mesurées |
| 3 | Servomoteur | Vérifier le balayage 0-90° |
| 4 | Moteurs & Encodeurs | Tester chaque roue individuellement |
| 5 | Homologation | Avancer et s'arrêter sur obstacle |
| 6 | Calibrer GAIN_MM_TO_TICKS | Mesurez la distance réelle et ajustez le gain |
| 7 | Calibrer GAIN_ANGLE_TO_TICKS | Mesurez l'angle réel et ajustez le gain |
| 8 | Calibrer K_NAIF | Ajuster le temps de mouvement naïf (distance) |
| 9 | Calibrer K_ANGLE_NAIF | Ajuster le temps de rotation naïve |

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
| 🗺️ **Mesure_pos** | Position absolue du PAMI sur le terrain (cinématique différentielle) |
| 🤖 **Pami** | Classe principale - combine les autres composants |
| 💻 **main** | Programme à adapter - définit la stratégie |

---

## 📚 Référence des fonctions

### 🤖 Classe `Pami` (les plus utiles)

#### Informations
```cpp
pami.print_log();                    // Affiche : temps, distance IR, étape globale
pami.print_encodeur();               // Affiche : position de chaque encodeur (ticks & cm)
pami.print_changes_in_interrupteur();     // Affiche : numéro PAMI, tirette, équipe
```

#### Mouvements naïfs (sans asservissement)
```cpp
pami.avancer(distance_cm);           // Avancer d'une distance (cm). Recule si < 0
pami.tourner(angle_deg);             // Tourner d'un angle (degrés). > 0 = trigo, < 0 = horaire
```

#### Mouvements séquentiels (asservi uniquement pendant le mouvement)
```cpp
pami.avancer_asservi(num_etape, distance_mm);
pami.tourner_asservi(num_etape, angle_deg);
```

#### Mouvements continus (asservi permanent avec distance + angle)
```cpp
pami.asserv(distance_mm, angle_deg);              // Une seule consigne
pami.asserv_list(tableau_2d, nb_mouvements);     // Liste de consignes
```

#### Moteurs & servo
```cpp
pami.set_speed(vitesse_droite, vitesse_gauche);   // 0-255
pami.stop();                                        // ⚠️ Arrêt instantané
pami.linear_stop();                                 // Arrêt linéaire bloquant (rampe de freinage)
pami.non_blocking_linear_stop(init);               // Arrêt linéaire non bloquant
pami.blink_servo(theta1, theta2, temps_ms);        // Balancer le servo
```

#### Position absolue
```cpp
pami.set_initial_position();     // Initialise (x, y, θ) au démarrage
pami.update_mesure_position();          // Met à jour la position basée sur les encodeurs
// Récupérer la position :
float x = pami.pos_x;            // Position x en mm
float y = pami.pos_y;            // Position y en mm
float angle = pami.pos_angle;    // Angle en degrés
```

## 💡 Exemples de code générique

### Exemple 1️⃣ : Asservissement continu avec liste (mouvements complexes)

```cpp
// Définir la liste des mouvements : {distance en mm, angle en degrés}
float mouvements[][2] = {
    {150, 0},    // Avancer 150 mm tout droit
    {100, 90},   // Avancer 100 mm à 90° (courbe)
    {-50, 0},    // Reculer 50 mm
};
int nb_mouvements = sizeof(mouvements) / sizeof(mouvements[0]);

void loop() {
    pami.update_mesure_position();

    // Arrêt d'urgence à 100s
    if (millis() - pami.m_time_match >= END_TIME) {
        pami.linear_stop();
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

**Avantages :**
- Trajectoires **courbes** et **diagonales** possibles
- Distance et angle **asservis simultanément**
- Passage auto à la consigne suivante

---

### Exemple 2️⃣ : Mouvements séquentiels (étapes linéaires)

```cpp
int etape_globale = 0;

void loop() {
    pami.update_mesure_position();

    if ((millis() - pami.m_time_match) > START_TIME &&
        (millis() - pami.m_time_match) < END_TIME) {
        avancer_asservi(0, 150);   // Étape 0 : avancer 150 mm
        tourner_asservi(1, 90);    // Étape 1 : tourner 90° (s'exécute après étape 0)
        avancer_asservi(2, 100);   // Étape 2 : avancer 100 mm
    }
}
```

**Avantages :**
- Mouvements **simples et directs** (ligne droite ou rotation)
- Idéal pour les **stratégies basiques**
- Encodeurs réinitialisés après chaque étape

**Inconvénient :**
- Pas possible de faire des courbes (pas d'angle pendant avancer)

---

### Exemple 3️⃣ : Détection d'obstacle et arrêt d'urgence

```cpp
void loop() {
    pami.update_mesure_position();

    // ⚠️ Vérifie les obstacles en permanence
    float dist = pami.get_IR_distance();
    if (dist < DISTANCE_MIN && dist > 0.5) {
        Serial.println("⚠️ Obstacle détecté ! Arrêt.");
        pami.linear_stop();  // Rampe de freinage
        return;  // Quitter loop() jusqu'au prochain cycle
    }

    // ... reste de la stratégie ...
}
```

---

### Exemple 5️⃣ : Arrêt linéaire non bloquant

```cpp
void loop() {
    pami.update_mesure_position();

    static bool stop_initiated = false;

    // Initier l'arrêt après 90 secondes
    if (millis() - pami.m_time_match > 90000 && !stop_initiated) {
        pami.non_blocking_linear_stop(true);  // Démarre la rampe
        stop_initiated = true;
    }

    // L'arrêt se poursuit en arrière-plan (10 étapes de 25ms)
    pami.non_blocking_linear_stop();  // Continue le freinage

    // Reste du code peut s'exécuter normalement
}
```

**Avantages :**
- Ne bloque pas le `loop()`
- Rampe de freinage progressive (évite les à-coups)
- Utile pour arrêter le servo **pendant** le freinage



## 🎯 Comment choisir le type d'asservissement?

| Situation | À utiliser | Raison |
|-----------|-----------|---------|
| Mouvements **simples** : avancer tout droit, tourner 90° | `avancer_asservi()` / `tourner_asservi()` | Plus simple, encodeurs réinitialisés auto |
| Trajectoires **curvilignes** : courbes, diagonales | `asserv_list()` | Distance + angle asservis simultanément |
| Mouvements **rapides** à coder | `asserv_list()` | Une simple liste `{distance, angle}` |
| Besoin d'arrêt **sans bloquer** | `non_blocking_linear_stop()` | Permet à servo/logs de continuer |
| Arrêt **d'urgence** | `linear_stop()` | Rampe de freinage, peut bloquer |
| Test/debug | `avancer()` / `tourner()` naïf | Bloquant mais simple |

---

## ⚡ Conseils pratiques

| ✅ À faire | ❌ À éviter |
|-----------|-----------|
| Mettre `delay()` uniquement dans `setup()` | `delay()` dans `loop()` |
| Utiliser des `if` pour les conditions | Utiliser des `while` bloquantes |
| Tester constamment dans `loop()` | Espérer que les événements se produisent |
| Paramétrer gains dans `define.h` | Hardcoder les gains dans les fonctions |
| Appeler `pami.update_mesure_position()` chaque loop | Oublier la mise à jour position (elle dérive) |
| Calibrer gains avant chaque compétition | Utiliser les gains du match précédent |
| Ajuster KP/KD petit à petit (0.5 à la fois) | Augmenter les gains brutalement |

---

## 🔍 Débogage : Asservissement qui ne converge pas

**Si la PAMI :**
- **Dépasse la consigne** → Diminuer **KP**, augmenter **KD**
- **Oscille autour de la cible** → Augmenter **KD** (damping)
- **N'atteint jamais la cible** → Augmenter **KP**
- **Reste bloquée** → Vérifier les encodeurs avec test 4

**Si la roue gauche dépasse :**
→ Ajuster `KP_DISTANCE` pour corriger l'écart

**Si le robot tourne pendant qu'il avance :**
→ Vérifier les gains de conversion `GAIN_MM_TO_TICKS` et `GAIN_ANGLE_TO_TICKS`

**Si la position absolue dérive :**
→ Vérifier la cinématique dans `Mesure_pos.cpp`, calibrer les gains avec une règle

---

## 📌 Résumé des fonctions d'asservissement

```
┌─────────────────────────────────────────────────────┐
│      COMMENT FAIRE AVANCER LA PAMI ?                │
├─────────────────────────────────────────────────────┤
│                                                     │
│  1. Bloquant, sans asserv (TEST UNIQUEMENT) :       │
│     pami.avancer(150);                              │
│                                                     │
│  2. Asservi linéaire simple (ligne droite) :        │
│     avancer_asservi(etape, 150);                    │
│     tourner_asservi(etape, 90);                     │
│                                                     │
│  3. Asservi continu complexe (courbes) :            │
│     pami.asserv(150, 0);         // Une consigne    │
│     pami.asserv_list(mouvements, nb);  // Liste     │
│                                                     │
└─────────────────────────────────────────────────────┘
```

*Voilà le code qu'on a réussi à pondre à la coupe en 2026. J'espère que c'est plus clair ! N'hésitez pas à demander aux auteurs (Jules, Flo, Antoine) si vous avez des doutes ! 🤖*