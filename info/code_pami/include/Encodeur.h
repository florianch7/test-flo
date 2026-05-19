#include <ESP32Encoder.h>
#include <Arduino.h>

#ifndef ENCODEUR_H
#define ENCODEUR_H

class Encodeur
{
private:
    ESP32Encoder encoder;
    int m_clk;   // Pin clock de l'encodeur
    int m_dt;    // Pin dt de l'encodeur
    bool m_inv;  // Inversion du sens de l'encodeur
    long m_time; // temps rafraichissement mesure

public:
    /**
     * @brief Constructeur
     */
    Encodeur(int clk, int dt, bool inv = false);

    /**
     * @brief Initialisation de l'encodeur
     */
    void setup();

    /**
     * @brief Remet le compteur de l'encodeur à zéro
     */
    void clear_count();

    /**
     * @brief Boucle de l'encodeur, ne sert que pour tester l'encodeur
     */
    void loop();

    /**
     * @brief Mesure vitesse roue
     * @return le nombre de tics de roue de l'encodeur
     */
    long mesure();
};

#endif
