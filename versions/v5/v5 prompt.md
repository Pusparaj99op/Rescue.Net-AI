DETAILED PROMPT FOR VSCODE COPILOT - RescueNet AI Hackathon Prototype

MASTER PROMPT FOR COPILOT
Create a complete RescueNet AI emergency response system prototype for hackathon with the following specifications:

HARDWARE COMPONENTS:
- ESP32 (main controller with WiFi/Bluetooth)
- Arduino Nano (sensor data processor)
- BMP180 (pressure/altitude sensor)
- Two 3.7V 1000mAh batteries with charging circuit
- Buzzer for audio alerts
- Two vibration motors for haptic feedback
- Neo-6M GPS module for location tracking
- 128x64 OLED display for user interface
- DS18B20 temperature sensor
- REES52 pulse sensor for heart rate monitoring
- ADXL335 accelerometer for fall detection
- Micro SD card module for data logging

SYSTEM REQUIREMENTS:
1. Real-time health monitoring (heart rate, temperature, fall detection)
2. GPS location tracking and emergency alerts
3. Web dashboard for monitoring multiple users
4. Telegram bot integration for emergency notifications
5. MongoDB database for user data and emergency logs
6. Mobile-responsive web interface
7. Emergency contact management system
8. Historical data visualization and analytics
9. Battery management and charging status
10. Multi-user support with authentication

TECHNICAL SPECIFICATIONS:
- ESP32 handles WiFi communication, web server, and database connectivity
- Arduino Nano processes sensor data and communicates with ESP32 via I2C
- OLED display shows real-time vitals and system status
- SD card logs all sensor data for offline backup
- Telegram API integration for instant emergency alerts
- RESTful API for web dashboard communication
- MongoDB Atlas for cloud database storage
- Real-time location sharing via Google Maps API
- Configurable emergency thresholds and alert parameters

USER FEATURES:
- User registration with personal details (name, age, gender, medical conditions)
- Emergency contact configuration (up to 5 contacts)
- Real-time health monitoring dashboard
- Emergency button functionality (manual trigger)
- Automatic fall detection and emergency alerts
- Location tracking and geofencing
- Historical health data visualization
- Battery status monitoring
- Device pairing and configuration

DELIVERABLES NEEDED:
1. Complete wiring diagram and pin configuration table
2. ESP32 main code with WiFi, web server, and API integration
3. Arduino Nano sensor processing code
4. Web dashboard (HTML/CSS/JavaScript) with real-time updates
5. MongoDB database schema and connection code
6. Telegram bot code for emergency notifications
7. Mobile app interface (PWA) for user interaction
8. Installation and setup documentation
9. Testing procedures and calibration guides
10. Deployment instructions for all components

Create separate files for each component with detailed comments and error handling.

SPECIFIC FILE GENERATION PROMPTS
1. Hardware Connection Table Prompt
Generate a detailed pin connection table for RescueNet AI prototype:

Components to connect:
- ESP32 DevKit V1 (main controller)
- Arduino Nano (sensor processor)
- BMP180 barometric sensor
- Neo-6M GPS module
- 128x64 I2C OLED display
- DS18B20 temperature sensor
- REES52 pulse sensor
- ADXL335 accelerometer
- Micro SD card module
- Buzzer (active)
- Two vibration motors
- Battery management circuit

Create a comprehensive table with:
- Component name
- Pin number on component
- Connection to ESP32/Arduino Nano
- Pin function (I2C, SPI, Digital, Analog)
- Voltage requirements
- Additional notes for wiring

Include power distribution, I2C addressing, and communication protocols between ESP32 and Arduino Nano.
2. ESP32 Main Code Prompt
Create ESP32 main code (main.cpp) for RescueNet AI with:

CORE FUNCTIONALITY:
- WiFi connection management with automatic reconnection
- Web server hosting dashboard and API endpoints
- MongoDB Atlas connection for data storage
- Telegram bot integration for emergency alerts
- I2C communication with Arduino Nano for sensor data
- Real-time data processing and emergency detection
- GPS location tracking and geofencing
- Battery monitoring and charging status
- OTA (Over-The-Air) updates capability

API ENDPOINTS:
- POST /api/register - User registration
- POST /api/login - User authentication
- GET /api/vitals - Current vital signs
- POST /api/emergency - Manual emergency trigger
- GET /api/location - Current GPS coordinates
- GET /api/history - Historical health data
- POST /api/contacts - Emergency contact management
- GET /api/status - Device status and battery level

FEATURES:
- JSON data handling for API responses
- Emergency threshold configuration
- Automatic emergency detection algorithms
- Location-based emergency services
- Multi-user support with session management
- Real-time WebSocket connections for live updates
- Secure authentication with JWT tokens
- Error handling and logging to SD card

Use Arduino IDE compatible code with proper library includes and pin definitions.
3. Arduino Nano Code Prompt
Create Arduino Nano sensor processing code (nano_sensors.cpp) for:

SENSOR MANAGEMENT:
- BMP180 pressure and altitude readings
- DS18B20 temperature measurements
- REES52 pulse sensor for heart rate detection
- ADXL335 accelerometer for fall detection and activity monitoring
- Sensor calibration and filtering algorithms
- Data validation and error correction

COMMUNICATION:
- I2C slave mode communication with ESP32
- Structured data packet transmission
- Sensor status reporting
- Emergency trigger detection
- Battery level monitoring from analog pin

DATA PROCESSING:
- Heart rate calculation with peak detection
- Fall detection algorithm using accelerometer data
- Temperature compensation for accurate readings
- Moving average filters for noise reduction
- Emergency threshold checking
- Activity level calculation

FEATURES:
- Low-power modes for battery conservation
- Interrupt-driven sensor readings
- Calibration routines for all sensors
- Diagnostic modes for testing
- Structured data output format
- Error handling and sensor fault detection

Include proper sensor initialization, calibration procedures, and data formatting for ESP32 communication.
4. Web Dashboard Prompt
Create a complete web dashboard (index.html, style.css, script.js) for RescueNet AI:

DASHBOARD FEATURES:
- Real-time vital signs display (heart rate, temperature, blood pressure)
- Live location tracking with Google Maps integration
- Emergency contact management interface
- Historical data visualization with charts
- User profile management
- Device status and battery monitoring
- Emergency alert system
- Multi-user support with role-based access

UI COMPONENTS:
- Responsive design for mobile and desktop
- Real-time data updates using WebSockets
- Interactive charts for health trends
- Emergency button with confirmation dialog
- Location sharing controls
- Contact management forms
- User authentication interface
- Settings and configuration panels

TECHNICAL REQUIREMENTS:
- HTML5 with semantic markup
- CSS3 with responsive grid layout
- JavaScript with ES6+ features
- Chart.js for data visualization
- Google Maps API for location services
- WebSocket connections for real-time updates
- PWA (Progressive Web App) capabilities
- Offline functionality with service workers

SECURITY:
- JWT token-based authentication
- Input validation and sanitization
- HTTPS enforcement
- CORS configuration
- Session management
- Secure API communication

Create separate files for HTML structure, CSS styling, and JavaScript functionality.
5. Database Schema Prompt
Create MongoDB database schema and connection code for RescueNet AI:

DATABASE COLLECTIONS:
1. Users Collection:
   - Personal information (name, age, gender, medical conditions)
   - Emergency contacts (name, phone, relationship, priority)
   - Device settings and preferences
   - Authentication credentials
   - Registration timestamp and status

2. VitalSigns Collection:
   - User ID reference
   - Timestamp of reading
   - Heart rate, temperature, blood pressure
   - Activity level and fall detection status
   - GPS coordinates and location accuracy
   - Battery level and device status

3. EmergencyLogs Collection:
   - Emergency ID and type (manual/automatic)
   - User ID and location at time of emergency
   - Vital signs at emergency trigger
   - Response time and actions taken
   - Contact notification status
   - Resolution timestamp and notes

4. DeviceStatus Collection:
   - Device ID and user assignment
   - Last communication timestamp
   - Battery level and charging status
   - Sensor calibration data
   - Firmware version and update status

CONNECTION CODE:
- MongoDB Atlas connection with authentication
- Connection pooling and error handling
- CRUD operations for all collections
- Data validation and schema enforcement
- Indexing for performance optimization
- Backup and recovery procedures

Include mongoose schemas, connection utilities, and API helper functions.
6. Telegram Bot Integration Prompt
Create Telegram bot integration (telegram_bot.js) for emergency notifications:

BOT FUNCTIONALITY:
- Emergency alert broadcasting to registered contacts
- Location sharing with Google Maps links
- Real-time vital signs updates
- Device status notifications
- User registration and verification
- Emergency contact management via bot commands
- Historical emergency log access

COMMANDS:
- /start - Bot initialization and welcome
- /register - User registration process
- /status - Current device and health status
- /location - Share current location
- /emergency - Manual emergency trigger
- /contacts - Manage emergency contacts
- /history - View emergency history
- /settings - Configure alert preferences

FEATURES:
- Automatic emergency notifications with location
- Group chat support for family emergency channels
- Inline keyboard for quick responses
- File sharing for health reports
- Multi-language support
- Secure user verification
- Rate limiting and spam protection

INTEGRATION:
- MongoDB database connectivity
- ESP32 webhook endpoints
- Real-time data synchronization
- Google Maps API for location services
- Encrypted message handling
- Error logging and monitoring

Include bot token configuration, webhook setup, and message formatting utilities.
7. Installation and Setup Prompt
Create comprehensive installation and setup documentation:

HARDWARE ASSEMBLY:
- Step-by-step wiring instructions with diagrams
- Component placement and mounting guidelines
- Power supply configuration and battery management
- Antenna placement for GPS and WiFi
- Enclosure design and weatherproofing
- Testing procedures for each component

SOFTWARE SETUP:
- Arduino IDE installation and configuration
- ESP32 board package installation
- Required library installation list
- MongoDB Atlas account setup
- Telegram bot creation and token generation
- Google Maps API key configuration
- WiFi credentials and network setup

DEPLOYMENT PROCESS:
- Code compilation and upload procedures
- ESP32 and Arduino Nano programming steps
- Web server deployment instructions
- Database initialization and seeding
- Telegram bot webhook configuration
- Testing and calibration procedures

TROUBLESHOOTING GUIDE:
- Common connection issues and solutions
- Sensor calibration problems
- WiFi connectivity troubleshooting
- Database connection issues
- Telegram bot setup problems
- Performance optimization tips

MAINTENANCE:
- Regular calibration schedules
- Battery replacement procedures
- Firmware update processes
- Data backup and recovery
- Security updates and patches

Include screenshots, code snippets, and detailed explanations for each step.

FINAL INTEGRATION PROMPT
Create the complete RescueNet AI system integration with:

SYSTEM ARCHITECTURE:
- ESP32 as main controller with web server and database connectivity
- Arduino Nano as sensor data processor with I2C communication
- Web dashboard with real-time updates and emergency management
- Telegram bot for instant emergency notifications
- MongoDB database for persistent data storage
- Mobile-responsive PWA for user interaction

REAL-TIME FEATURES:
- WebSocket connections for live data streaming
- GPS location tracking with geofencing
- Automatic emergency detection with configurable thresholds
- Multi-user support with role-based access control
- Battery management with low-power modes
- OTA updates for remote firmware management

EMERGENCY WORKFLOW:
1. Continuous health monitoring and data logging
2. Automatic emergency detection based on vital signs
3. GPS location capture and emergency contact notification
4. Telegram alerts with location sharing
5. Web dashboard emergency status updates
6. Historical data logging and analysis

TESTING PROCEDURES:
- Sensor calibration and validation tests
- Emergency detection algorithm testing
- Communication protocol verification
- Database connectivity and data integrity checks
- Web interface responsiveness testing
- Battery life and charging cycle testing

Provide complete, production-ready code with proper error handling, security measures, and documentation for a 24-hour hackathon prototype demonstration.