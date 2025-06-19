/*
 * RescueNet AI - Arduino Nano Health Monitor
 * Version: 6.12 (Nano Edition)
 * 
 * This version is for Arduino Nano + ESP8266 WiFi module
 * Simpler alternative to ESP32 for basic health monitoring
 * 
 * Hardware Requirements:
 * - Arduino Nano V3.0
 * - ESP8266 WiFi Module (ESP-01)
 * - DS18B20 Temperature Sensor
 * - MPU6050 Accelerometer/Gyroscope
 * - MAX30102 Heart Rate Sensor
 * - SSD1306 OLED Display
 * - Buzzer and LEDs
 * - Emergency Button
 */

#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <MAX30105.h>
#include <heartRate.h>
#include <MPU6050.h>

// Pin Definitions for Arduino Nano
#define TEMP_SENSOR_PIN 4      // DS18B20 temperature sensor
#define BUZZER_PIN 7           // Active buzzer
#define LED_STATUS_PIN 5       // Green status LED
#define LED_EMERGENCY_PIN 6    // Red emergency LED
#define BUTTON_EMERGENCY_PIN 2 // Emergency button (interrupt pin)
#define ESP8266_RX_PIN 8       // ESP8266 RX (connected to Nano TX)
#define ESP8266_TX_PIN 9       // ESP8266 TX (connected to Nano RX)

// I2C Pins (A4=SDA, A5=SCL on Nano)
#define SDA_PIN A4
#define SCL_PIN A5

// Display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// WiFi Configuration (for ESP8266)
const String WIFI_SSID = "YOUR_WIFI_SSID";
const String WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const String SERVER_IP = "192.168.1.100";
const String SERVER_PORT = "3000";

// User Configuration
String userId = "1234567890"; // User's phone number

// Sensor Objects
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature temperatureSensor(&oneWire);
MPU6050 mpu;
MAX30105 particleSensor;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SoftwareSerial esp8266(ESP8266_TX_PIN, ESP8266_RX_PIN);

// Global Variables
float heartRate = 0;
float temperature = 0;
float bloodPressure = 0; // Simulated
float accelX, accelY, accelZ;
bool emergencyDetected = false;
bool wifiConnected = false;
unsigned long lastSensorRead = 0;
unsigned long lastDataSend = 0;
unsigned long lastDisplayUpdate = 0;
volatile bool emergencyButtonPressed = false;

// Emergency thresholds
const float HEART_RATE_MIN = 50.0;
const float HEART_RATE_MAX = 120.0;
const float TEMP_MIN = 35.0;
const float TEMP_MAX = 38.5;
const float FALL_THRESHOLD = 15.0;

void setup() {
  Serial.begin(9600);
  esp8266.begin(9600);
  
  Serial.println("RescueNet AI - Arduino Nano Starting...");
  
  // Initialize pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_STATUS_PIN, OUTPUT);
  pinMode(LED_EMERGENCY_PIN, OUTPUT);
  pinMode(BUTTON_EMERGENCY_PIN, INPUT_PULLUP);
  
  // Emergency button interrupt
  attachInterrupt(digitalPinToInterrupt(BUTTON_EMERGENCY_PIN), emergencyButtonISR, FALLING);
  
  // Initialize I2C
  Wire.begin();
  
  // Initialize sensors
  initializeSensors();
  
  // Initialize display
  initializeDisplay();
  
  // Initialize ESP8266 WiFi
  initializeWiFi();
  
  Serial.println("System initialized successfully!");
  displayMessage("System Ready", "Monitoring...");
  digitalWrite(LED_STATUS_PIN, HIGH);
}

void loop() {
  // Check emergency button
  if (emergencyButtonPressed) {
    emergencyButtonPressed = false;
    triggerEmergency("Manual emergency button pressed");
  }
  
  // Read sensors every 5 seconds
  if (millis() - lastSensorRead > 5000) {
    readSensors();
    detectEmergency();
    lastSensorRead = millis();
  }
  
  // Send data every 30 seconds
  if (millis() - lastDataSend > 30000) {
    if (wifiConnected) {
      sendHealthData();
    }
    lastDataSend = millis();
  }
  
  // Update display every 2 seconds
  if (millis() - lastDisplayUpdate > 2000) {
    updateDisplay();
    lastDisplayUpdate = millis();
  }
  
  // Handle emergency state
  if (emergencyDetected) {
    handleEmergency();
  }
  
  delay(100);
}

void emergencyButtonISR() {
  emergencyButtonPressed = true;
}

void initializeSensors() {
  Serial.println("Initializing sensors...");
  
  // Temperature sensor
  temperatureSensor.begin();
  
  // MPU6050 accelerometer
  if (mpu.initialize()) {
    Serial.println("MPU6050 initialized");
  } else {
    Serial.println("Failed to initialize MPU6050");
  }
  
  // MAX30105 heart rate sensor
  if (particleSensor.begin()) {
    Serial.println("MAX30105 initialized");
    particleSensor.setup();
    particleSensor.setPulseAmplitudeRed(0x0A);
    particleSensor.setPulseAmplitudeGreen(0);
  } else {
    Serial.println("Failed to initialize MAX30105");
  }
}

void initializeDisplay() {
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialized");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("RescueNet AI");
    display.println("Initializing...");
    display.display();
  } else {
    Serial.println("Failed to initialize OLED");
  }
}

void initializeWiFi() {
  Serial.println("Initializing ESP8266...");
  
  // Reset ESP8266
  esp8266.println("AT+RST");
  delay(2000);
  
  // Set to station mode
  esp8266.println("AT+CWMODE=1");
  delay(1000);
  
  // Connect to WiFi
  String connectCmd = "AT+CWJAP=\"" + WIFI_SSID + "\",\"" + WIFI_PASSWORD + "\"";
  esp8266.println(connectCmd);
  delay(5000);
  
  if (esp8266.find("OK")) {
    wifiConnected = true;
    Serial.println("WiFi connected!");
    displayMessage("WiFi Connected", "Ready to monitor");
  } else {
    Serial.println("WiFi connection failed");
    displayMessage("WiFi Failed", "Check settings");
  }
}

void readSensors() {
  // Read temperature
  temperatureSensor.requestTemperatures();
  temperature = temperatureSensor.getTempCByIndex(0);
  if (temperature == DEVICE_DISCONNECTED_C) {
    temperature = 36.5 + random(-10, 10) / 10.0; // Fallback simulation
  }
  
  // Read accelerometer
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);
  accelX = ax / 16384.0; // Convert to g-force
  accelY = ay / 16384.0;
  accelZ = az / 16384.0;
  
  // Read heart rate (simplified)
  long irValue = particleSensor.getIR();
  if (checkForBeat(irValue)) {
    static unsigned long lastBeat = 0;
    long delta = millis() - lastBeat;
    lastBeat = millis();
    
    if (delta > 300 && delta < 3000) { // Valid heart rate range
      heartRate = 60000 / delta;
    }
  }
  
  // Simulate blood pressure
  bloodPressure = 100 + random(-20, 40);
  
  Serial.print("Vitals - HR: ");
  Serial.print(heartRate);
  Serial.print(", Temp: ");
  Serial.print(temperature);
  Serial.print("C, Accel: ");
  Serial.print(sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ));
  Serial.println("g");
}

void detectEmergency() {
  bool emergency = false;
  String reason = "";
  
  // Check vital signs
  if (heartRate > HEART_RATE_MAX || (heartRate > 0 && heartRate < HEART_RATE_MIN)) {
    emergency = true;
    reason = "Abnormal heart rate: " + String(heartRate) + " BPM";
  }
  
  if (temperature > TEMP_MAX || temperature < TEMP_MIN) {
    emergency = true;
    if (reason.length() > 0) reason += "; ";
    reason += "Abnormal temperature: " + String(temperature) + "C";
  }
  
  // Check for fall detection
  float totalAccel = sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ);
  if (totalAccel > FALL_THRESHOLD) {
    emergency = true;
    if (reason.length() > 0) reason += "; ";
    reason += "Fall detected";
  }
  
  if (emergency && !emergencyDetected) {
    triggerEmergency(reason);
  }
}

void triggerEmergency(String reason) {
  Serial.println("EMERGENCY TRIGGERED: " + reason);
  
  emergencyDetected = true;
  
  // Visual and audio alerts
  digitalWrite(LED_EMERGENCY_PIN, HIGH);
  tone(BUZZER_PIN, 2000, 1000);
  
  displayMessage("EMERGENCY!", reason.substring(0, 20));
  
  // Send emergency notification
  if (wifiConnected) {
    sendEmergencyAlert(reason);
  }
}

void handleEmergency() {
  // Flash emergency LED
  static unsigned long lastFlash = 0;
  if (millis() - lastFlash > 500) {
    digitalWrite(LED_EMERGENCY_PIN, !digitalRead(LED_EMERGENCY_PIN));
    lastFlash = millis();
  }
  
  // Periodic emergency beep
  static unsigned long lastBeep = 0;
  if (millis() - lastBeep > 5000) {
    tone(BUZZER_PIN, 1500, 200);
    lastBeep = millis();
  }
}

void sendHealthData() {
  // Create JSON data string
  String jsonData = "{";
  jsonData += "\"userId\":\"" + userId + "\",";
  jsonData += "\"timestamp\":\"" + String(millis()) + "\",";
  jsonData += "\"vitals\":{";
  jsonData += "\"heartRate\":" + String(heartRate) + ",";
  jsonData += "\"temperature\":" + String(temperature) + ",";
  jsonData += "\"bloodPressure\":" + String(bloodPressure);
  jsonData += "},";
  jsonData += "\"accelerometer\":{";
  jsonData += "\"x\":" + String(accelX) + ",";
  jsonData += "\"y\":" + String(accelY) + ",";
  jsonData += "\"z\":" + String(accelZ);
  jsonData += "}";
  jsonData += "}";
  
  // Send HTTP POST request via ESP8266
  sendHTTPPost("/api/health-data", jsonData);
}

void sendEmergencyAlert(String reason) {
  // Create emergency JSON data
  String jsonData = "{";
  jsonData += "\"userId\":\"" + userId + "\",";
  jsonData += "\"reason\":\"" + reason + "\",";
  jsonData += "\"timestamp\":\"" + String(millis()) + "\",";
  jsonData += "\"vitals\":{";
  jsonData += "\"heartRate\":" + String(heartRate) + ",";
  jsonData += "\"temperature\":" + String(temperature) + ",";
  jsonData += "\"bloodPressure\":" + String(bloodPressure);
  jsonData += "}";
  jsonData += "}";
  
  // Send emergency alert
  sendHTTPPost("/api/emergency", jsonData);
}

void sendHTTPPost(String endpoint, String data) {
  // Start TCP connection
  String startCmd = "AT+CIPSTART=\"TCP\",\"" + SERVER_IP + "\"," + SERVER_PORT;
  esp8266.println(startCmd);
  delay(2000);
  
  if (!esp8266.find("OK")) {
    Serial.println("TCP connection failed");
    return;
  }
  
  // Prepare HTTP request
  String httpRequest = "POST " + endpoint + " HTTP/1.1\r\n";
  httpRequest += "Host: " + SERVER_IP + ":" + SERVER_PORT + "\r\n";
  httpRequest += "Content-Type: application/json\r\n";
  httpRequest += "Content-Length: " + String(data.length()) + "\r\n";
  httpRequest += "Connection: close\r\n\r\n";
  httpRequest += data;
  
  // Send data length
  String sendCmd = "AT+CIPSEND=" + String(httpRequest.length());
  esp8266.println(sendCmd);
  delay(1000);
  
  if (esp8266.find(">")) {
    // Send HTTP request
    esp8266.print(httpRequest);
    delay(2000);
    
    if (esp8266.find("OK")) {
      Serial.println("Data sent successfully");
    } else {
      Serial.println("Failed to send data");
    }
  }
  
  // Close connection
  esp8266.println("AT+CIPCLOSE");
  delay(1000);
}

void updateDisplay() {
  display.clearDisplay();
  
  // Title
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("RescueNet AI");
  
  // WiFi status
  display.setCursor(85, 0);
  display.println(wifiConnected ? "WiFi OK" : "No WiFi");
  
  // Vitals
  display.setTextSize(1);
  display.setCursor(0, 16);
  display.print("HR: ");
  display.print((int)heartRate);
  display.println(" BPM");
  
  display.setCursor(0, 28);
  display.print("Temp: ");
  display.print(temperature, 1);
  display.println("C");
  
  display.setCursor(0, 40);
  display.print("Accel: ");
  display.print(sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ), 1);
  display.println("g");
  
  display.setCursor(0, 52);
  display.print("Status: ");
  display.println(emergencyDetected ? "EMERGENCY" : "Normal");
  
  display.display();
}

void displayMessage(String title, String message) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(title);
  display.setCursor(0, 16);
  display.println(message);
  display.display();
  delay(2000);
}
