#ifndef CR_Motor_h
#define CR_Motor_h
#include "Arduino.h"

class Motor
{
public:
	Motor(int pinIn1, int pinIn2, int pinPwm);

	void begin();
	void drive(int speed);
	void forward(int speed);
	void backward(int speed);
	void brake();
	void stop();

private:
	int calcSpeed(int speed);
	void driveForward(int speed);
	void driveBackward(int speed);
	void activateBrake();
	void activateStop();

	int _pinIn1;
	int _pinIn2;
	int _pinPwm;
};

#endif