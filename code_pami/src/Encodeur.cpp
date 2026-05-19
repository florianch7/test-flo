#include <Encodeur.h>

Encodeur::Encodeur(int clk, int dt, bool inv)
{
    m_clk = clk;
    m_dt = dt;
    m_inv = inv;
}

void Encodeur::setup()
{
    m_time = millis();
    encoder.attachHalfQuad(m_dt, m_clk);
    encoder.setCount(0);
}

long Encodeur::mesure()
{
    return (m_inv == true) ? -encoder.getCount() : encoder.getCount();
}

void Encodeur::clear_count()
{
    encoder.setCount(0); // Remet le compteur physique à zéro
}

void Encodeur::loop()
{
    if (millis() - m_time > 100)
    {
        Serial.print("Encoder : ");
        Serial.println(mesure()); // affiche la valeur du compteur de l'encodeur
        m_time = millis();
    }
}