/**
 * @file Moteur.h
 * @brief Classe pour controler un moteur
 *
 */

#include <Arduino.h>

#ifndef MOTEUR_H
#define MOTEUR_H

class Moteur
{
private:
    int m_EN;   // pin de la pwm
    int m_IN1;  // pin de direction 1
    int m_IN2;  // pin de direction 2
    bool m_inv; // flag pour l'inversion de la direction
    long m_vitesse;
    // Facteur pour passer d'un entier entre 0 ET 255 à une vitesse en cm/s
    float K_conv;

    // Variables pour le freinage linéaire
    int step_freinage = 10;

public:
    /**
     * @brief met la vitesse du moteur à vitesse
     *
     * @param vitesse : speed of the motor is a int between 0 and 255
     */
    void set_speed(int vitesse);
    /**
     * @brief Arrête le moteur de manière linéaire
     */
    void linear_stop();

    /**
     * @brief Retourne la dernière vitesse envoyée au moteur
     * @return La vitesse du moteur
     */
    long get_speed();

    /**
     * @brief constructeur
     */
    Moteur(int EN, int IN1, int IN2, bool inv = false);
    /**
     * @brief Initialisation du moteur
     */
    void setup();
    void loop();
};

#endif