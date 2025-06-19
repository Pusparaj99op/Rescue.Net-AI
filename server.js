require('dotenv').config();
const express = require('express');
const mongoose = require('mongoose');
const cors = require('cors');
const bodyParser = require('body-parser');
const path = require('path');
const WebSocket = require('ws');
const helmet = require('helmet');
const rateLimit = require('express-rate-limit');
const twilio = require('twilio');
const nodemailer = require('nodemailer');
const EmergencyServices = require('./utils/emergencyServices');
const HealthAnalytics = require('./utils/healthAnalytics');
const bcrypt = require('bcryptjs');
const jwt = require('jsonwebtoken');
const crypto = require('crypto');
const session = require('express-session');
const MongoStore = require('connect-mongo');
const csrfTokens = require('csrf-tokens');

const app = express();
const PORT = process.env.PORT || 3000;
const WEBSOCKET_PORT = process.env.WEBSOCKET_PORT || 8080;

// Security middleware
app.use(helmet());

// Rate limiting
const limiter = rateLimit({
  windowMs: parseInt(process.env.RATE_LIMIT_WINDOW_MS) || 15 * 60 * 1000, // 15 minutes
  max: parseInt(process.env.RATE_LIMIT_MAX_REQUESTS) || 100, // limit each IP to 100 requests per windowMs
  message: 'Too many requests from this IP, please try again later.'
});
app.use('/api/', limiter);

// Middleware
app.use(cors());
app.use(bodyParser.json({ limit: '10mb' }));
app.use(express.static('public'));

// MongoDB connection
const mongoUri = process.env.MONGODB_URI || 'mongodb://localhost:27017/rescuenet';
mongoose.connect(mongoUri, {
  useNewUrlParser: true,
  useUnifiedTopology: true
}).then(() => {
  console.log('Connected to MongoDB');
}).catch(err => {
  console.error('MongoDB connection error:', err);
});

// Initialize Twilio client
let twilioClient = null;
if (process.env.TWILIO_ACCOUNT_SID && process.env.TWILIO_AUTH_TOKEN) {
  twilioClient = twilio(process.env.TWILIO_ACCOUNT_SID, process.env.TWILIO_AUTH_TOKEN);
}

// Initialize email transporter
let emailTransporter = null;
if (process.env.EMAIL_USER && process.env.EMAIL_PASS) {
  emailTransporter = nodemailer.createTransporter({
    host: process.env.EMAIL_HOST || 'smtp.gmail.com',
    port: process.env.EMAIL_PORT || 587,
    secure: false,
    auth: {
      user: process.env.EMAIL_USER,
      pass: process.env.EMAIL_PASS
    }
  });
}

// Initialize CSRF protection
const csrf = csrfTokens();

// Session configuration
app.use(session({
  secret: process.env.SESSION_SECRET || 'rescuenet-ai-session-secret',
  resave: false,
  saveUninitialized: false,
  store: MongoStore.create({
    mongoUrl: mongoUri,
    touchAfter: 24 * 3600 // lazy session update
  }),
  cookie: {
    secure: process.env.NODE_ENV === 'production',
    httpOnly: true,
    maxAge: 1000 * 60 * 60 * 24 * 7 // 1 week
  }
}));

// Enhanced User Schema
const userSchema = new mongoose.Schema({
  // Basic Information
  fullName: { type: String, required: true },
  email: { type: String, required: true, unique: true },
  phoneNumber: { type: String, required: true },
  password: { type: String, required: true },
  
  // Profile Information  
  dateOfBirth: Date,
  address: {
    street: String,
    city: String,
    state: String,
    zipCode: String,
    country: String
  },
  
  // Medical Information
  medicalInfo: {
    bloodGroup: String,
    height: Number,
    weight: Number,
    allergies: [String],
    medications: [String],
    medicalConditions: [String],
    emergencyMedicalInfo: String,
    physicianName: String,
    physicianPhone: String
  },
  
  // Emergency Contacts
  emergencyContacts: [{
    name: { type: String, required: true },
    relationship: String,
    phone: { type: String, required: true },
    email: String,
    isPrimary: { type: Boolean, default: false }
  }],
  
  // Security Settings
  security: {
    twoFactorEnabled: { type: Boolean, default: false },
    twoFactorSecret: String,
    lastLogin: Date,
    loginAttempts: { type: Number, default: 0 },
    lockUntil: Date,
    passwordResetToken: String,
    passwordResetExpires: Date,
    emailVerificationToken: String,
    emailVerified: { type: Boolean, default: false }
  },
  
  // Preferences
  preferences: {
    notifications: {
      emergencyAlerts: { type: Boolean, default: true },
      healthReminders: { type: Boolean, default: true },
      marketingEmails: { type: Boolean, default: false }
    },
    privacy: {
      shareResearchData: { type: Boolean, default: false },
      allowThirdPartyIntegrations: { type: Boolean, default: false },
      locationTracking: { type: Boolean, default: true },
      emergencyDataSharing: { type: Boolean, default: true }
    },
    device: {
      monitoringFrequency: { type: String, default: 'Every 5 minutes' },
      alertSensitivity: { type: String, default: 'Medium' }
    }
  },
  
  // Account Status
  isActive: { type: Boolean, default: true },
  createdAt: { type: Date, default: Date.now },
  updatedAt: { type: Date, default: Date.now }
});

// Add password hashing middleware
userSchema.pre('save', async function(next) {
  if (!this.isModified('password')) return next();
  
  try {
    const salt = await bcrypt.genSalt(12);
    this.password = await bcrypt.hash(this.password, salt);
    next();
  } catch (error) {
    next(error);
  }
});

// Password comparison method
userSchema.methods.comparePassword = async function(candidatePassword) {
  return bcrypt.compare(candidatePassword, this.password);
};

// Generate password reset token
userSchema.methods.createPasswordResetToken = function() {
  const resetToken = crypto.randomBytes(32).toString('hex');
  this.security.passwordResetToken = crypto.createHash('sha256').update(resetToken).digest('hex');
  this.security.passwordResetExpires = Date.now() + 10 * 60 * 1000; // 10 minutes
  return resetToken;
};

// JWT token generation
const generateToken = (userId) => {
  return jwt.sign({ userId }, process.env.JWT_SECRET || 'rescuenet-ai-jwt-secret', {
    expiresIn: '7d'
  });
};

// Authentication middleware
const authenticateToken = async (req, res, next) => {
  try {
    const token = req.header('Authorization')?.replace('Bearer ', '') || req.session.token;
    
    if (!token) {
      return res.status(401).json({ error: 'Access denied. No token provided.' });
    }

    const decoded = jwt.verify(token, process.env.JWT_SECRET || 'rescuenet-ai-jwt-secret');
    const user = await User.findById(decoded.userId).select('-password');
    
    if (!user || !user.isActive) {
      return res.status(401).json({ error: 'Invalid token or user deactivated.' });
    }

    req.user = user;
    next();
  } catch (error) {
    res.status(401).json({ error: 'Invalid token.' });
  }
};

// CSRF token endpoint
app.get('/api/csrf-token', (req, res) => {
  const secret = csrf.secretSync();
  const token = csrf.create(secret);
  req.session.csrfSecret = secret;
  res.json({ csrfToken: token });
});

// CSRF validation middleware
const validateCSRF = (req, res, next) => {
  if (process.env.NODE_ENV === 'development') {
    return next(); // Skip CSRF in development
  }
  
  const token = req.header('X-CSRF-Token') || req.body.csrfToken;
  const secret = req.session.csrfSecret;
  
  if (!token || !secret || !csrf.verify(secret, token)) {
    return res.status(403).json({ error: 'Invalid CSRF token' });
  }
  
  next();
};

// Rate limiting for authentication endpoints
const authLimiter = rateLimit({
  windowMs: 15 * 60 * 1000, // 15 minutes
  max: 5, // limit each IP to 5 requests per windowMs
  message: 'Too many authentication attempts, please try again later.',
  standardHeaders: true,
  legacyHeaders: false,
});

// User Registration API
app.post('/api/auth/register', authLimiter, validateCSRF, async (req, res) => {
  try {
    const {
      fullName,
      email,
      phoneNumber,
      password,
      dateOfBirth,
      address,
      medicalInfo,
      emergencyContacts
    } = req.body;

    // Validation
    if (!fullName || !email || !phoneNumber || !password) {
      return res.status(400).json({ error: 'Missing required fields' });
    }

    // Check if user already exists
    const existingUser = await User.findOne({ 
      $or: [{ email }, { phoneNumber }] 
    });
    
    if (existingUser) {
      return res.status(400).json({ error: 'User with this email or phone number already exists' });
    }

    // Create new user
    const user = new User({
      fullName,
      email,
      phoneNumber,
      password,
      dateOfBirth,
      address,
      medicalInfo,
      emergencyContacts,
      security: {
        emailVerificationToken: crypto.randomBytes(32).toString('hex')
      }
    });

    await user.save();

    // Generate JWT token
    const token = generateToken(user._id);
    req.session.token = token;
    req.session.userId = user._id;

    // Remove sensitive information
    const userResponse = user.toObject();
    delete userResponse.password;
    delete userResponse.security.passwordResetToken;
    delete userResponse.security.emailVerificationToken;

    res.status(201).json({
      message: 'User registered successfully',
      user: userResponse,
      token
    });

  } catch (error) {
    console.error('Registration error:', error);
    res.status(500).json({ error: 'Internal server error' });
  }
});

// User Login API
app.post('/api/auth/login', authLimiter, validateCSRF, async (req, res) => {
  try {
    const { email, password, rememberMe } = req.body;

    if (!email || !password) {
      return res.status(400).json({ error: 'Email and password are required' });
    }

    // Find user and check if account is locked
    const user = await User.findOne({ email });
    
    if (!user || !user.isActive) {
      return res.status(401).json({ error: 'Invalid credentials' });
    }

    // Check if account is locked
    if (user.security.lockUntil && user.security.lockUntil > Date.now()) {
      return res.status(423).json({ error: 'Account temporarily locked due to too many failed attempts' });
    }

    // Verify password
    const isMatch = await user.comparePassword(password);
    
    if (!isMatch) {
      // Increment login attempts
      user.security.loginAttempts += 1;
      
      if (user.security.loginAttempts >= 5) {
        user.security.lockUntil = Date.now() + 15 * 60 * 1000; // Lock for 15 minutes
      }
      
      await user.save();
      return res.status(401).json({ error: 'Invalid credentials' });
    }

    // Reset login attempts on successful login
    user.security.loginAttempts = 0;
    user.security.lockUntil = undefined;
    user.security.lastLogin = new Date();
    await user.save();

    // Generate JWT token
    const tokenExpiry = rememberMe ? '30d' : '7d';
    const token = jwt.sign({ userId: user._id }, process.env.JWT_SECRET || 'rescuenet-ai-jwt-secret', {
      expiresIn: tokenExpiry
    });

    req.session.token = token;
    req.session.userId = user._id;

    // Update session expiry based on "Remember Me"
    if (rememberMe) {
      req.session.cookie.maxAge = 1000 * 60 * 60 * 24 * 30; // 30 days
    }

    // Remove sensitive information
    const userResponse = user.toObject();
    delete userResponse.password;
    delete userResponse.security.passwordResetToken;
    delete userResponse.security.emailVerificationToken;
    delete userResponse.security.twoFactorSecret;

    res.json({
      message: 'Login successful',
      user: userResponse,
      token,
      requiresTwoFactor: user.security.twoFactorEnabled
    });

  } catch (error) {
    console.error('Login error:', error);
    res.status(500).json({ error: 'Internal server error' });
  }
});

// Get User Profile API
app.get('/api/user/profile', authenticateToken, async (req, res) => {
  try {
    const user = await User.findById(req.user._id).select('-password -security.passwordResetToken -security.emailVerificationToken -security.twoFactorSecret');
    
    if (!user) {
      return res.status(404).json({ error: 'User not found' });
    }

    res.json({ user });
  } catch (error) {
    console.error('Profile fetch error:', error);
    res.status(500).json({ error: 'Internal server error' });
  }
});

// Update User Profile API
app.put('/api/user/profile', authenticateToken, validateCSRF, async (req, res) => {
  try {
    const updates = req.body;
    const userId = req.user._id;

    // Remove sensitive fields that shouldn't be updated via this endpoint
    delete updates.password;
    delete updates.security;
    delete updates._id;
    delete updates.createdAt;

    // Update timestamp
    updates.updatedAt = new Date();

    const user = await User.findByIdAndUpdate(
      userId,
      { $set: updates },
      { new: true, runValidators: true }
    ).select('-password -security.passwordResetToken -security.emailVerificationToken -security.twoFactorSecret');

    if (!user) {
      return res.status(404).json({ error: 'User not found' });
    }

    res.json({
      message: 'Profile updated successfully',
      user
    });

  } catch (error) {
    console.error('Profile update error:', error);
    res.status(500).json({ error: 'Internal server error' });
  }
});

// Update User Preferences API
app.put('/api/user/preferences', authenticateToken, validateCSRF, async (req, res) => {
  try {
    const { preferences } = req.body;
    const userId = req.user._id;

    const user = await User.findByIdAndUpdate(
      userId,
      { 
        $set: { 
          preferences,
          updatedAt: new Date()
        }
      },
      { new: true, runValidators: true }
    ).select('-password -security.passwordResetToken -security.emailVerificationToken -security.twoFactorSecret');

    if (!user) {
      return res.status(404).json({ error: 'User not found' });
    }

    res.json({
      message: 'Preferences updated successfully',
      preferences: user.preferences
    });

  } catch (error) {
    console.error('Preferences update error:', error);
    res.status(500).json({ error: 'Internal server error' });
  }
});

// Change Password API
app.put('/api/user/change-password', authenticateToken, validateCSRF, async (req, res) => {
  try {
    const { currentPassword, newPassword } = req.body;
    const userId = req.user._id;

    if (!currentPassword || !newPassword) {
      return res.status(400).json({ error: 'Current password and new password are required' });
    }

    const user = await User.findById(userId);
    
    if (!user) {
      return res.status(404).json({ error: 'User not found' });
    }

    // Verify current password
    const isMatch = await user.comparePassword(currentPassword);
    
    if (!isMatch) {
      return res.status(401).json({ error: 'Current password is incorrect' });
    }

    // Update password
    user.password = newPassword;
    user.updatedAt = new Date();
    await user.save();

    res.json({ message: 'Password changed successfully' });

  } catch (error) {
    console.error('Password change error:', error);
    res.status(500).json({ error: 'Internal server error' });
  }
});

// Logout API
app.post('/api/auth/logout', authenticateToken, (req, res) => {
  req.session.destroy((err) => {
    if (err) {
      return res.status(500).json({ error: 'Could not log out' });
    }
    res.clearCookie('connect.sid');
    res.json({ message: 'Logged out successfully' });
  });
});

// Demo accounts for testing
app.get('/api/demo-accounts', (req, res) => {
  res.json({
    accounts: [
      {
        email: 'demo@rescuenet.ai',
        password: 'demo123',
        name: 'Demo User',
        description: 'Standard user account with sample data'
      },
      {
        email: 'admin@rescuenet.ai', 
        password: 'admin123',
        name: 'Admin User',
        description: 'Administrative access with full permissions'
      }
    ]
  });
});

// User registration/login
app.post('/api/register', async (req, res) => {
 try {
   const { phone, name, age, gender, bloodGroup, emergencyContact, medicalHistory, lastPeriodDate } = req.body;
   
   const user = new User({
     phone,
     name,
     age,
     gender,
     bloodGroup,
     emergencyContact,
     medicalHistory,
     lastPeriodDate: lastPeriodDate ? new Date(lastPeriodDate) : null
   });
   
   await user.save();
   res.json({ success: true, message: 'User registered successfully', user });
 } catch (error) {
   if (error.code === 11000) {
     res.status(400).json({ success: false, message: 'Phone number already registered' });
   } else {
     res.status(500).json({ success: false, message: error.message });
   }
 }
});

app.get('/api/user/:phone', async (req, res) => {
 try {
   const user = await User.findOne({ phone: req.params.phone });
   if (!user) {
     return res.status(404).json({ success: false, message: 'User not found' });
   }
   res.json({ success: true, user });
 } catch (error) {
   res.status(500).json({ success: false, message: error.message });
 }
});

// Health data endpoint with anomaly detection
app.post('/api/health-data', async (req, res) => {
  try {
    const healthData = new HealthData(req.body);
    await healthData.save();
      // Check for anomalies and trigger emergency if needed
    const anomalies = detectHealthAnomalies(healthData);
    if (anomalies.length > 0) {
      const user = await User.findOne({ phone: healthData.userId });
      if (user) {
        const emergency = new Emergency({
          userId: healthData.userId,
          reason: `Health anomaly detected: ${anomalies.join(', ')}`,
          location: healthData.location,
          vitals: healthData.vitals,
          autoDetected: true
        });
        
        await emergency.save();
        
        // Send emergency notifications (including SMS)
        await handleEmergencyNotifications(user, emergency);
        
        // Send emergency SMS if enabled
        if (user.emergencyContact) {
          await EmergencyServices.sendEmergencySMS(
            user.emergencyContact,
            emergency,
            user
          );
        }
        
        // Broadcast emergency alert
        broadcast({
          type: 'emergency',
          data: emergency
        });
      }
    }
    
    // Broadcast real-time data
    broadcast({
      type: 'health_data',
      data: healthData
    });
    
    res.json({ success: true, message: 'Health data saved successfully', anomalies });
  } catch (error) {
    console.error('Health data error:', error);
    res.status(500).json({ success: false, message: error.message });
  }
});

// Function to detect health anomalies
function detectHealthAnomalies(healthData) {
  const anomalies = [];
  const vitals = healthData.vitals;
  
  if (vitals.heartRate) {
    if (vitals.heartRate > 120 || vitals.heartRate < 50) {
      anomalies.push(`Abnormal heart rate: ${vitals.heartRate} BPM`);
    }
  }
  
  if (vitals.temperature) {
    if (vitals.temperature > 38.5 || vitals.temperature < 35.0) {
      anomalies.push(`Abnormal temperature: ${vitals.temperature}°C`);
    }
  }
  
  if (vitals.bloodPressure) {
    if (vitals.bloodPressure > 180 || vitals.bloodPressure < 70) {
      anomalies.push(`Abnormal blood pressure: ${vitals.bloodPressure} mmHg`);
    }
  }
  
  // Check accelerometer for fall detection
  if (healthData.accelerometer) {
    const { x, y, z } = healthData.accelerometer;
    const magnitude = Math.sqrt(x*x + y*y + z*z);
    if (magnitude > 15) { // Threshold for fall detection
      anomalies.push('Potential fall detected');
    }
  }
  
  return anomalies;
}

// Emergency endpoint with enhanced notifications including SMS
app.post('/api/emergency', async (req, res) => {
  try {
    const { userId, reason, location, vitals, userInfo } = req.body;
    
    const emergency = new Emergency({
      userId,
      reason,
      location,
      vitals,
      userInfo: userInfo || {}
    });
    
    await emergency.save();
    
    // Get user details
    const user = await User.findOne({ phone: userId });
    if (user) {
      // Send all emergency notifications
      await handleEmergencyNotifications(user, emergency);
      
      // Send emergency SMS if contact available
      if (user.emergencyContact) {
        console.log(`Sending emergency SMS to ${user.emergencyContact}...`);
        await EmergencyServices.sendEmergencySMS(
          user.emergencyContact,
          emergency,
          user
        );
      }
    }
    
    // Broadcast emergency alert
    broadcast({
      type: 'emergency',
      data: emergency
    });
    
    res.json({ 
      success: true, 
      message: 'Emergency alert sent via all channels', 
      emergencyId: emergency._id,
      smsEnabled: !!user?.emergencyContact
    });
  } catch (error) {
    console.error('Emergency error:', error);
    res.status(500).json({ success: false, message: error.message });
  }
});

// Handle emergency notifications
async function handleEmergencyNotifications(user, emergency) {
  const locationStr = emergency.location ? 
    `Location: https://maps.google.com/maps?q=${emergency.location.lat},${emergency.location.lng}` : 
    'Location: Not available';
  
  const vitalsStr = emergency.vitals ? 
    `Heart Rate: ${emergency.vitals.heartRate || 'N/A'} BPM, Temperature: ${emergency.vitals.temperature || 'N/A'}°C` : 
    'Vitals: Not available';
  
  const message = `
🚨 EMERGENCY ALERT 🚨

Patient: ${user.name}
Age: ${user.age}
Blood Group: ${user.bloodGroup || 'Unknown'}
Medical History: ${user.medicalHistory || 'None specified'}

Emergency: ${emergency.reason}
Time: ${new Date(emergency.timestamp).toLocaleString()}
${locationStr}
${vitalsStr}

This person needs immediate medical attention!
  `;
  
  // Send SMS to emergency contact
  if (user.emergencyContact) {
    await sendSMSAlert(user.emergencyContact, message);
  }
  
  // Send email if available
  if (user.email) {
    await sendEmailAlert(user.email, '🚨 Emergency Alert - Immediate Action Required', message);
  }
  
  // Here you would typically integrate with:
  // 1. Local emergency services API
  // 2. Hospital management systems
  // 3. Ambulance dispatch services
  // 4. Insurance providers
  
  console.log('Emergency notifications sent for user:', user.phone);
}

// Get user's health history
app.get('/api/health-history/:userId', async (req, res) => {
 try {
   const limit = parseInt(req.query.limit) || 100;
   const healthHistory = await HealthData.find({ userId: req.params.userId })
     .sort({ timestamp: -1 })
     .limit(limit);
   
   res.json({ success: true, data: healthHistory });
 } catch (error) {
   res.status(500).json({ success: false, message: error.message });
 }
});

// Get emergency history
app.get('/api/emergency-history/:userId', async (req, res) => {
 try {
   const emergencies = await Emergency.find({ userId: req.params.userId })
     .sort({ timestamp: -1 });
   
   res.json({ success: true, data: emergencies });
 } catch (error) {
   res.status(500).json({ success: false, message: error.message });
 }
});

// Enhanced dashboard data with analytics
app.get('/api/dashboard/:userId', async (req, res) => {
  try {
    const userId = req.params.userId;
    
    // Get user details
    const user = await User.findOne({ phone: userId });
    if (!user) {
      return res.status(404).json({ success: false, message: 'User not found' });
    }
    
    // Get latest health data
    const latestHealth = await HealthData.findOne({ userId }).sort({ timestamp: -1 });
    
    // Get health history for analytics
    const healthHistory = await HealthData.find({ userId })
      .sort({ timestamp: -1 })
      .limit(100);
    
    // Analyze health trends
    const trends = HealthAnalytics.analyzeHealthTrends(healthHistory);
    
    // Detect patterns
    const patterns = HealthAnalytics.detectPatterns(healthHistory);
    
    // Generate insights
    const insights = HealthAnalytics.generateHealthInsights(user, healthHistory, trends, patterns);
    
    // Get health stats for last 7 days
    const sevenDaysAgo = new Date();
    sevenDaysAgo.setDate(sevenDaysAgo.getDate() - 7);
    
    const weeklyStats = await HealthData.aggregate([
      { $match: { userId, timestamp: { $gte: sevenDaysAgo } } },
      {
        $group: {
          _id: null,
          avgHeartRate: { $avg: '$vitals.heartRate' },
          avgTemperature: { $avg: '$vitals.temperature' },
          avgBloodPressure: { $avg: '$vitals.bloodPressure' },
          maxHeartRate: { $max: '$vitals.heartRate' },
          minHeartRate: { $min: '$vitals.heartRate' },
          totalReadings: { $sum: 1 },
          activeDays: { $addToSet: { $dateToString: { format: "%Y-%m-%d", date: "$timestamp" } } }
        }
      }
    ]);
    
    // Get emergency count
    const emergencyCount = await Emergency.countDocuments({ userId });
    
    const statsData = weeklyStats[0] || {};
    if (statsData.activeDays) {
      statsData.activeDays = statsData.activeDays.length;
    }
    statsData.emergencyCount = emergencyCount;
    
    res.json({
      success: true,
      data: {
        user,
        latestHealth,
        weeklyStats: statsData,
        emergencyCount,
        trends,
        patterns,
        insights
      }
    });
  } catch (error) {
    console.error('Dashboard error:', error);
    res.status(500).json({ success: false, message: error.message });
  }
});

// Serve main HTML file
app.get('/', (req, res) => {
 res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

app.listen(PORT, () => {
  console.log(`🚀 RescueNet AI Server running on port ${PORT}`);
  console.log(`📡 WebSocket server running on port ${WEBSOCKET_PORT}`);
  console.log(`🌐 Dashboard: http://localhost:${PORT}`);
  console.log(`📱 WebSocket: ws://localhost:${WEBSOCKET_PORT}`);
  
  if (process.env.NODE_ENV === 'development') {
    console.log('\n📋 Development Configuration:');
    console.log(`   MongoDB: ${mongoUri}`);
    console.log(`   Twilio SMS: ${twilioClient ? '✅ Configured' : '❌ Not configured'}`);
    console.log(`   Email: ${emailTransporter ? '✅ Configured' : '❌ Not configured'}`);
  }
});