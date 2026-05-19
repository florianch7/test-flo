#include <Moteur.h>

Moteur::Moteur(int EN, int IN1, int IN2, bool inv)
{
    m_EN = EN;
    m_IN1 = IN1;
    m_IN2 = IN2;
    m_inv = inv;
}

void Moteur::setup()
{
    // Setup les broches en output
    pinMode(m_EN, OUTPUT);  // PWM (vitesse)
    pinMode(m_IN1, OUTPUT); // Direction
    pinMode(m_IN2, OUTPUT); // Direction

    // Ecrit sur les broche l'etat initiale des PIN de directions
    digitalWrite(m_IN1, 0);
    digitalWrite(m_IN2, 1);
}

void Moteur::set_speed(int vitesse)
{
    if (m_inv)
    {
        vitesse = -vitesse;
    }

    if (abs(vitesse) > 255)
    {
        if (vitesse > 0)
        {
            vitesse = 255;
        }
        else
        {
            vitesse = -255;
        }
    }

    if (vitesse < 0)
    {
        digitalWrite(m_IN1, 1); // set le sens de rotation
        digitalWrite(m_IN2, 0);
    }
    else if (vitesse == 0)
    {
        digitalWrite(m_IN1, 0);
        digitalWrite(m_IN2, 0);
    }
    else
    {
        digitalWrite(m_IN1, 0); // set le sens de rotation
        digitalWrite(m_IN2, 1);
    }

    m_vitesse = abs(vitesse);     // Setup la vitesse en valeur absolue
    analogWrite(m_EN, m_vitesse); // envoie la command de vitesse
}

void Moteur::stop()
{
    unsigned long local_time = millis();

    for (int i = 10; i > 0; i--)
    {
        float current_speed = 0.1 * i * m_vitesse;
        if (m_vitesse < 0)
        {
            m_vitesse = 0;
        }
        while (millis() - local_time < 25)
        {
            ;
        }
        analogWrite(m_EN, current_speed);
    }
    digitalWrite(m_IN1, 0);
    digitalWrite(m_IN2, 0);
    local_time = millis();
}

void Moteur::loop()
{
}
