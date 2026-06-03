/**
 * @file Screen.h
 * @brief Classe servant à controler un écran LCD
 * Pas utilisé pour la coupe 2024
 */

/**
 * Classe écran LCD
 */
#ifndef SCREEN_H
#define SCREEN_H
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <define.h>

class Screen
{

private:
    LiquidCrystal_I2C lcd_screen;
    int m_SCL_PIN;
    int m_SDA_PIN;
    int m_LPN_PIN;

    // $ Check le 16 x 2 (dimensions écran)
    int width = 16;
    int height = 2;
    int m_ligne_actuelle = 0;

public:
    Screen(int m_sda_pin, int m_scl_pin, uint8_t lcd_I2C_addr = 0x27);

    void setup();
    void loop();

    // Affiche sur la ligne actuelle
    void print(String text);

    // Affiche sur la ligne actuelle et passe à la ligne suivante
    void println(String text);

    // Passe à la ligne suivante
    void nextline();

    // Se positionne à une ligne spécifique
    void setline(int line);

    // Efface l'écran et remet le curseur en haut à gauche
    void clear();
};

#endif