#include <Moteur.h>
#include <define.h>

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

// Eteint les moteurs de manière linéaire pour éviter les à-coups
// Est bloquante, à utiliser pour arrêter le robot à la fin du match ou en cas d'obstacle détecté
void Moteur::stop()
{
    int base_speed = abs(m_vitesse);

    for (int i = step_freinage; i > 0; i--)
    {
        int current_speed = base_speed * (i / (float)step_freinage);
        analogWrite(m_EN, current_speed);

        delay(25);
    }

    digitalWrite(m_IN1, 0);
    digitalWrite(m_IN2, 0);
    analogWrite(m_EN, 0);
}

void Moteur::loop()
{
}
