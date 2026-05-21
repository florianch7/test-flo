#include <Pami.h>

Pami::Pami(Moteur *p_moteur_d, Moteur *p_moteur_g, Encodeur *p_encodeur_d, Encodeur *p_encodeur_g, Serv *p_servo, Irsensor *p_ir_sensor, Ultrason *p_ultrason)
{
    moteur_d = p_moteur_d;
    moteur_g = p_moteur_g;
    encodeur_d = p_encodeur_d;
    encodeur_g = p_encodeur_g;
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
4 =
5 = Encodeurs & Odométrie (À pousser à la main)
6 = Moteurs Individuels (Puissance brute)
7 = Homologation : Avance et s'arrête en fonction du capteur IR
8 = Avancer, reculer, tourner (Sans asservissement, juste pour voir si les fonctions de base marchent &  régler les gains)
*/
void Pami::test(int mode)
{
    Serial.print("\n========== LANCEMENT DU TEST MODE : ");
    Serial.print(mode);
    Serial.println(" ==========");

    switch (mode)
    {
    case 1: // --- TEST 1 : TIRETTE & INTERRUPTEURS ---
    {
        Serial.println("Test Interrupteurs... Modifiez leurs etats ! (Boucle infinie)");
        while (true)
        {
            this->print_infos_interrupteur();
            Serial.println("-------------------------");
            delay(1000); // On attend 1s pour ne pas spammer le terminal
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
        Serial.println("Test Servomoteur : Va-et-vient de 3 secondes");
        while (true)
        {
            this->blink_servo(0, 90);
        }
        Serial.println("Fin du test Servomoteur.");
        break;
    }
    case 4: // --- Test 4 : GAINS ASSERVISSEMENT ---
    {
        float time = millis();
        float oldtime = time;
        while (true)
        {
            this->avancer_asservi(0, 5000);
            this->print_encodeur();
            oldtime = time;
        }
        break;
    }
    case 5: // Essais roue droite & gauche indépendament
    {
        Serial.println("Test Moteurs Individuels : Attention, le robot va tester chaque roue indépendamment !");

        while (true)
        {
            Serial.println("\n-> Test Roue Droite (Vitesse 200)");
            moteur_d->set_speed(SPEED);
            moteur_g->stop();
            this->print_encodeur();
            delay(1500);

            Serial.println("\n-> Arret");
            moteur_d->stop();
            delay(1500);

            Serial.println("\n-> Test Roue Gauche (Vitesse 200)");
            moteur_d->stop();
            moteur_g->set_speed(SPEED);
            this->print_encodeur();
            delay(1500);

            Serial.println("\n-> Arret Definitif");
            moteur_d->stop();
            moteur_g->stop();
            delay(1500);
            Serial.println("Fin du test Moteurs Individuels.");
        }
        break;
    }
    case 7: // Homologation
    {
        Serial.println("Homologation : Le robot avance et s'arrête lorsque le capteur IR détecte un obstacle à moins de 5 cm (Boucle infinie)");

        while (true)
        {
            float dist = this->get_IR_distance();
            Serial.print("Distance mesuree : ");
            Serial.print(dist / 10.0);
            Serial.println(" cm");

            if (dist < DISTANCE_MIN && dist > 0.5) // Si un obstacle est détecté à moins de 20 cm
            {
                Serial.println("Obstacle détecté ! Arrêt du robot.");
                this->stop();
            }
            else
            {
                this->set_speed(SPEED, SPEED);
            }
            delay(200);
        }
        break;
    }
    case 8:
    {
        Serial.println("Test Avancer/Reculer/Tourner... Attention, le robot va avancer, reculer puis tourner !");

        this->avancer(100);
        delay(2000);
        this->tourner(180);
    }

    default:
    {
        Serial.println("Erreur : Mode de test inconnu ! (Choisissez entre 1 et 6)");
        break;
    }
    }
}

/*
Asservissement style rcva - fonction test
*/
void Pami::asserv_list(float mouvements[][2], int nb_mvt)
{
    static int num_current_mvt = 0;
    static int pos_ref_distance = 0;
    static int pos_ref_angle = 0;

    if (millis() - this->m_time_asserv >= INTERVAL_ASSERV)
    {
        float consigne_distance_mm = mouvements[num_current_mvt][0];
        float consigne_angle_degre = mouvements[num_current_mvt][1];

        // Serial.print("Consigne distance (mm) : " + String(consigne_distance_mm));
        // Serial.println(" | Consigne angle (°) : " + String(consigne_angle_degre));

        // Serial.println("---------");

        int pos_encodeur_d = encodeur_d->mesure();
        int pos_encodeur_g = encodeur_g->mesure();

        Serial.print("Pos_ref distance (mm) : " + String(pos_ref_distance / GAIN_MM_TO_TICKS));
        Serial.println(" | Pos_ref angle (°) : " + String(pos_ref_angle / GAIN_ANGLE_TO_TICKS));

        // Calculer la distance et l'angle du mouvement actuel en ticks
        // On repart de l'origin à chaque nouveau mouvement, donc on soustrait la position de référence qui est la position d'arrivée du précédent
        float distance_actuelle_ticks = (pos_encodeur_d + pos_encodeur_g) / 2.0 - pos_ref_distance;
        float angle_actuel_ticks = (pos_encodeur_d - pos_encodeur_g) - pos_ref_angle;

        Serial.print("Distance actuelle (mm) : " + String(distance_actuelle_ticks / GAIN_MM_TO_TICKS));
        Serial.println(" | Angle actuel (°) : " + String(angle_actuel_ticks / GAIN_ANGLE_TO_TICKS));

        // Calculer les erreurs en ticks
        float erreur_distance_ticks = GAIN_MM_TO_TICKS * consigne_distance_mm - distance_actuelle_ticks;
        float erreur_angle_ticks_ticks = GAIN_ANGLE_TO_TICKS * consigne_angle_degre - angle_actuel_ticks;

        Serial.print("erreur distance (mm) : " + String(erreur_distance_ticks / GAIN_MM_TO_TICKS));
        Serial.println(" | erreur angle (°) : " + String(erreur_angle_ticks_ticks / GAIN_ANGLE_TO_TICKS));

        Serial.println("---------");

        // Appliquer les corrections proportionnelles
        float correction_distance = KP_DISTANCE * erreur_distance_ticks;
        float correction_angle = KP_ANGLE * erreur_angle_ticks_ticks;

        // Convertir les corrections en commandes moteur
        int commande_moteur_d = (int)(correction_distance + correction_angle);
        int commande_moteur_g = (int)(correction_distance - correction_angle);

        // Limiter les commandes entre -255 et 255
        commande_moteur_d = constrain(commande_moteur_d, -255, 255);
        commande_moteur_g = constrain(commande_moteur_g, -255, 255);

        if (abs(consigne_distance_mm - distance_actuelle_ticks / GAIN_MM_TO_TICKS) < ERREUR_DISTANCE && abs(consigne_angle_degre - angle_actuel_ticks / GAIN_ANGLE_TO_TICKS) < MARGE_ERREUR_TICKS)
        {
            if (num_current_mvt < nb_mvt - 1)
            {
                num_current_mvt++;

                // On récupère la position actuelle pour la prendre comme origine au prochain tour
                pos_ref_distance = distance_actuelle_ticks;
                pos_ref_angle = angle_actuel_ticks;
            }
        }

        // Envoyer les commandes aux moteurs
        this->set_speed(commande_moteur_d, commande_moteur_g);

        this->m_time_asserv = millis();
    }
}

/*
Asservissement style rcva - fonction test
*/
void Pami::asserv(float consigne_distance_mm, float consigne_angle_degre)
{
    if (millis() - this->m_time_asserv >= INTERVAL_ASSERV)
    {
        int pos_encodeur_d = encodeur_d->mesure();
        int pos_encodeur_g = encodeur_g->mesure();

        // Calculer la distance et l'angle actuels
        float distance_actuelle_ticks = (pos_encodeur_d + pos_encodeur_g) / 2.0;
        float angle_actuel_ticks = (pos_encodeur_d - pos_encodeur_g);

        // Calculer les erreurs
        float erreur_distance_ticks = consigne_distance_mm * GAIN_MM_TO_TICKS - distance_actuelle_ticks;
        float erreur_angle_ticks = consigne_angle_degre * GAIN_ANGLE_TO_TICKS - angle_actuel_ticks;

        // Appliquer les corrections proportionnelles
        float correction_distance = KP_DISTANCE * erreur_distance_ticks;
        float correction_angle = KP_ANGLE * erreur_angle_ticks;

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
    if (etape_d_appel == etape_globale && (millis() - this->m_time_asserv) >= INTERVAL_ASSERV)
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
        int pwmD = SPEED + KP * erreur;
        int pwmG = SPEED - KP * erreur;
        // si on recule, on remplace les valeurs
        if (consigne_mm < 0)
        {
            pwmD = -SPEED + KP * erreur;
            pwmG = -SPEED - KP * erreur;
        }

        pwmD = constrain(pwmD, -255, 255);
        pwmG = constrain(pwmG, -255, 255);

        // --- Commande moteurs ---
        this->set_speed(pwmD, pwmG);

        // --- Condition d’arrêt en ticks ---
        if (abs(consigne_mm * GAIN_MM_TO_TICKS - ticks_g) < MARGE_ERREUR_TICKS && abs(consigne_mm * GAIN_MM_TO_TICKS - ticks_d) < MARGE_ERREUR_TICKS)
        {
            // ON RENTRE & on nettoie les encodeurs !!
            this->set_speed(0, 0);
            // this->stop();
            encodeur_g->clear_count();
            encodeur_d->clear_count();
            etape_globale++;
        }

        this->m_time_asserv = millis();
    }
    /* But du gain proportionnel : faire une correction proportionnelle à l'erreur.
        En gros :
        erreur = ticksG - ticksD
        correction = Kp * erreur

        Puis on ajuste le pwm :
        pwmG = pwmBase - correction
        pwmD = pwmBase + correction
    */
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
    if (etape_d_appel == etape_globale && (millis() - this->m_time_asserv) >= INTERVAL_ASSERV)
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
        int pwmD = SPEED - KP * erreur;
        int pwmG = -SPEED - KP * erreur;
        // sens horaire
        if (consigne_angle < 0)
        {
            pwmD = -SPEED - KP * erreur;
            pwmG = SPEED - KP * erreur;
        }

        pwmD = constrain(pwmD, -255, 255);
        pwmG = constrain(pwmG, -255, 255);

        // --- Commande moteurs ---
        this->set_speed(pwmD, pwmG);

        // --- Condition d’arrêt en ticks ---
        // idem faites confiance ou utilisez votre cerveau
        if (abs(consigne_angle * GAIN_ANGLE_TO_TICKS + ticks_g) < MARGE_ERREUR_TICKS && abs(consigne_angle * GAIN_ANGLE_TO_TICKS - ticks_d) < MARGE_ERREUR_TICKS)
        {
            // ON RENTRE !!
            // this->set_speed(0, 0);
            this->stop();
            encodeur_g->clear_count();
            encodeur_d->clear_count();
            etape_globale++;
        }

        this->m_time_asserv = millis();
    }
}

/*
Fonctions de déplacement basiques (sans asservissement, juste pour tester les fonctions de base et régler les gains K_NAIF et K_ANGLE_NAIF
*/
void Pami::go_to(float distance_x, float distance_y, int speed)
{
    this->avancer(distance_x, speed);
    delay(200);
    this->tourner(90, speed);
    delay(200);
    this->avancer(distance_y, speed);
    delay(200);
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
            this->stop();
        }
        else
        {
            this->set_speed(speed, speed);
        }
    }
    this->stop();
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
            this->stop();
        }
        else
        {
            this->set_speed(speed, -speed);
        }
    }
    this->stop();
}

/*
Arrête les moteurs de manière linéaire au lieu d'un échelon
Est bloquante
*/
void Pami::stop()
{
    moteur_d->stop();
    moteur_g->stop();
}

/*
Allume les deux moteurs à une vitesse en (entre 0 et 255)
$ TO DO : Faire un trapèze
*/
void Pami::set_speed(int speed_d, int speed_g)
{
    moteur_d->set_speed(speed_d);
    moteur_g->set_speed(speed_g);
}

/*
Initialise la position initiale de la PAMI pour définir sa position absolue sur le terrain
Dépend du numéro de la PAMI & de l'équipe
*/
void Pami::set_initial_position()
{
    // $ Faire du if/else pour equipe & n° pami
    this->pos_x = J_POSITION_1_DEPART_X;
    this->pos_y = J_POSITION_1_DEPART_Y;
    // A modifier si angle de départ différent de 0
    this->pos_angle = 0;
}

/*
Fonction pour bouger le servo entre deux angles en un temps donné
*/
void Pami::blink_servo(int angle1, int angle2, long blink_time)
{
    servo->blink(angle1, angle2, blink_time);
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
    if (millis() - m_time_log > 250)
    {
        float ticks_g = encodeur_g->mesure();
        float ticks_d = encodeur_d->mesure();

        Serial.print("Encodeur gauche : " + String(ticks_g / GAIN_MM_TO_TICKS) + " mm");
        Serial.println(" | Encodeur droit : " + String(ticks_d / GAIN_MM_TO_TICKS) + " mm");
    }
}

void Pami::update_position()
{
    // A faire : utiliser les ticks des encodeurs pour calculer la position absolue de la PAMI sur le terrain
    // En utilisant la cinématique différentielle et en intégrant les mouvements au cours du temps
    // On peut aussi utiliser les données du capteur IR pour corriger la position si un obstacle est détecté
    //$ Trouver la formule pour calculer la position (x, y, theta) à partir des ticks des encodeurs
    // pami.pos_x = encodeur_d->mesure() / GAIN_MM_TO_TICKS;
    // pami.pos_y = encodeur_g->mesure() / GAIN_MM_TO_TICKS;
    // pami.pos_angle = encodeur_d->mesure() / GAIN_MM_TO_TICKS;
}

/*
Affiche les modifications d'interrupteur
*/
void Pami::print_infos_interrupteur()
{
    if (digitalRead(PIN_TIRETTE) != tirette)
    {
        tirette = digitalRead(PIN_TIRETTE);
        Serial.print(tirette == 1 ? "Tirette en place \n" : "Tirette enlevée \n");
    }
    if (digitalRead(PIN_READEQUIPE) != equipe)
    {
        equipe = digitalRead(PIN_READEQUIPE);
        Serial.print(equipe == 1 ? "Equipe : JAUNE \n" : "Equipe : BLEUE \n");
    }
    if (digitalRead(PIN_INT_PAMI_1) != int_pami_1 || digitalRead(PIN_INT_PAMI_2) != int_pami_2)
    {
        int_pami_1 = digitalRead(PIN_INT_PAMI_1);
        int_pami_2 = digitalRead(PIN_INT_PAMI_2);
        num_pami = (int_pami_1 * 2) + int_pami_2 + 1;
        Serial.print("PAMI n°" + String(num_pami) + "\n");
    }
}

/*
Affiche des logs sur la pami
*/
void Pami::print_log()
{
    if (m_time_log + 500 < millis()) // Log toutes les secondes
    {
        Serial.println("--- Time since match started : " + String((millis() - m_time_match) / 1000) + " s ---");
        Serial.println("Etape des fonctions non bloquantes : " + String(etape_globale));
        Serial.println("Distance Ir: " + String(this->get_IR_distance()) + " mm");
        this->print_infos_interrupteur();
        this->print_encodeur();

        m_time_log = millis();
    }
}
