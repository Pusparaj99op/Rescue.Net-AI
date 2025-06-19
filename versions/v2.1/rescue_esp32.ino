#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <SSD1306Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <FastLED.h>

// Pin Definitions
#define GPS_RX 16
#define GPS_TX 17
#define SIM_RX 18
#define SIM_TX 19
#define TEMP_PIN 4
#define PULSE_PIN 34
#define BUZZER_PIN 25
#define VIBRATOR1_PIN 26
#define VIBRATOR2_PIN 27
#define BUTTON_PIN 32
#define LED_PIN 33
#define NANO_RX 2
#define NANO_TX 3

// Constants
#define NUM_LEDS 5
#define WIFI_SSID "YourWiFiSSID"
#define WIFI_PASSWORD "YourWiFiPassword"
#define TELEGRAM_BOT_TOKEN "YOUR_BOT_TOKEN"
#define TELEGRAM_CHAT_ID "YOUR_CHAT_ID"

// Objects
SoftwareSerial gpsSerial(GPS_RX, GPS_TX);
SoftwareSerial simSerial(SIM_RX, SIM_TX);
SoftwareSerial nanoSerial(NANO_RX, NANO_TX);
TinyGPSPlus gps;
Adafruit_BMP085 bmp;
SSD1306Wire display(0x3c, 21, 22);
OneWire oneWire(TEMP_PIN);
DallasTemperature temperatureSensor(&oneWire);
WebServer server(80);
CRGB leds[NUM_LEDS];

// Variables
struct SensorData {
  float temperature;
  float pressure;
  float altitude;
  int heartRate;
  double latitude;
  double longitude;
  bool emergencyTriggered;
  unsigned long timestamp;
} currentData;

bool emergencyMode = false;
unsigned long lastSensorRead = 0;
unsigned long lastTelegramSend = 0;
String deviceID = "RN001";

void setup() {
  Serial.begin(115200);

// Initialize pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(VIBRATOR1_PIN, OUTPUT);
  pinMode(VIBRATOR2_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

// Initialize components
  Wire.begin();
  display.init();
  display.setFont(ArialMT_Plain_10);

  if (!bmp.begin()) {
    Serial.println("BMP180 init failed!");
  }

  temperatureSensor.begin();
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);

// Initialize serials
  gpsSerial.begin(9600);
  simSerial.begin(9600);
  nanoSerial.begin(9600);

// Connect to WiFi
  connectToWiFi();

// Initialize web server
  setupWebServer();

// Initialize SIM module
  initializeSIM();

  Serial.println("RescueNet AI Device Initialized");
  updateLEDStatus(0, 0, 255, 0);// Green - Ready
}

void loop() {
// Handle web server
  server.handleClient();

// Read sensors every 5 seconds
  if (millis() - lastSensorRead >= 5000) {
    readAllSensors();
    lastSensorRead = millis();
  }

// Check emergency button
  if (digitalRead(BUTTON_PIN) == LOW) {
    triggerEmergency("Manual button press");
  }

// Check for emergency conditions
  checkEmergencyConditions();

// Send data to Nano
  sendDataToNano();

// Handle GPS data
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      if (gps.location.isValid()) {
        currentData.latitude = gps.location.lat();
        currentData.longitude = gps.location.lng();
      }
    }
  }

// Send periodic updates
  if (millis() - lastTelegramSend >= 30000) {// Every 30 seconds
    sendTelegramUpdate();
    lastTelegramSend = millis();
  }

// Update display
  updateDisplay();

  delay(100);
}

void readAllSensors() {
// Read temperature
  temperatureSensor.requestTemperatures();
  currentData.temperature = temperatureSensor.getTempCByIndex(0);

// Read pressure and altitude
  currentData.pressure = bmp.readPressure() / 100.0F;// hPa
  currentData.altitude = bmp.readAltitude();

// Read heart rate (processed by Nano)
  currentData.heartRate = getHeartRateFromNano();

  currentData.timestamp = millis();

  Serial.printf("Temp: %.2f°C, BP: %.2fhPa, HR: %d BPM\n",
                currentData.temperature, currentData.pressure, currentData.heartRate);
}

int getHeartRateFromNano() {
  nanoSerial.println("GET_HR");
  delay(100);
  if (nanoSerial.available()) {
    String response = nanoSerial.readString();
    return response.toInt();
  }
  return 0;
}

void checkEmergencyConditions() {
  bool emergency = false;
  String reason = "";

// Heart rate check
  if (currentData.heartRate > 120 || currentData.heartRate < 50) {
    emergency = true;
    reason += "Abnormal heart rate (" + String(currentData.heartRate) + " BPM) ";
  }

// Temperature check
  if (currentData.temperature > 39.0 || currentData.temperature < 35.0) {
    emergency = true;
    reason += "Abnormal temperature (" + String(currentData.temperature) + "°C) ";
  }

  if (emergency && !emergencyMode) {
    triggerEmergency(reason);
  }
}

void triggerEmergency(String reason) {
  emergencyMode = true;
  currentData.emergencyTriggered = true;

  Serial.println("EMERGENCY TRIGGERED: " + reason);

// Activate alarms
  activateAlarms();

// Send emergency alerts
  sendEmergencyAlert(reason);

// Update LED to red
  updateLEDStatus(255, 0, 0, 2);// Red blinking
}

void activateAlarms() {
// Buzzer pattern
  for (int i = 0; i < 5; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(VIBRATOR1_PIN, HIGH);
    digitalWrite(VIBRATOR2_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(VIBRATOR1_PIN, LOW);
    digitalWrite(VIBRATOR2_PIN, LOW);
    delay(200);
  }
}

void sendEmergencyAlert(String reason) {
// Send SMS
  sendSMS("+1234567890", "EMERGENCY ALERT from " + deviceID + ": " + reason +
          " Location: " + String(currentData.latitude, 6) + "," + String(currentData.longitude, 6));

// Send Telegram message
  sendTelegramMessage("🚨 EMERGENCY ALERT 🚨\n" +
                     "Device: " + deviceID + "\n" +
                     "Reason: " + reason + "\n" +
                     "Location: " + String(currentData.latitude, 6) + "," + String(currentData.longitude, 6) + "\n" +
                     "Heart Rate: " + String(currentData.heartRate) + " BPM\n" +
                     "Temperature: " + String(currentData.temperature) + "°C\n" +
                     "Time: " + String(millis()/1000) + "s");
}

void sendSMS(String number, String message) {
  simSerial.println("AT+CMGF=1");
  delay(100);
  simSerial.println("AT+CMGS=\"" + number + "\"");
  delay(100);
  simSerial.print(message);
  delay(100);
  simSerial.write(26);// Ctrl+Z
  delay(1000);
}

void sendTelegramMessage(String message) {
  if (WiFi.status() == WL_CONNECTED) {
// Implementation for Telegram API call// This would require HTTP client to send POST request
    Serial.println("Sending Telegram: " + message);
  }
}

void sendTelegramUpdate() {
  if (!emergencyMode) {
    String status = "📊 Health Status Update\n";
    status += "Device: " + deviceID + "\n";
    status += "❤️ Heart Rate: " + String(currentData.heartRate) + " BPM\n";
    status += "🌡️ Temperature: " + String(currentData.temperature) + "°C\n";
    status += "📍 Location: " + String(currentData.latitude, 6) + "," + String(currentData.longitude, 6) + "\n";
    status += "⏰ Time: " + String(millis()/1000) + "s";

    sendTelegramMessage(status);
  }
}

void updateDisplay() {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);

// Title
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "RescueNet AI");

// Status
  display.setFont(ArialMT_Plain_10);
  if (emergencyMode) {
    display.drawString(0, 20, "STATUS: EMERGENCY");
  } else {
    display.drawString(0, 20, "STATUS: MONITORING");
  }

// Sensor data
  display.drawString(0, 32, "HR: " + String(currentData.heartRate) + " BPM");
  display.drawString(0, 44, "Temp: " + String(currentData.temperature, 1) + "C");
  display.drawString(0, 56, "GPS: " + String(currentData.latitude, 4));

  display.display();
}

void updateLEDStatus(int r, int g, int b, int mode) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(r, g, b);
  }

  if (mode == 1) {// Breathing effect
    for (int brightness = 0; brightness < 255; brightness++) {
      FastLED.setBrightness(brightness);
      FastLED.show();
      delay(10);
    }
    for (int brightness = 255; brightness > 0; brightness--) {
      FastLED.setBrightness(brightness);
      FastLED.show();
      delay(10);
    }
  } else if (mode == 2) {// Blinking
    FastLED.setBrightness(255);
    FastLED.show();
    delay(500);
    FastLED.setBrightness(0);
    FastLED.show();
    delay(500);
  } else {
    FastLED.setBrightness(100);
    FastLED.show();
  }
}

void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());
}

void setupWebServer() {
  server.on("/", handleRoot);
  server.on("/api/status", handleStatus);
  server.on("/api/emergency", handleEmergencyAPI);
  server.on("/api/data", handleData);
  server.begin();
  Serial.println("Web server started");
}

void handleRoot() {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>RescueNet AI Dashboard</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            color: #333;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
        }
        .header {
            background: rgba(255,255,255,0.95);
            padding: 20px;
            border-radius: 15px;
            margin-bottom: 20px;
            box-shadow: 0 8px 32px rgba(0,0,0,0.1);
            text-align: center;
        }
        .header h1 {
            color: #2c3e50;
            font-size: 2.5em;
            margin-bottom: 10px;
        }
        .status-badge {
            display: inline-block;
            padding: 8px 20px;
            border-radius: 25px;
            font-weight: bold;
            font-size: 1.1em;
            text-transform: uppercase;
        }
        .status-normal { background: #2ecc71; color: white; }
        .status-emergency { background: #e74c3c; color: white; animation: pulse 1s infinite; }
        @keyframes pulse {
            0% { opacity: 1; }
            50% { opacity: 0.5; }
            100% { opacity: 1; }
        }
        .dashboard {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 20px;
        }
        .card {
            background: rgba(255,255,255,0.95);
            padding: 20px;
            border-radius: 15px;
            box-shadow: 0 8px 32px rgba(0,0,0,0.1);
        }
        .card h3 {
            color: #2c3e50;
            margin-bottom: 15px;
            font-size: 1.4em;
        }
        .metric {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 10px 0;
            border-bottom: 1px solid #eee;
        }
        .metric:last-child { border-bottom: none; }
        .metric-label {
            font-weight: 600;
            color: #555;
        }
        .metric-value {
            font-size: 1.2em;
            font-weight: bold;
            color: #2c3e50;
        }
        .emergency-btn {
            background: #e74c3c;
            color: white;
            border: none;
            padding: 15px 30px;
            font-size: 1.1em;
            border-radius: 25px;
            cursor: pointer;
            transition: all 0.3s;
            margin: 10px;
        }
        .emergency-btn:hover {
            background: #c0392b;
            transform: translateY(-2px);
        }
        .map-container {
            height: 400px;
            border-radius: 15px;
            overflow: hidden;
            box-shadow: 0 8px 32px rgba(0,0,0,0.1);
        }
        #map { width: 100%; height: 100%; }
        .charts {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
            margin-top: 20px;
        }
        .chart-container {
            background: rgba(255,255,255,0.95);
            padding: 20px;
            border-radius: 15px;
            box-shadow: 0 8px 32px rgba(0,0,0,0.1);
        }
        canvas { width: 100% !important; height: 200px !important; }
    </style>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <script src="https://maps.googleapis.com/maps/api/js?key=YOUR_API_KEY&callback=initMap" async defer></script>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🛡️ RescueNet AI Dashboard</h1>
            <div class="status-badge" id="statusBadge">System Status</div>
        </div>

        <div class="dashboard">
            <div class="card">
                <h3>📊 Vital Signs</h3>
                <div class="metric">
                    <span class="metric-label">❤️ Heart Rate</span>
                    <span class="metric-value" id="heartRate">-- BPM</span>
                </div>
                <div class="metric">
                    <span class="metric-label">🌡️ Temperature</span>
                    <span class="metric-value" id="temperature">-- °C</span>
                </div>
                <div class="metric">
                    <span class="metric-label">🎯 Blood Pressure</span>
                    <span class="metric-value" id="bloodPressure">-- hPa</span>
                </div>
                <div class="metric">
                    <span class="metric-label">📏 Altitude</span>
                    <span class="metric-value" id="altitude">-- m</span>
                </div>
            </div>

            <div class="card">
                <h3>📍 Location & Status</h3>
                <div class="metric">
                    <span class="metric-label">🌐 Latitude</span>
                    <span class="metric-value" id="latitude">--</span>
                </div>
                <div class="metric">
                    <span class="metric-label">🌐 Longitude</span>
                    <span class="metric-value" id="longitude">--</span>
                </div>
                <div class="metric">
                    <span class="metric-label">⏰ Last Update</span>
                    <span class="metric-value" id="lastUpdate">--</span>
                </div>
                <button class="emergency-btn" onclick="triggerEmergency()">🚨 Emergency Alert</button>
            </div>
        </div>

        <div class="card">
            <h3>🗺️ Live Location</h3>
            <div class="map-container">
                <div id="map"></div>
            </div>
        </div>

        <div class="charts">
            <div class="chart-container">
                <h3>❤️ Heart Rate Trend</h3>
                <canvas id="heartRateChart"></canvas>
            </div>
            <div class="chart-container">
                <h3>🌡️ Temperature Trend</h3>
                <canvas id="temperatureChart"></canvas>
            </div>
        </div>
    </div>

    <script>
        let map, marker;
        const heartRateData = [];
        const temperatureData = [];
        const timeLabels = [];

// Initialize Google Maps
        function initMap() {
            map = new google.maps.Map(document.getElementById('map'), {
                zoom: 15,
                center: { lat: 21.1458, lng: 79.0882 },// Nagpur
                styles: [
                    {
                        featureType: 'all',
                        elementType: 'geometry.fill',
                        stylers: [{ weight: '2.00' }]
                    },
                    {
                        featureType: 'all',
                        elementType: 'geometry.stroke',
                        stylers: [{ color: '#9c9c9c' }]
                    }
                ]
            });

            marker = new google.maps.Marker({
                position: { lat: 21.1458, lng: 79.0882 },
                map: map,
                title: 'RescueNet Device Location',
                icon: {
                    url: 'data:image/svg+xml;charset=UTF-8,<svg xmlns="http://www.w3.org/2000/svg" width="40" height="40" viewBox="0 0 40 40"><circle cx="20" cy="20" r="18" fill="%23e74c3c" stroke="%23fff" stroke-width="2"/><text x="20" y="26" text-anchor="middle" fill="white" font-size="16">🛡️</text></svg>',
                    scaledSize: new google.maps.Size(40, 40)
                }
            });
        }

// Initialize Charts
        const heartRateChart = new Chart(document.getElementById('heartRateChart'), {
            type: 'line',
            data: {
                labels: timeLabels,
                datasets: [{
                    label: 'Heart Rate (BPM)',
                    data: heartRateData,
                    borderColor: '#e74c3c',
                    backgroundColor: 'rgba(231, 76, 60, 0.1)',
                    tension: 0.4,
                    fill: true
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                scales: {
                    y: {
                        beginAtZero: false,
                        min: 40,
                        max: 120
                    }
                },
                plugins: {
                    legend: { display: false }
                }
            }
        });

        const temperatureChart = new Chart(document.getElementById('temperatureChart'), {
            type: 'line',
            data: {
                labels: timeLabels,
                datasets: [{
                    label: 'Temperature (°C)',
                    data: temperatureData,
                    borderColor: '#3498db',
                    backgroundColor: 'rgba(52, 152, 219, 0.1)',
                    tension: 0.4,
                    fill: true
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                scales: {
                    y: {
                        beginAtZero: false,
                        min: 35,
                        max: 40
                    }
                },
                plugins: {
                    legend: { display: false }
                }
            }
        });

// Fetch data from device
        async function fetchData() {
            try {
                const response = await fetch('/api/data');
                const data = await response.json();

// Update UI
                document.getElementById('heartRate').textContent = data.heartRate + ' BPM';
                document.getElementById('temperature').textContent = data.temperature.toFixed(1) + ' °C';
                document.getElementById('bloodPressure').textContent = data.pressure.toFixed(1) + ' hPa';
                document.getElementById('altitude').textContent = data.altitude.toFixed(1) + ' m';
                document.getElementById('latitude').textContent = data.latitude.toFixed(6);
                document.getElementById('longitude').textContent = data.longitude.toFixed(6);
                document.getElementById('lastUpdate').textContent = new Date().toLocaleTimeString();

// Update status
                const statusBadge = document.getElementById('statusBadge');
                if (data.emergencyTriggered) {
                    statusBadge.textContent = '🚨 EMERGENCY';
                    statusBadge.className = 'status-badge status-emergency';
                } else {
                    statusBadge.textContent = '✅ Normal';
                    statusBadge.className = 'status-badge status-normal';
                }

// Update map
                if (data.latitude && data.longitude) {
                    const newPosition = { lat: data.latitude, lng: data.longitude };
                    marker.setPosition(newPosition);
                    map.setCenter(newPosition);
                }

// Update charts
                const now = new Date().toLocaleTimeString();
                timeLabels.push(now);
                heartRateData.push(data.heartRate);
                temperatureData.push(data.temperature);

// Keep only last 10 data points
                if (timeLabels.length > 10) {
                    timeLabels.shift();
                    heartRateData.shift();
                    temperatureData.shift();
                }

                heartRateChart.update();
                temperatureChart.update();

            } catch (error) {
                console.error('Error fetching data:', error);
            }
        }

// Trigger emergency
        async function triggerEmergency() {
            if (confirm('Are you sure you want to trigger an emergency alert?')) {
                try {
                    await fetch('/api/emergency', { method: 'POST' });
                    alert('Emergency alert sent!');
                } catch (error) {
                    alert('Error sending emergency alert');
                }
            }
        }

// Update data every 5 seconds
        setInterval(fetchData, 5000);
        fetchData();// Initial load
    </script>
</body>
</html>
  )";
  server.send(200, "text/html", html);
}

void handleStatus() {
  DynamicJsonDocument doc(1024);
  doc["status"] = emergencyMode ? "emergency" : "normal";
  doc["deviceId"] = deviceID;
  doc["timestamp"] = millis();

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleData() {
  DynamicJsonDocument doc(1024);
  doc["heartRate"] = currentData.heartRate;
  doc["temperature"] = currentData.temperature;
  doc["pressure"] = currentData.pressure;
  doc["altitude"] = currentData.altitude;
  doc["latitude"] = currentData.latitude;
  doc["longitude"] = currentData.longitude;
  doc["emergencyTriggered"] = currentData.emergencyTriggered;
  doc["timestamp"] = currentData.timestamp;
  doc["deviceId"] = deviceID;

 String response;
 serializeJson(doc, response);
 server.send(200, "application/json", response);
}

void handleEmergencyAPI() {
 triggerEmergency("Manual web trigger");
 server.send(200, "application/json", "{\"status\":\"emergency_triggered\"}");
}

void initializeSIM() {
 Serial.println("Initializing SIM800L...");
 simSerial.println("AT");
 delay(1000);
 simSerial.println("AT+CMGF=1");// Text mode
 delay(1000);
 simSerial.println("AT+CNMI=1,2,0,0,0");// Show SMS directly
 delay(1000);
 Serial.println("SIM800L initialized");
}

void sendDataToNano() {
 DynamicJsonDocument doc(512);
 doc["cmd"] = "UPDATE_DATA";
 doc["hr"] = currentData.heartRate;
 doc["temp"] = currentData.temperature;
 doc["emergency"] = emergencyMode;

 String jsonString;
 serializeJson(doc, jsonString);
 nanoSerial.println(jsonString);
}
