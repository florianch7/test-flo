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
            time = this->avancer_asservi(0, 5000, oldtime);
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
Mets à jour tous les interrupteurs pour l'équipe & n°PAMI
*/
void Pami::update_interrupteur()
{
    // Equipe de la PAMI
    int read_equipe = digitalRead(PIN_READEQUIPE);
    // Position initiale de la PAMI avec deux interrupteurs;
    int int_pami_1 = digitalRead(PIN_INT_PAMI_1);
    int int_pami_2 = digitalRead(PIN_INT_PAMI_2);
    int read_num_pami = (int_pami_1 * 2) + int_pami_2 + 1;
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
Fonction pour bouger le servo entre deux angles en un temps donné
*/
void Pami::blink_servo(int angle1, int angle2, long temps_blink)
{
    servo->blink(angle1, angle2, temps_blink);
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

        Serial.println("Encodeur gauche : " + String(ticks_g) + " ticks & " + String(ticks_g / GAIN_CM_TO_TICKS) + " cm");
        Serial.println(" | Encodeur droit : " + String(ticks_d) + " ticks & " + String(ticks_d / GAIN_CM_TO_TICKS) + " cm");
    }
}

/*
Affiche des logs sur la pami
*/
void Pami::print_log()
{
    if (m_time_log + 500 < millis()) // Log toutes les secondes
    {
        Serial.println("------------- Time since match started : " + String((millis() - m_time_match) / 1000) + " s -------------");
        Serial.println("Etape des fonctions non bloquantes : " + String(etape_globale));
        Serial.println("Distance Ir: " + String(this->get_IR_distance()) + " mm");
        this->print_infos_interrupteur();

        m_time_log = millis();
    }
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
Avancer en ligne droite, on veut que chaque moteur avance de consigne_angle cm
Recule si consigne_cm < 0
*/
unsigned long Pami::avancer_asservi(int etape_d_appel, float consigne_cm, unsigned long oldtime)
{
    // Si c'est pas l'étape à laquelle on veut l'appeler,
    // aucune des variables du main n'est modifiée
    if (etape_d_appel != etape_globale)
    {
        return oldtime;
    }
    /* But du gain proportionnel : faire une correction proportionnelle à l'erreur.
        En gros :
        erreur = ticksG - ticksD
        correction = Kp * erreur

        Puis on ajuste le pwm :
        pwmG = pwmBase - correction
        pwmD = pwmBase + correction
    */

    // Si l’intervalle n’est pas écoulé -> on ne fait rien
    if (millis() - oldtime < INTERVAL_ASSERV)
    {
        return oldtime;
    }
    else
    {
        // Sinon, c'est qu'on vient de dépasser l'invervalle d'asservissement.
        // il faut donc asservir de nouveau

        // --- Mesures actuelles ---
        float ticks_d = encodeur_d->mesure();
        float ticks_g = encodeur_g->mesure();

        // Serial.print("Roue droite (cm) : " + String(ticks_d / GAIN_CM_TO_TICKS));
        // Serial.println(" | Roue gauche (cm) : " + String(ticks_g / GAIN_CM_TO_TICKS));

        // --- Erreurs ---
        // l'erreur peut-être négative
        float erreur = ticks_g - ticks_d;

        // --- Correction --
        // si on avance
        int pwmD = SPEED + KP * erreur;
        int pwmG = SPEED - KP * erreur;
        // si on recule, on remplace les valeurs
        if (consigne_cm < 0)
        {
            pwmD = -SPEED + KP * erreur;
            pwmG = -SPEED - KP * erreur;
        }

        pwmD = constrain(pwmD, -255, 255);
        pwmG = constrain(pwmG, -255, 255);

        // --- Commande moteurs ---
        this->set_speed(pwmD, pwmG);

        // --- Condition d’arrêt en ticks ---
        if (abs(consigne_cm * GAIN_CM_TO_TICKS - ticks_g) < MARGE_ERREUR_TICKS && abs(consigne_cm * GAIN_CM_TO_TICKS - ticks_d) < MARGE_ERREUR_TICKS)
        {
            // ON RENTRE & on nettoie les encodeurs !!
            // this->set_speed(0, 0);
            this->stop();
            encodeur_g->clear_count();
            encodeur_d->clear_count();
            etape_globale++;
        }

        // On renvoie les nouvelles valeurs
        return millis();
    }
}

/*
Tourne sur lui même, on veut que chaque moteur avance de consigne_angle cm dans des sens opposés
consigne angle > 0 = sens trigo
consigne angle < 0 = sens horaire
*/
unsigned long Pami::tourner_asservi(int etape_d_appel, float consigne_angle, unsigned long oldtime)
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
    if (etape_d_appel != etape_globale)
    {
        return oldtime;
    }
    // Si l’intervalle n’est pas écoulé -> on ne fait rien
    if (millis() - oldtime < INTERVAL_ASSERV)
    {
        return oldtime;
    }
    else
    {
        // Sinon, c'est qu'on vient de dépasser l'invervalle d'asservissement.
        // il faut donc asservir de nouveau

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

        // On renvoie les nouvelles valeurs
        return millis();
    }
}
