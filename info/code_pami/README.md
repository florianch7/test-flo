# 🤖 Code pour les PAMIs - by Jules & Flo & Antoine (2026)

## 📑 Sommaire
- [État du projet](#état-du-projet)
- [🚀 Pour commencer](#-pour-commencer)
- [⚖️ Architecture & concepts clés](#-architecture-du-code)
    - [⏱️ Fonctions non bloquantes](#️-code-non-bloquant-séquentiel)
    - [🧩 Déplacements](#-déplacements--trois-types-de-fonctions)
    - [⚙️ Asservissement](#️-asservissement-pd)
- [📍 Mesure de position](#-mesure-de-position)
- [📚 Listes des fonctions utiles](#-référence-des-fonctions)
- [⚙️ Configuration `define.h`](#️-paramètres-existants-dans-defineh)

---

## État du projet

### ✅ Fonctionnalités implémentées
- 🚨 Détection d'obstacles (IR ou ultrason)
- ⚙️ Asservissement **PD** (proportionnel + dérivateur) en continu avec support de listes
- ⚙️ Asservissement **PD** pour mouvements séquentiels (avancer/tourner)
- 🔧 Fonction diagnostique (9 modes de test)
- 📊 Logs structurées
- 🏎️ Déplacement simple sans asservissement (naïf)
- 🎯 Arrêt linéaire bloquant et non bloquant (rampe de freinage)
- 📍 Mesure de position absolue (cinématique différentielle)

### 📋 À faire
- 📍 Vérifier la calibration de (x, y, θ) avec une règle

---

## 🚀 Pour commencer

### Règles et rôle des PAMI

- **Toujours relire les règles de votre annéee**

Les PAMIs sont des petits robots qui deviennent actifs **au bout de 85s** sur les 100s que durent un match. <br>
Leur unique rôle est de **rejoindre un zone précise de la carte** qui change chaque année. <br>
Ils doivent avoir un **système d'évitement d'obstacle** (on utilise de l'infrarouge) et **avoir un actionneur actif** à la fin du match.


### Configuration obligatoire dans `define.h`
- 📌 **Pins** : Configurer les pins de chaque composant avec l'électronique
- 📍 **Positions** : Modifier les valeurs de départ pour chaque PAMI
- ⚙️ **Asservissement** : Régler les gains (KP, KD, GAIN_TICKS_TO_MM, GAIN_TICKS_TO_ANGLE, etc)

> 💡 **Fichier à modifier** : Uniquement `main.cpp` pour écrire la stratégie & `define.h` pour la config. Les classes `Pami`, `Moteur`, etc. contiennent le code bas niveau.

---

## 📘 Architecture du code

Voilà l'architecture globale. Le code est décomposé en plusieurs fichiers **(une classe = un composant/une fonctionnalité)** :

| Classe | Rôle |
|--------|------|
| 🔢 **Encodeur** | Récupère les ticks des encodeurs |
| 🏎️ **Moteur** | Envoie les ordres aux moteurs (futur : avec rampe) |
| 🔴 **Irsensor** | Capteur ToF (VL53L5CX) - distance à l'obstacle le plus proche |
| 🔊 **Ultrason** | Capteur HC-SR04 - distance à l'obstacle le plus proche (pas utilisé) |
| 📐 **Serv** | Contrôle le servomoteur |
| 📺 **Screen** | Écran LCD |
| 🗺️ **Mesure_pos** | Position absolue du PAMI sur le terrain |
| 🤖 **Pami** | Classe principale - combine les autres composants |
| 💻 **main** | Programme à adapter - définit la stratégie |

---

- ### ⏱️ Code non bloquant

Le code doit s'exécuter à haute fréquence (au moins de manière continue) et **ne jamais bloquer**. Voici pourquoi :

#### Problème : Les fonctions bloquantes

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

#### ✅ Solution : Testez constamment dans `loop()`

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
C'est très subtile donc ne modifiez pas sans être 100% sur de vous !

---

- ### 🧩 Déplacements : Trois types de fonctions

#### 1️⃣ Fonctions **naïves** (pas d'asservissement)
Vous spécifiez la distance/angle → la PAMI bouge sans correction pendant le temps estimé.

**Exemples :**
```cpp
pami.avancer(150);      // Avancer 150 cm - bloquant !
pami.tourner(90);       // Tourner 90° sens trigonométrique - bloquant !
```

⚠️ **Attention :** Ces fonctions sont **bloquantes** et utilisent `delay()`. À utiliser uniquement pour du test. Le temps de mouvement est calculé par :
- `temps = K_NAIF * (distance / SPEED) * 1000 ms`


#### 2️⃣ Fonctions **séquentielles** avec numéro d'appel
Asservissement **P uniquement** et  **actif uniquement pendant le mouvement**, arrêt automatique quand l'objectif est atteint.
Nécessite de passer un numéro d'étape qui sera comparé à `etape_globale` (définit dans `main.cpp` & `define.h`).
C'est une variable `extern`.

**Correction P appliquée :**
- **Avancer** : corrige l'écart latéral entre les roues (différence des ticks)
- **Tourner** : corrige la position du centre de rotation (somme des ticks)

**Utilisation :**
- Avancer tout droit et tourner sur place
- Idéal pour les **mouvements basiques** (ligne droite uniquement puis tourner puis reculer)
- On ne combine pas les deux en même temps

**Fonctionnement :**
```cpp
int etape_globale = 0;  // Variable de suivi d'étape globale (existe déjà dans main.cpp)

// L'étape 2 ne s'éxecute que quand la 1 est terminée etc
avancer_asservi(0, 150);   // Étape 0 : avancer 150 mm
tourner_asservi(1, 90);    // Étape 1 : tourner 90°
avancer_asservi(2, 100);   // Étape 2 : avancer 100 mm
```

**Comment ça marche :**
1. La fonction teste `if (etape_globale == mon_numero)`
2. Si oui, elle **asservit en continu** jusqu'à atteindre la consigne
3. Une fois finie, elle incrémente `etape_globale` pour passer à la suivante
4. Les encodeurs sont **réinitialisés** à zéro après chaque étape


#### 3️⃣ Fonctions **continues** avec asservissement PD permanent
Vous donnez une consigne (distance + angle) → la PAMI la maintient indéfiniment.

**Utilisation :**
- Trajectoires **complexes curvilignes** (courbes, diagonales)
- Mouvements **composites** (avancer + tourner simultanément)
- Maintenir une position en attendant

**Exemple avec une seule consigne :**
```cpp
pami.asserv(100, 45);  // Avancer 100 mm à 45°
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
En mode liste, la PAMI passe automatiquement à la consigne suivante quand la précédente est atteinte (à l'intérieur des tolérances `ERREUR_DISTANCE` et `ERREUR_ANGLE`).

#### 📌 Résumé des fonctions de déplacement

```
┌───────────────────────────────────────────────────────────┐
│               COMMENT FAIRE AVANCER LA PAMI ?             │
├───────────────────────────────────────────────────────────┤
│                                                           │
│  1. Bloquant, sans asserv (TEST UNIQUEMENT) :             │
│     pami.avancer(150);                                    │
│                                                           │
│  2. Asservi linéaire simple (ligne droite) :              │
│     avancer_asservi(etape, 150);                          │
│     tourner_asservi(etape, 90);                           │
│                                                           │
│  3. Asservi continu complexe (courbes) :                  │
│     pami.asserv(150, 0);                  // Une consigne │
│     pami.asserv_list(mouvements, nb);     // Liste        │
│                                                           │
└───────────────────────────────────────────────────────────┘
```

---

- ### ⚙️ Asservissement PD

L'asservissement des fonctions `asserv` & `asserv_list` utilise une correction **proportionnelle-dérivée (PD)** pour corriger les erreurs :

**📊 Principe du PD**

*Pour chaque roue :*
```cpp
erreur_distance = consigne_distance - mesure_distance
correction_distance = KP_distance * erreur_distance ± KD_distance * vitesse

erreur_angle = consigne_angle - mesure_angle
correction_angle =  KP_angle * erreur_angle ± KD_angle * vitesse

commande_moteur = correction_distance ± correction_angle
```

- **KP** (gain proportionnel) : Plus l'erreur est grande, plus on corrige fort
- **KD** (gain dérivateur) : Évite les dépassements en freinant si l'erreur diminue trop vite

---

**🔍 Régler l'asservissement**

**Tuto :**
- Partir de KP = KD = 0
- Augmenter KP jusqu'a oscillation relativement forte
- Garder la valeur en tête (c'est un maximum) et le diminuer (~ KP/2)
- Augmenter KD jusqu'a suppression des oscillations
- Re augmenter KP

**Si la PAMI :**
- **Dépasse la consigne** → Diminuer **KP**, augmenter **KD**
- **Oscille autour de la cible** → Augmenter **KD** (damping)
- **N'atteint jamais la cible** → Augmenter **KP**
- **Reste bloquée** → Vérifier les encodeurs avec test 4

---

## 📍 Mesure de position

Le robot possède deux encodeurs afin de connaitre sa position

---

## 🔧 Modes de test diagnostique

La fonction `pami.test(mode)` permet de tester chaque système individuellement. Appellez-la dans `setup()` pour déboguer :

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

## 📚 Référence des fonctions

### 🤖 Classe `Pami` (les plus utiles)

#### Informations
```cpp
pami.print_log();                       // Affiche : temps, distance IR, étape globale
pami.print_encodeur();                  // Affiche : position de chaque encodeur (ticks & cm)
pami.print_changes_in_interrupteur();   // Affiche : numéro PAMI, tirette, équipe
```

#### Mouvements naïfs (sans asservissement)
```cpp
pami.avancer(distance_cm);      // Avancer d'une distance (cm). Recule si < 0
pami.tourner(angle_deg);        // Tourner d'un angle (degrés). > 0 = trigo, < 0 = horaire
```

#### Mouvements séquentiels (asservi uniquement pendant le mouvement)
```cpp
pami.avancer_asservi(num_etape, distance_mm);   // Avancer d'une distance (cm). Recule si < 0
pami.tourner_asservi(num_etape, angle_deg);     // Tourner d'un angle (degrés). > 0 = trigo, < 0 = horaire
```

#### Mouvements continus (asservi permanent avec distance + angle)
```cpp
pami.asserv(distance_mm, angle_deg);            // Une seule consigne
pami.asserv_list(tableau_2d, nb_mouvements);    // Liste de consignes
```

#### Moteurs & servo
```cpp
pami.set_speed(vitesse_droite, vitesse_gauche);     // Allume le moteur à une certaine vitesse(0-255) de manière linéaire
pami.stop();                                        // ⚠️ Arrêt instantané
pami.linear_stop();                                 // Arrêt linéaire bloquant (rampe de freinage)
pami.non_blocking_linear_stop(init);                // Arrêt linéaire non bloquant
pami.blink_servo(theta1, theta2, temps_ms);         // Bouger le servo entre deux angles tous les temps_ms
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

---

## ⚙️ Paramètres existants dans `define.h`

### 📌 Tous les pins, les GPIO

### 📏 Tous les paramètres globaux
- **Periodes** : Les temps de tout (début & fin du match, fréquence d'asservissement, du servomoteur, de l'infrarouge, des logs, etc)
- **Géométrie** : L'empatement du robot, la vitesse moteur par défaut (0-255 pour PWM)
- **Tolérances** : Toutes les tolérances [déplacement en distance (mm) et angle (°), détection d'obstacle]

### 🚀 Gains pour mouvements naïfs (bloquants)
- **K_NAIF** : Facteur de temps pour avancer (test 8)
- **K_ANGLE_NAIF** : Facteur de temps pour tourner (test 9)

### 🎮 Gains PD pour asservissement
- **KP_DISTANCE / KD_DISTANCE** : Correction proportionnelle/dérivative pour avancer tout droit
- **KP_ANGLE / KD_ANGLE** : Correction proportionnelle/dérivative pour tourner sur place

### 📏 Gains de conversion encodeur
- **GAIN_MM_TO_TICKS** : Nombre de ticks par mm parcouru → calibré avec test 6
- **GAIN_ANGLE_TO_TICKS** : Nombre de ticks par degré tournés → calibré avec test 7

### 📍 Positions de départ
Les positions initiales pour chaque PAMI pour avoir la position absolue de la PAMI sur la scène :
- Un tableau avec 4 entrées - une pour chaque pami (on peut en rajouter si plus de pami)
- Pour chaque pami une équipe (Jaune ou Bleue)
- Pour chaque équipe une position (x, y) de départ donc on a :
```cpp
{.j = {x_depart_j, y_depart_j}, .b = {x_depart_b, y_depart_b}}`
```
- Pour y accèder (voir exemple) : .j ou .b pour l'équipe puis .x ou .y pour la coordonnée

```cpp
inline const Robot pami_start_pos[4] = {
    {.j = {0, 0}, .b = {0, 0}},
    {.j = {0, 0}, .b = {0, 0}},
    {.j = {0, 0}, .b = {0, 0}},
    {.j = {0, 0}, .b = {0, 0}};

int pos_start_jaune_x = pami_start_pos[2].j.start.x
int pos_start_bleue_y = pami_start_pos[2].b.start.y
```

---

*Voilà le code qu'on a réussi à pondre à la coupe en 2026. J'espère que c'est plus clair ! N'hésitez pas à demander aux auteurs (Jules, Flo, Antoine) si vous avez des doutes ! 🤖*