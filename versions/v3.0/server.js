

javascript
const express = require('express');
const mongoose = require('mongoose');
const cors = require('cors');
const WebSocket = require('ws');
const http = require('http');
const path = require('path');
const TelegramBot = require('node-telegram-bot-api');

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

// Middleware
app.use(cors());
app.use(express.json());
app.use(express.static('public'));

// MongoDB connection
mongoose.connect('mongodb://localhost:27017/rescuenet', {
    useNewUrlParser: true,
    useUnifiedTopology: true
});

// Telegram Bot
const bot = new TelegramBot('YOUR_TELEGRAM_BOT_TOKEN', { polling: false });
const CHAT_ID = 'YOUR_TELEGRAM_CHAT_ID';

// MongoDB Schemas
const userSchema = new mongoose.Schema({
    name: String,
    phone: { type: String, unique: true },
    emergencyContact: String,
    age: Number,
    gender: String,
    medicalConditions: String,
    lastPeriod: Date,
    registrationDate: { type: Date, default: Date.now },
    baselineHeartRate: { type: Number, default: 75 },
    baselineTemperature: { type: Number, default: 36.5 }
});

const healthDataSchema = new mongoose.Schema({
    userId: String,
    heartRate: Number,
    temperature: Number,
    bloodPressure: Number,
    latitude: Number,
    longitude: Number,
    altitude: Number,
    emergencyStatus: { type: Boolean, default: false },
    timestamp: { type: Date, default: Date.now },
    deviceId: String,
    additionalNotes: String
});

const emergencySchema = new mongoose.Schema({
    userId: String,
    emergencyType: String,
    triggerReason: String,
    location: {
        latitude: Number,
        longitude: Number
    },
    healthData: {
        heartRate: Number,
        temperature: Number,
        bloodPressure: Number
    },
    responseTeam: [String],
    status: { type: String, default: 'active' },// active, resolved, false_alarm
    timestamp: { type: Date, default: Date.now },
    resolvedAt: Date,
    notes: String
});

const User = mongoose.model('User', userSchema);
const HealthData = mongoose.model('HealthData', healthDataSchema);
const Emergency = mongoose.model('Emergency', emergencySchema);

// WebSocket connection handling
wss.on('connection', (ws) => {
    console.log('New WebSocket connection established');

    ws.on('message', (message) => {
        console.log('Received:', message);
    });

    ws.on('close', () => {
        console.log('WebSocket connection closed');
    });
});

// Broadcast data to all connected clients
function broadcastHealthData(data) {
    wss.clients.forEach((client) => {
        if (client.readyState === WebSocket.OPEN) {
            client.send(JSON.stringify(data));
        }
    });
}

// API Routes// User registration/update
app.post('/api/users', async (req, res) => {
    try {
        const user = await User.findOneAndUpdate(
            { phone: req.body.phone },
            req.body,
            { upsert: true, new: true }
        );
        res.json({ success: true, user });
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
});

// Get user by phone
app.get('/api/users/:phone', async (req, res) => {
    try {
        const user = await User.findOne({ phone: req.params.phone });
        res.json(user);
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
});

// Health data endpoint (from ESP32)
app.post('/api/health-data', async (req, res) => {
    try {
        const healthData = new HealthData(req.body);
        await healthData.save();

// Broadcast to connected clients
        broadcastHealthData(req.body);

// Check for emergency conditions
        if (isEmergencyCondition(req.body)) {
            await handleEmergency(req.body);
        }

        res.json({ success: true, id: healthData._id });
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
});

// Get latest health data
app.get('/api/health-data/latest', async (req, res) => {
    try {
        const latestData = await HealthData.findOne().sort({ timestamp: -1 });
        res.json(latestData);
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
});

// Emergency endpoint
app.post('/api/emergency', async (req, res) => {
    try {
        await handleEmergency(req.body);
        res.json({ success: true });
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
});

// Telegram alert endpoint
app.post('/api/telegram-alert', async (req, res) => {
    try {
        await bot.sendMessage(CHAT_ID, req.body.message);
        res.json({ success: true });
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
});

// Get emergency history
app.get('/api/emergencies/:userId', async (req, res) => {
    try {
        const emergencies = await Emergency.find({ userId: req.params.userId })
            .sort({ timestamp: -1 })
            .limit(10);
        res.json(emergencies);
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
});

// Health analytics endpoint
app.get('/api/analytics/:userId', async (req, res) => {
    try {
        const { timeRange = '24h' } = req.query;
        let startDate = new Date();

        switch(timeRange) {
            case '1h':
                startDate.setHours(startDate.getHours() - 1);
                break;
            case '24h':
                startDate.setDate(startDate.getDate() - 1);
                break;
            case '7d':
                startDate.setDate(startDate.getDate() - 7);
                break;
            case '30d':
                startDate.setDate(startDate.getDate() - 30);
                break;
        }

        const healthData = await HealthData.find({
            userId: req.params.userId,
            timestamp: { $gte: startDate }
        }).sort({ timestamp: 1 });

// Calculate analytics
        const analytics = calculateHealthAnalytics(healthData);
        res.json(analytics);
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
});

// Helper Functions

function isEmergencyCondition(healthData) {
    const { heartRate, temperature, bloodPressure } = healthData;

// Define emergency thresholds
    const emergencyConditions = [
        heartRate < 40 || heartRate > 140,
        temperature < 34 || temperature > 39,
        bloodPressure < 70 || bloodPressure > 140
    ];

    return emergencyConditions.some(condition => condition);
}

async function handleEmergency(data) {
    try {
// Create emergency record
        const emergency = new Emergency({
            userId: data.userId,
            emergencyType: data.manualTrigger ? 'manual' : 'automatic',
            triggerReason: data.reason || 'Health parameter anomaly',
            location: {
                latitude: data.latitude,
                longitude: data.longitude
            },
            healthData: {
                heartRate: data.heartRate,
                temperature: data.temperature,
                bloodPressure: data.bloodPressure
            }
        });

        await emergency.save();

// Get user details
        const user = await User.findOne({ phone: data.userId });

        if (user) {
// Send Telegram alert
            const alertMessage = `🚨 EMERGENCY ALERT 🚨\n\n` +
                `Patient: ${user.name}\n` +
                `Age: ${user.age}, ${user.gender}\n` +
                `Phone: ${user.phone}\n` +
                `Emergency Contact: ${user.emergencyContact}\n` +
                `Heart Rate: ${data.heartRate} BPM\n` +
                `Temperature: ${data.temperature}°C\n` +
                `Blood Pressure: ${data.bloodPressure} mmHg\n` +
                `Location: ${data.latitude}, ${data.longitude}\n` +
                `Time: ${new Date().toLocaleString()}\n` +
                `Medical Conditions: ${user.medicalConditions}\n\n` +
                `Google Maps: https://maps.google.com/?q=${data.latitude},${data.longitude}`;

            await bot.sendMessage(CHAT_ID, alertMessage);

// Broadcast emergency to all connected clients
            broadcastHealthData({
                ...data,
                emergencyStatus: true,
                emergencyId: emergency._id
            });

            console.log(`Emergency alert sent for user: ${user.name}`);
        }

    } catch (error) {
        console.error('Error handling emergency:', error);
    }
}

function calculateHealthAnalytics(healthData) {
    if (healthData.length === 0) {
        return { message: 'No data available for analysis' };
    }

    const heartRates = healthData.map(d => d.heartRate).filter(hr => hr);
    const temperatures = healthData.map(d => d.temperature).filter(t => t);
    const bloodPressures = healthData.map(d => d.bloodPressure).filter(bp => bp);

    return {
        heartRate: {
            average: heartRates.reduce((a, b) => a + b, 0) / heartRates.length,
            min: Math.min(...heartRates),
            max: Math.max(...heartRates),
            trend: calculateTrend(heartRates)
        },
        temperature: {
            average: temperatures.reduce((a, b) => a + b, 0) / temperatures.length,
            min: Math.min(...temperatures),
            max: Math.max(...temperatures),
            trend: calculateTrend(temperatures)
        },
        bloodPressure: {
            average: bloodPressures.reduce((a, b) => a + b, 0) / bloodPressures.length,
            min: Math.min(...bloodPressures),
            max: Math.max(...bloodPressures),
            trend: calculateTrend(bloodPressures)
        },
        dataPoints: healthData.length,
        timeRange: {
            start: healthData[0].timestamp,
            end: healthData[healthData.length - 1].timestamp
        }
    };
}

function calculateTrend(values) {
    if (values.length < 2) return 'stable';

    const firstHalf = values.slice(0, Math.floor(values.length / 2));
    const secondHalf = values.slice(Math.floor(values.length / 2));

    const firstAvg = firstHalf.reduce((a, b) => a + b, 0) / firstHalf.length;
    const secondAvg = secondHalf.reduce((a, b) => a + b, 0) / secondHalf.length;

    const diff = secondAvg - firstAvg;
    const threshold = firstAvg * 0.05;// 5% threshold

    if (diff > threshold) return 'increasing';
    if (diff < -threshold) return 'decreasing';
    return 'stable';
}

// Serve the main HTML file
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// Start server
const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
    console.log(`RescueNet AI Server running on port ${PORT}`);
    console.log(`Dashboard: http://localhost:${PORT}`);
});

module.exports = app;
