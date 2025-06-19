# RescueNet AI - Demo Login Credentials

## 🔐 Demo User Accounts

For testing and demonstration purposes, you can use these pre-configured demo accounts:

### 👤 Demo User 1 - John Doe
- **Phone Number (Login ID)**: `1234567890`
- **Name**: John Doe
- **Age**: 35
- **Gender**: Male
- **Blood Group**: O+
- **Emergency Contact**: +1-555-EMERGENCY
- **Medical History**: No known allergies, occasional hypertension

### 👤 Demo User 2 - Jane Smith  
- **Phone Number (Login ID)**: `9876543210`
- **Name**: Jane Smith
- **Age**: 28
- **Gender**: Female
- **Blood Group**: A+  
- **Emergency Contact**: +1-555-HELP-ME
- **Medical History**: Diabetes Type 1, insulin dependent

### 👤 Demo User 3 - Robert Johnson
- **Phone Number (Login ID)**: `5555551234`
- **Name**: Robert Johnson
- **Age**: 67
- **Gender**: Male
- **Blood Group**: B-
- **Emergency Contact**: +1-555-FAMILY
- **Medical History**: Heart condition, pacemaker, blood thinners

### 👤 Demo User 4 - Maria Garcia
- **Phone Number (Login ID)**: `7777778888`
- **Name**: Maria Garcia
- **Age**: 42
- **Gender**: Female
- **Blood Group**: AB+
- **Emergency Contact**: +1-555-SPOUSE
- **Medical History**: Asthma, allergic to penicillin

### 👤 Demo User 5 - David Wilson
- **Phone Number (Login ID)**: `1111222233`
- **Name**: David Wilson
- **Age**: 24
- **Gender**: Male
- **Blood Group**: O-
- **Emergency Contact**: +1-555-PARENT
- **Medical History**: No known medical conditions

## 🚀 Quick Test Instructions

### Option 1: Use Existing Demo Users
1. **Open the dashboard**: `http://localhost:3000`
2. **Click "Login"**
3. **Enter any phone number above** (e.g., `1234567890`)
4. **Click Login** - You'll access the demo account

### Option 2: Create New Account
1. **Click "Register here"**
2. **Fill in your own details**
3. **Use your real phone number** as login ID
4. **Complete registration**

## 📊 Demo Data Features

Each demo account includes:
- ✅ **Simulated Health Data** - Realistic vital signs
- ✅ **Historical Records** - Past 30 days of data
- ✅ **Emergency History** - Sample emergency events
- ✅ **Health Trends** - Charts and analytics
- ✅ **Location Data** - Nagpur, India coordinates

## 🎮 Testing Scenarios

### Scenario 1: Normal Health Monitoring
- **Login**: `1234567890` (John Doe)
- **View**: Normal vital signs, stable trends
- **Test**: Real-time dashboard updates

### Scenario 2: Emergency Alert Testing  
- **Login**: `9876543210` (Jane Smith)
- **Action**: Click red "EMERGENCY" button
- **Observe**: Alert notifications, emergency history

### Scenario 3: Senior Patient Monitoring
- **Login**: `5555551234` (Robert Johnson) 
- **View**: Age-specific health insights
- **Monitor**: Heart condition-related alerts

### Scenario 4: Female Health Tracking
- **Login**: `7777778888` (Maria Garcia)
- **Features**: Female-specific health insights
- **Monitor**: Asthma and allergy considerations

### Scenario 5: Young Adult Monitoring
- **Login**: `1111222233` (David Wilson)
- **View**: Baseline health monitoring
- **Test**: Activity and fitness tracking

## 🔧 Admin Testing

### System Testing Checklist:
```
□ User Registration - Create new account
□ User Login - Test with demo IDs  
□ Dashboard Display - All widgets working
□ Real-time Updates - WebSocket connection
□ Health Data - Charts and statistics
□ Emergency Alerts - Button and auto-detection
□ Location Services - Map functionality
□ Mobile Responsive - Test on phone/tablet
```

## 🎯 Development Testing

### API Testing with Demo Users:
```bash
# Test user lookup
curl http://localhost:3000/api/user/1234567890

# Test dashboard data
curl http://localhost:3000/api/dashboard/1234567890

# Test emergency trigger
curl -X POST http://localhost:3000/api/emergency \
  -H "Content-Type: application/json" \
  -d '{"userId":"1234567890","reason":"Test emergency"}'
```

## 📱 Mobile Testing

Test the responsive design on:
- **Desktop** - Full dashboard experience
- **Tablet** - Optimized layout
- **Mobile** - Touch-friendly interface

## 🚨 Emergency Testing

### Safe Emergency Testing:
1. **Use Demo Mode**: Test with demo accounts only
2. **Disable Real Alerts**: SMS/Email disabled by default
3. **Monitor Console**: Check server logs for responses
4. **Test Recovery**: Verify emergency resolution

## 🔒 Security Note

**Demo accounts are for testing only:**
- No real emergency services contacted
- SMS/Email alerts disabled by default  
- Data is simulated for demonstration
- Do not use for actual medical monitoring

## 🎬 Demo Script

### 5-Minute Demo Walkthrough:
1. **[0:00-1:00]** Login with `1234567890`
2. **[1:00-2:00]** Tour dashboard features
3. **[2:00-3:00]** View health charts and trends
4. **[3:00-4:00]** Test emergency button (safe mode)
5. **[4:00-5:00]** Show mobile responsive design

## 🔄 Resetting Demo Data

To reset demo accounts and generate fresh data:
```bash
# Stop server
Ctrl+C

# Clear database (optional)
# mongo rescuenet --eval "db.dropDatabase()"

# Restart server
npm start

# Demo data will regenerate automatically
```

---

**Ready to test your RescueNet AI system!** 🚑💖

Use these demo credentials to explore all features safely without real emergency alerts.
