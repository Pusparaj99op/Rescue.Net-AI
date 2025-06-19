# RescueNet AI - Emergency Response System

A comprehensive IoT-based emergency response system that monitors health vitals in real-time and automatically alerts emergency services when anomalies are detected.

## 🚀 Features

- **Real-time Health Monitoring**: Continuous monitoring of heart rate, body temperature, and blood pressure
- **Fall Detection**: Advanced accelerometer-based fall detection using ESP32
- **Emergency SMS Alerts**: Direct SMS notifications via SIM800L GSM module
- **Emergency Notifications**: Email notifications to emergency contacts
- **Live Dashboard**: Web-based dashboard with real-time vital signs visualization
- **WebSocket Integration**: Real-time data streaming for instant updates
- **Mobile Responsive**: Works on all devices (desktop, tablet, mobile)
- **Data Analytics**: Health trend analysis and pattern detection
- **Emergency History**: Complete log of all emergency incidents
- **Cellular Backup**: SIM800L provides connectivity when WiFi is unavailable

## 🛠️ Technology Stack



### Backend
- **Node.js** with Express.js
- **MongoDB** for data storage
- **WebSocket** for real-time communication
- **SMS Integration** via SIM800L or Twilio
- **Nodemailer** for email alerts

### Frontend
- **HTML5/CSS3/JavaScript**
- **Chart.js** for data visualization
- **Leaflet.js** for mapping
- **WebSocket** for real-time updates

### Hardware (ESP32)
- **ESP32** development board
- **MAX30102** heart rate & SpO2 sensor
- **DS18B20** temperature sensor
- **MPU6050** accelerometer/gyroscope
- **SIM800L** GSM module for SMS alerts
- **OLED Display** for local status
- **GPS Module** for location tracking

## 📋 Prerequisites

- Node.js (v14 or higher)
- MongoDB (local or cloud)
- Arduino IDE (for ESP32 programming)
- **Active SIM card** with SMS plan (for SIM800L)
- Twilio account (optional, alternative to SIM800L)
- Gmail account (for emails)

## 🔧 Installation

### 1. Clone the Repository
```bash
git clone https://github.com/yourusername/rescuenet-ai.git
cd rescuenet-ai
```

### 2. Install Dependencies
```bash
npm install
```

### 3. Environment Configuration
Copy `.env.example` to `.env` and configure your settings:
```bash
cp .env.example .env
```

Edit `.env` with your configuration:
```env
# Database
MONGODB_URI=mongodb://localhost:27017/rescuenet

# Server
PORT=3000
WEBSOCKET_PORT=8080

# Twilio (SMS notifications)
TWILIO_ACCOUNT_SID=your_account_sid
TWILIO_AUTH_TOKEN=your_auth_token
TWILIO_PHONE_NUMBER=your_twilio_number

# Email notifications
EMAIL_USER=your_email@gmail.com
EMAIL_PASS=your_app_password
```

### 4. Start MongoDB
```bash
# Windows
net start MongoDB

# macOS/Linux
sudo systemctl start mongod
```

### 5. Run the Application
```bash
# Development mode
npm run dev

# Production mode
npm start
```

### 6. Access the Dashboard
Open your browser and go to: `http://localhost:3000`

## 🔌 ESP32 Setup

### Hardware Connections
```
ESP32 Pin    | Component
-------------|----------------
GPIO 4       | DS18B20 Temperature Sensor
GPIO 21      | I2C SDA (MAX30102, MPU6050, OLED)
GPIO 22      | I2C SCL (MAX30102, MPU6050, OLED)
GPIO 2       | Buzzer
GPIO 5       | Status LED
GPIO 18      | Emergency LED
GPIO 0       | Emergency Button
```

### Arduino Libraries Required
Install these libraries through Arduino IDE Library Manager:
- WiFi (ESP32 built-in)
- ArduinoJson
- WebSocketsClient
- OneWire
- DallasTemperature
- MPU6050
- MAX30105
- SSD1306Wire
- HTTPClient

### Programming the ESP32
1. Open `esp32_enhanced.ino` in Arduino IDE
2. Update WiFi credentials and server IP
3. Update user ID (phone number)
4. Upload to ESP32

## 📱 Usage

### User Registration
1. Open the dashboard
2. Click "Register here"
3. Fill in personal and medical information
4. Complete registration

### Health Monitoring
1. Login with your phone number
2. The system automatically starts monitoring
3. View real-time vitals on the dashboard
4. Check health trends and statistics

### Emergency Features
- **Manual Emergency**: Press the red emergency button
- **Automatic Detection**: System automatically detects anomalies
- **Fall Detection**: ESP32 detects sudden falls
- **Instant Alerts**: SMS and email sent to emergency contacts

## 🚨 Emergency Thresholds

The system triggers emergency alerts when:
- Heart Rate: < 50 BPM or > 120 BPM
- Body Temperature: < 35°C or > 38.5°C
- Fall Detection: Sudden acceleration > 15g
- Manual Emergency: Emergency button pressed

## 📊 Dashboard Features

### Real-time Monitoring
- Live vital signs display
- Current location on map
- Connection status indicators
- Last update timestamps

### Health Analytics
- Historical data charts
- Trend analysis
- Weekly statistics
- Pattern detection

### Emergency Management
- Emergency history log
- Response tracking
- Contact management
- Status updates

## 🔧 API Endpoints

### User Management
- `POST /api/register` - Register new user
- `GET /api/user/:phone` - Get user details

### Health Data
- `POST /api/health-data` - Submit health data
- `GET /api/health-history/:userId` - Get health history
- `GET /api/dashboard/:userId` - Get dashboard data

### Emergency
- `POST /api/emergency` - Trigger emergency
- `GET /api/emergency-history/:userId` - Get emergency history

## 🛡️ Security Features

- Rate limiting on API endpoints
- Input validation and sanitization
- Secure headers with Helmet.js
- Environment-based configuration
- Error handling and logging

## 🔄 WebSocket Events

### Client to Server
- `subscribe` - Subscribe to user-specific updates

### Server to Client
- `health_data` - Real-time health data updates
- `emergency` - Emergency alert notifications
- `health_alert` - Health warnings

## 📈 Monitoring & Analytics

The system provides comprehensive analytics including:
- Health trend analysis
- Daily/weekly patterns
- Anomaly detection
- Statistical insights
- Personalized recommendations

## 🚑 Emergency Response Workflow

1. **Detection**: System detects anomaly or manual trigger
2. **Validation**: Confirms emergency conditions
3. **Notification**: Sends alerts to emergency contacts
4. **Location**: Shares precise location coordinates
5. **Medical Info**: Provides complete medical profile
6. **Tracking**: Monitors response and resolution

## 🔧 Troubleshooting

### Common Issues

**ESP32 not connecting to WiFi**
- Check WiFi credentials
- Ensure ESP32 is in range
- Verify network allows IoT devices

**No data on dashboard**
- Check MongoDB connection
- Verify ESP32 is sending data
- Check server logs for errors

**Emergency alerts not sent**
- Verify Twilio configuration
- Check email settings
- Ensure emergency contacts are valid

### Debug Mode
Enable debug logging by setting `NODE_ENV=development` in `.env`

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 📞 Support

For support or questions:
- Create an issue on GitHub
- Email: support@rescuenet.ai
- Documentation: [Wiki](https://github.com/yourusername/rescuenet-ai/wiki)

## 🔮 Future Enhancements

- Machine learning for predictive health analytics
- Integration with hospital management systems
- Mobile app for iOS and Android
- Multi-language support
- Advanced sensor integration (ECG, glucose, etc.)
- Blockchain for secure health data storage

## 🙏 Acknowledgments

- ESP32 community for hardware support
- Node.js and MongoDB teams
- Chart.js and Leaflet.js contributors
- Medical professionals for guidance
- Open source community

---

**⚠️ Medical Disclaimer**: This system is for monitoring purposes only and should not replace professional medical advice, diagnosis, or treatment. Always consult healthcare professionals for medical concerns.
