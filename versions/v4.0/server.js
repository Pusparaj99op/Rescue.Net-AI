const express = require('express');
const mongoose = require('mongoose');
const cors = require('cors');
const http = require('http');
const socketIo = require('socket.io');
const TelegramBot = require('node-telegram-bot-api');
const cron = require('node-cron');
require('dotenv').config();

const app = express();
const server = http.createServer(app);
const io = socketIo(server, {
  cors: {
    origin: "*",
    methods: ["GET", "POST"]
  }
});

// Middleware
app.use(cors());
app.use(express.json());
app.use(express.static('public'));

// MongoDB Connection
mongoose.connect(process.env.MONGODB_URI || 'mongodb://localhost:27017/rescuenet', {
  useNewUrlParser: true,
  useUnifiedTopology: true
});

// Telegram Bot
const bot = new TelegramBot(process.env.TELEGRAM_TOKEN, { polling: true });

// User Schema
const userSchema = new mongoose.Schema({
  phone: { type: String, required: true, unique: true },
  name: { type: String, required: true },
 email: { type: String },
 age: { type: Number, required: true },
 gender: { type: String, enum: ['Male', 'Female', 'Other'], required: true },
 bloodType: { type: String },
 emergencyContacts: [{
   name: String,
   phone: String,
   relationship: String
 }],
 medicalHistory: { type: String },
 medications: [String],
 allergies: [String],
 lastPeriodDate: { type: Date },
 isPregnant: { type: Boolean, default: false },
 createdAt: { type: Date, default: Date.now },
 isActive: { type: Boolean, default: true }
});

// Health Data Schema
const healthDataSchema = new mongoose.Schema({
 userId: { type: String, required: true },
 timestamp: { type: Date, default: Date.now },
 heartRate: { type: Number },
 temperature: { type: Number },
 bloodPressure: {
   systolic: Number,
   diastolic: Number
 },
 location: {
   latitude: Number,
   longitude: Number,
   altitude: Number
 },
 accelerometer: {
   x: Number,
   y: Number,
   z: Number
 },
 batteryLevel: { type: Number },
 fallDetected: { type: Boolean, default: false },
 emergencyTriggered: { type: Boolean, default: false }
});

// Emergency Log Schema
const emergencySchema = new mongoose.Schema({
 userId: { type: String, required: true },
 timestamp: { type: Date, default: Date.now },
 type: { type: String, enum: ['manual', 'auto', 'fall', 'vitals'], required: true },
 message: { type: String, required: true },
 location: {
   latitude: Number,
   longitude: Number
 },
 vitalSigns: {
   heartRate: Number,
   temperature: Number,
   bloodPressure: {
     systolic: Number,
     diastolic: Number
   }
 },
 status: { type: String, enum: ['active', 'resolved', 'cancelled'], default: 'active' },
 responseTime: { type: Number }, // in seconds
 responders: [{
   name: String,
   phone: String,
   type: String, // 'ambulance', 'volunteer', 'family'
   eta: Number
 }]
});

const User = mongoose.model('User', userSchema);
const HealthData = mongoose.model('HealthData', healthDataSchema);
const Emergency = mongoose.model('Emergency', emergencySchema);

// Routes
// User Registration
app.post('/api/register', async (req, res) => {
 try {
   const user = new User(req.body);
   await user.save();
   res.status(201).json({ success: true, message: 'User registered successfully', userId: user._id });
 } catch (error) {
   res.status(400).json({ success: false, message: error.message });
 }
});

// User Login
app.post('/api/login', async (req, res) => {
 try {
   const { phone } = req.body;
   const user = await User.findOne({ phone });
   
   if (!user) {
     return res.status(404).json({ success: false, message: 'User not found' });
   }
   
   res.json({ success: true, user });
 } catch (error) {
   res.status(500).json({ success: false, message: error.message });
 }
});

// Health Data Endpoint
app.post('/api/health_data', async (req, res) => {
 try {
   const healthData = new HealthData(req.body);
   await healthData.save();
   
   // Emit real-time data to connected clients
   io.emit('healthUpdate', healthData);
   
   res.json({ success: true, message: 'Health data saved' });
 } catch (error) {
   res.status(500).json({ success: false, message: error.message });
 }
});

// Emergency Endpoint
app.post('/api/emergency', async (req, res) => {
 try {
   const emergency = new Emergency(req.body);
   await emergency.save();
   
   // Emit real-time emergency alert
   io.emit('emergencyAlert', emergency);
   
   // Send Telegram notifications
   await sendTelegramAlert(emergency);
   
   // Trigger emergency response protocol
   await triggerEmergencyResponse(emergency);
   
   res.json({ success: true, message: 'Emergency alert sent', emergencyId: emergency._id });
 } catch (error) {
   res.status(500).json({ success: false, message: error.message });
 }
});

// Get User Dashboard Data
app.get('/api/dashboard/:userId', async (req, res) => {
 try {
   const { userId } = req.params;
   
   const user = await User.findById(userId);
   const recentHealthData = await HealthData.find({ userId }).sort({ timestamp: -1 }).limit(100);
   const emergencies = await Emergency.find({ userId }).sort({ timestamp: -1 }).limit(10);
   
   res.json({
     success: true,
     data: {
       user,
       healthData: recentHealthData,
       emergencies
     }
   });
 } catch (error) {
   res.status(500).json({ success: false, message: error.message });
 }
});

// Get Real-time Health Stats
app.get('/api/stats/:userId', async (req, res) => {
 try {
   const { userId } = req.params;
   const timeRange = req.query.range || '24h';
   
   let startTime = new Date();
   switch(timeRange) {
     case '1h':
       startTime.setHours(startTime.getHours() - 1);
       break;
     case '24h':
       startTime.setHours(startTime.getHours() - 24);
       break;
     case '7d':
       startTime.setDate(startTime.getDate() - 7);
       break;
     default:
       startTime.setHours(startTime.getHours() - 24);
   }
   
   const healthData = await HealthData.find({
     userId,
     timestamp: { $gte: startTime }
   }).sort({ timestamp: 1 });
   
   const stats = calculateHealthStats(healthData);
   
   res.json({ success: true, stats, data: healthData });
 } catch (error) {
   res.status(500).json({ success: false, message: error.message });
 }
});

// Emergency Response Functions
async function sendTelegramAlert(emergency) {
 const user = await User.findOne({ phone: emergency.userId });
 if (!user) return;
 
 const message = `🚨 EMERGENCY ALERT 🚨\n\n` +
   `User: ${user.name}\n` +
   `Phone: ${user.phone}\n` +
   `Emergency: ${emergency.message}\n` +
   `Location: ${emergency.location.latitude}, ${emergency.location.longitude}\n` +
   `Time: ${emergency.timestamp}\n\n` +
   `Vital Signs:\n` +
   `❤️ Heart Rate: ${emergency.vitalSigns.heartRate} BPM\n` +
   `🌡️ Temperature: ${emergency.vitalSigns.temperature}°C\n` +
   `🩸 Blood Pressure: ${emergency.vitalSigns.bloodPressure.systolic}/${emergency.vitalSigns.bloodPressure.diastolic}\n\n` +
   `📍 Google Maps: https://maps.google.com/?q=${emergency.location.latitude},${emergency.location.longitude}`;
 
 // Send to emergency contacts
 for (const contact of user.emergencyContacts) {
   try {
     await bot.sendMessage(contact.phone, message);
   } catch (error) {
     console.error('Failed to send Telegram message:', error);
   }
 }
}

async function triggerEmergencyResponse(emergency) {
 // Simulate emergency response system
 const nearbyAmbulances = await findNearbyAmbulances(emergency.location);
 const volunteers = await findNearbyVolunteers(emergency.location);
 
 // Update emergency with responder information
 emergency.responders = [
   ...nearbyAmbulances.map(amb => ({
     name: amb.name,
     phone: amb.phone,
     type: 'ambulance',
     eta: amb.eta
   })),
   ...volunteers.map(vol => ({
     name: vol.name,
     phone: vol.phone,
     type: 'volunteer',
     eta: vol.eta
   }))
 ];
 
 await emergency.save();
}

function calculateHealthStats(healthData) {
 if (!healthData.length) return {};
 
 const heartRates = healthData.map(d => d.heartRate).filter(hr => hr);
 const temperatures = healthData.map(d => d.temperature).filter(t => t);
 
 return {
   heartRate: {
     avg: heartRates.reduce((a, b) => a + b, 0) / heartRates.length,
     min: Math.min(...heartRates),
     max: Math.max(...heartRates)
   },
   temperature: {
     avg: temperatures.reduce((a, b) => a + b, 0) / temperatures.length,
     min: Math.min(...temperatures),
     max: Math.max(...temperatures)
   },
   totalDataPoints: healthData.length,
   emergencyCount: healthData.filter(d => d.emergencyTriggered).length
 };
}

// Mock functions for demo
async function findNearbyAmbulances(location) {
 return [
   { name: 'City Ambulance 1', phone: '+919876543210', eta: 8 },
   { name: 'Private Ambulance', phone: '+919876543211', eta: 12 }
 ];
}

async function findNearbyVolunteers(location) {
 return [
   { name: 'Dr. Volunteer 1', phone: '+919876543212', eta: 3 },
   { name: 'First Aid Volunteer', phone: '+919876543213', eta: 5 }
 ];
}

// Socket.IO Connection
io.on('connection', (socket) => {
 console.log('Client connected:', socket.id);
 
 socket.on('joinRoom', (userId) => {
   socket.join(userId);
   console.log(`User ${userId} joined room`);
 });
 
 socket.on('disconnect', () => {
   console.log('Client disconnected:', socket.id);
 });
});

// Scheduled Tasks
cron.schedule('*/5 * * * *', async () => {
 // Check for inactive users (no data received in last 10 minutes)
 const tenMinutesAgo = new Date(Date.now() - 10 * 60 * 1000);
 const inactiveUsers = await HealthData.aggregate([
   { $group: { _id: '$userId', lastUpdate: { $max: '$timestamp' } } },
   { $match: { lastUpdate: { $lt: tenMinutesAgo } } }
 ]);
 
 for (const user of inactiveUsers) {
   io.emit('userInactive', { userId: user._id, lastSeen: user.lastUpdate });
 }
});

const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
 console.log(`RescueNet AI Server running on port ${PORT}`);
});

    