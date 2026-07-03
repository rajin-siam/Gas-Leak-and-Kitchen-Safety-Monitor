# Smart Gas Leak & Kitchen Safety Monitor

## Course
IoT and Robotics

## Project Overview

The **Smart Gas Leak & Kitchen Safety Monitor** is an IoT-based safety system built using an ESP32 microcontroller. The system continuously monitors gas concentration and kitchen temperature to detect potentially hazardous conditions such as gas leaks and overheating.

When dangerous conditions are detected, the system automatically performs safety actions including activating an alarm, closing the gas valve, turning on the exhaust fan, and notifying the user through the Blynk IoT platform.

The project is developed and tested using the Wokwi simulator, allowing the complete IoT workflow to be demonstrated without physical hardware.

---

## Objectives

- Detect gas leaks using a gas sensor.
- Monitor kitchen temperature and humidity.
- Automatically respond to dangerous conditions.
- Provide remote monitoring through Blynk Cloud.
- Display real-time information on an OLED display.
- Simulate an automated kitchen safety system.

---

## Technologies Used

- ESP32 DevKit
- Wokwi Simulator
- Blynk IoT Cloud
- Arduino Framework

---

## Hardware Components

| Component | Purpose |
|----------|---------|
| ESP32 DevKit | Main controller |
| Gas Sensor | Detects gas concentration |
| DHT22 | Measures temperature and humidity |
| Servo Motor | Simulates gas valve control |
| Relay Module | Controls exhaust fan |
| Active Buzzer | Alarm notification |
| RGB LED | System status indicator |
| SSD1306 OLED | Displays sensor readings and system status |

---

## Pin Configuration

| Component | ESP32 Pin |
|-----------|-----------|
| Gas Sensor (AOUT) | GPIO 34 |
| DHT22 Data | GPIO 4 |
| Servo PWM | GPIO 13 |
| Buzzer | GPIO 25 |
| RGB Red | GPIO 26 |
| RGB Green | GPIO 27 |
| RGB Blue | GPIO 14 |
| Relay IN | GPIO 32 |
| OLED SDA | GPIO 21 |
| OLED SCL | GPIO 22 |

---

## Safety Logic

### Safe State

Condition:

- Gas level ≤ 800
- Temperature ≤ 45°C

Actions:

- Green LED
- Fan OFF
- Valve OPEN
- Buzzer OFF

---

### Caution State

Condition:

- Gas level > 800

Actions:

- Exhaust fan ON
- Yellow LED
- Valve remains OPEN
- No siren

---

### Danger State

Condition:

- Gas level > 1500

Actions:

- Red LED
- Siren activated
- Gas valve CLOSED
- Exhaust fan ON
- Blynk notification sent

---

### Overheat State

Condition:

- Temperature > 45°C

Actions:

- Same response as gas leak
- Alarm activated
- Valve closed
- Exhaust fan ON
- Notification sent

---

## Blynk Datastreams

| Virtual Pin | Description |
|-------------|-------------|
| V0 | Gas Level |
| V1 | Temperature |
| V2 | Humidity |
| V3 | Gas Valve Control |
| V4 | Exhaust Fan Control |
| V5 | Alarm Reset |

---

## OLED Display

The OLED continuously displays:

- System Status
- Gas Level
- Temperature
- Humidity
- Valve Status
- Fan Status

---

## Alarm Behaviour

When a dangerous condition is detected:

1. Alarm siren starts.
2. RGB LED changes to red.
3. Gas valve closes.
4. Exhaust fan turns on.
5. Notification is sent through Blynk Cloud.

The alarm remains active until the user presses the **Reset** button in the Blynk dashboard.

---

## Simulation Instructions

1. Open the project in Wokwi.
2. Run the simulation.
3. Connect to the Blynk Cloud.
4. Adjust the gas sensor concentration.
5. Adjust the DHT22 temperature.
6. Observe:
   - OLED Display
   - RGB LED
   - Relay
   - Servo
   - Buzzer
   - Blynk Dashboard

---

## Project Workflow

```
Gas Sensor
        │
        ▼
Temperature Sensor
        │
        ▼
      ESP32
        │
 ┌──────┼─────────┐
 │      │         │
 ▼      ▼         ▼
OLED   Blynk   Safety Logic
                │
                ▼
      Servo • Relay • LED • Buzzer
```

---

## Features

- Real-time gas monitoring
- Temperature monitoring
- Humidity monitoring
- Automatic gas valve control
- Automatic ventilation control
- OLED status display
- RGB status indication
- Audible alarm
- Remote monitoring through Blynk
- Manual alarm reset
- Wokwi simulation support

---

## Libraries Used

- WiFi
- Blynk
- DHT Sensor Library
- ESP32Servo
- Wire
- Adafruit GFX
- Adafruit SSD1306

---

## Learning Outcomes

This project demonstrates:

- IoT device development
- ESP32 programming
- Cloud-based monitoring using Blynk
- Sensor integration
- Actuator control
- Embedded systems programming
- Safety automation concepts
- Real-time data acquisition
- Human-machine interface using OLED

---

## Author

**Md. Rajin Mashrur Siam**

Junior Software Engineer

Course Assignment — IoT and Robotics
