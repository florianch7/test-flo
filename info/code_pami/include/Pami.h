/**
 * @file Pami.h
 * @brief Classe pour controler une PAMI
 *
 */

#ifndef PAMI_H
#define PAMI_H

#include <Arduino.h>
#include <Encodeur.h>
#include <Moteur.h>
#include <Irsensor.h>
#include <Ultrason.hpp>
#include <Serv.h>
#include <define.h>

class Pami
{
public:
    Moteur *moteur_d;
    Moteur *moteur_g;
    Encodeur *encodeur_d;
    Encodeur *encodeur_g;
    Serv *servo;
    Ultrason *ultrason;
    Irsensor *ir_sensor;

    int tirette = 1;                                  // Etat par défaut de la tirette
    int equipe = 0;                                   // Equipe par défaut (1 = gauche = jaune)
    int int_pami_1 = 0;                               // Etat interrupteur 1
    int int_pami_2 = 0;                               // Etat interrupteur 2
    int num_pami = (int_pami_1 * 2) + int_pami_2 + 1; // Numéro de la pami

    long m_time_log;
    long m_time_match;

    Pami(Moteur *p_moteur_d, Moteur *p_moteur_g, Encodeur *p_encodeur_d, Encodeur *p_encodeur_g, Serv *p_servo, Irsensor *p_ir_sensor = nullptr, Ultrason *p_ultrason = nullptr);

    // $ Fonctions de test pour chaque élement (à faire)
    void test(int mode);

    // Fonctions non bloquantes qui nécessite un numéro pour être appelée dans l'ordre
    // Sinon elles s'executeraient toutes en même temps
    // $ Faire PID & asserv rcva (à faire)
    unsigned long avancer_asservi(int etape_d_appel, float consigne_cm, unsigned long oldtime);
    unsigned long tourner_asservi(int etape_d_appel, float consigne_angle, unsigned long oldtime);

    // Avancer basiquement sans asserv
    void avancer(float distance, int speed = SPEED);
    void tourner(float angle_degres, float speed = SPEED);
    void go_to(float distance_x, float distance_y, int speed = SPEED);

    // $ Allume (TODO) et éteint les moteurs avec un trapèze i.e une rampe (à faire)
    void set_speed(int speed_d, int speed_g);
    void stop();

    void blink_servo(int angle1, int angle2, long temps_blink = TEMPS_BLINK);
    double get_IR_distance();

    // $ Ecran (à faire)
    void afficher_ecran(String ligne1);

    // $ Potentiomètre (à faire)
    int get_numero_pami(int pin);

    // Fonctions de log
    void print_log();
    void print_encodeur();
    void print_infos_interrupteur();
};

#endif
