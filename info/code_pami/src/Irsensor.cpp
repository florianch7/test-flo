#include <Wire.h>
#include <Arduino.h>
#include <Irsensor.h>

#include <SparkFun_VL53L5CX_Library.h> //http://librarymanager/All#SparkFun_VL53L5CX

// Create an object for our VL53L5CX sensor
Irsensor::Irsensor(int SDA_PIN, int SCL_PIN, int LPN_PIN)
{
    m_SDA_PIN = SDA_PIN;
    m_SCL_PIN = SCL_PIN;
    m_LPN_PIN = LPN_PIN;
    dataReady = false;
}

// Interruption part
void Irsensor::interruptRoutine()
{ // Just set the flag that we have updated data and return from the ISR
    dataReady = true;
}

void Irsensor::setup()
{
    Serial.begin(115200);
    delay(1500);

    pinMode(m_LPN_PIN, OUTPUT);
    digitalWrite(m_LPN_PIN, HIGH);
    delay(50);

    Wire.begin(m_SDA_PIN, m_SCL_PIN); // This resets I2C bus to 100kHz
    Wire.setClock(500000);            // Sensor has max I2C freq of 1MHz

    if (myImager.begin() == false)
    {
        Serial.println(F("## FAILED : Sensor not found - check your wiring. Freezing ##"));
        while (1)
            ; // Infinite loop
    }

    myImager.setResolution(8 * 8); // Enable all 64 pads

    imageResolution = myImager.getResolution(); // Query sensor for current resolution - either 4x4 or 8x8
    imageWidth = sqrt(imageResolution);         // Calculate printing width

    // SETUP la fréquence d'acquisition
    // Using 4x4, min frequency is 1Hz and max is 60Hz
    // Using 8x8, min frequency is 1Hz and max is 15Hz
    bool response = myImager.setRangingFrequency(15); // une mesure toutes les 66ms
    if (response == true)
    {
        int frequency = myImager.getRangingFrequency();
        if (frequency > 0)
        {
            Serial.print("Ranging frequency set to ");
            Serial.print(frequency);
            Serial.println(" Hz.");
        }
        else
            Serial.println(F("Error recovering ranging frequency."));
    }
    else
    {
        Serial.println(F("Cannot set ranging frequency requested. Freezing..."));
        while (1)
            ; // Infinite loop
    }

    myImager.startRanging();
}

void Irsensor::loop()
{
    if (millis() - m_time >= m_dt) // 10ms delay between readings
    {

        // Poll sensor for new data
        if (myImager.isDataReady() == true)
        {
            if (myImager.getRangingData(&measurementData)) // Read distance data into array
            {
                // Recupère les distances de gauche à droite en faisant la moyenne vertical de chaque colonne (8x8) et enregistre dans la variable 'vision'
                // Recupère le minimum au passage
                int min_distance = -1;
                for (int col = 0; col < 8; col++)
                {
                    int tot_tmp = 0;
                    for (int line = 0; line < 8; line++)
                    {
                        tot_tmp += measurementData.distance_mm[line * 8 + col];
                    }
                    vision[col] = tot_tmp / 8;
                    if (min_distance == -1 || vision[col] < min_distance)
                    {
                        min_distance = vision[col];
                    }
                }
                ir_minimum_distance = min_distance;
            }
        }
        m_time = millis();
    }
}
