/**
 * @file Serv.h
 * @brief Classe servant à controler un servo moteur
 * Pas utilisé pour la coupe 2024
 */

/**
 * Classe servo
 */
#ifndef SERV_H
#define SERV_H
#include <Arduino.h>
#include <ESP32Servo.h>
#include <define.h>

class Serv
{

private:
    Servo servo;
    int etat;
    int pin;
    long temps_servo;

public:
    Serv(int pin);

    /* Fait bouger le servomoteur entre deux angles
      - Temps en ms
      - Angles en degrés
    */
    void blink(int angle1, int angle2, long blink_time = BLINK_TIME);
    void setup();
    void loop();
};

#endif