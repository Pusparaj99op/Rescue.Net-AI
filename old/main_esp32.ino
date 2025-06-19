#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <SSD1306.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SD.h>
#include <SPI.h>
#include <time.h>

// Pin definitions
#define PULSE_SENSOR_PIN 34
#define TEMP_SENSOR_PIN 4
#define BUZZER_PIN 25
#define VIBRATOR1_PIN 26
#define VIBRATOR2_PIN 27
#define EMERGENCY_BTN_PIN 0
#define STATUS_LED_PIN 2
#define ACCEL_X_PIN 35
#define ACCEL_Y_PIN 32
#define ACCEL_Z_PIN 33
#define SD_CS_PIN 5

// GPS Serial
SoftwareSerial gpsSerial(16, 17);

// Sensor objects
Adafruit_BMP085 bmp;
SSD1306 display(0x3c, 21, 22);
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature tempSensor(&oneWire);

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Server endpoints
const char* serverURL = "http://your-server.com/api";
const char* telegramToken = "YOUR_TELEGRAM_BOT_TOKEN";
const char* telegramChatID = "YOUR_CHAT_ID";

// User data structure
struct UserData {
  String name;
  String phone;
  String emergencyContact;
  int age;
  String gender;
  String bloodGroup;
  String medicalHistory;
  String lastPeriodDate; // for female users
};

UserData currentUser;

// Health data structure
struct HealthData {
  float heartRate;
  float temperature;
  float bloodPressure;
  float altitude;
  double latitude;
  double longitude;
  float accelX, accelY, accelZ;
  bool emergencyDetected;
  unsigned long timestamp;
};

HealthData currentHealth;

// Global variables
bool emergencyMode = false;
unsigned long lastHeartbeat = 0;
unsigned long lastDataSend = 0;
int heartRateBuffer[10];
int bufferIndex = 0;
bool fallDetected = false;

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600);
  
  // Initialize pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(VIBRATOR1_PIN, OUTPUT);
  pinMode(VIBRATOR2_PIN, OUTPUT);
  pinMode(EMERGENCY_BTN_PIN, INPUT_PULLUP);
  pinMode(STATUS_LED_PIN, OUTPUT);
  
  // Initialize I2C
  Wire.begin(21, 22);
  
  // Initialize display
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  
  // Initialize sensors
  if (!bmp.begin()) {
    Serial.println("BMP180 sensor not found!");
  }
  
  tempSensor.begin();
  
  // Initialize SD card
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD Card initialization failed!");
  }
  
  // Connect to WiFi
  connectToWiFi();
  
  // Load user data
  loadUserData();
  
  // Initialize time
  configTime(19800, 0, "pool.ntp.org"); // IST timezone
  
  displayWelcomeMessage();
  Serial.println("RescueNet AI Device Ready!");
}

void loop() {
  // Read sensors
  readAllSensors();
  
  // Check for emergency conditions
  checkEmergencyConditions();
  
  // Check emergency button
  if (digitalRead(EMERGENCY_BTN_PIN) == LOW) {
    delay(50); // Debounce
    if (digitalRead(EMERGENCY_BTN_PIN) == LOW) {
      triggerEmergency("Manual activation");
    }
  }
  
  // Update display
  updateDisplay();
  
  // Send data to server (every 30 seconds)
  if (millis() - lastDataSend > 30000) {
    sendDataToServer();
    lastDataSend = millis();
  }
  
  // Log data to SD card
  logDataToSD();
  
  delay(1000);
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void readAllSensors() {
  // Read heart rate
  int pulseValue = analogRead(PULSE_SENSOR_PIN);
  currentHealth.heartRate = calculateHeartRate(pulseValue);
  
  // Read temperature
  tempSensor.requestTemperatures();
  currentHealth.temperature = tempSensor.getTempCByIndex(0);
  
  // Read pressure and altitude
  currentHealth.bloodPressure = bmp.readPressure() / 100.0F; // Convert to hPa
  currentHealth.altitude = bmp.readAltitude();
  
  // Read accelerometer
  currentHealth.accelX = (analogRead(ACCEL_X_PIN) - 512) * 3.3 / 1024;
  currentHealth.accelY = (analogRead(ACCEL_Y_PIN) - 512) * 3.3 / 1024;
  currentHealth.accelZ = (analogRead(ACCEL_Z_PIN) - 512) * 3.3 / 1024;
  
  // Read GPS
  readGPS();
  
  // Check for fall detection
  checkFallDetection();
  
  currentHealth.timestamp = millis();
}

float calculateHeartRate(int sensorValue) {
  // Simple heart rate calculation
  heartRateBuffer[bufferIndex] = sensorValue;
  bufferIndex = (bufferIndex + 1) % 10;
  
  // Calculate average and detect peaks
  int sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += heartRateBuffer[i];
  }
  int average = sum / 10;
  
  // Detect peak and calculate BPM
  static unsigned long lastPeak = 0;
  static int peakCount = 0;
  static unsigned long startTime = millis();
  
  if (sensorValue > average + 50 && millis() - lastPeak > 300) {
    lastPeak = millis();
    peakCount++;
    
    if (millis() - startTime > 10000) { // Calculate over 10 seconds
      float bpm = (peakCount * 60.0) / ((millis() - startTime) / 1000.0);
      peakCount = 0;
      startTime = millis();
      return bpm;
    }
  }
  
  return currentHealth.heartRate; // Return previous value if no new calculation
}

void readGPS() {
  while (gpsSerial.available()) {
    String gpsData = gpsSerial.readStringUntil('\n');
    
    if (gpsData.startsWith("$GPGGA")) {
      // Parse NMEA sentence for latitude and longitude
      int commaIndex[15];
      int commaCount = 0;
      
      for (int i = 0; i < gpsData.length(); i++) {
        if (gpsData.charAt(i) == ',') {
          commaIndex[commaCount] = i;
          commaCount++;
        }
      }
      
      if (commaCount >= 6) {
        String latStr = gpsData.substring(commaIndex[1] + 1, commaIndex[2]);
        String latDir = gpsData.substring(commaIndex[2] + 1, commaIndex[3]);
        String lngStr = gpsData.substring(commaIndex[3] + 1, commaIndex[4]);
        String lngDir = gpsData.substring(commaIndex[4] + 1, commaIndex[5]);
        
        if (latStr.length() > 0 && lngStr.length() > 0) {
          currentHealth.latitude = convertDMSToDD(latStr, latDir);
          currentHealth.longitude = convertDMSToDD(lngStr, lngDir);
        }
      }
    }
  }
}

double convertDMSToDD(String dms, String direction) {
  if (dms.length() < 4) return 0.0;
  
  double degrees = dms.substring(0, 2).toDouble();
  double minutes = dms.substring(2).toDouble();
  double dd = degrees + minutes / 60.0;
  
  if (direction == "S" || direction == "W") {
    dd = -dd;
  }
  
  return dd;
}

void checkFallDetection() {
  float totalAccel = sqrt(pow(currentHealth.accelX, 2) + 
                         pow(currentHealth.accelY, 2) + 
                         pow(currentHealth.accelZ, 2));
  
  // Fall detection threshold (adjust based on testing)
  if (totalAccel > 3.0 || totalAccel < 0.5) {
    static unsigned long fallTime = 0;
    if (fallTime == 0) {
      fallTime = millis();
    } else if (millis() - fallTime > 2000) { // Confirm fall after 2 seconds
      fallDetected = true;
      fallTime = 0;
    }
  }
}

void checkEmergencyConditions() {
  bool emergency = false;
  String emergencyReason = "";
  
  // Heart rate anomalies
  if (currentHealth.heartRate > 120 || currentHealth.heartRate < 50) {
    emergency = true;
    emergencyReason += "Heart rate anomaly (" + String(currentHealth.heartRate) + " BPM). ";
  }
  
  // Temperature anomalies
  if (currentHealth.temperature > 39.0 || currentHealth.temperature < 35.0) {
    emergency = true;
    emergencyReason += "Temperature anomaly (" + String(currentHealth.temperature) + "°C). ";
  }
  
  // Fall detection
  if (fallDetected) {
    emergency = true;
    emergencyReason += "Fall detected. ";
    fallDetected = false;
  }
  
  if (emergency && !emergencyMode) {
    triggerEmergency(emergencyReason);
  }
}

void triggerEmergency(String reason) {
  emergencyMode = true;
  currentHealth.emergencyDetected = true;
  
  Serial.println("EMERGENCY TRIGGERED: " + reason);
  
  // Activate alert systems
  digitalWrite(STATUS_LED_PIN, HIGH);
  
  // Sound buzzer pattern
  for (int i = 0; i < 5; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(VIBRATOR1_PIN, HIGH);
    digitalWrite(VIBRATOR2_PIN, HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(VIBRATOR1_PIN, LOW);
    digitalWrite(VIBRATOR2_PIN, LOW);
    delay(300);
  }
  
  // Send emergency alerts
  sendEmergencyAlert(reason);
  sendTelegramAlert(reason);
  
  // Log emergency
  logEmergencyToSD(reason);
  
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "EMERGENCY!");
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 20, reason);
  display.drawString(0, 35, "Help is coming...");
  display.display();
}

void sendEmergencyAlert(String reason) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(String(serverURL) + "/emergency");
    http.addHeader("Content-Type", "application/json");
    
    DynamicJsonDocument doc(1024);
    doc["userId"] = currentUser.phone;
    doc["emergency"] = true;
    doc["reason"] = reason;
    doc["location"]["lat"] = currentHealth.latitude;
    doc["location"]["lng"] = currentHealth.longitude;
    doc["vitals"]["heartRate"] = currentHealth.heartRate;
    doc["vitals"]["temperature"] = currentHealth.temperature;
    doc["timestamp"] = time(nullptr);
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    int httpResponseCode = http.POST(jsonString);
    
    if (httpResponseCode > 0) {
      Serial.println("Emergency alert sent successfully");
    } else {
      Serial.println("Error sending emergency alert");
    }
    
    http.end();
  }
}

void sendTelegramAlert(String reason) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://api.telegram.org/bot" + String(telegramToken) + "/sendMessage";
    
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    
    String message = "🚨 EMERGENCY ALERT 🚨\n\n";
    message += "User: " + currentUser.name + "\n";
    message += "Phone: " + currentUser.phone + "\n";
    message += "Reason: " + reason + "\n";
    message += "Location: " + String(currentHealth.latitude, 6) + ", " + String(currentHealth.longitude, 6) + "\n";
    message += "Heart Rate: " + String(currentHealth.heartRate) + " BPM\n";
    message += "Temperature: " + String(currentHealth.temperature, 1) + "°C\n";
    message += "Time: " + String(time(nullptr)) + "\n\n";
    message += "Google Maps: https://maps.google.com/?q=" + String(currentHealth.latitude, 6) + "," + String(currentHealth.longitude, 6);
    
    DynamicJsonDocument doc(1024);
    doc["chat_id"] = telegramChatID;
    doc["text"] = message;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    int httpResponseCode = http.POST(jsonString);
    http.end();
  }
}

void sendDataToServer() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(String(serverURL) + "/health-data");
    http.addHeader("Content-Type", "application/json");
    
    DynamicJsonDocument doc(2048);
    doc["userId"] = currentUser.phone;
    doc["timestamp"] = time(nullptr);
    doc["vitals"]["heartRate"] = currentHealth.heartRate;
    doc["vitals"]["temperature"] = currentHealth.temperature;
    doc["vitals"]["bloodPressure"] = currentHealth.bloodPressure;
    doc["location"]["lat"] = currentHealth.latitude;
    doc["location"]["lng"] = currentHealth.longitude;
    doc["location"]["altitude"] = currentHealth.altitude;
    doc["accelerometer"]["x"] = currentHealth.accelX;
    doc["accelerometer"]["y"] = currentHealth.accelY;
    doc["accelerometer"]["z"] = currentHealth.accelZ;
    doc["emergency"] = currentHealth.emergencyDetected;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    int httpResponseCode = http.POST(jsonString);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Data sent successfully: " + response);
    }
    
    http.end();
  }
}

void updateDisplay() {
  display.clear();
  
  // Header
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "RescueNet AI");
  display.drawString(80, 0, WiFi.status() == WL_CONNECTED ? "WiFi" : "No WiFi");
  
  // User info
  display.drawString(0, 12, currentUser.name + " (" + String(currentUser.age) + ")");
  
  // Vital signs
  display.drawString(0, 24, "HR: " + String(currentHealth.heartRate, 0) + " BPM");
  display.drawString(70, 24, "Temp: " + String(currentHealth.temperature, 1) + "C");
  
  // Location
  if (currentHealth.latitude != 0 && currentHealth.longitude != 0) {
    display.drawString(0, 36, "GPS: " + String(currentHealth.latitude, 4));
    display.drawString(0, 48, "     " + String(currentHealth.longitude, 4));
  } else {
    display.drawString(0, 36, "GPS: Searching...");
  }
  
  // Status
  display.drawString(0, 60, emergencyMode ? "EMERGENCY MODE" : "Normal");
  
  display.display();
}

void loadUserData() {
  // Load from SD card or use defaults
  if (SD.exists("/user_data.txt")) {
    File file = SD.open("/user_data.txt", FILE_READ);
    if (file) {
      currentUser.name = file.readStringUntil('\n');
      currentUser.phone = file.readStringUntil('\n');
      currentUser.emergencyContact = file.readStringUntil('\n');
      currentUser.age = file.readStringUntil('\n').toInt();
      currentUser.gender = file.readStringUntil('\n');
      currentUser.bloodGroup = file.readStringUntil('\n');
      currentUser.medicalHistory = file.readStringUntil('\n');
      currentUser.lastPeriodDate = file.readStringUntil('\n');
      file.close();
    }
  } else {
    // Default values - should be set via web interface
    currentUser.name = "Test User";
    currentUser.phone = "9999999999";
    currentUser.emergencyContact = "8888888888";
    currentUser.age = 25;
    currentUser.gender = "M";
    currentUser.bloodGroup = "O+";
    currentUser.medicalHistory = "None";
    currentUser.lastPeriodDate = "";
  }
}

void logDataToSD() {
  File dataFile = SD.open("/health_log.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.print(time(nullptr));
    dataFile.print(",");
    dataFile.print(currentHealth.heartRate);
    dataFile.print(",");
    dataFile.print(currentHealth.temperature);
    dataFile.print(",");
    dataFile.print(currentHealth.bloodPressure);
    dataFile.print(",");
    dataFile.print(currentHealth.latitude, 6);
    dataFile.print(",");
    dataFile.print(currentHealth.longitude, 6);
    dataFile.print(",");
    dataFile.print(currentHealth.accelX);
    dataFile.print(",");
    dataFile.print(currentHealth.accelY);
    dataFile.print(",");
    dataFile.print(currentHealth.accelZ);
    dataFile.print(",");
    dataFile.println(currentHealth.emergencyDetected ? "1" : "0");
    dataFile.close();
  }
}

void logEmergencyToSD(String reason) {
  File emergencyFile = SD.open("/emergency_log.txt", FILE_WRITE);
  if (emergencyFile) {
    emergencyFile.println("=== EMERGENCY LOG ===");
    emergencyFile.println("Time: " + String(time(nullptr)));
    emergencyFile.println("Reason: " + reason);
    emergencyFile.println("User: " + currentUser.name);
    emergencyFile.println("Location: " + String(currentHealth.latitude, 6) + ", " + String(currentHealth.longitude, 6));
    emergencyFile.println("Heart Rate: " + String(currentHealth.heartRate));
    emergencyFile.println("Temperature: " + String(currentHealth.temperature));
    emergencyFile.println("=====================");
    emergencyFile.close();
  }
}

void displayWelcomeMessage() {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "RescueNet AI");
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 20, "Initializing...");
  display.drawString(0, 35, "Your Safety Guardian");
  display.display();
  delay(3000);
}