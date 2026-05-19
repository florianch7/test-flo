#include <Wire.h>
#include <Arduino.h>
#include <SparkFun_VL53L5CX_Library.h>
#ifndef IRSENSOR_H
#define IRSENSOR_H

class Irsensor
{

private:
    int m_SCL_PIN;
    int m_SDA_PIN;
    int m_LPN_PIN;
    long m_dt = 10;

public:
    SparkFun_VL53L5CX myImager;
    VL53L5CX_ResultsData measurementData; // Result data class structure, 1356 byes of RAM
    volatile bool dataReady = false;      // Goes true when interrupt fires
    int imageResolution = 0;              // Used to pretty print output
    int imageWidth = 0;                   // Used to pretty print output

    // Constructor
    Irsensor(int SDA_PIN, int SCL_PIN, int LPN_PIN = 0);

    int ir_minimum_distance = 1000; // Distance absolue que l'on récupère du capteur
    int vision[8];                  // liste des 8 distances moyennes des capteurs de gauche à droite
    long m_time;

    /**
     * @brief Fonction d'interruption
     * ne marche pas pour le moment
     */
    void interruptRoutine(); // TODO : A voir si on l'utilise ? la faire marcher si on a le temps

    /**
     * @brief Initialisation du capteur IR
     * Si ne marche pas, affiche une erreur et s'arrête
     */
    void setup();

    /**
     * @brief Boucle de lecture du capteur IR, met à jour la distance
     */
    void loop();
};

#endif