#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TinyGPS++.h>
#include <PubSubClient.h>
#include <time.h>

// Pin Definitions
#define GPS_RX 16
#define GPS_TX 17
#define TEMP_PIN 4
#define PULSE_PIN 32
#define ACCEL_X 33
#define ACCEL_Y 34
#define ACCEL_Z 35
#define BUZZER_PIN 5
#define VIBRATOR1_PIN 18
#define VIBRATOR2_PIN 19
#define BUTTON_PIN 0
#define LED_PIN 2
#define BATTERY_PIN 36
#define SD_CS_PIN 15

// Display Settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// Global Objects
WiFiClient wifiClient;
HTTPClient http;
PubSubClient mqttClient(wifiClient);
SoftwareSerial gpsSerial(GPS_RX, GPS_TX);
TinyGPSPlus gps;
Adafruit_BMP085 bmp;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
OneWire oneWire(TEMP_PIN);
DallasTemperature tempSensor(&oneWire);

// Configuration
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* server_url = "http://your-server.com/api";
const char* telegram_bot_token = "YOUR_TELEGRAM_BOT_TOKEN";
const char* telegram_chat_id = "YOUR_CHAT_ID";

// User Data Structure
struct UserData {
  String name;
  String phone;
  String emergencyContact;
  int age;
  String gender;
  String bloodType;
  String medicalHistory;
  String lastPeriodDate;
  bool isPregnant;
};

struct HealthData {
  float temperature;
  float heartRate;
  float bloodPressure_sys;
  float bloodPressure_dia;
  double latitude;
  double longitude;
  float altitude;
  float accelX, accelY, accelZ;
  bool fallDetected;
  int batteryLevel;
  unsigned long timestamp;
};

UserData user;
HealthData healthData;
bool emergencyMode = false;
unsigned long lastHealthCheck = 0;
unsigned long lastDataSend = 0;

void setup() {
  Serial.begin(115200);
  
  // Initialize pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(VIBRATOR1_PIN, OUTPUT);
  pinMode(VIBRATOR2_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  // Initialize I2C
  Wire.begin();
  
  // Initialize sensors
  initializeSensors();
  
  // Initialize WiFi
  connectWiFi();
  
  // Initialize SD card
  initializeSD();
  
  // Load user data
  loadUserData();
  
  // Initialize MQTT
  mqttClient.setServer("mqtt.eclipse.org", 1883);
  mqttClient.setCallback(mqttCallback);
  
  Serial.println("RescueNet AI - Emergency Response System Initialized");
  displayStatus("System Ready");
}

void loop() {
  unsigned long currentTime = millis();
  
  // Handle emergency button
  if (digitalRead(BUTTON_PIN) == LOW) {
    handleEmergencyButton();
  }
  
  // Read sensor data every 5 seconds
  if (currentTime - lastHealthCheck > 5000) {
    readSensorData();
    analyzeHealthData();
    lastHealthCheck = currentTime;
  }
  
  // Send data to server every 30 seconds
  if (currentTime - lastDataSend > 30000) {
    sendDataToServer();
    lastDataSend = currentTime;
  }
  
  // Handle GPS data
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      if (gps.location.isValid()) {
        healthData.latitude = gps.location.lat();
        healthData.longitude = gps.location.lng();
        healthData.altitude = gps.altitude.meters();
      }
    }
  }
  
  // Maintain MQTT connection
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();
  
  // Update display
  updateDisplay();
  
  delay(100);
}

void initializeSensors() {
  // Initialize BMP180
  if (!bmp.begin()) {
    Serial.println("Could not find BMP180 sensor");
  }
  
  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Initialize temperature sensor
  tempSensor.begin();
  
  // Initialize GPS
  gpsSerial.begin(9600);
  
  Serial.println("Sensors initialized successfully");
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.println("IP address: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nFailed to connect to WiFi");
  }
}

void initializeSD() {
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD Card initialization failed");
    return;
  }
  Serial.println("SD Card initialized successfully");
}

void loadUserData() {
  // Load from SD card or use defaults
  user.name = "John Doe";
  user.phone = "+919876543210";
  user.emergencyContact = "+919876543211";
  user.age = 25;
  user.gender = "Male";
  user.bloodType = "O+";
  user.medicalHistory = "None";
  user.lastPeriodDate = "2024-06-01";
  user.isPregnant = false;
}

void readSensorData() {
  // Read temperature
  tempSensor.requestTemperatures();
  healthData.temperature = tempSensor.getTempCByIndex(0);
  
  // Read pulse sensor
  int pulseValue = analogRead(PULSE_PIN);
  healthData.heartRate = calculateHeartRate(pulseValue);
  
  // Read accelerometer
  healthData.accelX = (analogRead(ACCEL_X) - 512) * 3.3 / 512;
  healthData.accelY = (analogRead(ACCEL_Y) - 512) * 3.3 / 512;
  healthData.accelZ = (analogRead(ACCEL_Z) - 512) * 3.3 / 512;
  
  // Detect fall
  float totalAccel = sqrt(pow(healthData.accelX, 2) + pow(healthData.accelY, 2) + pow(healthData.accelZ, 2));
  healthData.fallDetected = (totalAccel > 3.0 || totalAccel < 0.5);
  
  // Read battery level
  int batteryReading = analogRead(BATTERY_PIN);
  healthData.batteryLevel = map(batteryReading, 0, 4095, 0, 100);
  
  // Read atmospheric pressure (for blood pressure estimation)
  float pressure = bmp.readPressure() / 100.0F;
  healthData.bloodPressure_sys = estimateBloodPressure(healthData.heartRate, true);
  healthData.bloodPressure_dia = estimateBloodPressure(healthData.heartRate, false);
  
  healthData.timestamp = millis();
}

float calculateHeartRate(int sensorValue) {
  // Simple heart rate calculation - in real implementation, use more sophisticated algorithm
  static int lastValue = 0;
  static unsigned long lastBeat = 0;
  static int beatCount = 0;
  static unsigned long startTime = millis();
  
  if (sensorValue > (lastValue + 100) && (millis() - lastBeat) > 300) {
    beatCount++;
    lastBeat = millis();
    
    if (millis() - startTime > 10000) { // Calculate BPM over 10 seconds
      float bpm = (beatCount * 60000.0) / (millis() - startTime);
      beatCount = 0;
      startTime = millis();
      return bpm;
    }
  }
  
  lastValue = sensorValue;
  return healthData.heartRate; // Return previous value if no new calculation
}

float estimateBloodPressure(float heartRate, bool systolic) {
  // Simplified blood pressure estimation based on heart rate
  // This is a placeholder - real implementation would use more sophisticated methods
  if (systolic) {
    return 80 + (heartRate - 60) * 0.5;
  } else {
    return 50 + (heartRate - 60) * 0.3;
  }
}

void analyzeHealthData() {
  bool emergencyDetected = false;
  String alertMessage = "";
  
  // Heart rate analysis
  if (healthData.heartRate < 50 || healthData.heartRate > 120) {
    emergencyDetected = true;
    alertMessage += "Abnormal heart rate: " + String(healthData.heartRate) + " BPM. ";
  }
  
  // Temperature analysis
  if (healthData.temperature < 35.0 || healthData.temperature > 38.5) {
    emergencyDetected = true;
    alertMessage += "Abnormal temperature: " + String(healthData.temperature) + "°C. ";
  }
  
  // Fall detection
  if (healthData.fallDetected) {
    emergencyDetected = true;
    alertMessage += "Fall detected! ";
  }
  
  // Blood pressure analysis
  if (healthData.bloodPressure_sys > 140 || healthData.bloodPressure_dia > 90) {
    emergencyDetected = true;
    alertMessage += "High blood pressure detected. ";
  }
  
  if (emergencyDetected && !emergencyMode) {
    triggerEmergency(alertMessage);
  }
}

void triggerEmergency(String message) {
  emergencyMode = true;
  
  // Visual and audio alerts
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);
  digitalWrite(VIBRATOR1_PIN, HIGH);
  digitalWrite(VIBRATOR2_PIN, HIGH);
  
  delay(1000);
  
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(VIBRATOR1_PIN, LOW);
  digitalWrite(VIBRATOR2_PIN, LOW);
  
  // Send emergency alerts
  sendTelegramAlert(message);
  sendServerAlert(message);
  saveEmergencyLog(message);
  
  Serial.println("EMERGENCY TRIGGERED: " + message);
  displayStatus("EMERGENCY ACTIVE");
}

void handleEmergencyButton() {
  static unsigned long buttonPressTime = 0;
  static bool buttonPressed = false;
  
  if (!buttonPressed) {
    buttonPressed = true;
    buttonPressTime = millis();
  }
  
  if (millis() - buttonPressTime > 3000) { // Long press for 3 seconds
    triggerEmergency("Manual emergency button activated");
    buttonPressed = false;
  }
  
  if (digitalRead(BUTTON_PIN) == HIGH) {
    buttonPressed = false;
  }
}

void sendTelegramAlert(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    String url = "https://api.telegram.org/bot" + String(telegram_bot_token) + "/sendMessage";
    
    DynamicJsonDocument doc(1024);
    doc["chat_id"] = telegram_chat_id;
    doc["text"] = "🚨 EMERGENCY ALERT 🚨\n\n" + message + 
                  "\n\nUser: " + user.name +
                  "\nPhone: " + user.phone +
                  "\nLocation: " + String(healthData.latitude, 6) + "," + String(healthData.longitude, 6) +
                  "\nTime: " + getCurrentTime() +
                  "\n\nVital Signs:" +
                  "\n❤️ Heart Rate: " + String(healthData.heartRate) + " BPM" +
                  "\n🌡️ Temperature: " + String(healthData.temperature) + "°C" +
                  "\n🩸 BP: " + String(healthData.bloodPressure_sys) + "/" + String(healthData.bloodPressure_dia) +
                  "\n📍 Google Maps: https://maps.google.com/?q=" + String(healthData.latitude, 6) + "," + String(healthData.longitude, 6);
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsonString);
    
    if (httpResponseCode > 0) {
      Serial.println("Telegram alert sent successfully");
    } else {
      Serial.println("Error sending Telegram alert");
    }
    
    http.end();
  }
}

void sendServerAlert(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    DynamicJsonDocument doc(2048);
    doc["type"] = "emergency";
    doc["user_id"] = user.phone;
    doc["message"] = message;
    doc["timestamp"] = getCurrentTime();
    doc["location"]["latitude"] = healthData.latitude;
    doc["location"]["longitude"] = healthData.longitude;
    doc["location"]["altitude"] = healthData.altitude;
    doc["vital_signs"]["heart_rate"] = healthData.heartRate;
    doc["vital_signs"]["temperature"] = healthData.temperature;
    doc["vital_signs"]["blood_pressure"]["systolic"] = healthData.bloodPressure_sys;
    doc["vital_signs"]["blood_pressure"]["diastolic"] = healthData.bloodPressure_dia;
    doc["user_info"]["name"] = user.name;
    doc["user_info"]["phone"] = user.phone;
    doc["user_info"]["emergency_contact"] = user.emergencyContact;
    doc["user_info"]["age"] = user.age;
    doc["user_info"]["gender"] = user.gender;
    doc["user_info"]["blood_type"] = user.bloodType;
    doc["user_info"]["medical_history"] = user.medicalHistory;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    http.begin(server_url + "/emergency");
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsonString);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Server alert sent successfully: " + response);
    } else {
      Serial.println("Error sending server alert");
    }
    
    http.end();
  }
}

void sendDataToServer() {
  if (WiFi.status() == WL_CONNECTED) {
    DynamicJsonDocument doc(1024);
    doc["type"] = "health_data";
    doc["user_id"] = user.phone;
    doc["timestamp"] = getCurrentTime();
    doc["vital_signs"]["heart_rate"] = healthData.heartRate;
    doc["vital_signs"]["temperature"] = healthData.temperature;
    doc["vital_signs"]["blood_pressure"]["systolic"] = healthData.bloodPressure_sys;
    doc["vital_signs"]["blood_pressure"]["diastolic"] = healthData.bloodPressure_dia;
    doc["location"]["latitude"] = healthData.latitude;
    doc["location"]["longitude"] = healthData.longitude;
    doc["location"]["altitude"] = healthData.altitude;
    doc["accelerometer"]["x"] = healthData.accelX;
    doc["accelerometer"]["y"] = healthData.accelY;
    doc["accelerometer"]["z"] = healthData.accelZ;
    doc["battery_level"] = healthData.batteryLevel;
    doc["fall_detected"] = healthData.fallDetected;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    http.begin(server_url + "/health_data");
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsonString);
    
    http.end();
  }
}

void saveEmergencyLog(String message) {
  File logFile = SD.open("/emergency_log.txt", FILE_APPEND);
  if (logFile) {
    logFile.println(getCurrentTime() + " - " + message);
    logFile.close();
    Serial.println("Emergency logged to SD card");
  }
}

void updateDisplay() {
  display.clearDisplay();
  
  // Title
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("RescueNet AI");
  
  // Status
  display.setCursor(0, 12);
  if (emergencyMode) {
    display.println("STATUS: EMERGENCY");
  } else {
    display.println("STATUS: MONITORING");
  }
  
  // Vital signs
  display.setCursor(0, 24);
  display.println("HR: " + String(healthData.heartRate) + " BPM");
  
  display.setCursor(0, 32);
  display.println("TEMP: " + String(healthData.temperature, 1) + "C");
  
  display.setCursor(0, 40);
  display.println("BP: " + String(healthData.bloodPressure_sys) + "/" + String(healthData.bloodPressure_dia));
  
  // Battery and GPS
  display.setCursor(0, 48);
  display.println("BAT: " + String(healthData.batteryLevel) + "%");
  
  display.setCursor(0, 56);
  if (gps.location.isValid()) {
    display.println("GPS: LOCKED");
  } else {
    display.println("GPS: SEARCHING");
  }
  
  display.display();
}

void displayStatus(String status) {
  display.clearDisplay();
  display.setCursor(10, 20);
  display.setTextSize(2);
  display.println(status);
  display.display();
  delay(2000);
}

String getCurrentTime() {
  time_t now = time(nullptr);
  return String(ctime(&now));
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  // Handle MQTT commands
  if (String(topic) == "rescuenet/command") {
    if (message == "emergency_cancel") {
      emergencyMode = false;
      digitalWrite(LED_PIN, LOW);
      Serial.println("Emergency cancelled via MQTT");
    }
  }
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    if (mqttClient.connect("RescueNetDevice")) {
      mqttClient.subscribe("rescuenet/command");
      Serial.println("MQTT connected");
    } else {
      delay(5000);
    }
  }
}