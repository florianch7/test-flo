#include "Ultrason.hpp"

Ultrason::Ultrason(int echo_PIN, int trigger_PIN)
{
    m_echo_PIN = echo_PIN;
    m_trigger_PIN = trigger_PIN;
    m_distance = 0;
    m_duration = 0;
}

void Ultrason::setup()
{
    pinMode(m_echo_PIN, INPUT);     // Broche Echo du HC-SR04 en INPUT
    pinMode(m_trigger_PIN, OUTPUT); // Broche Trigger du HC-SR04 en OUTPUT
    digitalWrite(m_trigger_PIN, LOW);
}

void Ultrason::loop() // !! Code jamais testé !! //
{
    if (m_reading)
    {
        if (millis() - m_t_reading > m_dt)
        {                                     // Si l'acquisition à duré > m_dt (= 10ms)
            digitalWrite(m_trigger_PIN, LOW); // Stop l'acquisition

            // Acquisition
            m_duration = pulseIn(m_echo_PIN, HIGH, max_duration); // Lecture de la durée de l'echo
            // Serial.print("Duration: ");
            // Serial.println(m_duration);

            // Calcul de la distance en cm
            m_distance = m_duration * 0.034 / 2;
            // affichage de la distance (à commenter si pas besoin)
            // Serial.print("Distance: ");
            // Serial.println(m_distance);

            m_reading = false;
        }
    }
    else
    {
        digitalWrite(m_trigger_PIN, HIGH); // Démarre l'acquisition
        m_reading = true;                  // Informe qu'on est en lecture
        m_t_reading = millis();            // reset le timer d'acquisition
    }
}