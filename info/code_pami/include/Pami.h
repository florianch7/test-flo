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
#include <Mesure_pos.h>

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
    Mesure_pos *mesure_pos;

    // Etat de la PAMI
    int tirette = 1;                                       // Etat par défaut de la tirette
    int equipe = 0;                                        // Equipe par défaut (1 = gauche = jaune)
    String equipe_color = equipe == 1 ? "JAUNE" : "BLEUE"; // Couleur de l'équipe
    int int_pami_1 = 0;                                    // Etat interrupteur 1
    int int_pami_2 = 0;                                    // Etat interrupteur 2
    int num_pami = (int_pami_1 * 2) + int_pami_2;          // Numéro de la pami (entre 0 & 3)

    // Temps pour les fréquences d'éxecution
    long m_time_log;
    long m_time_match;
    long m_time_asserv;
    long m_time_position;

    // Position absolue de la PAMI
    float pos_x;
    float pos_y;
    float pos_angle;

    Pami(Moteur *p_moteur_d, Moteur *p_moteur_g, Encodeur *p_encodeur_d, Encodeur *p_encodeur_g, Mesure_pos *mesure_pos, Serv *p_servo, Irsensor *p_ir_sensor = nullptr, Ultrason *p_ultrason = nullptr);

    // Fonctions de test pour chaque élément
    // Modes: 1=Interrupteurs, 2=CapteurIR, 3=Servo, 4=Moteurs&Encodeurs, 5=MouvementComplet, 6=Homologation, 7=GainsAsserv, 8=GainsNaifs
    void test(int mode);

    // Permet d'asservir le robot a partir d'une liste de mouvements ou un seul
    // $ Faire PID
    void asserv_list(float mouvements[][2], int nb_mvt);
    void asserv(float consigne_distance_mm, float consigne_angle_degre);

    // Permet de faire des lignes droites ou des rotations sur lui même avec asservissement
    // Le n° d'appel permet de faire une suite d'action non bloquante
    void ligne_droite(int etape_d_appel, float consigne_mm);
    void rotation(int etape_d_appel, float consigne_angle);

    // Avancer/tourner basiquement sans asserv
    void avancer(float distance, int speed = SPEED);
    void tourner(float angle_degres, float speed = SPEED);

    // Fonctions de contrôle des moteurs
    void set_speed(int speed_d, int speed_g);
    void non_blocking_linear_stop(bool init = false); // $ Weird to use - make it easier ? (how)
    void linear_stop();
    void stop();

    // Renvoie la distance au prochain obstacle en mm du capteur IR
    double get_IR_distance();

    // Initialise la position initiale de la PAMI pour définir sa position absolue sur le terrain
    void set_initial_position();
    void update_mesure_position();

    // $ Potentiomètre (à faire)
    int get_numero_pami();

    // Fonctions de log
    void print_log();
    void print_encodeur();
    void print_changes_in_interrupteur(bool only_changes = true);
};

#endif