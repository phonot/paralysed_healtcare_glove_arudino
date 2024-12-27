#include <Wire.h>               // Library for I2C communication
#include <LiquidCrystal_I2C.h>   // Library for I2C LCD
#include <SoftwareSerial.h>      // Needed for GSM module communication
#include <MPU6050.h>             // Library for MPU6050 (You need to install this library from the Library Manager)

// Pin definitions
const int flexSensorPins[4] = {A0, A1, A2, A3}; // Analog pins connected to the flex sensors
const int speakerPin = 9;                       // Digital pin connected to the speaker or audio module
const int gsmRxPin = 10;                        // RX pin of GSM module
const int gsmTxPin = 11;                        // TX pin of GSM module

// Thresholds for each flex sensor
const int thresholds[4] = {315, 300, 320, 245}; // Adjust these values based on your flex sensor readings

// Initialize the LCD with the I2C address 0x27 and dimensions (16 columns, 2 rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Create an MPU6050 object
MPU6050 mpu;

// Create a SoftwareSerial object for GSM module
SoftwareSerial gsm(gsmRxPin, gsmTxPin);

// Fall detection thresholds for each axis
const float FALL_THRESHOLD_X = 2.0; // Threshold for detecting fall on X axis
const float FALL_THRESHOLD_Y = 0.17; // Threshold for detecting fall on Y axis
const float FALL_THRESHOLD_Z = 2.0; // Threshold for detecting fall on Z axis

void setup() {
  pinMode(speakerPin, OUTPUT);
  Serial.begin(9600);  // For debugging
  gsm.begin(9600);     // Initialize GSM communication
  
  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("System Initialized");
  delay(2000);
  lcd.clear();

  Serial.println("System Initialized");

  // Initialize the MPU6050 sensor
  Wire.begin();
  mpu.initialize();
  if (!mpu.testConnection()) {
    lcd.clear();
    lcd.print("MPU6050 not found!");
    Serial.println("MPU6050 connection failed");
    while (1); // Halt if the MPU6050 is not found
  }
  Serial.println("MPU6050 initialized");

  // Initialize GSM module
  gsm.println("AT");           // Test GSM connection
  delay(1000);
  gsm.println("AT+CMGF=1");    // Set SMS mode to text
  delay(1000);
}

void loop() {
  // Read the accelerometer data from the MPU6050
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  // Read the temperature from the MPU6050
  int16_t rawTemperature = mpu.getTemperature(); // Raw temperature value from MPU6050
  float temperature = rawTemperature / 340.0 + 36.53; // Convert to Celsius

  // Convert raw accelerometer values to g
  float accelX = ax / 16384.0; // Convert raw value to g
  float accelY = ay / 16384.0; // Convert raw value to g
  float accelZ = az / 16384.0; // Convert raw value to g

  // Print the accelerometer and temperature data for debugging
  Serial.print("Accel X: ");
  Serial.print(accelX);
  Serial.print(" Y: ");
  Serial.print(accelY);
  Serial.print(" Z: ");
  Serial.print(accelZ);
  Serial.print(" Temp: ");
  Serial.println(temperature);

  // Display the temperature on the LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature, 1); // Display 1 decimal point
  lcd.print(" C");

  // Fall detection: Check if any axis exceeds its threshold value
  if (abs(accelX) > FALL_THRESHOLD_X || abs(accelY) > FALL_THRESHOLD_Y || abs(accelZ) > FALL_THRESHOLD_Z) {
    triggerFallAlarm(accelX, accelY, accelZ);  // Trigger fall alarm based on axis values
  } else {
    // Check the flex sensors
    for (int i = 0; i < 4; i++) {
      int sensorValue = analogRead(flexSensorPins[i]); // Read the flex sensor value
      Serial.print("Flex Sensor ");
      Serial.print(i + 1);
      Serial.print(" Value: ");
      Serial.println(sensorValue);

      if (sensorValue > thresholds[i]) {
        triggerAlarm(i); // Trigger alarm if any flex sensor threshold is exceeded
        return; // Exit loop if a sensor is triggered
      }
    }

    showSmileyFace(); // Show smiley face if no sensors are triggered
  }

  delay(200); // Small delay for stability
}

void triggerFallAlarm(float x, float y, float z) {
  Serial.println("Fall detected!");

  // Display the fall detection message on the LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Fall Detected!");

  // Display the axis with the largest deviation
  lcd.setCursor(0, 1);
  if (abs(x) > abs(y) && abs(x) > abs(z)) {
    lcd.print("Fall on X Axis");
  } else if (abs(y) > abs(x) && abs(y) > abs(z)) {
    lcd.print("Fall on Y Axis");
  } else {
    lcd.print("Fall on Z Axis");
  }
//_ah22bm012
//_ah22bm013
  tone(speakerPin, 1500, 500); // High tone for fall detection
  sendSMS("1234567890", "Fall Detected!"); // Replace with the desired phone number
  delay(2000); // Keep the message on display for 2 seconds
}

void sendSMS(String phoneNumber, String message) {
  gsm.println("AT+CMGS=\"" + phoneNumber + "\""); // Send command to start SMS
  delay(100);
  gsm.println(message);                           // Send the message
  delay(100);
  gsm.write(26);                                  // End SMS with Ctrl+Z
  delay(1000);
}

void triggerAlarm(int sensorIndex) {
  Serial.print("Flex sensor ");
  Serial.print(sensorIndex + 1);
  Serial.println(" triggered!");

  // Display the corresponding message on the LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  switch (sensorIndex) {
    case 0:
      lcd.print("Food or Water");
      tone(speakerPin, 500, 500); // Low tone
      break;
    case 1:
      lcd.print("Emergency");
      tone(speakerPin, 1000, 500); // Medium tone
      break;
    case 2:
      lcd.print("Need Fresh Air");
      tone(speakerPin, 1500, 500); // High tone
      break;
    case 3:
      lcd.print("Washroom");
      tone(speakerPin, 2000, 500); // Very high tone
      break;
  }

  delay(2000); // Keep the message on display for 2 seconds
}

void showSmileyFace() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(":)"); // Display a simple smiley face
}


//ph
