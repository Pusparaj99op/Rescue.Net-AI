# RescueNet AI - Complete Circuit Connections

## Table of Contents
1. [Project Overview](#project-overview)
2. [Component List](#component-list)
3. [ESP32 Configuration](#esp32-configuration)
4. [Arduino Nano Alternative Configuration](#arduino-nano-alternative-configuration)
5. [Sensor Connections](#sensor-connections)
   - [MAX30102 Heart Rate & SpO2 Sensor](#max30102-heart-rate--spo2-sensor)
   - [MPU6050 Accelerometer](#mpu6050-accelerometer)
   - [DS18B20 Temperature Sensor](#ds18b20-temperature-sensor)
   - [SSD1306 OLED Display](#ssd1306-oled-display)
6. [SIM800L GSM Module Connection](#sim800l-gsm-module-connection)
7. [Additional Components](#additional-components)
   - [LEDs and Button](#leds-and-button)
   - [Buzzer](#buzzer)
8. [Power Management](#power-management)
9. [Detailed Pin Connection Table](#detailed-pin-connection-table)
10. [Common Issues & Troubleshooting](#common-issues--troubleshooting)

## Project Overview

RescueNet AI is an IoT-based emergency response system that monitors health vitals in real-time and automatically alerts emergency services when anomalies are detected. The system is built around an ESP32 microcontroller (with an Arduino Nano alternative option), connected to various sensors for health monitoring and communication modules.

## Component List

### Main Components
- **ESP32 Development Board** (Main controller)
- **MAX30102** Heart Rate & SpO2 Sensor
- **DS18B20** Temperature Sensor
- **MPU6050** Accelerometer/Gyroscope
- **SIM800L** GSM Module (for SMS alerts)
- **SSD1306 OLED Display** (128x64)
- **NEO-6M GPS Module** (optional)

### Additional Components
- Buzzer for emergency alerts
- Green LED for status indication
- Red LED for emergency indication
- Push button for manual emergency trigger
- Antenna for SIM800L
- 3.7V LiPo battery with charging circuit
- Various resistors (220Ω, 4.7kΩ, 10kΩ)
- Breadboard and jumper wires

## ESP32 Configuration

### ESP32 Pin Mapping

| ESP32 Pin | Connected To             | Function                   |
|-----------|--------------------------|----------------------------|
| 3.3V      | Sensors power            | 3.3V power supply          |
| GND       | Common ground            | Ground reference           |
| GPIO 0    | Emergency Button         | Manual emergency trigger   |
| GPIO 2    | Buzzer (+)               | Audio alert output         |
| GPIO 4    | DS18B20 Data             | Temperature sensor data    |
| GPIO 5    | Status LED (Green)       | System status indicator    |
| GPIO 14   | SIM800L RST              | GSM module reset control   |
| GPIO 15   | SIM800L PWR              | GSM module power control   |
| GPIO 16   | SIM800L RXD              | Serial data from GSM       |
| GPIO 17   | SIM800L TXD              | Serial data to GSM         |
| GPIO 18   | Emergency LED (Red)      | Emergency status indicator |
| GPIO 21   | I2C SDA                  | I2C Data (multiple devices)|
| GPIO 22   | I2C SCL                  | I2C Clock (multiple devices)|

## Arduino Nano Alternative Configuration

### Arduino Nano Pin Mapping

| Arduino Nano Pin | Connected To             | Function                   |
|------------------|--------------------------|----------------------------|
| 5V               | Sensors power            | 5V power supply            |
| 3.3V             | ESP8266 power            | 3.3V power supply          |
| GND              | Common ground            | Ground reference           |
| D2               | Emergency Button         | Manual emergency trigger   |
| D3               | DS18B20 Data             | Temperature sensor data    |
| D4               | Status LED (Green)       | System status indicator    |
| D5               | Buzzer (+)               | Audio alert output         |
| D6               | Emergency LED (Red)      | Emergency status indicator |
| D8 (RX)          | ESP8266 TX               | WiFi data from module      |
| D9 (TX)          | ESP8266 RX               | WiFi data to module        |
| A4 (SDA)         | I2C SDA                  | I2C Data (multiple devices)|
| A5 (SCL)         | I2C SCL                  | I2C Clock (multiple devices)|

## Sensor Connections

### MAX30102 Heart Rate & SpO2 Sensor

| MAX30102 Pin | ESP32 Connection | Arduino Nano Connection | Notes                |
|--------------|------------------|-------------------------|----------------------|
| VCC          | 3.3V             | 5V                      | Power supply         |
| GND          | GND              | GND                     | Common ground        |
| SDA          | GPIO 21          | A4                      | I2C Data line        |
| SCL          | GPIO 22          | A5                      | I2C Clock line       |
| INT          | Not connected    | Not connected           | Interrupt (optional) |

**Notes**: 
- The MAX30102 operates at 3.3V, but is generally tolerant of 5V on Arduino Nano.
- This sensor provides heart rate and blood oxygen level (SpO2) readings.
- Place finger gently on the sensor during readings.

### MPU6050 Accelerometer

| MPU6050 Pin | ESP32 Connection | Arduino Nano Connection | Notes                |
|-------------|------------------|-------------------------|----------------------|
| VCC         | 3.3V             | 5V                      | Power supply         |
| GND         | GND              | GND                     | Common ground        |
| SDA         | GPIO 21          | A4                      | I2C Data line        |
| SCL         | GPIO 22          | A5                      | I2C Clock line       |
| INT         | Not connected    | Not connected           | Interrupt (optional) |
| XDA         | Not connected    | Not connected           | Auxiliary I2C (unused)|
| XCL         | Not connected    | Not connected           | Auxiliary I2C (unused)|
| AD0         | GND              | GND                     | I2C Address select   |

**Notes**:
- Used for fall detection and movement analysis.
- The MPU6050 contains both an accelerometer and a gyroscope.
- Connect AD0 to GND to set I2C address to 0x68.

### DS18B20 Temperature Sensor

| DS18B20 Pin    | ESP32 Connection      | Arduino Nano Connection     | Notes                       |
|----------------|------------------------|----------------------------|----------------------------- |
| VCC (Pin 3)    | 3.3V                  | 5V                         | Power supply                 |
| GND (Pin 1)    | GND                   | GND                        | Common ground                |
| DATA (Pin 2)   | GPIO 4                | D3                         | 1-Wire data with 4.7kΩ pullup|

**Notes**:
- A 4.7kΩ pullup resistor is required between DATA and VCC.
- The waterproof version may have different wire colors: 
  - Red = VCC
  - Black = GND
  - Yellow/White = DATA
- Suitable for body temperature measurement.

### SSD1306 OLED Display

| SSD1306 Pin | ESP32 Connection | Arduino Nano Connection | Notes                |
|-------------|------------------|-------------------------|----------------------|
| VCC         | 3.3V             | 5V                      | Power supply         |
| GND         | GND              | GND                     | Common ground        |
| SDA         | GPIO 21          | A4                      | I2C Data line        |
| SCL         | GPIO 22          | A5                      | I2C Clock line       |
| RES         | Not connected    | Not connected           | Reset (optional)     |

**Notes**:
- The 128x64 OLED uses I2C communication.
- The display shows heart rate, temperature, and system status.
- Default I2C address is 0x3C.

## SIM800L GSM Module Connection

| SIM800L Pin | ESP32 Connection | Notes                                |
|-------------|------------------|--------------------------------------|
| VCC         | 3.7-4.2V         | **DO NOT CONNECT TO 5V!**            |
| GND         | GND              | Common ground                        |
| RXD         | GPIO 17          | Serial data from ESP32               |
| TXD         | GPIO 16          | Serial data to ESP32                 |
| RST         | GPIO 14          | Reset control                        |
| PWR         | GPIO 15          | Power key control                    |

**Important Power Notes**:
1. The SIM800L requires a dedicated 3.7V-4.2V power supply. **DO NOT connect to 5V** as it will permanently damage the module!
2. Use a separate power supply with a large capacitor (1000μF) to handle current spikes.
3. Connect a GSM antenna to the module for better network reception.
4. Insert an active SIM card with SMS capability.

## Additional Components

### LEDs and Button

#### Status LED (Green)
- **ESP32**: Connect anode (+) to GPIO 5 through a 220Ω resistor, cathode (-) to GND
- **Arduino Nano**: Connect anode (+) to D4 through a 220Ω resistor, cathode (-) to GND

#### Emergency LED (Red)
- **ESP32**: Connect anode (+) to GPIO 18 through a 220Ω resistor, cathode (-) to GND
- **Arduino Nano**: Connect anode (+) to D6 through a 220Ω resistor, cathode (-) to GND

#### Emergency Button
- **ESP32**: Connect one terminal to GPIO 0, the other to GND (uses internal pull-up)
- **Arduino Nano**: Connect one terminal to D2, the other to GND (uses internal pull-up)
- Alternatively, use a 10kΩ pull-up resistor from the pin to VCC

### Buzzer

- **ESP32**: Connect positive (+) to GPIO 2 through a 100Ω resistor, negative (-) to GND
- **Arduino Nano**: Connect positive (+) to D5 through a 100Ω resistor, negative (-) to GND

## Power Management

### ESP32 Power Options
1. Via USB connection (development)
2. Via 5V pin (regulated supply)
3. Via 3.7V LiPo battery with TP4056 charging module

### SIM800L Power Requirements
1. **Voltage**: 3.7V-4.2V (Li-Po battery ideal)
2. **Current**: Can spike to 2A during transmission
3. **Recommended setup**:
   - Li-Po battery (3.7V) → SIM800L VCC
   - 1000μF capacitor across VCC and GND
   - Keep wires short to minimize voltage drop

## Detailed Pin Connection Table

| ESP32 Pin | Component        | Component Pin | Function           | Wire Color |
|-----------|------------------|---------------|---------------------|------------|
| 3.3V      | MAX30102         | VCC           | Power Supply        | Red        |
| 3.3V      | MPU6050          | VCC           | Power Supply        | Red        |
| 3.3V      | SSD1306          | VCC           | Power Supply        | Red        |
| 3.3V      | DS18B20          | VCC (Pin 3)   | Power Supply        | Red        |
| 3.3V      | 4.7kΩ Resistor   | One End       | DS18B20 Pullup      | Red        |
| GND       | All Components   | GND           | Common Ground       | Black      |
| GPIO 0    | Emergency Button | Terminal 1    | Emergency Trigger   | White      |
| GPIO 2    | Buzzer           | Positive (+)  | Audio Alert         | Yellow     |
| GPIO 4    | DS18B20          | DATA (Pin 2)  | Temperature Data    | Blue       |
| GPIO 5    | Status LED       | Anode (+)     | System Status       | Green      |
| GPIO 14   | SIM800L          | RST           | Reset Control       | Yellow     |
| GPIO 15   | SIM800L          | PWR           | Power Control       | Orange     |
| GPIO 16   | SIM800L          | TXD           | Serial Data RX      | Blue       |
| GPIO 17   | SIM800L          | RXD           | Serial Data TX      | Green      |
| GPIO 18   | Emergency LED    | Anode (+)     | Emergency Alert     | Red        |
| GPIO 21   | All I2C Devices  | SDA           | I2C Data            | White      |
| GPIO 22   | All I2C Devices  | SCL           | I2C Clock           | Gray       |

## Common Issues & Troubleshooting

### Power Issues
- **SIM800L Not Powering Up**: Ensure voltage is between 3.7V-4.2V and current capacity is sufficient. Add a 1000μF capacitor.
- **ESP32 Resetting When SIM800L Transmits**: Separate power supplies needed; SIM800L draws high current spikes.

### Communication Issues
- **I2C Devices Not Responding**: Check if all devices share common ground. Verify correct addresses.
- **SIM800L Not Sending SMS**: Check network signal strength, SIM card status, and correct AT commands.

### Sensor Issues
- **DS18B20 Reading Errors**: Verify 4.7kΩ pullup resistor is connected between DATA and VCC.
- **MAX30102 Not Reading**: Ensure finger is placed correctly and that sensor is not in direct bright light.
- **MPU6050 Invalid Readings**: Calibrate the sensor at startup when device is at rest.

### Connection Best Practices
1. Use solid breadboard connections or solder joints for reliability.
2. Keep I2C lines short to minimize noise.
3. Add 4.7kΩ pullup resistors to I2C lines if communication is unstable.
4. Use 100nF decoupling capacitors near sensor VCC pins.
5. Separate digital and analog grounds if possible.

---

*Last Updated: June 18, 2025*

*RescueNet AI - Emergency Response System*
