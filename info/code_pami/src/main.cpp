/**
 * @file ain.cpp
 * @brief Programme principal , a implementer dans la pami
 */

#include <Pami.h>

// Initialise les différents objets
Serv servo = Serv(SERVPIN);
Irsensor ir_sensor = Irsensor(IR_SDA_PIN, IR_SCL_PIN, IR_LPN_PIN);
Moteur moteur_d = Moteur(EN_R, IN1_R, IN2_R, INV_MOT_R);
Moteur moteur_g = Moteur(EN_L, IN1_L, IN2_L, INV_MOT_L);
Encodeur encodeur_d = Encodeur(CLK_R, DT_R, INV_ENC_R);
Encodeur encodeur_g = Encodeur(CLK_L, DT_L, INV_ENC_L);
Mesure_pos mesure_pos = Mesure_pos(&encodeur_g, &encodeur_d);

int etape_globale = 0;

// La pami en elle même
Pami pami = Pami(&moteur_d, &moteur_g, &encodeur_d, &encodeur_g, &mesure_pos, &servo, &ir_sensor);

void setup()
{
    Serial.begin(9600);
    Serial.println("---------- Setup starting ----------");

    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, HIGH); // LED ON pour indiquer le début du setup

    // Setup capteur IR
    if (&ir_sensor != nullptr)
    {
        ir_sensor.setup();
        Serial.println("Setup Done : IR Sensor");
    }

    // Setup servo
    servo.setup();
    Serial.println("Setup Done : Servo");

    // Setup moteur droit & gauche
    moteur_d.setup();
    moteur_g.setup();
    Serial.println("Setup Done : Moteurs");

    // Setup encodeur droit & gauche
    encodeur_d.setup();
    encodeur_g.setup();
    Serial.println("Setup Done : Encodeurs");

    // Mesure position
    mesure_pos.setup();
    Serial.println("Setup Done : Mesure position");

    pinMode(PIN_TIRETTE, INPUT);
    pinMode(PIN_READEQUIPE, INPUT);
    pinMode(PIN_INT_PAMI_1, INPUT);
    pinMode(PIN_INT_PAMI_2, INPUT);
    Serial.println("Setup Done : Tirette & Equipe & PAMI");

    Serial.println("\n---------- Setup over ----------\n\n");
    digitalWrite(PIN_LED, LOW);
    delay(500);

    unsigned long last_read = 0;

    // false permet d'afficher les infos actuelles
    pami.print_changes_in_interrupteur(false);

    while (digitalRead(PIN_TIRETTE) == 1)
    {
        if (millis() - last_read > 500)
        {
            pami.print_changes_in_interrupteur();
            last_read = millis();
        }
        delay(10);
    }

    // On remet a 0 les positions car la roue tourne pendant l'upload
    encodeur_g.clear_count();
    encodeur_d.clear_count();

    pami.set_initial_position();

    // Temps du match, des logs, de l'asserv, de la position
    pami.m_time_log = millis();
    pami.m_time_match = millis();
    pami.m_time_asserv = millis();
    pami.m_time_position = millis();
}

// Liste des mouvemnts à réaliser : {distance en mm, angle en degrés}
float mouvements[][2] = {
    {300, 0}, // 0 : avancer 300 mm
    // {0, 45},  // 1 : tourner 90°
    // {-30, 0}, // 2 : reculer 300 mm
    // {150, 0}, // 3 : avancer 50 mm
};
int nb_mouvements = sizeof(mouvements) / sizeof(mouvements[0]);

void loop()
{
    pami.update_mesure_position();

    // Condition de fin de match
    if (millis() - pami.m_time_match >= END_TIME)
    {
        Serial.println("Temps de match écoulé - Arrêt du robot.");
        pami.linear_stop();
        while (true)
        {
            pami.blink_servo(0, 90);
        }
    }

    float dist = pami.get_IR_distance();
    if (dist < DISTANCE_MIN && dist > 0.5) // Si un obstacle est détecté à moins de DISTANCE_MIN cm
    {
        Serial.println("Obstacle détecté ! Arrêt du robot.");
        pami.linear_stop();
        return;
    }

    if ((millis() - pami.m_time_match) > START_TIME && (millis() - pami.m_time_match) < END_TIME)
    {
        // Si on veut une suite de mouvements asservis curvilignes
        pami.asserv_list(mouvements, nb_mouvements);

        // Si on veut aller d'un point A à un point B en faisant (avancer - tourner - avancer)
        // if (pami.equipe_color == "JAUNE")
        // {
        //     pami.avancer(600);
        //     pami.tourner(90);
        //     pami.avancer(200);
        // }
        // else
        // {
        //     pami.avancer(800);
        //     pami.tourner(-90);
        //     pami.avancer(300);
        // }
    }

    if (millis() - pami.m_time_log >= PERIODE_LOG)
    {
        // pami.print_log();
        pami.m_time_log = millis();
    }
}