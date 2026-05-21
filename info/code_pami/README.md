# 🤖 Code pour les PAMIs - by Jules & Flo & Antoine (2026)

// Parler des TICKS_TO_CM - on utilise des ticks et convertir blabla
// Rampes set_speed & stop
// Il faut inclure les rampes accéleration & freinage dans avancer. Pas de fonctions autres. On peut les avoir aussi mais sinon set_speed & stop sont forcément bloquants. Ils ne sont appelés qu'une fois avant de passer à autre chose, on a pas le luxe de faire des boucles if et d'attendre ces phases dans avancer & tourner

## ⚠️ IMPORTANT : À LIRE AVANT D'ALLER FOUILLER LES FONCTIONS

> 💡 **Note des auteurs :** Le code est bien fait mais pas forcément explicite au premier abord. Cette aide est censée vous aider à y voir plus clair.
> En gros, les fonctions de `pami.cpp` sont implémentées de manière à avoir un **code non bloquant séquentiel** *(explication dessous, bizuth)*.

### 🛠️ À FAIRE dans `define.h`
* 📌 Configurer les pins de chaque composant avec l'élec.
* ⚙️ Régler les gains de l'asservissement.
* 📍 Modifier les valeurs de départ et d'arrivée dans `define.h` pour chaque pami *(à voir si mesure_pos mais pas pour l'instant)*.

> 📊 Pour l'instant l'asservissement est uniquement proportionnel mais, à terme, on aura un PID.

💻 **`main.cpp` :** Seul fichier à modifier, en théorie, afin d'écrire la stratégie. On explique les fonctions qui existent déjà et comment les utiliser dans ce fichier.

---

# ⚙️ Fonctionnement du code

## 🧠 À savoir
L'ESP32 exécute une fois la fonction `setup()` du main au démarrage, puis `loop()` en boucle.
* **Au `setup()` :** On initialise tous les composants, les temps, etc.
* **Dans `loop()` :** S'éxecute en boucle sur l'esp32.

🚨 **Le Problème :** Les fonctions bloquantes comme `delay(dt)` qui permettent d'attendre dt ms ou les boucles `while`. Pendant ces dt ms, **plus rien ne se passe**. Donc, par exemple, si le match se termine (100s), le code lui n'a pas encore fini et le servomoteur ne se lance pas. Ou si un obstacle arrive, il ne sera pas détecté, etc.

🚫 **Solution :** On ne veut **surtout pas** de fonctions bloquantes, et donc un code non bloquant, mais ça demande une autre manière de réfléchir. On utilise beaucoup de `if`/`else` et il faut se dire qu'il n'y a **pas d'ordre d'exécution**, sinon on attendrait la fin de la fonction n°x avant de lancer la n°(x+1) et on ne veut pas attendre car ça implique d'être bloqué -> l'ordre d'appel n'est plus l'ordre normal des lignes.

---

### 👀 Ce qu'il se passe dans notre `loop`
Notre boucle s'exécute à une très grande fréquence. Elle va juste tester différents cas en continu (non exhaustif - à vous de les rajouter) :
* ⏱️ **Temps global écoulé :** `> 100s` -> éteint moteur & lance servomoteur.
* 🚨 **Obstacle détecté :** Éteint moteur & attend *(ou autre stratégie, à vous de choisir)*.
* 📈 **Temps de faire des logs :** On affiche des infos toutes les 1 seconde par exemple.

*🔄 **Sinon :** On peut aussi constamment exécuter des fonctions (bouger le servomoteur, récupérer la distance au prochain obstacle, etc), il suffit de les mettre dans la `loop` sans conditions d'exécutions.*

---

### 🧩 Le Problème du Séquentiel (et sa solution !)
Pour définir une séquence d'action (ex: déplacement), on veut attendre que la précédente soit terminée avant de lancer la suivante. **Comment lui dire "avance PUIS tourne PUIS recule" si tout s'exécute en même temps ?**

💡 **La Solution :** Pour enlever ce problème, on a créé une file dans laquelle on numérote les actions qui nécessitent un ordre d'appel. C'est ça le **code séquentiel**.
* Une variable globale `etape_globale` est définie dans `define.h`. Elle suit le n° de l'étape en cours d'exécution.
* Chaque fonction met à jour cette variable lorsqu'elle a terminé son job.
* En gros, on teste : `if (ton tour de jouer car on t'appelle) { execute toi } else { passe ton tour }`. C'est vraiment ça l'idée générale.

⚠️ *Toutes les fonctions ne sont pas comme ça, il faut les réécrire pour chaque action pour pouvoir utiliser cette file.*

#### Actuellement, on a :
* ⏱️ `delay_non_bloquant`
* 🏎️ `avancer_asservi`
* 🔄 `tourner_asservi`

---

### 🛠️ Comment utilise-t-on ces fonctions ?

On a une variable d'étape (`etape_globale`) & une variables de temps (`callbacktime`).

On leur donne en paramètre un **numéro d'étape** qui sera comparé à `etape_globale` pour savoir si son tour est venu.
On leur donne aussi le **dernier temps réel d'exécution** (`callbacktime`) pour gérer leur fréquence d'exécution. *(Pour l'asservissement on l'exécute toutes les `FREQ_ASSERV` sinon ça explose, ça va trop vite, et on a besoin du dernier temps d'exécution pour ça)*.

* **Si ce n'est pas leur tour / pas le bon moment d'être appelée** (fréquence pas atteinte) : on retourne le dernier temps d'exécution sans rien modifier.
* **Si c'est leur tour & que la fréquence voulue est atteinte** : on exécute le corps de la fonction & on met à jour le temps d'exécution.
* **Si elle a fini son rôle** (certaines fonctions s'éxecutent plusieurs fois avant d'avoir fini, avancer_asserv par exemple) : on met à jour la variable d'étape globale et on passe à la suivante

> 🔤 **Structure :** `callbacktime = fonction_non_bloquante_sequentielle(num_appel, autres_params, callbacktime);`

---

<!-- ### 🔍 Questions / Pistes de réflexion sur le code : -->

`delay_non_bloquant` est un peu différent, il ne retourne pas de temps. En effet son rôle est de "bloquer" le temps mais uniquement celui de la file d'appel. Il s'agit simplement d'une étape qui ne passe pas à l'étape suivante durant toute la durée du delay souhaité. <br>
Il a donc une étape d'appel comme les autres et, en théorie, pourrait retourner constamment `callbacktime` le temps précédent -> aucun intérêt donc ne retourne rien.

> 🔤 **Structure :** `delay_non_bloquant(n° d'appel, callbacktime, temps de delay);` *// Delay de DELAY_TIME par défaut*

---

🎒 Voilà le code qu'on a réussi à pondre à la coupe en 2026. J'espère que c'est à peu près clair, sinon demandez aux 2A/auteurs du code !

---

# 🏗️ Architecture du code

Après ce long tunnel sur le fonctionnement des fonctions, voilà l'architecture globale ainsi que les fonctions utiles. Le code est décomposé en plusieurs fichiers **(une classe = un composant/une fonctionnalité)** :

* 🔢 **Encodeur :** Permet d'accéder aux 'ticks' des encodeurs.
* 🏎️ **Moteur :** Permet d'envoyer des ordres aux moteurs *(à terme, avec une rampe & pas un échelon)*.
* 🔴 **Irsensor :** Capteur ToF (VL53L5CX) - Permet d'accéder aux données du capteur IR. Stocke la distance à l'obstacle le plus proche.
* 🔊 **Ultrason :** Capteur ultrason (HC-SR04) - Permet d'accéder aux données du capteur ultrason. Stocke la distance à l'obstacle le plus proche.
* 📐 **Serv :** Permet d'envoyer des ordres à un servomoteur.
* 📺 **Screen :** Permet d'interagir avec l'écran.
* 🗺️ **Mesure Position :** Permet d'avoir la position absolue du pami sur le terrain *(à faire, n'existe pas encore)*.
* 🤖 **Pami :** La pami en elle-même - Contient toutes les fonctions utiles.
* 💻 **Main :** À upload sur la pami, assemble toutes les fonctions de `Pami.cpp` pour la stratégie.

---

## 🛠️ Commandes utiles par classes

### 💻 `main`
* `delay_non_bloquant (n° d'appel, temps)` : Permet de faire un delay dans la file d'appel, sans bloquer le reste du code.

### 🤖 `Pami` *(On utilise principalement celles-ci - elles reprennent plus simplement les classes du dessous)*
* `print_log` : Affiche le temps écoulé depuis le début du match, la distance au prochain obstacle (IR), et la position dans la file d'appel.
* `print_encodeur` : Affiche la position de chaque encodeur en ticks & cm *(attention, ils sont remis à 0 à la fin de chaque déplacement - à faire avec mesure_pos)*.
* `print_infos_interrupteur` : Affiche le n° de la pami, l'état de la tirette, et l'équipe si un état est modifié.

#### 🚫 Fonctions sans aucun asservissement (Naïves)
*Allume les moteurs pendant un certain temps qui dépend de gains (`K_NAIF` & `K_ANGLE_NAIF`) à régler à la main pour chaque robot.*
* `avancer (distance)`  : Avancer d'une distance en cm & recule si `distance < 0`.
* `tourner (theta)`     : Tourne d'un angle theta en degrés *(sens trigo > 0 & sens horaire < 0)*.
* `go_to (x, y)`        : Va à la position (x, y) du plateau. Avance d'abord de x, tourne de 90°, puis avance de y.

#### ⚙️ Fonctions génériques pour chaque composant
* `set_speed (vitesse_d, vitesse_g)`    : Envoie une commande aux moteurs (entre 0 & 255).
* `stop`                                : Coupe les moteurs linéairement mais est bloquante - ne pas utiliser pendant le match
* `blink_servo (theta1, theta2, dt)`    : Tourne le servo de theta1 à theta2 en dt ms.
* `get_IR_distance`                     : Retourne la distance au prochain obstacle.

#### ⏳ Fonctions non bloquantes qui nécessitent un numéro d'appel
* `avancer_asserv (n° d'appel, distance, temps)`    : Avancer d'une distance en cm.
* `tourner_asserv (n° d'appel, angle, temps)`       : Recule d'une distance en cm.

### 🔢 `Encodeur`
* `mesure` : Retourne le nombre de ticks de roue de l'encodeur.