#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// WiFi Credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Web Server
WebServer server(80);

// Database API Endpoint
const char* apiEndpoint = "http://your-api-endpoint.com/data"; // Replace with your API endpoint

// I2C Address for Arduino Nano
#define NANO_ADDR 8

// Pin for SD Card
#define SD_CS 5

void setup() {
  Serial.begin(115200);
  Wire.begin(); // Join I2C bus as master

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  // Initialize SD Card
  if (!SD.begin(SD_CS)) {
    Serial.println("Card Mount Failed");
    return;
  }

  // Web server routes
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
}

void loop() {
  server.handleClient();
  readFromNanoAndPost();
  delay(5000); // Read and post data every 5 seconds
}

void handleRoot() {
  server.send(200, "text/plain", "RescueNet AI is active!");
}

void handleData() {
  // Handle data requests from the web dashboard
  // This could return logged data from the SD card or real-time data
  String json = getSensorDataJson();
  server.send(200, "application/json", json);
}

void readFromNanoAndPost() {
  Wire.requestFrom(NANO_ADDR, 32); // Request 32 bytes from Nano
  String data = "";
  while (Wire.available()) {
    char c = Wire.read();
    data += c;
  }

  if (data.length() > 0) {
    Serial.println("Received from Nano: " + data);
    logToSD(data);
    postDataToApi(data);
  }
}

String getSensorDataJson() {
  // This is a placeholder. You should read from the Nano or SD card
  // and format it as JSON.
  StaticJsonDocument<200> doc;
  doc["heart_rate"] = 75;
  doc["temperature"] = 36.5;
  doc["fall_detected"] = false;
  String json;
  serializeJson(doc, json);
  return json;
}

void logToSD(String data) {
  File file = SD.open("/datalog.txt", FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.println(data)) {
    Serial.println("Data logged to SD card.");
  } else {
    Serial.println("Write to SD card failed.");
  }
  file.close();
}

void postDataToApi(String data) {
  HTTPClient http;
  http.begin(apiEndpoint);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(data);
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("HTTP Response code: " + String(httpResponseCode));
    Serial.println(response);
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}
