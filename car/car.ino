#include <ArduinoBLE.h>
#include "config.h"

// Bluetooth® Low Energy Service
BLEService newService("19B10000-E8F2-537E-4F6C-D104768A1214");

// Bluetooth® Low Energy Joystick Characteristic - Custom 128-bit UUID, read and writable by central
BLEIntCharacteristic leftJoystickX("19b10001-e8f2-537e-4f6c-d104768a1214", BLERead | BLEWrite);
BLEIntCharacteristic leftJoystickY("19b10001-e8f2-537e-4f6c-d104768a1215", BLERead | BLEWrite);
BLEIntCharacteristic rightJoystickX("19b10001-e8f2-537e-4f6c-d104768a1216", BLERead | BLEWrite);
BLEIntCharacteristic rightJoystickY("19b10001-e8f2-537e-4f6c-d104768a1217", BLERead | BLEWrite);

long previousMillis = 0;

bool enableBrakeOld = false;

int leftValueX = 0;
int leftValueY = 0;
int rightValueX = 0;
int rightValueY = 0;

void setup()
{
	// Initialize serial communication
	Serial.begin(9600);

	// Initialize BLE
	setupBLE();

	// Built in LED to indicate when a central is connected
	pinMode(LED_BUILTIN, OUTPUT);

	// Motor channel A for motors on left side
	pinMode(LEFT_DIRECTION_PIN, OUTPUT);
	pinMode(LEFT_SPEED_PIN, OUTPUT);
	pinMode(LEFT_BRAKE_PIN, OUTPUT);

	// Motor channel B for motors on right side
	pinMode(RIGHT_DIRECTION_PIN, OUTPUT);
	pinMode(RIGHT_SPEED_PIN, OUTPUT);
	pinMode(RIGHT_BRAKE_PIN, OUTPUT);
}

void setupBLE()
{
	// Initialize ArduinoBLE library
	if (!BLE.begin())
	{
		Serial.println("[BLE]: Starting Bluetooth® Low Energy failed!");
		while (1)
			;
	}

	// Setting a name that will appear when scanning for Bluetooth® devices
	BLE.setLocalName("CR Arduino Car");
	BLE.setAdvertisedService(newService);

	// Add joystick characteristics to a service
	newService.addCharacteristic(leftJoystickX);
	newService.addCharacteristic(leftJoystickY);
	newService.addCharacteristic(rightJoystickX);
	newService.addCharacteristic(rightJoystickY);

	// Adding the service
	BLE.addService(newService);

	// Set initial value for characteristics
	leftJoystickX.writeValue(0);
	leftJoystickY.writeValue(0);
	rightJoystickX.writeValue(0);
	rightJoystickY.writeValue(0);

	// Start advertising the service
	BLE.advertise();
	Serial.println("[BLE]: Bluetooth® device active, waiting for connections...");
}

void loop()
{
	BLEDevice central = BLE.central(); // wait for a Bluetooth® Low Energy central

	// If a central is connected to the peripheral
	if (central)
	{
		// Print the central's BT address
		Serial.print("[BLE]: Connected to central: ");
		Serial.println(central.address());

		// Turn on LED when connected
		digitalWrite(LED_BUILTIN, HIGH);

		// While the central is connected
		while (central.connected())
		{
			centralConnectedLoop();
		}

		// Turn off LED when disconnected
		digitalWrite(LED_BUILTIN, LOW);

		Serial.print("[BLE]: Disconnected from central: ");
		Serial.println(central.address());
	}
}

void centralConnectedLoop()
{
	long currentMillis = millis();

	// If 200ms have passed, check for new values
	if (currentMillis - previousMillis >= 200)
	{
		previousMillis = currentMillis;

		if (leftJoystickX.written())
			leftValueX = leftJoystickX.value();

		if (leftJoystickY.written())
			leftValueY = leftJoystickY.value();

		if (rightJoystickX.written())
			rightValueX = rightJoystickX.value();

		if (rightJoystickY.written())
			rightValueY = rightJoystickY.value();

		carLogic();
	}
}

void enableBrakes(bool value)
{
	if (enableBrakeOld != value)
	{
		if (value)
			Serial.println("[Brakes]: Activated");
		else
			Serial.println("[Brakes]: Released");
	}

	enableBrakeOld = value;

	digitalWrite(LEFT_BRAKE_PIN, value);
	digitalWrite(RIGHT_BRAKE_PIN, value);
}

void setDirection(bool forwardLeft, bool forwardRight)
{
	digitalWrite(LEFT_DIRECTION_PIN, !forwardLeft);
	digitalWrite(RIGHT_DIRECTION_PIN, !forwardRight);
}

void setSpeed(int left, int right)
{
	analogWrite(LEFT_SPEED_PIN, left);
	analogWrite(RIGHT_SPEED_PIN, right);
}

void carLogic()
{
	// Set direction
	digitalWrite(LEFT_DIRECTION_PIN, leftValueY < 0);
	digitalWrite(RIGHT_DIRECTION_PIN, rightValueY < 0);

	// Release brakes
	digitalWrite(LEFT_BRAKE_PIN, LOW);
	digitalWrite(RIGHT_BRAKE_PIN, LOW);

	// Get speed
	int leftSpeed = leftValueY;
	int rightSpeed = rightValueY;

	// Invert speed if negative
	if (leftSpeed < 0)
		leftSpeed *= -1;
	if (rightSpeed < 0)
		rightSpeed *= -1;

	// Set minimum speed
	if (leftSpeed > 0 && leftSpeed < 30)
		leftSpeed = 30;
	if (rightSpeed > 0 && rightSpeed < 30)
		rightSpeed = 30;

	// Set maximum speed
	if (leftSpeed > 80)
		leftSpeed = 80;
	if (rightSpeed > 80)
		rightSpeed = 80;

	// Set speed
	analogWrite(LEFT_SPEED_PIN, leftSpeed);
	analogWrite(RIGHT_SPEED_PIN, rightSpeed);

	// Print commands
	Serial.print("LeftDir: ");
	Serial.print(leftValueY < 0);
	Serial.print("\tRightDir: ");
	Serial.print(rightValueY < 0);
	Serial.print("\tLeftSpeed: ");
	Serial.print(leftSpeed);
	Serial.print("\tRightSpeed: ");
	Serial.print(rightSpeed);
	Serial.println();
}

void driveStop()
{
	enableBrakes(true);
	setSpeed(0, 0);
}

void driveForward()
{
	setDirection(true, true);
	enableBrakes(false);
	setSpeed(100, 100);
}

void driveBackward()
{
	setDirection(false, false);
	enableBrakes(false);
	setSpeed(100, 100);
}

void driveLeft()
{
	setDirection(true, false);
	enableBrakes(false);
	setSpeed(100, 100);
}

void driveRight()
{
	setDirection(false, true);
	enableBrakes(false);
	setSpeed(100, 100);
}

void driveForwardLeft()
{
	setDirection(true, true);
	enableBrakes(false);
	setSpeed(100, 50);
}

void driveForwardRight()
{
	setDirection(true, true);
	enableBrakes(false);
	setSpeed(50, 100);
}

void driveBackwardLeft()
{
	setDirection(false, false);
	enableBrakes(false);
	setSpeed(100, 50);
}

void driveBackwardRight()
{
	setDirection(false, false);
	enableBrakes(false);
	setSpeed(50, 100);
}