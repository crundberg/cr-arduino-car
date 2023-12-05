#include <ArduinoBLE.h>
#include "Arduino_LED_Matrix.h"
#include "config.h"

ArduinoLEDMatrix matrix;

// Bluetooth® Low Energy Service
BLEService newService("19B10000-E8F2-537E-4F6C-D104768A1214");

// Bluetooth® Low Energy Joystick Characteristic - Custom 128-bit UUID, read and writable by central
BLEIntCharacteristic leftJoystickX("19b10001-e8f2-537e-4f6c-d104768a1214", BLERead | BLEWrite);
BLEIntCharacteristic leftJoystickY("19b10001-e8f2-537e-4f6c-d104768a1215", BLERead | BLEWrite);
BLEIntCharacteristic rightJoystickX("19b10001-e8f2-537e-4f6c-d104768a1216", BLERead | BLEWrite);
BLEIntCharacteristic rightJoystickY("19b10001-e8f2-537e-4f6c-d104768a1217", BLERead | BLEWrite);

long previousMillis = 0;

const uint32_t animation[][4] = {
    {0x0, 0x0, 0x0},                   // Stopped
    {0x80080080, 0x8008008, 0x800800}, // Forward
    {0x100100, 0x10010010, 0x1001001}, // Backward
    {0x0, 0x0, 0xfff},                 // Left
    {0xfff00000, 0x0, 0x0},            // Right
    {0x0, 0x8, 0x800e00},              // Forward left
    {0xe0080080, 0x0, 0x0},            // Forward right
    {0x0, 0x0, 0x1001007},             // Backward left
    {0x700100, 0x10000000, 0x0}        // Backward right
};

bool enableBrakeOld = false;

void setup()
{
  // Initialize serial communication
  Serial.begin(9600);

  // Initialize LED matrix
  matrix.begin();

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

    // Turn on the LED to indicate the connection
    digitalWrite(LED_BUILTIN, HIGH);

    // Check the battery level every 200ms
    // While the central is connected
    while (central.connected())
    {
      long currentMillis = millis();

      // If 200ms have passed, we check the battery level
      if (currentMillis - previousMillis >= 200)
      {
        previousMillis = currentMillis;

        if (leftJoystickX.written())
        {
          int value = leftJoystickX.value();
          Serial.print("Left X: ");
          Serial.println(value, DEC);
        }

        if (leftJoystickY.written())
        {
          int value = leftJoystickY.value();
          Serial.print("Left Y: ");
          Serial.println(value, DEC);
        }

        if (rightJoystickX.written())
        {
          int value = rightJoystickX.value();
          Serial.print("Right X: ");
          Serial.println(value, DEC);
        }

        if (rightJoystickY.written())
        {
          int value = rightJoystickY.value();
          Serial.print("Right Y: ");
          Serial.println(value, DEC);
        }
      }
    }

    digitalWrite(LED_BUILTIN, LOW); // when the central disconnects, turn off the LED
    Serial.print("[BLE]: Disconnected from central: ");
    Serial.println(central.address());
  }
}

void enableBrakes(bool value)
{
  if (enableBrakeOld != value)
  {
    if (value)
    {
      Serial.println("[Brakes]: Activated");
    }
    else
    {
      Serial.println("[Brakes]: Released");
    }
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

void driveStop()
{
  // Update LED matrix
  matrix.loadFrame(animation[0]);

  // Commands to motors
  enableBrakes(true);
  setSpeed(0, 0);
}

void driveForward()
{
  // Update LED matrix
  matrix.loadFrame(animation[1]);

  // Commands to motors
  setDirection(true, true);
  enableBrakes(false);
  setSpeed(100, 100);
}

void driveBackward()
{
  // Update LED matrix
  matrix.loadFrame(animation[2]);

  // Commands to motors
  setDirection(false, false);
  enableBrakes(false);
  setSpeed(100, 100);
}

void driveLeft()
{
  // Update LED matrix
  matrix.loadFrame(animation[3]);

  // Commands to motors
  setDirection(true, false);
  enableBrakes(false);
  setSpeed(100, 100);
}

void driveRight()
{
  // Update LED matrix
  matrix.loadFrame(animation[4]);

  // Commands to motors
  setDirection(false, true);
  enableBrakes(false);
  setSpeed(100, 100);
}

void driveForwardLeft()
{
  // Update LED matrix
  matrix.loadFrame(animation[5]);

  // Commands to motors
  setDirection(true, true);
  enableBrakes(false);
  setSpeed(100, 50);
}

void driveForwardRight()
{
  // Update LED matrix
  matrix.loadFrame(animation[6]);

  // Commands to motors
  setDirection(true, true);
  enableBrakes(false);
  setSpeed(50, 100);
}

void driveBackwardLeft()
{
  // Update LED matrix
  matrix.loadFrame(animation[7]);

  // Commands to motors
  setDirection(false, false);
  enableBrakes(false);
  setSpeed(100, 50);
}

void driveBackwardRight()
{
  // Update LED matrix
  matrix.loadFrame(animation[8]);

  // Commands to motors
  setDirection(false, false);
  enableBrakes(false);
  setSpeed(50, 100);
}