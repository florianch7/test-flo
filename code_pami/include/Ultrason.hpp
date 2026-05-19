/**
 * @file Ultrason.h
 * @brief Classe servant à controler un capteur ultrason
 */

/**
 * Classe ultrason
 */
#ifndef ULTRASON_H
#define ULTRASON_H
#include <Arduino.h>
#include <ESP32Servo.h>

class Ultrason
{
private:
    int m_echo_PIN;            // Broche Echo du HC-SR04
    int m_trigger_PIN;         // Broche Trigger du HC-SR04
    double m_duration;         // Durée de l'echo
    long m_dt = 10;            // Délai entre les lectures (10ms)
    bool m_reading = false;    // Etat pour indiquer si on est en train de lire ou pas
    long m_t_reading = 0;      // Pour mesurer le temps depuis que l'acquisition est lancé
    long max_duration = 50000; // Durée max de l'echo

public:
    double m_distance; // Distance mesurée
    double m_time;

    Ultrason(int m_echo_PIN, int m_trigger_PIN); // Constructeur

    /**
     * @brief Fonction d'initialisation du capteur ultrason
     */
    void setup();

    /**
     * @brief Boucle de lecture du capteur ultrason, met à jour la distance
     */
    void loop();
};

#endif