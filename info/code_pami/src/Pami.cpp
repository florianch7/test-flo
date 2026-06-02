#include <Pami.h>

Pami::Pami(Moteur *p_moteur_d, Moteur *p_moteur_g, Encodeur *p_encodeur_d, Encodeur *p_encodeur_g, Mesure_pos *p_mesure_pos, Serv *p_servo, Irsensor *p_ir_sensor, Ultrason *p_ultrason)
{
    moteur_d = p_moteur_d;
    moteur_g = p_moteur_g;
    encodeur_d = p_encodeur_d;
    encodeur_g = p_encodeur_g;
    mesure_pos = p_mesure_pos;
    servo = p_servo;
    ultrason = p_ultrason;
    ir_sensor = p_ir_sensor;
}

/*
Fonction de diagnostic général du robot
Modes :
1 = Interrupteurs & Tirette
2 = Capteur IR (ToF)
3 = Servomoteur
4 = Moteurs & Encodeurs individuels (tests des sens)
5 = Homologation : Avance et s'arrête si obstacle (capteur IR)
6 = Réglage gains de convertion mm (GAIN_MM_TO_TICKS)
7 = Réglage gains de convertion angle (GAIN_ANGLE_TO_TICKS)
8 = Réglage gains distance (K_NAIF)
9 = Réglage gains angle (K_ANGLE_NAIF)
*/
void Pami::test(int mode)
{
    Serial.print("\n========== LANCEMENT DU TEST MODE : " + String(mode) + " ==========\n");

    switch (mode)
    {
    case 1: // --- TEST 1 : TIRETTE & INTERRUPTEURS ---
    {
        Serial.println("Test Interrupteurs... Modifiez leurs etats ! (Boucle infinie)");
        while (true)
        {
            this->print_changes_in_interrupteur();
            delay(500);
        }
        break;
    }
    case 2: // --- TEST 2 : CAPTEUR IR ---
    {
        Serial.println("Test Capteur IR... Passez votre main devant ! (Boucle infinie)");
        while (true)
        {
            float dist = this->get_IR_distance();
            Serial.print("Distance mesuree : ");
            Serial.print(dist / 10.0);
            Serial.println(" cm");
            delay(200);
        }
        break;
    }
    case 3: // --- TEST 3 : SERVOMOTEUR ---
    {
        Serial.println("Test Servomoteur : Va-et-vient de 3 secondes (Boucle infinie)");
        while (true)
        {
            this->blink_servo(0, 90);
        }
        break;
    }
    case 4: // --- TEST 4 : MOTEURS & ENCODEURS INDIVIDUELS ---
    {
        Serial.println("Test Moteurs & Encodeurs : test de chaque roue independamment");
        Serial.println("--- Verifiez que les valeurs des encodeurs sont POSITIVES quand on avance ---");

        while (true)
        {
            Serial.println("\n>>> TEST ROUE DROITE (Vitesse 200, 2 sec)");
            encodeur_d->clear_count();
            encodeur_g->clear_count();
            moteur_d->set_speed(200);
            moteur_g->linear_stop();

            moteur_d->linear_stop();
            Serial.print("  Enc D: " + String(encodeur_d->mesure()) + " ticks | Enc G: " + String(encodeur_g->mesure()) + " ticks");
            delay(1000);

            Serial.println("\n>>> TEST ROUE GAUCHE (Vitesse 200, 2 sec)");
            encodeur_d->clear_count();
            encodeur_g->clear_count();
            moteur_d->linear_stop();
            moteur_g->set_speed(200);

            Serial.println("  Enc D: " + String(encodeur_d->mesure()) + " ticks | Enc G: " + String(encodeur_g->mesure()) + " ticks");
            moteur_g->linear_stop();
            delay(1000);

            Serial.println("\n>>> TEST DEUX ROUES ENSEMBLE (Vitesse 200, 2 sec)");
            encodeur_d->clear_count();
            encodeur_g->clear_count();
            moteur_d->set_speed(200);
            moteur_g->set_speed(200);

            Serial.println("Enc D: " + String(encodeur_d->mesure()) + " ticks | Enc G: " + String(encodeur_g->mesure()) + " ticks");
            moteur_d->linear_stop();
            moteur_g->linear_stop();

            Serial.println("\n--- Fin d'une serie de tests. Restart dans 3 sec ---");
            delay(3000);
        }
        break;
    }
    case 5: // --- TEST 5 : HOMOLOGATION ---
    {
        Serial.println("Homologation : Robot avance tout droit et s'arrete si obstacle (< 8cm)");
        Serial.println("Pressez la tirette pour commencer");

        while (digitalRead(PIN_TIRETTE) == 1)
        {
            delay(100);
        }

        unsigned long last_print = millis();

        while (true)
        {
            float dist = this->get_IR_distance();

            if (dist < DISTANCE_MIN && dist > 0.5)
            {
                Serial.println(">>> OBSTACLE DETECTE ! Arret du robot");
                this->linear_stop();
            }
            else
            {
                this->set_speed(SPEED, SPEED);

                if (millis() - last_print >= 1000) // Affiche tous les 1s
                {
                    Serial.print("Distance: " + String(dist / 10.0) + " cm - Avance...");
                    last_print = millis();
                }
            }
            delay(50);
        }

        this->linear_stop();
        Serial.println("Homologation terminee.");
        break;
    }
    case 6: // --- TEST 6 : REGLAGE GAINS CONVERTION MM <-> TICKS ---
    {
        Serial.println("Reglage GAIN_MM_TO_TICKS & GAIN_ANGLE_TO_TICKS");

        // Test translation droite
        Serial.println(">>> TEST TRANSLATION : Roues avancent ensemble");
        Serial.println("Le robot va avancer pendant un certain temps. Mesurez la distance reelle parcourue");
        Serial.println("Calculer ensuite le rapport entre le nombre de ticks moyen et la distance reelle pour trouver GAIN_MM_TO_TICKS");
        encodeur_d->clear_count();
        encodeur_g->clear_count();
        Serial.print("Départ dans 5 sec");
        delay(5000);

        this->set_speed(SPEED, SPEED);
        delay(2000);
        this->linear_stop();

        float ticks_d = encodeur_d->mesure();
        float ticks_g = encodeur_g->mesure();
        float ticks_avg = (ticks_d + ticks_g) / 2.0;

        Serial.print("\n Translation totale - Ticks D: " + String(ticks_d) + " | Ticks G: " + String(ticks_g) + " | Moyenne: " + String(ticks_avg));
        break;
    }
    case 7: // --- TEST 7 : REGLAGE GAINS CONVERTION ANGLE <-> TICKS ---
    {       // Test rotation
        Serial.println("\n>>> TEST ROTATION : Roues tournent en sens oppose");
        Serial.println("Le robot va tourner pendant un certain temps. Mesurez l'angle reelle parcourue");
        Serial.println("Calculer ensuite le rapport entre le nombre de ticks moyen et l'angle reelle pour trouver GAIN_ANGLE_TO_TICKS");
        Serial.print("Départ dans 5 sec");
        encodeur_d->clear_count();
        encodeur_g->clear_count();
        delay(5000);

        this->set_speed(SPEED, -SPEED);
        delay(2000);
        this->linear_stop();

        float ticks_d = encodeur_d->mesure();
        float ticks_g = encodeur_g->mesure();
        float ticks_diff = ticks_d - ticks_g;

        Serial.print("\nFinal - Ticks D: " + String(ticks_d) + " | Ticks G: " + String(ticks_g) + " | Diff: " + String(ticks_diff));
        break;
    }
    case 8: // --- TEST 8 : REGLAGE GAINS DISTANCE NAIFS ---
    {
        Serial.println("Reglage K_NAIF (gains naifs pour avancer sans asserv)\n");
        Serial.println(">>> TEST K_NAIF : avance 500mm");
        encodeur_d->clear_count();
        encodeur_g->clear_count();
        delay(2000);

        long expected_time = K_NAIF * (500.0 / SPEED) * 1000;
        Serial.print("Temps theorique : " + String(expected_time) + " ms");

        this->set_speed(SPEED, SPEED);
        delay(expected_time); // On attend le temps théorique pour faire le mouvement
        this->linear_stop();

        float dist_d = encodeur_d->mesure() / GAIN_MM_TO_TICKS;
        float dist_g = encodeur_g->mesure() / GAIN_MM_TO_TICKS;
        float dist_avg = (dist_d + dist_g) / 2.0;

        Serial.print("\nDistance reelle parcourue: " + String(dist_avg) + " mm");
        Serial.print("K_NAIF actuel: " + String(K_NAIF));
        Serial.print("K_NAIF calcule: " + String(K_NAIF * (500.0 / dist_avg)));
    }
    case 9: // --- TEST 9 : REGLAGE GAINS ANGLE NAIFS ---
    {
        Serial.println("Reglage K_ANGLE_NAIF (gains naifs pour tourner sans asserv)\n");
        Serial.println("\n>>> TEST K_ANGLE_NAIF : tourne 360 degres");
        encodeur_d->clear_count();
        encodeur_g->clear_count();
        delay(2000);

        long expected_time = K_ANGLE_NAIF * (360.0 / SPEED) * 1000;
        Serial.print("Temps theorique : " + String(expected_time) + " ms");

        this->set_speed(SPEED, -SPEED);
        delay(expected_time); // On attend le temps théorique pour faire le mouvement
        this->linear_stop();

        float angle = encodeur_d->mesure() / GAIN_ANGLE_TO_TICKS;
        Serial.print("\nAngle reel : " + String(angle) + " deg");
        Serial.print("K_ANGLE_NAIF actuel: " + String(K_ANGLE_NAIF));
        Serial.print("K_ANGLE_NAIF calcule: " + String(K_ANGLE_NAIF * (360.0 / angle)));

        break;
    }
    default:
    {
        Serial.println("Erreur : Mode de test inconnu ! (Choisissez entre 1 et 9)");
        break;
    }
    }
}

/*
Permet d'éxecuter plusieurs consignes de manière séquentielle non bloquant
Asservissement en consigne de distance et d'angle en même temps, pour faire du mouvement curviligne
Si pas précisé dans le nom, c'est en ticks, sinon on met _cm ou _mm ou _degre
$ C'est pas mieux de tout gérer en mm ? Non car ça permet de faire du asservissement en ticks -> Sinon les angles c'est bizarre à gérer en mm
$ Vérifier le KD et l'asservissement dérivateur
*/
void Pami::asserv_list(float mouvements[][2], int nb_mvt)
{
    static int num_current_mvt = 0;
    static int pos_ref_distance = 0;
    static int pos_ref_angle = 0;
    static int prev_pos_encodeur_d = 0;
    static int prev_pos_encodeur_g = 0;

    if (millis() - this->m_time_asserv >= PERIODE_ASSERV)
    {
        float consigne_distance_mm = mouvements[num_current_mvt][0];
        float consigne_angle_degre = mouvements[num_current_mvt][1];

        // On récupère la position actuelle des encodeurs en ticks
        int pos_encodeur_d = encodeur_d->mesure();
        int pos_encodeur_g = encodeur_g->mesure();

        // Vitesse en ticks/s
        float speed_d = (pos_encodeur_d - prev_pos_encodeur_d) / ((millis() - this->m_time_asserv) / 1000.0);
        float speed_g = (pos_encodeur_g - prev_pos_encodeur_g) / ((millis() - this->m_time_asserv) / 1000.0);

        // Sauvegarde les positions pour le prochain calcul de vitesse
        prev_pos_encodeur_d = pos_encodeur_d;
        prev_pos_encodeur_g = pos_encodeur_g;

        // Calculer la distance et l'angle du mouvement actuel en ticks
        // On repart de l'origin à chaque nouveau mouvement, donc on soustrait la position de référence qui est la position d'arrivée du précédent

        // Calcul des écarts de distance et de la correction
        float distance = (pos_encodeur_d + pos_encodeur_g) / 2.0 - pos_ref_distance;
        float vitesse = (speed_d + speed_g) / 2.0;
        float ecart_distance = GAIN_MM_TO_TICKS * consigne_distance_mm - distance;
        float correction_distance = KP_DISTANCE * ecart_distance - KD_DISTANCE * vitesse;

        // Calcul des ecarts angulaires et de la correction
        float angle = (pos_encodeur_d - pos_encodeur_g) - pos_ref_angle;
        float vitesse_angulaire = (speed_d - speed_g);
        float ecart_angle = GAIN_ANGLE_TO_TICKS * consigne_angle_degre - angle;
        float correction_angle = KP_ANGLE * ecart_angle - KD_ANGLE * vitesse_angulaire;

        // Convertir les corrections en commandes moteur
        int commande_moteur_d = (int)(correction_distance + correction_angle);
        int commande_moteur_g = (int)(correction_distance - correction_angle);

        // Limiter les commandes entre -255 et 255
        commande_moteur_d = constrain(commande_moteur_d, -255, 255);
        commande_moteur_g = constrain(commande_moteur_g, -255, 255);

        // Vérifie si on a atteint la consigne
        if (abs(consigne_distance_mm - distance / GAIN_MM_TO_TICKS) < ERREUR_DISTANCE && abs(consigne_angle_degre - angle / GAIN_ANGLE_TO_TICKS) < ERREUR_ANGLE)
        {
            if (num_current_mvt < nb_mvt - 1)
            {
                // On récupère la position actuelle pour la prendre comme origine au prochain tour
                pos_ref_distance = distance;
                pos_ref_angle = angle;

                num_current_mvt++;
            }
        }

        // Envoyer les commandes aux moteurs
        this->set_speed(commande_moteur_d, commande_moteur_g);
        this->m_time_asserv = millis();
    }
}

/*
Asservissement en consigne de distance et d'angle en même temps, pour faire du mouvement curviligne
*/
void Pami::asserv(float consigne_distance_mm, float consigne_angle_degre)
{
    static int prev_pos_encodeur_d = 0;
    static int prev_pos_encodeur_g = 0;

    if (millis() - this->m_time_asserv >= PERIODE_ASSERV)
    {
        int pos_encodeur_d = encodeur_d->mesure();
        int pos_encodeur_g = encodeur_g->mesure();

        // Vitesse en ticks/s
        float speed_d = (pos_encodeur_d - prev_pos_encodeur_d) / ((millis() - this->m_time_asserv) / 1000.0);
        float speed_g = (pos_encodeur_g - prev_pos_encodeur_g) / ((millis() - this->m_time_asserv) / 1000.0);

        // Sauvegarde les positions pour le prochain calcul de vitesse
        prev_pos_encodeur_d = pos_encodeur_d;
        prev_pos_encodeur_g = pos_encodeur_g;

        // Calcul ecart en distance et la correction
        float distance = (pos_encodeur_d + pos_encodeur_g) / 2.0;
        float vitesse = (speed_d + speed_g) / 2.0;
        float ecart_distance = consigne_distance_mm * GAIN_MM_TO_TICKS - distance;
        float correction_distance = KP_DISTANCE * ecart_distance - KD_DISTANCE * vitesse;

        // Calcul ecart en angle et la correction
        float angle = (pos_encodeur_d - pos_encodeur_g);
        float vitesse_angulaire = (speed_d - speed_g);
        float ecart_angle = consigne_angle_degre * GAIN_ANGLE_TO_TICKS - angle;
        float correction_angle = KP_ANGLE * ecart_angle - KD_ANGLE * vitesse_angulaire;

        // Convertir les corrections en commandes moteur
        int commande_moteur_d = (int)(correction_distance + correction_angle);
        int commande_moteur_g = (int)(correction_distance - correction_angle);

        // Limiter les commandes entre -255 et 255
        commande_moteur_d = constrain(commande_moteur_d, -255, 255);
        commande_moteur_g = constrain(commande_moteur_g, -255, 255);

        // Envoyer les commandes aux moteurs
        this->set_speed(commande_moteur_d, commande_moteur_g);
        this->m_time_asserv = millis();
    }
}

/*
Avancer en ligne droite, on veut que chaque moteur avance de consigne_angle mm
Recule si consigne_mm < 0
*/
void Pami::avancer_asservi(int etape_d_appel, float consigne_mm)
{
    // Si c'est l'étape à laquelle on veut l'appeler & que l'intervalle d'asservissement est écoulé, alors on asservit
    if (etape_d_appel == etape_globale && (millis() - this->m_time_asserv) >= PERIODE_ASSERV)
    {
        // --- Mesures actuelles ---
        float ticks_d = encodeur_d->mesure();
        float ticks_g = encodeur_g->mesure();

        // Serial.print("Roue droite (mm) : " + String(ticks_d / GAIN_MM_TO_TICKS));
        // Serial.println(" | Roue gauche (mm) : " + String(ticks_g / GAIN_MM_TO_TICKS));

        // --- Erreurs ---
        // l'erreur peut-être négative
        float erreur = ticks_g - ticks_d;

        // --- Correction --
        // si on avance
        int pwmD = SPEED + KP_DISTANCE * erreur;
        int pwmG = SPEED - KP_DISTANCE * erreur;
        // si on recule, on remplace les valeurs
        if (consigne_mm < 0)
        {
            pwmD = -SPEED + KP_DISTANCE * erreur;
            pwmG = -SPEED - KP_DISTANCE * erreur;
        }

        pwmD = constrain(pwmD, -255, 255);
        pwmG = constrain(pwmG, -255, 255);

        // --- Commande moteurs ---
        this->set_speed(pwmD, pwmG);

        // --- Condition d’arrêt en ticks ---
        if (abs(consigne_mm - ticks_g / GAIN_MM_TO_TICKS) < ERREUR_DISTANCE && abs(consigne_mm - ticks_d / GAIN_MM_TO_TICKS) < ERREUR_DISTANCE)
        {
            // ON RENTRE & on nettoie les encodeurs !!
            // this->linear_stop();
            this->linear_stop();
            encodeur_g->clear_count();
            encodeur_d->clear_count();
            etape_globale++;
        }

        this->m_time_asserv = millis();
    }
}

/*
Tourne sur lui même, on veut que chaque moteur avance de consigne_angle dans des sens opposés
consigne angle > 0 = sens trigo
consigne angle < 0 = sens horaire
*/
void Pami::tourner_asservi(int etape_d_appel, float consigne_angle)
{
    /* But du gain proportionnel : faire une correction proportionnelle à l'erreur.
    On prend la somme pour que chaque moteur fasse le même nombre de ticks dans des sens opposés et donc une erreur nulle
    En gros :
        erreur = ticksG + ticksD
        correction = Kp * erreur

        Puis on ajuste le pwm :
        pwmG = pwmBase - correction
        pwmD = pwmBase + correction
    */

    // Si c'est pas l'étape à laquelle on veut l'appeler,
    // aucune des variables du main n'est modifiée
    if (etape_d_appel == etape_globale && (millis() - this->m_time_asserv) >= PERIODE_ASSERV)
    {
        // --- Mesures actuelles ---
        float ticks_d = encodeur_d->mesure();
        float ticks_g = encodeur_g->mesure();

        float angle_trigo = ticks_d / GAIN_ANGLE_TO_TICKS;

        // Serial.println("\t Angle en ° : " + String(ticks_d / GAIN_ANGLE_TO_TICKS));

        // --- Erreurs ---
        // On asservis le centre de gravité pour qu'il ne bouge pas,
        // Il faut que les ticks droits et gauches se compensent
        float erreur = ticks_g + ticks_d;

        // l'erreur peut-être négative,

        // --- Correction --
        // les signes ont étés vérifiés par Florian et Jules à minuit 09 mais soyez confiant, ils sont correctes !
        // Faites un schéma !
        // sens trigo
        int pwmD = SPEED - KP_ANGLE * erreur;
        int pwmG = -SPEED - KP_ANGLE * erreur;
        // sens horaire
        if (consigne_angle < 0)
        {
            pwmD = -SPEED - KP_ANGLE * erreur;
            pwmG = SPEED - KP_ANGLE * erreur;
        }

        pwmD = constrain(pwmD, -255, 255);
        pwmG = constrain(pwmG, -255, 255);

        // --- Commande moteurs ---
        this->set_speed(pwmD, pwmG);

        // --- Condition d’arrêt en ticks ---
        // idem faites confiance ou utilisez votre cerveau
        if (abs(consigne_angle + ticks_g / GAIN_ANGLE_TO_TICKS) < ERREUR_ANGLE && abs(consigne_angle - ticks_d / GAIN_ANGLE_TO_TICKS) < ERREUR_ANGLE)
        {
            // ON RENTRE !!
            // this->linear_stop();
            this->linear_stop();
            encodeur_g->clear_count();
            encodeur_d->clear_count();
            etape_globale++;
        }

        this->m_time_asserv = millis();
    }
}

/*
Avance d'une distance en attendant un certain temps sans aucun asservissement
Ce temps est corrigé par K_NAIF pour avoir la bonne distance
K_NAIF dépend du robot, a vous de le changer au début
*/
void Pami::avancer(float distance, int speed)
{
    if (distance < 0.)
    {
        speed = -speed;
    }

    long moving_time = K_NAIF * (abs(distance) / SPEED) * 1000;
    Serial.print("Temps estimé pour avancer de " + String(distance / 10.0) + " cm à la vitesse de " + String(speed) + " : " + String(moving_time) + " ms");

    unsigned long function_start_time = millis();
    while (millis() - function_start_time < moving_time)
    {
        float dist = this->get_IR_distance();

        if (dist < DISTANCE_MIN && dist > 0.5) // Si un obstacle est détecté à moins de 20 cm
        {
            Serial.println("Obstacle détecté ! Arrêt du robot.");
            this->linear_stop();
        }
        else
        {
            this->set_speed(speed, speed);
        }
    }
    this->linear_stop();
}

/*
Tourne d'un angle en attendant un certain temps sans aucun asservissement
Ce temps est corrigé par K_ANGLE_NAIF pour avoir le bon angle
K_ANGLE_NAIF dépend du robot, à vous de le changer au début
*/
void Pami::tourner(float angle_degres, float speed)
{
    if (angle_degres < 0)
    {
        speed = -speed;
    }

    long moving_time = K_ANGLE_NAIF * (abs(angle_degres) / 360.0) * 1000;
    Serial.print("Temps estimé pour tourner de " + String(angle_degres) + " ° à la vitesse de " + String(speed) + " : " + String(K_ANGLE_NAIF * (abs(angle_degres) / 360.0) * 1000) + " ms");

    unsigned long function_start_time = millis();
    while (millis() - function_start_time < moving_time)
    {
        float dist = this->get_IR_distance();

        if (dist < DISTANCE_MIN && dist > 0.5) // Si un obstacle est détecté à moins de 20 cm
        {
            Serial.println("Obstacle détecté ! Arrêt du robot.");
            this->linear_stop();
        }
        else
        {
            this->set_speed(speed, -speed);
        }
    }
    this->linear_stop();
}

/*
Allume les deux moteurs à une vitesse en (entre 0 et 255)
$ Make it non blocking
*/
void Pami::set_speed(int speed_d, int speed_g)
{
    for (int i = 1; i <= 10; i++)
    {
        int current_speed_d = speed_d * (i / 10.0);
        int current_speed_g = speed_g * (i / 10.0);
        moteur_d->set_speed(current_speed_d);
        moteur_g->set_speed(current_speed_g);

        delay(20);
    }

    // Sécurité : on s'assure que les moteurs sont bien à la vitesse souhaitée à la fin de la boucle
    moteur_d->set_speed(speed_d);
    moteur_g->set_speed(speed_g);
}

/*
Arrête les moteurs de manière instantanée (échelon)
*/
void Pami::stop()
{
    this->set_speed(0, 0);
}

/*
Arrête les moteurs de manière linéaire bloquante
*/
void Pami::linear_stop()
{
    int base_speed_d = moteur_d->get_speed();
    int base_speed_g = moteur_g->get_speed();

    for (int i = 10; i > 0; i--)
    {
        int current_speed_d = base_speed_d * (i / 10.0);
        int current_speed_g = base_speed_g * (i / 10.0);
        this->set_speed(current_speed_d, current_speed_g);

        delay(20);
    }

    // Sécurité : on s'assure que les moteurs sont bien à 0 à la fin de la boucle
    this->set_speed(0, 0);
}

/*
Arrête les moteurs de manière linéaire au lieu d'un échelon de manière non bloquante
*/
void Pami::non_blocking_linear_stop(bool init)
{
    static int step = 0;
    static unsigned long chrono = 0;
    static bool freinage_en_cours = false;

    // Si on déclenche l'arrêt (et qu'on ne freine pas déjà)
    if (init && !freinage_en_cours)
    {
        step = 10;
        freinage_en_cours = true;
    }

    // Si on est en train de freiner, on exécute une étape toutes les 25ms
    if (freinage_en_cours && (millis() - chrono >= 25))
    {
        chrono = millis();

        // Baisse la vitesse proportionnellement
        int current_speed_d = moteur_d->get_speed() * (step / 10.0);
        int current_speed_g = moteur_g->get_speed() * (step / 10.0);
        this->set_speed(current_speed_d, current_speed_g);

        step--;

        if (step < 0)
        {
            this->set_speed(0, 0);
            freinage_en_cours = false;
        }
    }
}

/*
Initialise la position initiale de la PAMI pour définir sa position absolue sur le terrain
Dépend du numéro de la PAMI & de l'équipe
*/
void Pami::set_initial_position()
{
    Point config = equipe_color.equalsIgnoreCase("JAUNE") ? pami_start_pos[num_pami].j : pami_start_pos[num_pami].b;
    this->pos_x = config.x;
    this->pos_y = config.y;
    this->pos_angle = 0;
}

/*
Met à jour la position absolue de la PAMI sur le terrain.
Utilise les ticks des encodeurs et la cinématique différentielle
$ Marche ?
*/
void Pami::update_mesure_position()
{
    mesure_pos->update_mesure_position();
    this->pos_x = mesure_pos->pos_x;
    this->pos_y = mesure_pos->pos_y;
    this->pos_angle = mesure_pos->pos_angle_deg;
}

/*
Fonction pour bouger le servo entre deux angles en un temps donné
*/
void Pami::blink_servo(int angle1, int angle2, long time_blink)
{
    servo->blink(angle1, angle2, time_blink);
}

/*
Fonction qui retourne la distance minimal au prochain obstacle détectée par le capteur infrarouge (ToF)
En mm
*/
double Pami::get_IR_distance()
{
    if (ir_sensor == nullptr)
    {
        Serial.println("Pas de capteur infrarouge");
        return -1;
    }
    else
    {
        ir_sensor->loop();
        return ir_sensor->ir_minimum_distance;
    }
}

/*
Affiche les valeurs des encodeurs en nombre de ticks et en cm
Faire attention car les valeurs des encodeurs sont reset après chaque action -> n'est utile que pendant un mouvement
*/
void Pami::print_encodeur()
{
    float ticks_g = encodeur_g->mesure();
    float ticks_d = encodeur_d->mesure();

    Serial.print("Encodeur gauche : " + String(ticks_g / GAIN_MM_TO_TICKS) + " mm");
    Serial.println(" | Encodeur droit : " + String(ticks_d / GAIN_MM_TO_TICKS) + " mm");
}

/*
Affiche les modifications d'interrupteur
only_changes = false -> affiche l'état de chaque interrupteur
*/
void Pami::print_changes_in_interrupteur(bool only_changes)
{
    if (digitalRead(PIN_TIRETTE) != tirette || !only_changes)
    {
        tirette = digitalRead(PIN_TIRETTE);
        Serial.println(tirette == 1 ? "Tirette en place" : "Tirette enlevée");
    }
    if (digitalRead(PIN_READEQUIPE) != equipe || !only_changes)
    {
        equipe = digitalRead(PIN_READEQUIPE);
        equipe_color = equipe == 1 ? "JAUNE" : "BLEUE";
        Serial.println("Equipe : " + equipe_color);
    }
    if (digitalRead(PIN_INT_PAMI_1) != int_pami_1 || digitalRead(PIN_INT_PAMI_2) != int_pami_2 || !only_changes)
    {
        int_pami_1 = digitalRead(PIN_INT_PAMI_1);
        int_pami_2 = digitalRead(PIN_INT_PAMI_2);
        num_pami = (int_pami_1 * 2) + int_pami_2;
        Serial.println("PAMI n°" + String(num_pami));
    }
}

/*
Affiche des logs sur la pami
*/
void Pami::print_log()
{
    Serial.println("--- Time since match started : " + String((millis() - m_time_match) / 1000) + " s ---");
    this->print_changes_in_interrupteur(false);
    Serial.println("Distance Ir: " + String(this->get_IR_distance()) + " mm");
    Serial.println("Position x: " + String(pos_x / 10.0) + " cm | Position y: " + String(pos_y / 10.0) + " cm | Angle: " + String(pos_angle) + " °\n");
    this->print_encodeur();
    Serial.println("--------------------------------------------");
}
