#include <Arduino.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_Sensor.h>

// I2C Address for this Nano
#define NANO_ADDR 8

// OneWire bus for DS18B20
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// BMP180 Sensor
Adafruit_BMP085 bmp;

// ADXL335 Pins
const int xPin = A1;
const int yPin = A2;
const int zPin = A3;

// Pulse Sensor Pin
const int pulsePin = A0;

void setup() {
  Wire.begin(NANO_ADDR); // Join I2C bus with address #8
  Wire.onRequest(requestEvent); // Register event
  Serial.begin(9600);
  sensors.begin();
  if (!bmp.begin()) {
	Serial.println("Could not find a valid BMP085 sensor, check wiring!");
	while (1) {}
  }
}

void loop() {
  delay(100); // Short delay
}

// Function that executes whenever data is requested by master
void requestEvent() {
  // Placeholder for sensor data
  float temperature = getTemperature();
  int heartRate = getHeartRate();
  bool fallDetected = detectFall();
  float altitude = bmp.readAltitude(101500); // Adjust for local pressure

  // Create a string with all sensor data
  String data = String(temperature) + "," + String(heartRate) + "," + String(fallDetected) + "," + String(altitude);

  // Send the string over I2C
  Wire.write(data.c_str());
}

float getTemperature() {
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);
}

int getHeartRate() {
  // More sophisticated algorithm needed for accurate BPM
  int pulseValue = analogRead(pulsePin);
  // Simple mapping for demonstration
  int bpm = map(pulseValue, 0, 1023, 60, 100);
  return bpm;
}

bool detectFall() {
  // Simple fall detection logic
  int x = analogRead(xPin);
  int y = analogRead(yPin);
  int z = analogRead(zPin);
  float totalAccel = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
  // Thresholds need to be calibrated
  if (totalAccel < 200 || totalAccel > 500) { 
    return true;
  }
  return false;
}
