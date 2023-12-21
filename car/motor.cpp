#include "Arduino.h"
#include "motor.h"

Motor::Motor(int pinIn1, int pinIn2, int pinPwm)
{
	_pinIn1 = pinIn1;
	_pinIn2 = pinIn2;
	_pinPwm = pinPwm;
}

void Motor::begin()
{
	activateStop();
}

void Motor::drive(int speed)
{
	if (speed == 0)
	{
		// Stop
		activateStop();
	}
	else if (speed < 0)
	{
		// Backward
		driveBackward(speed);
	}
	else
	{
		// Forward
		driveForward(speed);
	}
}

void Motor::forward(int speed)
{
	driveForward(speed);
}

void Motor::backward(int speed)
{
	driveBackward(speed);
}

void Motor::brake()
{
	activateBrake();
}

void Motor::stop()
{
	activateStop();
}

int Motor::calcSpeed(int speed)
{
	int pwm = speed;

	// Convert to positive number
	if (pwm < 0)
		pwm *= -1;

	// Limit speed to 100%
	if (pwm > 100)
		pwm = 100;

	// Scale speed from percent to PWM
	return pwm / 100 * 255;
}

void Motor::driveForward(int speed)
{
	int pwm = calcSpeed(speed);

	digitalWrite(_pinIn1, HIGH);
	digitalWrite(_pinIn2, LOW);
	analogWrite(_pinPwm, pwm);
}

void Motor::driveBackward(int speed)
{
	int pwm = calcSpeed(speed);

	digitalWrite(_pinIn1, LOW);
	digitalWrite(_pinIn2, HIGH);
	analogWrite(_pinPwm, pwm);
}

void Motor::activateBrake()
{
	digitalWrite(_pinIn1, HIGH);
	digitalWrite(_pinIn2, HIGH);
	analogWrite(_pinPwm, 0);
}

void Motor::activateStop()
{
	digitalWrite(_pinIn1, LOW);
	digitalWrite(_pinIn2, LOW);
	analogWrite(_pinPwm, 0);
}