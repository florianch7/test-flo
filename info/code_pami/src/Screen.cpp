#include "Screen.h"

Screen::Screen(int m_sda_pin, int m_scl_pin, uint8_t lcd_I2C_addr)
    : lcd_screen(lcd_I2C_addr, this->width, this->height)
{
    this->m_SDA_PIN = m_sda_pin;
    this->m_SCL_PIN = m_scl_pin;
}

void Screen::setup()
{
    Wire.begin(m_SDA_PIN, m_SCL_PIN);
    lcd_screen.init();
    lcd_screen.backlight();
    lcd_screen.setCursor(0, 0);

    this->m_ligne_actuelle = 0;
}

// Continue d'écrire après ce qui était déjà écrit
void Screen::print(String text)
{
    // On ajoute des espaces pour effacer les anciens caractères
    String texte_propre = text + "                ";
    lcd_screen.print(texte_propre.substring(0, 16));
}

void Screen::println(String text)
{
    lcd_screen.setCursor(0, m_ligne_actuelle);

    // On ajoute des espaces pour effacer les anciens caractères
    String texte_propre = text + "                ";
    lcd_screen.print(texte_propre.substring(0, 16));

    this->nextline();
}

void Screen::nextline()
{
    // Car on a que 2 lignes
    m_ligne_actuelle = 1 - m_ligne_actuelle;
    lcd_screen.setCursor(0, m_ligne_actuelle);
}

void Screen::setline(int line)
{
    if (line < 0 || line >= this->height)
    {
        line = 0;
    }
    m_ligne_actuelle = line;
    lcd_screen.setCursor(0, line);
}

void Screen::clear()
{
    lcd_screen.clear();
    m_ligne_actuelle = 0;
    lcd_screen.setCursor(0, 0);
}

void Screen::loop()
{
}