Code pour les PAMIs - 2026

!!!
Régler les gains de l'asservissement AVANT DE BOUGER LES FONCTIONS, ca modifie le comportement du robot
Configurer les pins de chaque composant avec l'élec
Modifier les valeurs de départ et d'arrivée dans define.h pour chaque pami
!!!

Décomposé en plusieurs fichiers (une classe - une fonction)
    - Asservissement    : Asserv & asservPID (le 1ème fichier implémente le correcteur PID du 2ème)
    - Moteur            : Moteur - envoie d'une commande
    - Encodeur          : Encodeur - Récupère la position du robot
    - Mesure Position   : Mesure_pos - Déduit position et vitesse des encodeurs
    - Irsensor          : Capteur ToF (VL53L5CX) - Récupère la distance aux obstacles (tableau de 8 valeurs - moyenne sur chaque colonne)
    - Ultrason          : Capteur ultrason (HC-SR04) - Récupère la distance aux obstacles
    - Serv              : Servomoteur - Fait bouger un servomoteur entre deux angles
    - Machine_etat      : Stratégie du robot - on l'utilise plus
    - main              : A upload sur la pami, assemble toutes les fonctions

Chaque fichier représente une classe qui possède au moins 2 fonctions
    - setup             : Est exécuté au démarrage une seule fois (configuration initiale)
    - loop              : Est exécuté en boucle tant que la pami est allumée
    - Peut possèder d'autres fonctions qui peuvent être appelées dans loop ou setup de cette classe ou d'une autre mais ne sont pas executées automatiquement


Commandes utiles par classes:
    - Pami (on utilise elles -  elles reprennent + simplement les autres dessous):
        - print_log, print_position, print_encodeur, print_speed() - Affiche des logs

        - go_to (x, y)          : Va a la position (x, y) du plateau. Marche bof (dépend de fou des correcteurs)
        - avancer (distance)    : Avancer d'une distance en cm
        - reculer (distance)    : Recule d'une distance en cm
        - tourner (theta)       : Tourne d'un angle theta en degrés (dépend de fou des correcteurs)

        - go_to_with_obstacle (x, y) : Va a la position (x, y) du plateau en évitant les obstacles. Marche bof (dépend de fou des correcteurs)
        - avancer_with_obstacle (distance) : Avancer d'une distance en cm en évitant les obstacles
        - reculer_with_obstacle (distance) : Recule d'une distance en cm en évitant les obstacles

        - allumer_moteur (speed)            : Allume les moteurs à une certaine vitesse en cm/s
        - blink_servo (theta1, theta2, dt)  : Tourne le servo de theta1 à theta2 en dt

        - get_ultrason_distance : Retourne la distance au prochain obstacle
        - get_IR_distance : Retourne la distance au prochain obstacle

        - ! Jamais testé par Flo ! start_match() : Fonction a appeler pour un match en théorie

    - Asserv :
        - asservissement (vitesse_l_consign, vitesse_r_consign)                 : Permet un asservissement en vitesse
        - asserv_angle (theta_consigne)                                         : Permet un asservissement en angle
        - asserv_global (vitesse_l_consign, vitesse_r_consign, theta_consigne)  : Permet un asservissement en angle et en vitesse

    - Moteur :
        - set_speed (int [0-255]) : Envoie une vitesse au moteur

    - Encodeur :
        - mesure : Retourne le nombre de tics de roue de l'encodeur

    - Mesure Position :
        - loop : Boucle de mise à jour des variables globales de position et vitesse dans le repère des roues et globales

    - Irsensor (Allez lire dans define.h comment brancher le capteur) :
        - loop  : Boucle de mise à jour d'une variable vision de la moyenne du capteur sur chacune des 8 colonnes

    - Ultrason :
        - loop : Boucle de mise à jour de la variable m_distance mesurée par le capteur ultrason

    - Serv :
        - blink (temps_blink, angle1, angle2) : Fait bouger le servomoteur entre deux angles en un temps donné

    - main :
        - setup : Initialise tous les composants du robot ainsi que l'asservissement
        - loop : Lance la boucle de commande du robot