## Installation Instructions for RescueNet AI

### Prerequisites Installation

#### 1. Install Node.js
1. Download Node.js from https://nodejs.org/
2. Install the LTS version (recommended)
3. Verify installation:
   ```cmd
   node --version
   npm --version
   ```

#### 2. Install MongoDB
**Option A: MongoDB Community Server (Local)**
1. Download from https://www.mongodb.com/try/download/community
2. Install MongoDB Community Server
3. Start MongoDB service:
   ```cmd
   net start MongoDB
   ```

**Option B: MongoDB Atlas (Cloud)**
1. Create account at https://cloud.mongodb.com/
2. Create a free cluster
3. Get connection string and update `.env`

#### 3. Install Git (if not already installed)
1. Download from https://git-scm.com/
2. Install with default settings

### Project Setup

#### 1. Download/Clone Project
```cmd
# If you have the project files locally, navigate to the directory
cd "C:\Users\kalvi\OneDrive\Documents\VS\Rescue.Net-AI\v6.12"

# If cloning from repository
git clone <repository-url>
cd rescuenet-ai
```

#### 2. Install Dependencies
```cmd
npm install
```

#### 3. Configure Environment
1. Copy `.env.example` to `.env`:
   ```cmd
   copy .env.example .env
   ```

2. Edit `.env` file with your settings:
   - Update MongoDB connection string
   - Add Twilio credentials (optional)
   - Add email credentials (optional)

#### 4. Start the Application
```cmd
# Start MongoDB (if local)
net start MongoDB

# Start the application
npm start
```

#### 5. Access the Application
Open your browser and navigate to: `http://localhost:3000`

### ESP32 Setup (Optional)

#### 1. Install Arduino IDE
1. Download from https://www.arduino.cc/en/software
2. Install with default settings

#### 2. Install ESP32 Board Package
1. Open Arduino IDE
2. Go to File → Preferences
3. Add this URL to "Additional Board Manager URLs":
   ```
   https://dl.espressif.com/dl/package_esp32_index.json
   ```
4. Go to Tools → Board → Boards Manager
5. Search for "esp32" and install "ESP32 by Espressif Systems"

#### 3. Install Required Libraries
In Arduino IDE, go to Tools → Manage Libraries and install:
- ArduinoJson
- WebSocketsClient
- OneWire
- DallasTemperature
- Adafruit MPU6050
- MAX30105 library
- ESP8266 and ESP32 OLED driver for SSD1306 displays

#### 4. Configure and Upload
1. Open `esp32_enhanced.ino`
2. Update WiFi credentials
3. Update server IP address
4. Select ESP32 board and port
5. Upload the sketch

### Troubleshooting

#### MongoDB Issues
```cmd
# Check if MongoDB is running
tasklist | findstr mongod

# Start MongoDB manually
"C:\Program Files\MongoDB\Server\5.0\bin\mongod.exe" --dbpath "C:\data\db"
```

#### Port Issues
If port 3000 is in use, change PORT in `.env` file:
```
PORT=3001
```

#### Missing Dependencies
```cmd
# Clear npm cache and reinstall
npm cache clean --force
rm -rf node_modules
npm install
```

### Quick Start Commands

```cmd
# Navigate to project directory
cd "C:\Users\kalvi\OneDrive\Documents\VS\Rescue.Net-AI\v6.12"

# Install dependencies
npm install

# Start application
npm start
```

### Default Configuration

- **Server Port**: 3000
- **WebSocket Port**: 8080
- **MongoDB**: localhost:27017/rescuenet
- **Default User**: Register through web interface

### Testing the System

1. **Web Interface**: Open `http://localhost:3000`
2. **Register**: Create a new user account
3. **Dashboard**: View the monitoring dashboard
4. **WebSocket**: Check browser console for real-time updates
5. **API**: Test endpoints using Postman or browser

### Production Deployment

For production deployment:

1. **Environment Variables**:
   ```
   NODE_ENV=production
   PORT=80
   MONGODB_URI=your_production_mongodb_uri
   ```

2. **Security**: Configure Twilio and email for alerts

3. **SSL**: Use HTTPS in production

4. **Process Manager**: Use PM2 for process management:
   ```cmd
   npm install -g pm2
   pm2 start server.js --name rescuenet
   ```

### Support

If you encounter issues:
1. Check the console for error messages
2. Verify all dependencies are installed
3. Ensure MongoDB is running
4. Check firewall settings
5. Review the README.md for detailed information
