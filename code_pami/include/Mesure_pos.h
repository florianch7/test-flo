#include <Encodeur.h>
#include <define.h>
#ifndef MESURE_POS_H
#define MESURE_POS_H

class Mesure_pos
{
private:
    float x_origin = 0.0;
    float y_origin = 0.0;

public:
    // Nombre totaux de ticks des encodeurs
    float ticks_abs_l;
    float ticks_abs_r;

    // Distance parcourue par chaque roue
    float distance_abs_l;
    float distance_abs_r;

    // Vitesse absolue de chaque roue;
    float vit_abs_l;
    float vit_abs_r;

    // Positions absolue de la pami
    float pos_abs_x;
    float pos_abs_y;
    float pos_theta;

    // Vitesse absolue de la pami
    float vit_abs_x;
    float vit_abs_y;
    float omega_abs;

    Mesure_pos(Encodeur *encodeur_l, Encodeur *encodeur_r);
};

#endif