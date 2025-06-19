#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Server Configuration
#define SERVER_URL "http://your-server.com/api"
#define MQTT_SERVER "mqtt.eclipse.org"
#define MQTT_PORT 1883

// Telegram Configuration
#define TELEGRAM_BOT_TOKEN "YOUR_TELEGRAM_BOT_TOKEN"
#define TELEGRAM_CHAT_ID "YOUR_CHAT_ID"

// Emergency Thresholds
#define MIN_HEART_RATE 50
#define MAX_HEART_RATE 120
#define MIN_TEMPERATURE 35.0
#define MAX_TEMPERATURE 38.5
#define MAX_SYSTOLIC_BP 140
#define MAX_DIASTOLIC_BP 90
#define FALL_THRESHOLD 3.0

// Timing Configuration
#define HEALTH_CHECK_INTERVAL 5000    // 5 seconds
#define DATA_SEND_INTERVAL 30000      // 30 seconds
#define EMERGENCY_BUTTON_TIMEOUT 3000 // 3 seconds

#endif