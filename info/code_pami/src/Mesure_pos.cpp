#include <Mesure_pos.h>
#include <Encodeur.h>
#include <define.h>
#include <math.h>

Mesure_pos::Mesure_pos(Encodeur *m_encodeur_g, Encodeur *m_encodeur_d)
{
    encodeur_g = m_encodeur_g;
    encodeur_d = m_encodeur_d;
}

void Mesure_pos::setup()
{
    encodeur_d->clear_count();
    encodeur_g->clear_count();

    ticks_g = 0.0;
    ticks_d = 0.0;

    distance_g = 0.0;
    distance_d = 0.0;

    pos_x = 0.0;
    pos_y = 0.0;
    pos_angle_deg = 0.0;

    m_last_update = millis();
    m_log_update = millis();
}

void Mesure_pos::update_mesure_position()
{
    if (millis() - m_last_update < PERIODE_POSITION)
        return;
    if (encodeur_g == nullptr || encodeur_d == nullptr)
        return;

    // Calculer les DELTAS des encodeurs depuis la dernière mise à jour
    float ticks_g_current = encodeur_g->mesure() - ticks_g;
    float ticks_d_current = encodeur_d->mesure() - ticks_d;

    // Convertir deltas en distances (mm)
    distance_g = ticks_g_current / GAIN_MM_TO_TICKS;
    distance_d = ticks_d_current / GAIN_MM_TO_TICKS;

    // Moyenne des distances et angle
    float distance_moyenne = (distance_g + distance_d) / 2.0;
    float delta_angle = (distance_d - distance_g) / LARGEUR_ROBOT_MM * (180.0 / PI);

    // AJOUTER les changements
    pos_angle_deg += delta_angle;
    pos_x += distance_moyenne * cos(pos_angle_deg * PI / 180.0);
    pos_y += distance_moyenne * sin(pos_angle_deg * PI / 180.0);

    if (millis() - m_log_update >= 250)
    {
        // Serial.println("Ticks - Gauche : " + String(ticks_g) + " | Droite : " + String(ticks_d));
        // Serial.println("Distance - Gauche : " + String(distance_g) + " mm | Droite : " + String(distance_d) + " mm");
        // Serial.println("Angle absolu : " + String(pos_angle_deg) + " deg");
        // Serial.println("Position absolue - X : " + String(pos_x) + " mm | Y : " + String(pos_y) + " mm");
        m_log_update = millis();
    }

    ticks_g = encodeur_g->mesure();
    ticks_d = encodeur_d->mesure();

    m_last_update = millis();
}

/*
Peut servir si on à des bumpers
*/
void Mesure_pos::reset_position(float x, float y, float theta)
{
    pos_x = x;
    pos_y = y;
    pos_angle_deg = theta;

    // Reset encoders if needed
    if (encodeur_g != nullptr)
        encodeur_g->clear_count();
    if (encodeur_d != nullptr)
        encodeur_d->clear_count();

    distance_g = 0.0;
    distance_d = 0.0;
}

void Mesure_pos::reset_encoders()
{
    if (encodeur_g != nullptr)
        encodeur_g->clear_count();
    if (encodeur_d != nullptr)
        encodeur_d->clear_count();

    ticks_g = 0.0;
    ticks_d = 0.0;
    distance_g = 0.0;
    distance_d = 0.0;
}