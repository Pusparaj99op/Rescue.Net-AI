# RescueNet AI - Hardware Connections and Pin Configuration

This document provides the wiring diagram and pin configuration for the RescueNet AI prototype.

## Components

- ESP32 (Main Controller)
- Arduino Nano (Sensor Processor)
- BMP180 (Pressure/Altitude Sensor)
- DS18B20 (Temperature Sensor)
- REES52 Pulse Sensor
- ADXL335 (Accelerometer)
- Neo-6M GPS Module
- 128x64 OLED Display (I2C)
- Micro SD Card Module
- Buzzer
- 2x Vibration Motors
- 2x 3.7V 1000mAh Batteries
- TP4056 Charging Circuit

## Pin Configuration Table

### ESP32 Pinout

| Pin      | Connection                  | Purpose                               |
|----------|-----------------------------|---------------------------------------|
| 3V3      | VCC of various sensors      | Power Supply                          |
| GND      | GND of various sensors      | Ground                                |
| GPIO 22  | Arduino Nano SCL            | I2C Clock for Nano communication      |
| GPIO 21  | Arduino Nano SDA            | I2C Data for Nano communication       |
| GPIO 17  | Neo-6M GPS TX               | GPS Data Input                        |
| GPIO 16  | Neo-6M GPS RX               | GPS Data Output                       |
| GPIO 5   | OLED SCL                    | I2C Clock for OLED Display            |
| GPIO 4   | OLED SDA                    | I2C Data for OLED Display             |
| GPIO 13  | Micro SD Card Module CLK    | SPI Clock for SD Card                 |
| GPIO 12  | Micro SD Card Module MISO   | SPI MISO for SD Card                  |
| GPIO 14  | Micro SD Card Module MOSI   | SPI MOSI for SD Card                  |
| GPIO 27  | Micro SD Card Module CS     | SPI Chip Select for SD Card           |
| GPIO 26  | Buzzer                      | Audio Alerts                          |
| GPIO 33  | Vibration Motor 1           | Haptic Feedback                       |
| GPIO 25  | Vibration Motor 2           | Haptic Feedback                       |
| GPIO 32  | Emergency Button            | Manual Emergency Trigger              |
| ADC1_CH6 (GPIO 34) | Battery Voltage Sensor | Battery Level Monitoring              |

### Arduino Nano Pinout

| Pin | Connection                | Purpose                          |
|-----|---------------------------|----------------------------------|
| 5V  | VCC of various sensors    | Power Supply                     |
| GND | GND of various sensors    | Ground                           |
| A4  | ESP32 SDA                 | I2C Data for ESP32 communication |
| A5  | ESP32 SCL                 | I2C Clock for ESP32 communication|
| A0  | REES52 Pulse Sensor       | Heart Rate                       |
| A1  | ADXL335 X-out             | Accelerometer X-axis             |
| A2  | ADXL335 Y-out             | Accelerometer Y-axis             |
| A3  | ADXL335 Z-out             | Accelerometer Z-axis             |
| D2  | DS18B20 Temperature Sensor| Temperature Data (OneWire)       |
| D3  | BMP180 SCL                | I2C Clock for BMP180             |
| D4  | BMP180 SDA                | I2C Data for BMP180              |


## Wiring Diagram Description

1.  **ESP32 and Arduino Nano:**
    *   Connect the I2C pins (SDA and SCL) of the ESP32 and Arduino Nano. Remember to connect the GNDs of both boards together.
    *   ESP32 `GPIO 22 (SDA)` -> Nano `A4 (SDA)`
    *   ESP32 `GPIO 21 (SCL)` -> Nano `A5 (SCL)`

2.  **Sensors to Arduino Nano:**
    *   **Pulse Sensor:** Connect the signal pin to Nano `A0`.
    *   **ADXL335:** Connect X, Y, Z outputs to Nano `A1`, `A2`, `A3`.
    *   **DS18B20:** Connect the data pin to Nano `D2` with a 4.7k pull-up resistor.
    *   **BMP180:** Connect to the I2C pins of the Nano.

3.  **GPS to ESP32:**
    *   Connect the Neo-6M's TX to ESP32's `GPIO 17` (RX) and RX to ESP32's `GPIO 16` (TX).

4.  **OLED Display to ESP32:**
    *   Connect the OLED's SDA and SCL to the ESP32's I2C pins (`GPIO 4` and `GPIO 5`).

5.  **SD Card Module to ESP32:**
    *   Connect the SD card module to the ESP32's SPI pins.

6.  **Actuators to ESP32:**
    *   Connect the buzzer to `GPIO 26`.
    *   Connect the vibration motors to `GPIO 33` and `GPIO 25` (consider using a transistor to drive them if they draw too much current).

7.  **Power:**
    *   The batteries are connected to the TP4056 charging circuit. The output of the charging circuit powers the ESP32 and Arduino Nano.
    *   A voltage divider can be used to connect the battery output to `GPIO 34` on the ESP32 to monitor battery voltage.

**Note:** Ensure all components share a common ground (GND).
