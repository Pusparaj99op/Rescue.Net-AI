// Health Analytics and Monitoring Utilities

class HealthAnalytics {
  
  // Analyze health trends over time
  static analyzeHealthTrends(healthData) {
    if (!healthData || healthData.length === 0) {
      return null;
    }
    
    const trends = {
      heartRate: this.calculateTrend(healthData, 'vitals.heartRate'),
      temperature: this.calculateTrend(healthData, 'vitals.temperature'),
      bloodPressure: this.calculateTrend(healthData, 'vitals.bloodPressure'),
      overallHealth: 'stable'
    };
    
    // Determine overall health trend
    const heartRateTrend = trends.heartRate.trend;
    const temperatureTrend = trends.temperature.trend;
    const bpTrend = trends.bloodPressure.trend;
    
    if (heartRateTrend === 'increasing' || temperatureTrend === 'increasing' || bpTrend === 'increasing') {
      trends.overallHealth = 'deteriorating';
    } else if (heartRateTrend === 'stable' && temperatureTrend === 'stable' && bpTrend === 'stable') {
      trends.overallHealth = 'stable';
    } else if (heartRateTrend === 'decreasing' && temperatureTrend === 'stable') {
      trends.overallHealth = 'improving';
    }
    
    return trends;
  }
  
  // Calculate trend for a specific vital sign
  static calculateTrend(data, property) {
    const values = data.map(d => this.getNestedProperty(d, property)).filter(v => v !== null && v !== undefined);
    
    if (values.length < 2) {
      return { trend: 'insufficient_data', slope: 0, values: values };
    }
    
    // Simple linear regression to determine trend
    const n = values.length;
    const x = Array.from({length: n}, (_, i) => i);
    const y = values;
    
    const sumX = x.reduce((a, b) => a + b, 0);
    const sumY = y.reduce((a, b) => a + b, 0);
    const sumXY = x.reduce((sum, xi, i) => sum + xi * y[i], 0);
    const sumXX = x.reduce((sum, xi) => sum + xi * xi, 0);
    
    const slope = (n * sumXY - sumX * sumY) / (n * sumXX - sumX * sumX);
    
    let trend = 'stable';
    if (slope > 0.1) trend = 'increasing';
    else if (slope < -0.1) trend = 'decreasing';
    
    return {
      trend,
      slope: slope.toFixed(3),
      average: (sumY / n).toFixed(2),
      latest: y[y.length - 1],
      values: values
    };
  }
  
  // Get nested property value (e.g., 'vitals.heartRate')
  static getNestedProperty(obj, path) {
    return path.split('.').reduce((current, key) => current && current[key], obj);
  }
  
  // Detect patterns in health data
  static detectPatterns(healthData) {
    if (!healthData || healthData.length < 10) {
      return [];
    }
    
    const patterns = [];
    
    // Check for daily patterns
    const hourlyData = this.groupByHour(healthData);
    if (hourlyData) {
      patterns.push(...this.findDailyPatterns(hourlyData));
    }
    
    // Check for weekly patterns
    const weeklyData = this.groupByDay(healthData);
    if (weeklyData) {
      patterns.push(...this.findWeeklyPatterns(weeklyData));
    }
    
    // Check for anomalies
    patterns.push(...this.findAnomalies(healthData));
    
    return patterns;
  }
  
  static groupByHour(data) {
    const groups = {};
    data.forEach(record => {
      const hour = new Date(record.timestamp).getHours();
      if (!groups[hour]) groups[hour] = [];
      groups[hour].push(record);
    });
    return groups;
  }
  
  static groupByDay(data) {
    const groups = {};
    data.forEach(record => {
      const day = new Date(record.timestamp).getDay();
      if (!groups[day]) groups[day] = [];
      groups[day].push(record);
    });
    return groups;
  }
  
  static findDailyPatterns(hourlyData) {
    const patterns = [];
    
    // Check for night vs day patterns
    let nightAvgHR = 0, dayAvgHR = 0, nightCount = 0, dayCount = 0;
    
    Object.keys(hourlyData).forEach(hour => {
      const hourInt = parseInt(hour);
      const avgHR = hourlyData[hour].reduce((sum, record) => 
        sum + (record.vitals?.heartRate || 0), 0) / hourlyData[hour].length;
      
      if (hourInt >= 22 || hourInt <= 6) {
        nightAvgHR += avgHR;
        nightCount++;
      } else {
        dayAvgHR += avgHR;
        dayCount++;
      }
    });
    
    if (nightCount > 0 && dayCount > 0) {
      nightAvgHR /= nightCount;
      dayAvgHR /= dayCount;
      
      if (Math.abs(nightAvgHR - dayAvgHR) > 10) {
        patterns.push({
          type: 'daily_rhythm',
          description: `Heart rate shows ${nightAvgHR < dayAvgHR ? 'normal' : 'abnormal'} day-night pattern`,
          significance: nightAvgHR < dayAvgHR ? 'normal' : 'concerning'
        });
      }
    }
    
    return patterns;
  }
  
  static findWeeklyPatterns(weeklyData) {
    const patterns = [];
    const days = ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'];
    
    // Analyze stress patterns (higher HR on certain days)
    const dayAverages = {};
    Object.keys(weeklyData).forEach(day => {
      const avgHR = weeklyData[day].reduce((sum, record) => 
        sum + (record.vitals?.heartRate || 0), 0) / weeklyData[day].length;
      dayAverages[day] = avgHR;
    });
    
    const maxDay = Object.keys(dayAverages).reduce((a, b) => dayAverages[a] > dayAverages[b] ? a : b);
    const minDay = Object.keys(dayAverages).reduce((a, b) => dayAverages[a] < dayAverages[b] ? a : b);
    
    if (dayAverages[maxDay] - dayAverages[minDay] > 15) {
      patterns.push({
        type: 'weekly_stress',
        description: `Higher stress levels detected on ${days[maxDay]}s`,
        significance: 'monitor'
      });
    }
    
    return patterns;
  }
  
  static findAnomalies(data) {
    const anomalies = [];
    
    // Find outliers using statistical methods
    const heartRates = data.map(d => d.vitals?.heartRate).filter(hr => hr);
    const temperatures = data.map(d => d.vitals?.temperature).filter(temp => temp);
    
    if (heartRates.length > 5) {
      const hrOutliers = this.findOutliers(heartRates);
      if (hrOutliers.length > 0) {
        anomalies.push({
          type: 'heart_rate_anomaly',
          description: `${hrOutliers.length} unusual heart rate readings detected`,
          values: hrOutliers,
          significance: 'investigate'
        });
      }
    }
    
    if (temperatures.length > 5) {
      const tempOutliers = this.findOutliers(temperatures);
      if (tempOutliers.length > 0) {
        anomalies.push({
          type: 'temperature_anomaly',
          description: `${tempOutliers.length} unusual temperature readings detected`,
          values: tempOutliers,
          significance: 'investigate'
        });
      }
    }
    
    return anomalies;
  }
  
  static findOutliers(data) {
    const sorted = [...data].sort((a, b) => a - b);
    const q1 = sorted[Math.floor(sorted.length * 0.25)];
    const q3 = sorted[Math.floor(sorted.length * 0.75)];
    const iqr = q3 - q1;
    const lowerBound = q1 - 1.5 * iqr;
    const upperBound = q3 + 1.5 * iqr;
    
    return data.filter(value => value < lowerBound || value > upperBound);
  }
  
  // Generate health insights
  static generateHealthInsights(user, healthData, trends, patterns) {
    const insights = [];
    
    // Age-specific insights
    if (user.age > 60) {
      insights.push({
        category: 'age_specific',
        message: 'As a senior, regular monitoring of blood pressure and heart rate is especially important.',
        action: 'Schedule regular check-ups with your healthcare provider.'
      });
    }
    
    // Gender-specific insights
    if (user.gender === 'F' && user.age >= 20 && user.age <= 50) {
      insights.push({
        category: 'reproductive_health',
        message: 'Tracking your menstrual cycle can provide additional health insights.',
        action: 'Consider logging your menstrual cycle dates for comprehensive health monitoring.'
      });
    }
    
    // Trend-based insights
    if (trends && trends.heartRate.trend === 'increasing') {
      insights.push({
        category: 'heart_health',
        message: 'Your heart rate has been trending upward. This could indicate increased stress or physical activity.',
        action: 'Monitor stress levels and consider consulting a healthcare provider if the trend continues.'
      });
    }
    
    // Pattern-based insights
    patterns.forEach(pattern => {
      if (pattern.significance === 'concerning') {
        insights.push({
          category: 'pattern_alert',
          message: pattern.description,
          action: 'Consider discussing this pattern with your healthcare provider.'
        });
      }
    });
    
    return insights;
  }
}

module.exports = HealthAnalytics;
