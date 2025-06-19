#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Pin Definitions
#define TEMP_PIN 2
#define PULSE_PIN A0
#define BUZZER_PIN 3
#define VIBRATOR1_PIN 5
#define VIBRATOR2_PIN 6
#define BUTTON_PIN 7
#define LED_PIN 8
#define ESP_RX 4
#define ESP_TX 9

// Objects
SoftwareSerial espSerial(ESP_RX, ESP_TX);
Adafruit_BMP085 bmp;
SSD1306AsciiWire oled;
OneWire oneWire(TEMP_PIN);
DallasTemperature temperatureSensor(&oneWire);

// Variables
volatile int pulseRate = 0;
volatile int lastBeatTime = 0;
volatile bool pulseDetected = false;
int heartRate = 0;
float temperature = 0;
bool emergencyMode = false;
unsigned long lastHeartRateCalc = 0;
unsigned long lastDataSend = 0;

// Heart rate calculation variables
const int PULSE_THRESHOLD = 550;
const int PULSE_FADE_TIME = 1500;
int pulseReadings[10];
int pulseIndex = 0;
bool pulseSensorConnected = true;

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);

// Initialize pins
  pinMode(PULSE_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(VIBRATOR1_PIN, OUTPUT);
  pinMode(VIBRATOR2_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

// Initialize I2C components
  Wire.begin();

// Initialize BMP180
  if (!bmp.begin()) {
    Serial.println("BMP180 init failed!");
  }

// Initialize OLED
  oled.begin(&Adafruit128x64, 0x3C);
  oled.setFont(Adafruit5x7);
  oled.clear();
  oled.println("RescueNet Nano");
  oled.println("Initializing...");

// Initialize temperature sensor
  temperatureSensor.begin();

// Initialize pulse readings array
  for (int i = 0; i < 10; i++) {
    pulseReadings[i] = 0;
  }

  Serial.println("Arduino Nano initialized");
  delay(2000);
}

void loop() {
// Read pulse sensor
  readPulseSensor();

// Calculate heart rate every 5 seconds
  if (millis() - lastHeartRateCalc >= 5000) {
    calculateHeartRate();
    lastHeartRateCalc = millis();
  }

// Read other sensors
  readSensors();

// Handle ESP32 communication
  handleESPCommunication();

// Check emergency button
  if (digitalRead(BUTTON_PIN) == LOW) {
    triggerLocalEmergency();
  }

// Update local display
  updateLocalDisplay();

// Send data to ESP32 every 10 seconds
  if (millis() - lastDataSend >= 10000) {
    sendDataToESP();
    lastDataSend = millis();
  }

  delay(50);
}

void readPulseSensor() {
  int pulseValue = analogRead(PULSE_PIN);

// Store reading in circular buffer
  pulseReadings[pulseIndex] = pulseValue;
  pulseIndex = (pulseIndex + 1) % 10;

// Simple peak detection
  if (pulseValue > PULSE_THRESHOLD) {
    unsigned long currentTime = millis();
    if (currentTime - lastBeatTime > 300) {// Minimum 300ms between beats
      pulseDetected = true;
      lastBeatTime = currentTime;
    }
  }
}

void calculateHeartRate() {
// Calculate average pulse value
  int sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += pulseReadings[i];
  }
  int average = sum / 10;

// Check if sensor is connected
  if (average < 100) {
    pulseSensorConnected = false;
    heartRate = 0;
    return;
  } else {
    pulseSensorConnected = true;
  }

// Calculate heart rate based on beat intervals
  static unsigned long beatTimes[5];
  static int beatIndex = 0;
  static int beatCount = 0;

  if (pulseDetected) {
    beatTimes[beatIndex] = lastBeatTime;
    beatIndex = (beatIndex + 1) % 5;
    if (beatCount < 5) beatCount++;

    if (beatCount >= 2) {
      unsigned long totalInterval = 0;
      int validIntervals = 0;

      for (int i = 1; i < beatCount; i++) {
        unsigned long interval = beatTimes[i] - beatTimes[i-1];
        if (interval > 300 && interval < 2000) {// Valid heart rate range
          totalInterval += interval;
          validIntervals++;
        }
      }

      if (validIntervals > 0) {
        unsigned long avgInterval = totalInterval / validIntervals;
        heartRate = 60000 / avgInterval;// Convert to BPM

// Constrain to realistic values
        heartRate = constrain(heartRate, 40, 180);
      }
    }

    pulseDetected = false;
  }

  Serial.print("Heart Rate: ");
  Serial.print(heartRate);
  Serial.println(" BPM");
}

void readSensors() {
// Read temperature
  temperatureSensor.requestTemperatures();
  temperature = temperatureSensor.getTempCByIndex(0);

  if (temperature == DEVICE_DISCONNECTED_C) {
    temperature = 0;
  }
}

void handleESPCommunication() {
  if (espSerial.available()) {
    String received = espSerial.readString();
    received.trim();

    if (received == "GET_HR") {
      espSerial.println(heartRate);
    } else if (received.startsWith("{")) {
// JSON command from ESP32
      DynamicJsonDocument doc(512);
      deserializeJson(doc, received);

      String cmd = doc["cmd"];
      if (cmd == "UPDATE_DATA") {
// Update local data from ESP32
        emergencyMode = doc["emergency"];
        if (emergencyMode) {
          activateLocalAlarm();
        }
      } else if (cmd == "EMERGENCY") {
        triggerLocalEmergency();
      }
    }
  }
}

void sendDataToESP() {
  DynamicJsonDocument doc(256);
  doc["heartRate"] = heartRate;
  doc["temperature"] = temperature;
  doc["sensorConnected"] = pulseSensorConnected;
  doc["timestamp"] = millis();

  String jsonString;
  serializeJson(doc, jsonString);
  espSerial.println(jsonString);
}

void triggerLocalEmergency() {
  emergencyMode = true;
  Serial.println("Local emergency triggered!");

// Notify ESP32
  espSerial.println("{\"cmd\":\"EMERGENCY\",\"source\":\"nano\"}");

// Activate local alarms
  activateLocalAlarm();
}

void activateLocalAlarm() {
// LED indication
  digitalWrite(LED_PIN, HIGH);

// Buzzer and vibrator pattern
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(VIBRATOR1_PIN, HIGH);
    digitalWrite(VIBRATOR2_PIN, HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(VIBRATOR1_PIN, LOW);
    digitalWrite(VIBRATOR2_PIN, LOW);
    delay(500);
  }

  digitalWrite(LED_PIN, LOW);
}

void updateLocalDisplay() {
  oled.clear();
  oled.setFont(Adafruit5x7);

// Title
  oled.println("RescueNet Nano");
  oled.println("==============");

// Status
  if (emergencyMode) {
    oled.println("STATUS: EMERGENCY");
  } else {
    oled.println("STATUS: MONITORING");
  }

  oled.println();

// Sensor data
  oled.print("Heart Rate: ");
  if (pulseSensorConnected) {
    oled.print(heartRate);
    oled.println(" BPM");
  } else {
    oled.println("No Sensor");
  }

  oled.print("Temperature: ");
  if (temperature > 0) {
    oled.print(temperature, 1);
    oled.println(" C");
  } else {
    oled.println("No Sensor");
  }

  oled.println();
  oled.print("Uptime: ");
  oled.print(millis() / 1000);
  oled.println("s");
}
