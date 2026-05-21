Code pour les PAMIs - by Jules & Flo & Antoine (2026)

# :warning: IMPORTANT

## !!! LIRE CE FICHIER AVANT D'ALLER FOUILLER LES FONCTIONS !!!
Le code est bien fait mais pas forcément explicite au premier abord<br>
Cette aide est censée voir aider à y voir plus clair<br>
En gros, les fonctions de pami.cpp sont implémentées de manière à avoir un code non bloquant séquentiel (explication dessous bizuth)<br>

## !!! A FAIRE dans define.h !!!
Configurer les pins de chaque composant avec l'élec<br>
Régler les gains de l'asservissement<br>
Modifier les valeurs de départ et d'arrivée dans define.h pour chaque pami (à voir si mesure_pos mais pas pour l'instant)<br>

Pour l'instant l'asservissement est uniquement proportionnel mais, à terme, on aura un PID<br>
Dans main.cpp : Seul fichier à modifier, en théorie, afin d'écrire la stratégie.<br>
On explique les fonctions qui existent déjà et comment les utiliser dans ce fichier<br>

# Fonctionnement du code

## A savoir
L'esp32 execute une fois la fonction `setup()` du main au démarrage puis `loop()` en boucle<br>
Donc au setup on initialise tout les composants, les temps, etc<br>
Et dans loop on a le problème des fonctions bloquantes comme delay(dt) qui permettent d'attendre dt ms ou les boucles while.<br>

Problème : pendant ces dt ms, plus rien ne se passe. Donc, par exemple, si le match se termine (100s) le code lui n'a pas encore finit et le servomoteur ne se lance pas ou si un obstacle arrive, il ne sera pas détecté, etc.<br>
On ne veut surtout pas de fonctions bloquantes, et donc un code non bloquant, mais ça demande une autre manière de réfléchir.<br>
On utilise beaucoup de if/else et il faut se dire que il n'y a pas d'ordre d'éxecution, sinon on attendrait la fin de la fonction n°x avant de lancer la n°(x+1) et on ne veut pas attendre car ça implique d'être bloqué.<br>


Dans notre loop (qui s'éxecute à une très grande fréquence) on va juste tester différents cas :
    - Temps global écoulé       : > 100s -> éteint moteur & lance servomoteur
    - Obstacle détecté          : Eteint moteur & attend (ou autre stratégie, à vous de choisir)
    - Temps de faire des logs   : On affiche des infos toutes les 1 seconde par exemple
Sinon on peut aussi constamment executer des fonctions (bouger le servomoteur, récupérer la distance au prochain obstacle, etc), il suffit de les mettre dans la loop sans conditions d'éxecutions<br>


Problème avec cette méthode :
Pour définir une séquence d'action (ex: déplacement) on veut attendre que la précedente soit terminée avant de lancer la suivante, comment lui dire avance PUIS tourne PUIS recule si tout s'éxecute en même temps ?<br>
Pour enlever ce problème, on a crée une file dans laquelle on numérote les actions qui nécessite un ordre d'appel.<br>
C'est ça le code séquentiel. Une variable globale etape_globale est définie dans define.h qui suit l'étape en cours d'éxecution et chaque fonction met à jour cette variable lorsqu'elle à terminé son job.<br>
En gros, on test if (ton tour de jouer) {execute toi} else {passe ton tour}<br>
C'est vraiment ça l'idée générale.


Toutes les fonctions ne sont pas comme ça, il faut les reécrire pour chaque action pour pouvoir utiliser cette file.<br>
Actuellement, on a :
    - `delay_non_bloquant`
    - `avancer_asservi`
    - `tourner_asservi`

Maintenant comment on utilise ces fonctions.<br>
On a donc une variable d'étape (`etape_globale`) & deux variables de temps (`oldtime` & `newtime`).<br>
Pourquoi deux de temps ? Pour savoir quand la fonction a été appelée<br>
On leur donne en paramètre un numéro d'étape qui sera comparé à étape_globale pour savoir si son tour est venu<br>
Et on leur donne leur dernier temps d'éxecution (`old_time`) pour gérer leur fréquence d'éxecution (pour l'asservissement on l'éxecute toutes les FREQ_ASSERV sinon ça explose ça va trop vite et on a besoin du dernier temps d'éxecution pour ça)<br>


Si ce n'est pas leur tour/pas le bon moment d'être appellée (frequence pas atteinte), on retourne le dernier temps d'éxecution sans rien modifier.<br>
Si c'est leur tour & que la fréquence voulue est atteinte, on éxecute le corps de la fonction & on met à jour le temps d'éxecution<br>
Vous pourriez vous dire 'mais pourquoi on donne oldtime si c'est pour qu'il devienne toujours newtime ?'<br>
En effet, soit il est retourné (donc newtime = oldtime), soit newtime = millis() & oldtime est l'ancien temps mais au début de la boucle oldtime = newtime. Et ici vous avez raison, tout est à cause de delay_non_bloquant.<br>
> Structure : `newtime = fonction_non_bloquante_sequentielle(num_appel, oldtime);`<br>

# Je peux pas supprimer la boucle du début et remplacer oldtime par newtime dans les appels de fonctions ?

`delay_non_bloquant` est un peu différent, il ne retourne pas de temps. En effet son rôle est de "bloquer" le temps mais uniquement celui de la file d'appel. Il à donc une étape d'appel comme les autres et, en théorie, pourrait retourner constamment oldtime le temps précédent -> aucun intérêt donc ne retourne rien.<br>
Il s'agit simplement d'une étape qui ne passe pas à l'étape suivante durant toute la durée du delay souhaité.
> Structure : `delay_non_bloquant(n° d'appel, oldtime, temps de delay);` // Delay de DELAY_TIME par défaut
# elles pourraient prendre newtime & on supprime le newtime = oldtime au début de chaque boucle -> oldtime useless ?<br>
Si on met à jour oldtime = newtime pour l'appel d'un delay, la condition `(millis() - oldtime) > delay` n'est jamais atteinte car oldtime = newtime (mais newtime vaut toujours oldtime car les autres fonctions de la file ne sont pas a la bonne etape...)
# tester d'enlever la boucle et voir si delay fonctionne encore. Si oui, tester aussi de supprimer oldtime


Voilà le code qu'on a réussi à pondre à la coupe en 2026.<br>
J'espère que c'est à peu près clair sinon demandez aux 2A/auteurs du code<br>

---

# Architecture du code

Après ce long tunnel sur le fonctionnement des fonctions, voilà l'architecture du code de manière générale ainsi que les fonctions utiles


Le code est décomposé en plusieurs fichiers (une classe - une composant/une fonctionnalité)<br>
    - Encodeur          : Permet d'accéder aux 'ticks' des encodeurs<br>
    - Moteur            : Permet d'envoyer des ordres aux moteurs (à terme, avec une rampe & pas un échelon)<br>
    - Irsensor          : Capteur ToF (VL53L5CX) - Permet d'accéder aux données du capteur IR. Stock la distance à l'obstacle le plus proche<br>
    - Ultrason          : Capteur ultrason (HC-SR04) - Permet d'accéder aux données du capteur ultrason. Stock la distance à l'obstacle le plus proche<br>
    - Serv              : Permet d'envoyer des ordres à un servomoteur<br>
    - Mesure Position   : Permet d'avoir la position absolue du pami sur le terrain (à faire, n'existe pas encore)<br>
    - Pami              : La pami en elle-même - Contient toutes les fonctions utiles<br>
    - main              : A upload sur la pami, assemble toutes les fonctions de Pami.cpp pour la stratégie<br>


Commandes utiles par classes:
    - main :
        - delay_non_bloquant (n° d'appel, temps) : Permet de faire un delay dans la file d'appel, sans bloquer le reste du code

    - Pami (on utilise elles -  elles reprennent + simplement les autres dessous):
        - print_log                 : Affiche le temps écoulé depuis le début du match, la distance au prochain obstacle (IR), et la position dans la file d'appel (explication dessous)
        - print_encodeur            : Affiche la position de chaque encodeurs en ticks & cm (attention, ils sont remis à 0 à la fin de chaque déplacement - à faire avec mesure_pos)
        - print_infos_interrupteur  : Affiche le n° de la pami, l'état de la tirette, et l'équipe si un état est modifié

        // Fonctions sans aucun asservissement. On avance/tourne pendant un certain temps
        // Ce temps dépend de gain (K_NAIF & K_ANGLE_NAIF) à régler à la main pour chaque robot
        - avancer (distance)    : Avancer d'une distance en cm & recule si distance < 0
        - tourner (theta)       : Tourne d'un angle theta en degrés (sens trigo > 0 & sens horaire < 0)
        - go_to (x, y)          : Va a la position (x, y) du plateau. Avance d'abord de x, tourne de 90°, puis avance de y

        // Fonctions génériques pour chaque composant
        - set_speed (vitesse_d, vitesse_g)  : Envoie une commande aux moteurs (entre 0 & 255)
        - blink_servo (theta1, theta2, dt)  : Tourne le servo de theta1 à theta2 en dt ms
        - get_IR_distance                   : Retourne la distance au prochain obstacle

        // Fonctions non bloquantes qui nécessite un numéro d'appel
        - avancer_asserv (n° d'appel, distance, temps) : Avancer d'une distance en cm
        - tourner_asserv (n° d'appel, angle, temps) : Recule d'une distance en cm

    - Encodeur :
        - mesure : Retourne le nombre de ticks de roue de l'encodeur
