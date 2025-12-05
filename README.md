# Smart Roads - Traffic Management System

An intelligent traffic light system using ESP32 and ultrasonic sensors to optimize traffic flow.

## Overview

Smart Roads detects cars using ultrasonic sensors, analyzes traffic patterns, and automatically adjusts traffic light timing to reduce congestion.

**How it works:**
1. Ultrasonic sensors count cars in each lane
2. ESP32 sends data to backend API
3. Backend algorithm decides which lane gets green light
4. ESP32 controls traffic lights based on decision
5. Dashboard displays real-time data

## Tech Stack

- **Hardware:** ESP32, HC-SR04 Ultrasonic Sensors, LEDs, BME280 (optional)
- **Backend:** Node.js, Express, Firebase Realtime Database
- **Dashboard:** Python (Flask)
- **Languages:** C++ (Arduino), JavaScript, Python

---

## Project Structure

```
Smart-Roads/
├── ESP32/          # ESP32 firmware
├── backend/        # Node.js API server
├── dashboard/      # Python Flask dashboard
└── README.md
```

## Quick Start

### Backend
```bash
cd backend
npm install
npm run dev
```

### Dashboard
```bash
cd dashboard
pip install -r requirements.txt
python app.py
```

### ESP32
Upload `ESP32/src/main.cpp` using PlatformIO or Arduino IDE.
Configure WiFi credentials and backend URL in the code.

---

## Features

- **Car Counting:** Ultrasonic sensors detect and count cars per lane
- **Priority Algorithm:** Prioritizes lanes with most traffic
- **Traffic Light Control:** Automated red/yellow/green light sequences
- **Real-time Dashboard:** Live visualization of traffic and decisions
- **Environmental Monitoring:** Optional BME280 sensor for temp/humidity/pressure
- **Firebase Integration:** Cloud storage for sensor data and decisions

## Team

- **AJ** - Backend & Database
- **Junior** - Hardware & ESP32
- **Ethan** - Hardware & ESP32
- **Julia** - Project Management
- **Elias** - Dashboard Development

## API Endpoints

- `POST /api/sensor-data` - ESP32 sends sensor readings
- `GET /api/sensor-data/latest` - Get latest sensor data
- `GET /api/decision/latest` - Get latest traffic decision
- `GET /health` - Health check

## License

MIT License
