# RescueNet AI - Quick Start Guide

## ✅ What's Been Completed

Your RescueNet AI system is now complete with the following files:

### 📁 Project Structure
```
v6.12/
├── public/
│   └── index.html          ✅ Complete web dashboard
├── utils/
│   ├── emergencyServices.js ✅ Emergency response system
│   └── healthAnalytics.js   ✅ Health analytics module
├── server.js               ✅ Enhanced Node.js backend
├── esp32_enhanced.ino      ✅ ESP32 firmware
├── package.json           ✅ Dependencies configuration
├── .env                   ✅ Environment variables
├── .env.example           ✅ Environment template
├── README.md              ✅ Complete documentation
├── INSTALL.md             ✅ Installation guide
├── CHANGELOG.md           ✅ Version history
├── start.bat              ✅ Windows startup script
└── .gitignore             ✅ Git ignore rules
```

## 🚀 How to Run (Windows)

### Option 1: Easy Start (Recommended)
1. **Double-click `start.bat`** - This will automatically:
   - Check Node.js installation
   - Install dependencies
   - Create configuration files
   - Start the server

### Option 2: Manual Start
1. **Open Command Prompt** in the project folder
2. **Install dependencies:**
   ```cmd
   npm install
   ```
3. **Start the server:**
   ```cmd
   npm start
   ```

### Option 3: Development Mode
```cmd
npm run dev
```

## 🌐 Access Your Application

Once started, open your browser and go to:
- **Dashboard**: http://localhost:3000
- **WebSocket**: ws://localhost:8080

## 👤 First Time Setup

1. **Register a new user** on the web interface
2. **Fill in your details** including:
   - Name, age, gender
   - Phone number (this is your user ID)
   - Blood group
   - Emergency contact
   - Medical history

3. **Start monitoring** - The dashboard will show:
   - Real-time vital signs
   - Your location on a map
   - Health trends and statistics
   - Emergency history

## 🔧 ESP32 Hardware Setup (Optional)

If you have ESP32 hardware:

1. **Install Arduino IDE**
2. **Install ESP32 board support**
3. **Install required libraries:**
   - ArduinoJson
   - WebSocketsClient
   - OneWire, DallasTemperature
   - MPU6050, MAX30105 libraries
   - SSD1306 OLED library

4. **Update `esp32_enhanced.ino`:**
   - WiFi credentials
   - Server IP address
   - User ID (phone number)

5. **Upload to ESP32**

## 📊 Features Available

### ✅ Real-time Monitoring
- Heart rate, temperature, blood pressure
- Location tracking
- Live dashboard updates

### ✅ Emergency System
- Manual emergency button
- Automatic anomaly detection
- SMS and email alerts
- Emergency contact notifications

### ✅ Health Analytics
- Trend analysis
- Pattern detection
- Health insights
- Statistical reports

### ✅ Data Visualization
- Interactive charts
- Real-time graphs
- Health history
- Emergency logs

## 🔔 Notifications Setup

To enable SMS and email alerts:

1. **Edit `.env` file:**
   ```env
   # For SMS (Twilio)
   TWILIO_ACCOUNT_SID=your_account_sid
   TWILIO_AUTH_TOKEN=your_auth_token
   TWILIO_PHONE_NUMBER=your_twilio_number
   
   # For Email (Gmail)
   EMAIL_USER=your_email@gmail.com
   EMAIL_PASS=your_app_password
   ```

2. **Restart the server** after changes

## 🚨 Emergency Testing

1. **Manual Test**: Click the red "EMERGENCY" button on dashboard
2. **Simulated Data**: The system generates sample health data for testing
3. **Check Logs**: Monitor console for emergency alerts

## 📱 Mobile Access

The dashboard is fully responsive and works on:
- Desktop computers
- Tablets
- Mobile phones
- Any device with a web browser

## 🔧 Troubleshooting

### Common Issues:

**"npm is not recognized"**
- Install Node.js from https://nodejs.org/

**"Cannot connect to MongoDB"**
- Install MongoDB Community Server
- Start MongoDB service: `net start MongoDB`

**Port 3000 already in use**
- Change PORT in `.env` file to 3001 or another port

**ESP32 not connecting**
- Check WiFi credentials
- Update server IP address
- Verify ESP32 is powered and in range

## 🛡️ Security Notes

- Change default passwords in `.env`
- Use HTTPS in production
- Keep dependencies updated
- Secure your WiFi network

## 📞 Support

If you need help:
1. Check `README.md` for detailed documentation
2. Review error messages in console
3. Verify all configuration files
4. Check hardware connections (ESP32)

## 🎯 Next Steps

1. **Test the web dashboard**
2. **Register users and monitor data**
3. **Set up emergency contacts**
4. **Configure notifications**
5. **Add ESP32 hardware** (optional)
6. **Customize for your needs**

---

**🚀 Your RescueNet AI system is ready to use!** 

The system includes everything needed for a complete health monitoring and emergency response solution. Start with the web dashboard and expand with hardware as needed.

**Important**: This is a monitoring system and should not replace professional medical care. Always consult healthcare professionals for medical emergencies.
