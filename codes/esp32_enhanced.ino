/*
 * RescueNet AI - ESP32 Health Monitoring Device (Enhanced Version with SIM800L)
 * Version: 6.12.1
 * Description: IoT device for continuous health monitoring and emergency detection with SMS capability
 * 
 * Hardware Requirements:
 * - ESP32 Development Board
 * - MAX30102 Heart Rate & SpO2 Sensor
 * - DS18B20 Temperature Sensor
 * - MPU6050 Accelerometer/Gyroscope
 * - SIM800L GSM Module (for SMS emergency alerts)
 * - OLED Display 128x64 (optional)
 * - GPS Module (optional)
 * - Buzzer for emergency alerts
 * - LED indicators
 * - Antenna for SIM800L
 */

#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <MPU6050.h>
#include <MAX30105.h>
#include <heartRate.h>
#include <SSD1306Wire.h>
#include <HTTPClient.h>
#include <HardwareSerial.h>
#include <time.h>

// Pin Definitions
#define TEMP_SENSOR_PIN 4
#define BUZZER_PIN 2
#define LED_STATUS_PIN 5
#define LED_EMERGENCY_PIN 18
#define BUTTON_EMERGENCY_PIN 0
#define SDA_PIN 21
#define SCL_PIN 22

// SIM800L Pin Definitions
#define SIM800L_TX_PIN 17
#define SIM800L_RX_PIN 16
#define SIM800L_RST_PIN 14
#define SIM800L_PWR_PIN 15

// WiFi Configuration
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// SIM800L Configuration
HardwareSerial sim800l(2);
String emergencyContact = "+1234567890"; // Emergency contact number
bool smsEnabled = true;
bool sim800lReady = false;

// Server Configuration
const char* serverHost = "192.168.1.100"; // Change to your server IP
const int serverPort = 8080;
const char* apiEndpoint = "http://192.168.1.100:3000/api/health-data";

// User Configuration
String userId = "1234567890"; // User's phone number

// Sensor Objects
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature temperatureSensor(&oneWire);
MPU6050 mpu;
MAX30105 particleSensor;
SSD1306Wire display(0x3c, SDA_PIN, SCL_PIN);

// WebSocket Client
WebSocketsClient webSocket;

// Global Variables
float heartRate = 0;
float temperature = 0;
float bloodPressure = 0; // Simulated for now
float accelX, accelY, accelZ;
bool emergencyDetected = false;
bool wifiConnected = false;
unsigned long lastSensorRead = 0;
unsigned long lastDataSend = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long buttonPressTime = 0;
bool buttonPressed = false;

// Emergency thresholds
const float HEART_RATE_MIN = 50.0;
const float HEART_RATE_MAX = 120.0;
const float TEMP_MIN = 35.0;
const float TEMP_MAX = 38.5;
const float FALL_THRESHOLD = 15.0;

void setup() {
  Serial.begin(115200);
  Serial.println("RescueNet AI - ESP32 Health Monitor Starting...");
    // Initialize pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_STATUS_PIN, OUTPUT);
  pinMode(LED_EMERGENCY_PIN, OUTPUT);
  pinMode(BUTTON_EMERGENCY_PIN, INPUT_PULLUP);
  pinMode(SIM800L_RST_PIN, OUTPUT);
  pinMode(SIM800L_PWR_PIN, OUTPUT);
  
  // Initialize I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // Initialize sensors
  initializeSensors();
  
  // Initialize display
  initializeDisplay();
  
  // Initialize SIM800L
  initializeSIM800L();
  
  // Connect to WiFi
  connectToWiFi();
  
  // Initialize WebSocket connection
  initializeWebSocket();
  
  // Configure time
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  
  Serial.println("System initialized successfully!");
  displayMessage("System Ready", "Monitoring...");
  digitalWrite(LED_STATUS_PIN, HIGH);
}

void loop() {
  // Handle WebSocket
  webSocket.loop();
  
  // Check emergency button
  checkEmergencyButton();
  
  // Read sensors every 5 seconds
  if (millis() - lastSensorRead > 5000) {
    readSensors();
    detectEmergency();
    lastSensorRead = millis();
  }
    // Send data every 30 seconds
  if (millis() - lastDataSend > 30000) {
    sendHealthData();
    lastDataSend = millis();
  }
  
  // Check SIM800L status
  checkSIM800LStatus();
  
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

void initializeSensors() {
  Serial.println("Initializing sensors...");
  
  // Temperature sensor
  temperatureSensor.begin();
  
  // MPU6050 accelerometer
  if (mpu.begin()) {
    Serial.println("MPU6050 initialized");
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
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
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.clear();
  display.drawString(0, 0, "RescueNet AI");
  display.drawString(0, 16, "Initializing...");
  display.display();
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println();
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());
    displayMessage("WiFi Connected", WiFi.localIP().toString());
  } else {
    Serial.println("Failed to connect to WiFi");
    displayMessage("WiFi Failed", "Check settings");
  }
}

void initializeWebSocket() {
  if (wifiConnected) {
    webSocket.begin(serverHost, serverPort, "/");
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000);
    Serial.println("WebSocket initialized");
  }
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("WebSocket Disconnected");
      break;
      
    case WStype_CONNECTED:
      Serial.printf("WebSocket Connected to: %s\n", payload);
      // Subscribe to user-specific messages
      String subscribeMessage = "{\"type\":\"subscribe\",\"userId\":\"" + userId + "\"}";
      webSocket.sendTXT(subscribeMessage);
      break;
      
    case WStype_TEXT:
      Serial.printf("Received: %s\n", payload);
      handleWebSocketMessage((char*)payload);
      break;
      
    default:
      break;
  }
}

void handleWebSocketMessage(String message) {
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, message);
  
  String type = doc["type"];
  
  if (type == "emergency_response") {
    Serial.println("Emergency response received!");
    displayMessage("Emergency", "Help is coming!");
    // Flash LED to indicate response
    for (int i = 0; i < 10; i++) {
      digitalWrite(LED_EMERGENCY_PIN, HIGH);
      delay(100);
      digitalWrite(LED_EMERGENCY_PIN, LOW);
      delay(100);
    }
  } else if (type == "health_alert") {
    String alert = doc["data"]["message"];
    displayMessage("Health Alert", alert);
    tone(BUZZER_PIN, 1000, 500);
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
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  accelX = a.acceleration.x;
  accelY = a.acceleration.y;
  accelZ = a.acceleration.z;
  
  // Read heart rate
  long irValue = particleSensor.getIR();
  if (checkForBeat(irValue)) {
    static unsigned long lastBeat = 0;
    long delta = millis() - lastBeat;
    lastBeat = millis();
    
    static byte rateArray[4];
    static byte rateSpot = 0;
    
    rateArray[rateSpot++] = (byte)(60000 / delta);
    rateSpot %= 4;
    
    long total = 0;
    for (byte i = 0; i < 4; i++) {
      total += rateArray[i];
    }
    heartRate = total / 4;
  }
  
  // Simulate blood pressure (would need actual BP sensor)
  bloodPressure = 100 + random(-20, 40);
  
  Serial.printf("Vitals - HR: %.1f, Temp: %.1f°C, BP: %.1f, Accel: %.1f,%.1f,%.1f\n", 
                heartRate, temperature, bloodPressure, accelX, accelY, accelZ);
}

void detectEmergency() {
  bool emergency = false;
  String reason = "";
  
  // Check vital signs
  if (heartRate > HEART_RATE_MAX || heartRate < HEART_RATE_MIN) {
    emergency = true;
    reason = "Abnormal heart rate: " + String(heartRate) + " BPM";
  }
  
  if (temperature > TEMP_MAX || temperature < TEMP_MIN) {
    emergency = true;
    if (reason.length() > 0) reason += "; ";
    reason += "Abnormal temperature: " + String(temperature) + "°C";
  }
  
  // Check for fall detection
  float totalAccel = sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ);
  if (totalAccel > FALL_THRESHOLD) {
    emergency = true;
    if (reason.length() > 0) reason += "; ";
    reason += "Fall detected";
  }
  
  if (emergency && !emergencyDetected) {
    emergencyDetected = true;
    triggerEmergency(reason);
  }
}

void checkEmergencyButton() {
  bool currentState = digitalRead(BUTTON_EMERGENCY_PIN) == LOW;
  
  if (currentState && !buttonPressed) {
    buttonPressTime = millis();
    buttonPressed = true;
  } else if (!currentState && buttonPressed) {
    buttonPressed = false;
    // Check if button was held for more than 2 seconds
    if (millis() - buttonPressTime > 2000) {
      triggerEmergency("Manual emergency button pressed");
    }
  }
}

void triggerEmergency(String reason) {
  Serial.println("EMERGENCY TRIGGERED: " + reason);
  
  // Visual and audio alerts
  digitalWrite(LED_EMERGENCY_PIN, HIGH);
  tone(BUZZER_PIN, 2000, 1000);
  
  displayMessage("EMERGENCY!", reason);
  
  // Send emergency notification via WebSocket
  sendEmergencyAlert(reason);
  
  // Send emergency SMS
  sendEmergencySMS();
  
  emergencyDetected = true;
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
  if (!wifiConnected) return;
  
  // Create JSON payload
  DynamicJsonDocument doc(1024);
  doc["userId"] = userId;
  doc["timestamp"] = getTimeString();
  
  JsonObject vitals = doc.createNestedObject("vitals");
  vitals["heartRate"] = heartRate;
  vitals["temperature"] = temperature;
  vitals["bloodPressure"] = bloodPressure;
  
  JsonObject location = doc.createNestedObject("location");
  location["lat"] = 21.1458; // Nagpur coordinates (would use GPS in production)
  location["lng"] = 79.0882;
  
  JsonObject accelerometer = doc.createNestedObject("accelerometer");
  accelerometer["x"] = accelX;
  accelerometer["y"] = accelY;
  accelerometer["z"] = accelZ;
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Send via HTTP
  HTTPClient http;
  http.begin(apiEndpoint);
  http.addHeader("Content-Type", "application/json");
  
  int httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Data sent successfully: " + String(httpResponseCode));
  } else {
    Serial.println("Error sending data: " + String(httpResponseCode));
  }
  
  http.end();
  
  // Also send via WebSocket if connected
  if (webSocket.isConnected()) {
    webSocket.sendTXT(jsonString);
  }
}

void sendEmergencyAlert(String reason) {
  if (!wifiConnected) return;
  
  DynamicJsonDocument doc(1024);
  doc["userId"] = userId;
  doc["reason"] = reason;
  doc["timestamp"] = getTimeString();
  
  JsonObject location = doc.createNestedObject("location");
  location["lat"] = 21.1458;
  location["lng"] = 79.0882;
  
  JsonObject vitals = doc.createNestedObject("vitals");
  vitals["heartRate"] = heartRate;
  vitals["temperature"] = temperature;
  vitals["bloodPressure"] = bloodPressure;
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Send emergency alert
  HTTPClient http;
  http.begin("http://192.168.1.100:3000/api/emergency");
  http.addHeader("Content-Type", "application/json");
  
  int httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode > 0) {
    Serial.println("Emergency alert sent: " + String(httpResponseCode));
  } else {
    Serial.println("Failed to send emergency alert: " + String(httpResponseCode));
  }
  
  http.end();
}

// SIM800L Functions
void initializeSIM800L() {
  Serial.println("Initializing SIM800L GSM Module...");
  
  // Power cycle SIM800L
  digitalWrite(SIM800L_PWR_PIN, LOW);
  delay(1000);
  digitalWrite(SIM800L_PWR_PIN, HIGH);
  delay(2000);
  
  // Reset SIM800L
  digitalWrite(SIM800L_RST_PIN, LOW);
  delay(100);
  digitalWrite(SIM800L_RST_PIN, HIGH);
  delay(3000);
  
  // Initialize serial communication
  sim800l.begin(9600, SERIAL_8N1, SIM800L_RX_PIN, SIM800L_TX_PIN);
  delay(3000);
  
  // Check if SIM800L is responsive
  sim800l.println("AT");
  delay(1000);
  
  if (sim800l.available()) {
    String response = sim800l.readString();
    if (response.indexOf("OK") > -1) {
      Serial.println("SIM800L: Connected successfully");
      sim800lReady = true;
      
      // Configure SMS settings
      configureSMS();
    } else {
      Serial.println("SIM800L: Failed to respond");
      sim800lReady = false;
    }
  } else {
    Serial.println("SIM800L: No response");
    sim800lReady = false;
  }
}

void configureSMS() {
  Serial.println("Configuring SMS settings...");
  
  // Set SMS text mode
  sim800l.println("AT+CMGF=1");
  delay(1000);
  
  // Set character set
  sim800l.println("AT+CSCS=\"GSM\"");
  delay(1000);
  
  // Check network registration
  sim800l.println("AT+CREG?");
  delay(1000);
  
  // Check signal strength
  sim800l.println("AT+CSQ");
  delay(1000);
  
  Serial.println("SMS configuration complete");
}

bool sendSMS(String phoneNumber, String message) {
  if (!sim800lReady || !smsEnabled) {
    Serial.println("SIM800L not ready or SMS disabled");
    return false;
  }
  
  Serial.println("Sending SMS to: " + phoneNumber);
  Serial.println("Message: " + message);
  
  // Set SMS recipient
  sim800l.println("AT+CMGS=\"" + phoneNumber + "\"");
  delay(1000);
  
  // Send message
  sim800l.print(message);
  delay(100);
  
  // Send Ctrl+Z to send SMS
  sim800l.println((char)26);
  delay(5000);
  
  // Check response
  if (sim800l.available()) {
    String response = sim800l.readString();
    Serial.println("SMS Response: " + response);
    
    if (response.indexOf("OK") > -1) {
      Serial.println("SMS sent successfully");
      return true;
    }
  }
  
  Serial.println("Failed to send SMS");
  return false;
}

void sendEmergencySMS() {
  if (!sim800lReady) {
    Serial.println("Cannot send emergency SMS - SIM800L not ready");
    return;
  }
  
  String emergencyMessage = "🚨 EMERGENCY ALERT - RescueNet AI 🚨\n";
  emergencyMessage += "User: " + userId + "\n";
  emergencyMessage += "Time: " + getTimeString() + "\n";
  emergencyMessage += "Heart Rate: " + String((int)heartRate) + " BPM\n";
  emergencyMessage += "Temperature: " + String(temperature, 1) + "°C\n";
  emergencyMessage += "SpO2: " + String((int)spO2) + "%\n";
  emergencyMessage += "Location: GPS coordinates if available\n";
  emergencyMessage += "Please respond immediately!";
  
  // Send to emergency contact
  bool smsSent = sendSMS(emergencyContact, emergencyMessage);
  
  if (smsSent) {
    Serial.println("Emergency SMS sent successfully");
    displayMessage("Emergency SMS", "Sent to contact");
  } else {
    Serial.println("Failed to send emergency SMS");
    displayMessage("SMS Failed", "Check SIM card");
  }
}

void checkSIM800LStatus() {
  if (!sim800lReady) return;
  
  // Check signal strength periodically
  static unsigned long lastSignalCheck = 0;
  if (millis() - lastSignalCheck > 60000) { // Check every minute
    sim800l.println("AT+CSQ");
    delay(1000);
    
    if (sim800l.available()) {
      String response = sim800l.readString();
      Serial.println("Signal strength: " + response);
    }
    
    lastSignalCheck = millis();
  }
}

void updateDisplay() {
  display.clear();
  
  // Title
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "RescueNet AI");
  
  // WiFi status
  display.drawString(85, 0, wifiConnected ? "WiFi OK" : "No WiFi");
  
  // Vitals
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 16, "HR: " + String((int)heartRate));
  display.drawString(0, 32, "Temp: " + String(temperature, 1) + "C");
  display.drawString(0, 48, "Status: " + String(emergencyDetected ? "EMERGENCY" : "Normal"));
  
  display.display();
}

void displayMessage(String title, String message) {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, title);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 20, message);
  display.display();
}

String getTimeString() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return String(millis());
  }
  
  char timeString[64];
  strftime(timeString, sizeof(timeString), "%Y-%m-%dT%H:%M:%S.000Z", &timeinfo);
  return String(timeString);
}
