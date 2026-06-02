/**
 * @file define.h
 * @brief fichier de configuration des pins pour la pami et des gains et des constantes utiles
 */

// TODO REGLER LES PINS & LES GAINS AVANT LA COUPE AVEC L'ELEC
#ifndef DEFINE_H
#define DEFINE_H

// Variable globale pour la file d'appel des fonctions non bloquantes
// Doit être défini dans un .h qui n'est pas une classe (#c'estChiant) mais donc ne pas supprimer
extern int etape_globale;

/*
MOTEURS
*/
#define EN_L 32  // M1 EN LEFT MOTEUR
#define IN1_L 14 // M1 IN1 LEFT MOTEUR
#define IN2_L 27 // M1 IN2 LEFT MOTEUR

#define EN_R 26  // M2 EN RIGHT MOTEUR
#define IN1_R 25 // M2 IN1 RIGHT MOTEUR
#define IN2_R 33 // M2 IN2 RIGHT MOTEUR

/*
ENCODEURS
*/
#define CLK_L 39 // C1 LEFT ENCODER
#define DT_L 36  // C2 LEFT ENCODER

#define CLK_R 35 // C1 RIGHT ENCODER
#define DT_R 34  // C2 RIGHT ENCODER

/*
Inversion du sens des moteurs & encodeurs si ils sont montés à l'envers dans la pami
A tester avec print_vitesse ou print_encodeur pour que les vitesses soient positives et que le Start_pos avance
    Mais modifier le sens moteur change le sens encodeur
    Mais modifier le sens encodeur ne change pas le sens moteur
# Mention abominable
*/
#define INV_ENC_R 1 // Inversion du sens de rotation de l'encodeur droit
#define INV_MOT_R 0 // Inversion du sens de rotation du moteur droit

#define INV_ENC_L 0 // Inversion du sens de rotation de l'encodeur gauche
#define INV_MOT_L 1 // Inversion du sens de rotation du moteur gauche

/*
SENSORS (IR, Ultrason, etc...)
L'IR Sensor demande :
    - PIN I2C SCL & SDA définits dessous
    - Pin AVDD & IOVDD à 3.3V (pas de 5V)
    - Pin LPn à 3.3V
*/
#define ULTRASON_ECHO 0    // Ultrason echo pin
#define ULTRASON_TRIGGER 0 // Ultrason trigger pin

#define IR_SCL_PIN 22 // Ligne SCL de l'I2C du capteur ToF
#define IR_SDA_PIN 21 // Ligne SDA de l'I2C du capteur ToF
#define IR_LPN_PIN 0  // Ligne LPn du capteur ToF (pour le réveiller) - Branché au 5V direct c'est plus simple

/*
SERVO
*/
#define SERVPIN 18 // Broche du servo moteur sur D15

/*
CONFIG DES PAMI
*/
// Le coté est défini quand on est face à la scène.
//  equipe = 1 : on est du coté gauche (jaune).
//  equipe = 0 : on est du coté droit (bleue).

#define PIN_LED 2         // PIN LED pour le setup
#define PIN_READEQUIPE 19 // PIN pour lire l'interrupteur qui defini l'équipe
#define PIN_TIRETTE 23    // PIN de la tirette pour lancer le match

// Low & Low : Première
// Low & High : Deuxième
// High & Low : Troisième
// High & High : Quatrième

#define PIN_INT_PAMI_1 16 // PIN interrupteur 1 pour le numéro de la PAMI
#define PIN_INT_PAMI_2 17 // PIN interrupteur 2 pour le numéro de la PAMI

/*
Paramètres globaux
*/
#define END_TIME 99000      // Temps global de la pami en ms (99s)
#define START_TIME 3000     // Temps de démarrage des pamis (85s).
#define DELAY_TIME 200      // Temps de delay pour les fonctions non bloquantes
#define PERIODE_LOG 1000    // Période entre chaque log (ms)
#define PERIODE_BLINK 1000  // Période de clignotement (ms)
#define PERIODE_ASSERV 50   // Période d'asservissement (ms)
#define PERIODE_POSITION 50 // Période de mise à jour de la position (ms)

#define LARGEUR_Start_pos_MM 150 // Largeur du Start_pos en mm (distance entre les roues)
#define SPEED 255                // Vitesse (255 est la vitesse max des moteurs pour pwm)
#define DISTANCE_MIN 80          // Distance minimale pour éviter un obstacle en mm
#define ERREUR_DISTANCE 1        // Incertitude position (mm)
#define ERREUR_ANGLE 3           // Incertitude position (degrés)

/*
Gains naifs pour réellement avancer de 10cm avec un delay
*/
#define K_NAIF 1.25
#define K_ANGLE_NAIF 1.41

/*
Paramètres de l'asservissement
    A régler pour que le Start_pos suive bien sa trajectoire
    Kp : gain proportionnel, plus il est grand plus le Start_pos réagit vite à une erreur de position, mais peut causer des oscillations si trop élevé.
*/
#define KP_DISTANCE 2.4 // Gain proportionnel pour la distance
#define KP_ANGLE 2.4    // Gain proportionnel pour l'angle

#define KD_DISTANCE 0.15 // Gain dérivé pour la distance
#define KD_ANGLE 0.15    // Gain dérivé pour l'angle

// Valeurs de conversion qui dépendent des moteurs/encodeurs/pami - à régler correctement
#define GAIN_MM_TO_TICKS 9.9
#define GAIN_ANGLE_TO_TICKS 23

/*
Positions en fonction des équipe (J = JAUNE (gauche), B = BLUE (droite)) & du n° de la pami (1, 2, 3, 4)
Chaque pami à ses propres positions de départ et d'arrivée en cm
*/
// Structure pour stocker les positions et mouvements
struct Point
{
    float x;
    float y;
};

struct StartPosition
{
    Point j;
    Point b;
};

/*
Tableau de config
4 Pami
2 équipes (J - JAUNE, B - BLUE)
Pour chaque équipe, la position de départ (x, y)
{.j = {x_depart, y_depart}, b = {x_depart, y_depart}}
On accède avec : pami[i].j.x    pami[i].b.y
*/

inline const StartPosition pami_start_pos[4] = {
    {.j = {10, 80}, .b = {10, 80}},
    {.j = {20, 90}, .b = {20, 90}},
    {.j = {30, 70}, .b = {30, 70}},
    {.j = {40, 60}, .b = {40, 60}}};

#endif