#include <ArduinoBLE.h>
#include "config.h"

int leftOldX = 0;
int leftOldY = 0;
int rightOldX = 0;
int rightOldY = 0;

void setup()
{
	// Initialize serial communication
	Serial.begin(9600);

	// Initialize ArduinoBLE library
	BLE.begin();
	Serial.println("[BLE]: Starting Bluetooth® Low Energy...");

	// Start scanning for peripherals
	BLE.scanForUuid("19B10000-E8F2-537E-4F6C-D104768A1214");
}

void loop()
{
	// Check if a peripheral has been discovered
	BLEDevice peripheral = BLE.available();

	if (peripheral)
	{
		// Discovered a peripheral
		Serial.print("[BLE]: Found ");
		Serial.print(peripheral.address());
		Serial.print(" '");
		Serial.print(peripheral.localName());
		Serial.print("' ");
		Serial.print(peripheral.advertisedServiceUuid());
		Serial.println();

		// Return if peripheral name is not the expected
		if (peripheral.localName() != "CR Arduino Car")
			return;

		// Stop scanning
		BLE.stopScan();

		// Control car
		controlCar(peripheral);

		// Peripheral disconnected, start scanning again
		BLE.scanForUuid("19b10000-e8f2-537e-4f6c-d104768a1214");
	}
}

void controlCar(BLEDevice peripheral)
{
	// Connect to the peripheral
	Serial.println("[BLE]: Connecting...");

	if (peripheral.connect())
	{
		Serial.println("[BLE]: Connected");
	}
	else
	{
		Serial.println("[BLE]: Failed to connect");
		return;
	}

	// Discover peripheral attributes
	Serial.println("[BLE]: Discover attributes...");
	if (peripheral.discoverAttributes())
	{
		Serial.println("[BLE]: Attributes discovered");
	}
	else
	{
		Serial.println("[BLE]: Attribute discovery failed");
		peripheral.disconnect();
		return;
	}

	// Retrieve the joystick characteristic
	BLECharacteristic leftJoystickX = peripheral.characteristic("19b10001-e8f2-537e-4f6c-d104768a1214");
	BLECharacteristic leftJoystickY = peripheral.characteristic("19b10001-e8f2-537e-4f6c-d104768a1215");
	BLECharacteristic rightJoystickX = peripheral.characteristic("19b10001-e8f2-537e-4f6c-d104768a1216");
	BLECharacteristic rightJoystickY = peripheral.characteristic("19b10001-e8f2-537e-4f6c-d104768a1217");

	// Verify joystick characteristics
	if (!leftJoystickX || !leftJoystickY || !rightJoystickX || !rightJoystickY)
	{
		Serial.println("[BLE]: Peripheral does not have joystick characteristic!");
		peripheral.disconnect();
		return;
	}
	else if (!leftJoystickX.canWrite() || !leftJoystickY.canWrite() || !rightJoystickX.canWrite() || !rightJoystickY.canWrite())
	{
		Serial.println("[BLE]: Peripheral does not have a writable joystick characteristic!");
		peripheral.disconnect();
		return;
	}

	// While the peripheral is connected
	while (peripheral.connected())
	{
		int updatedValues = 0;

		// Read values
		const int leftRawX = analogRead(PIN_JOYSTICK_LEFT_X);
		const int leftRawY = analogRead(PIN_JOYSTICK_LEFT_Y);
		const int rightRawX = analogRead(PIN_JOYSTICK_RIGHT_X);
		const int rightRawY = analogRead(PIN_JOYSTICK_RIGHT_Y);

		// Scale analog values
		int leftX = scaleValue(leftRawX, 100, -100);
		int leftY = scaleValue(leftRawY, 100, -100);
		int rightX = scaleValue(rightRawX, -100, 100);
		int rightY = scaleValue(rightRawY, -100, 100);

		// Update left X
		if (hysteresis(leftX, leftOldX))
		{
			leftOldX = leftX;
			updatedValues += 1;

			leftJoystickX.writeValue(leftX);
		}

		// Update left Y
		if (hysteresis(leftY, leftOldY))
		{
			leftOldY = leftY;
			updatedValues += 1;

			leftJoystickY.writeValue(leftY);
		}

		// Update right X
		if (hysteresis(rightX, rightOldX))
		{
			rightOldX = rightX;
			updatedValues += 1;

			rightJoystickX.writeValue(rightX);
		}

		// Update right Y
		if (hysteresis(rightY, rightOldY))
		{
			rightOldY = rightY;
			updatedValues += 1;

			rightJoystickY.writeValue(rightY);
		}

		// Print values if updated
		if (updatedValues > 0)
		{
			Serial.print("Left X: ");
			Serial.print(leftX, DEC);
			Serial.print("\tLeft Y: ");
			Serial.print(leftY, DEC);
			Serial.print("\tRight X: ");
			Serial.print(rightX, DEC);
			Serial.print("\tRight Y: ");
			Serial.print(rightY, DEC);
			Serial.println();
		}
	}
}

int scaleValue(int raw, int min, int max)
{
	int value = map(raw, 0, 4095, min, max);

	if (value >= JOYSTICK_HYSTERESIS_ZERO)
		return value;

	if (value <= -JOYSTICK_HYSTERESIS_ZERO)
		return value;

	return 0;
}

bool hysteresis(int value, int oldValue)
{
	if (value >= oldValue + JOYSTICK_HYSTERESIS)
		return true;

	if (value <= oldValue - JOYSTICK_HYSTERESIS)
		return true;

	return false;
}
