#ifndef MESURE_POS_H
#define MESURE_POS_H

#include <Encodeur.h>
#include <define.h>

class Mesure_pos
{
private:
    Encodeur *encodeur_g;
    Encodeur *encodeur_d;

    unsigned long m_last_update;
    unsigned long m_log_update;

public:
    // Nombre totaux de ticks des encodeurs
    float ticks_g;
    float ticks_d;

    // Distance parcourue par chaque roue (en mm)
    float distance_g;
    float distance_d;

    // Position absolue de la pami (en mm)
    float pos_x;
    float pos_y;
    float pos_angle_deg; // en degrés

    Mesure_pos(Encodeur *encodeur_g, Encodeur *encodeur_d);

    // Setup de la position
    void setup();

    // Update position based on current encoder readings
    void update_position();

    // Reset position to initial state
    void reset_position(float x = 0.0, float y = 0.0, float theta = 0.0);

    // Reset encoders without changing position
    void reset_encoders();
};

#endif