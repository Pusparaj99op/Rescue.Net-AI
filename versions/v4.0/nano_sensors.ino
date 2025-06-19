#include <SoftwareSerial.h>
#include <Wire.h>

// Pin definitions
#define PULSE_SENSOR_PIN A0
#define ACCEL_X_PIN A1
#define ACCEL_Y_PIN A2
#define ACCEL_Z_PIN A3
#define TEMP_SENSOR_PIN 4
#define BUZZER_PIN 5
#define VIBRATOR1_PIN 6
#define VIBRATOR2_PIN 7
#define BUTTON_PIN 8
#define LED_PIN 9

// Communication with ESP32
SoftwareSerial esp32Serial(2, 3);

struct SensorData {
  int heartRate;
  float temperature;
  float accelX, accelY, accelZ;
  bool buttonPressed;
  bool fallDetected;
};

SensorData sensorData;
unsigned long lastReading = 0;

void setup() {
  Serial.begin(9600);
  esp32Serial.begin(9600);
  
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(VIBRATOR1_PIN, OUTPUT);
  pinMode(VIBRATOR2_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  Serial.println("Arduino Nano Sensor Module Initialized");
}

void loop() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastReading > 1000) { // Read sensors every second
    readSensors();
    sendDataToESP32();
    lastReading = currentTime;
  }
  
  // Handle commands from ESP32
  if (esp32Serial.available()) {
    String command = esp32Serial.readString();
    command.trim();
    handleCommand(command);
  }
  
  delay(100);
}

void readSensors() {
  // Read pulse sensor
  int pulseValue = analogRead(PULSE_SENSOR_PIN);
  sensorData.heartRate = calculateHeartRate(pulseValue);
  
  // Read accelerometer
  int rawX = analogRead(ACCEL_X_PIN);
  int rawY = analogRead(ACCEL_Y_PIN);
  int rawZ = analogRead(ACCEL_Z_PIN);
  
  sensorData.accelX = (rawX - 512) * 3.3 / 512.0;
  sensorData.accelY = (rawY - 512) * 3.3 / 512.0;
  sensorData.accelZ = (rawZ - 512) * 3.3 / 512.0;
  
  // Detect fall
  float totalAccel = sqrt(pow(sensorData.accelX, 2) + pow(sensorData.accelY, 2) + pow(sensorData.accelZ, 2));
  sensorData.fallDetected = (totalAccel > 3.0 || totalAccel < 0.5);
  
  // Read button
  sensorData.buttonPressed = (digitalRead(BUTTON_PIN) == LOW);
}

int calculateHeartRate(int sensorValue) {
  static int lastValue = 0;
  static unsigned long lastBeat = 0;
  static int beatCount = 0;
  static unsigned long startTime = millis();
  static int currentBPM = 0;
  
  // Simple peak detection
  if (sensorValue > (lastValue + 50) && (millis() - lastBeat) > 300) {
    beatCount++;
    lastBeat = millis();
    
    // Calculate BPM over 10 seconds
    if (millis() - startTime > 10000) {
      currentBPM = (beatCount * 60000) / (millis() - startTime);
      beatCount = 0;
      startTime = millis();
    }
  }
  
  lastValue = sensorValue;
  return currentBPM;
}

void sendDataToESP32() {
  // Send data in JSON format
  esp32Serial.print("{");
  esp32Serial.print("\"heartRate\":" + String(sensorData.heartRate) + ",");
  esp32Serial.print("\"accelX\":" + String(sensorData.accelX) + ",");
  esp32Serial.print("\"accelY\":" + String(sensorData.accelY) + ",");
  esp32Serial.print("\"accelZ\":" + String(sensorData.accelZ) + ",");
  esp32Serial.print("\"buttonPressed\":" + String(sensorData.buttonPressed) + ",");
  esp32Serial.print("\"fallDetected\":" + String(sensorData.fallDetected));
  esp32Serial.println("}");
}

void handleCommand(String command) {
  if (command == "EMERGENCY_ALERT") {
    // Activate local alerts
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(VIBRATOR1_PIN, HIGH);
    digitalWrite(VIBRATOR2_PIN, HIGH);
    
    delay(1000);
    
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(VIBRATOR1_PIN, LOW);
    digitalWrite(VIBRATOR2_PIN, LOW);
    
  } else if (command == "EMERGENCY_CANCEL") {
    digitalWrite(LED_PIN, LOW);
    
  } else if (command == "STATUS_BLINK") {
    // Blink LED to show system is active
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
  }
}