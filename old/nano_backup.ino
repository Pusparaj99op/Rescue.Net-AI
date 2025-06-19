#include <Wire.h>
#include <SoftwareSerial.h>
#include <Adafruit_BMP085.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SD.h>

// Pin definitions
#define PULSE_SENSOR_PIN A0
#define TEMP_SENSOR_PIN 4
#define BUZZER_PIN 7
#define VIBRATOR1_PIN 8
#define VIBRATOR2_PIN 9
#define EMERGENCY_BTN_PIN 5
#define STATUS_LED_PIN 6
#define ACCEL_X_PIN A1
#define ACCEL_Y_PIN A2
#define ACCEL_Z_PIN A3
#define SD_CS_PIN 10

// GPS Serial
SoftwareSerial gpsSerial(2, 3);

// Sensor objects
Adafruit_BMP085 bmp;
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature tempSensor(&oneWire);

// Data structure
struct HealthData {
  float heartRate;
  float temperature;
  float bloodPressure;
  double latitude;
  double longitude;
  float accelX, accelY, accelZ;
  unsigned long timestamp;
};

HealthData healthData;
bool emergencyMode = false;

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);
  
  // Initialize pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(VIBRATOR1_PIN, OUTPUT);
  pinMode(VIBRATOR2_PIN, OUTPUT);
  pinMode(EMERGENCY_BTN_PIN, INPUT_PULLUP);
  pinMode(STATUS_LED_PIN, OUTPUT);
  
  // Initialize I2C
  Wire.begin();
  
  // Initialize sensors
  if (!bmp.begin()) {
    Serial.println("BMP180 not found!");
  }
  
  tempSensor.begin();
  
  // Initialize SD card
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD Card failed!");
  }
  
  Serial.println("Arduino Nano Backup System Ready");
}

void loop() {
  // Read all sensors
  readSensors();
  
  // Check emergency button
  if (digitalRead(EMERGENCY_BTN_PIN) == LOW) {
    delay(50);
    if (digitalRead(EMERGENCY_BTN_PIN) == LOW) {
      triggerEmergency();
    }
  }
  
  // Send data to ESP32 via Serial
  sendDataToESP32();
  
  // Log to SD card
  logToSD();
  
  delay(1000);
}

void readSensors() {
  // Heart rate (simplified)
  int pulseValue = analogRead(PULSE_SENSOR_PIN);
  healthData.heartRate = map(pulseValue, 0, 1023, 60, 120);
  
  // Temperature
  tempSensor.requestTemperatures();
  healthData.temperature = tempSensor.getTempCByIndex(0);
  
  // Pressure
  healthData.bloodPressure = bmp.readPressure() / 100.0F;
  
  // Accelerometer
  healthData.accelX = (analogRead(ACCEL_X_PIN) - 512) * 3.3 / 1024;
  healthData.accelY = (analogRead(ACCEL_Y_PIN) - 512) * 3.3 / 1024;
  healthData.accelZ = (analogRead(ACCEL_Z_PIN) - 512) * 3.3 / 1024;
  
  healthData.timestamp = millis();
}

void triggerEmergency() {
  emergencyMode = true;
  
  // Alert pattern
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(VIBRATOR1_PIN, HIGH);
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(VIBRATOR1_PIN, LOW);
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(500);
  }
  
  Serial.println("EMERGENCY_TRIGGERED");
}

void sendDataToESP32() {
  Serial.print("DATA:");
  Serial.print(healthData.heartRate);
  Serial.print(",");
  Serial.print(healthData.temperature);
  Serial.print(",");
  Serial.print(healthData.bloodPressure);
  Serial.print(",");
  Serial.print(healthData.accelX);
  Serial.print(",");
  Serial.print(healthData.accelY);
  Serial.print(",");
  Serial.print(healthData.accelZ);
  Serial.print(",");
  Serial.println(emergencyMode ? "1" : "0");
}

void logToSD() {
  File dataFile = SD.open("backup.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.print(healthData.timestamp);
    dataFile.print(",");
    dataFile.print(healthData.heartRate);
    dataFile.print(",");
    dataFile.print(healthData.temperature);
    dataFile.print(",");
    dataFile.print(healthData.bloodPressure);
    dataFile.print(",");
    dataFile.print(healthData.accelX);
    dataFile.print(",");
    dataFile.print(healthData.accelY);
    dataFile.print(",");
    dataFile.print(healthData.accelZ);
    dataFile.print(",");
    dataFile.println(emergencyMode ? "1" : "0");
    dataFile.close();
  }
}