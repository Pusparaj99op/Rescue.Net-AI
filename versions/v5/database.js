
const mongoose = require('mongoose');

// Replace with your MongoDB Atlas connection string
const MONGO_URI = 'mongodb+srv://<username>:<password>@<cluster-url>/<database-name>?retryWrites=true&w=majority';

const connectDB = async () => {
    try {
        await mongoose.connect(MONGO_URI, {
            useNewUrlParser: true,
            useUnifiedTopology: true,
        });
        console.log('MongoDB Connected...');
    } catch (err) {
        console.error(err.message);
        process.exit(1);
    }
};

// User Schema
const UserSchema = new mongoose.Schema({
    name: { type: String, required: true },
    age: { type: Number, required: true },
    gender: { type: String, required: true },
    medicalConditions: { type: String },
    emergencyContacts: [
        {
            name: { type: String, required: true },
            phone: { type: String, required: true },
            relationship: { type: String },
        },
    ],
    email: { type: String, required: true, unique: true },
    password: { type: String, required: true }, // Hashed password
    deviceId: { type: String, unique: true },
    registrationDate: { type: Date, default: Date.now },
});

// Vital Signs Schema
const VitalSignSchema = new mongoose.Schema({
    userId: { type: mongoose.Schema.Types.ObjectId, ref: 'User', required: true },
    timestamp: { type: Date, default: Date.now },
    heartRate: { type: Number },
    temperature: { type: Number },
    fallDetected: { type: Boolean, default: false },
    gps: {
        lat: { type: Number },
        lng: { type: Number },
    },
    batteryLevel: { type: Number },
});

// Emergency Log Schema
const EmergencyLogSchema = new mongoose.Schema({
    userId: { type: mongoose.Schema.Types.ObjectId, ref: 'User', required: true },
    timestamp: { type: Date, default: Date.now },
    type: { type: String, enum: ['manual', 'automatic'], required: true },
    location: {
        lat: { type: Number },
        lng: { type: Number },
    },
    vitalsAtEmergency: {
        heartRate: { type: Number },
        temperature: { type: Number },
    },
    status: { type: String, default: 'active' }, // e.g., active, resolved
});

const User = mongoose.model('User', UserSchema);
const VitalSign = mongoose.model('VitalSign', VitalSignSchema);
const EmergencyLog = mongoose.model('EmergencyLog', EmergencyLogSchema);

module.exports = {
    connectDB,
    User,
    VitalSign,
    EmergencyLog,
};
