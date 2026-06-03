#include <Serv.h>

Serv::Serv(int pin, uint8_t servo_I2C_addr)
{
	this->servo = Servo(); // Objet de la library Serv.h
	this->pin = pin;	   // Setup le PIN
	this->servo_I2C_addr = servo_I2C_addr;
}

// $ Check if need to setup the I2C address
void Serv::setup()
{
	temps_servo = millis();
	servo.attach(pin); // demandé par l'objet servo de la library

	etat = 90; // Initialise l'etat du servo
	servo.write(etat);
}

/*
	- Temps en ms
	- Angles en degrés
*/
void Serv::blink(int angle1, int angle2, long time_blink)
{
	if (millis() - temps_servo > time_blink)
	{
		if (etat == angle2)
		{
			this->servo.write(angle1);
			etat = angle1;
			Serial.print("Servo moved to ");
			Serial.println(angle1);
		}
		else
		{
			this->servo.write(angle2);
			etat = angle2;
			Serial.print("Servo moved to ");
			Serial.println(angle2);
		}
		temps_servo = millis();
	}
}

void Serv::loop()
{
}
