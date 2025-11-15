# 🚦 Smart Roads - AI Traffic Management System

An intelligent traffic light system using Arduino sensors, AI decision-making, and real-time data to optimize traffic flow.

---

## 📋 Overview

Smart Roads uses **ultrasonic sensors** to detect traffic density, analyzes the data with **AI algorithms**, and automatically adjusts traffic light timing to reduce congestion.

**How it works:**
1. Sensors detect cars in each lane
2. Arduino sends data to backend API
3. AI analyzes and decides which lane gets green light
4. Arduino controls the traffic lights
5. Python dashboard visualizes real-time data

---

## 🛠️ Tech Stack

- **Hardware:** Arduino/ESP32 + Ultrasonic Sensors + LEDs
- **Backend:** Node.js + Express + Firebase
- **Database:** Firebase Realtime Database
- **Dashboard:** Python (Flask/Streamlit)
- **Deployment:** Railway / Render

---

## 📁 Project Structure

```
Smart-Roads/
│
├── ESP32/          # Arduino firmware (see ESP32/README.md)
├── backend/        # Node.js API (see backend/README.md)
├── dashboard/      # Python dashboard (see dashboard/README.md)
└── README.md       # This file
```

---

## 🚀 Quick Start

### Backend
```bash
cd backend
npm install
npm run dev
```
See [backend/README.md](backend/README.md) for details.

### Dashboard
```bash
cd dashboard
pip install -r requirements.txt
python app.py
```
See [dashboard/README.md](dashboard/README.md) for details.

### Arduino
Upload `ESP32/traffic_light.ino` using Arduino IDE.  
See [ESP32/README.md](ESP32/README.md) for details.

---

## 👥 Team

| Name | Role |
|------|------|
| AJ | Backend & Database |
| Junior | AI/ML Engineer |
| Ethan | Hardware (Arduino) |
| Julia | Project Management & Dashboard |
| Elias | Dashboard (Python) |

---

## 📊 Current Status

- ✅ Backend API (100%)
- ✅ AI Decision Engine (100%)
- ✅ Firebase Database (100%)
- 🚧 Arduino Integration (In Progress)
- 🚧 Python Dashboard (In Progress)

**Demo Date:** December 7, 2025

---

## 🔗 Live Demo

- **Backend API:** https://smart-roads-backend.railway.app (Coming soon)
- **Dashboard:** TBA

---

## 📚 Documentation

- **Backend:** [backend/README.md](backend/README.md)
- **Dashboard:** [dashboard/README.md](dashboard/README.md)
- **Arduino:** [ESP32/README.md](ESP32/README.md)

---

## 📄 License

MIT License - See LICENSE file for details.

---

**Built by the Smart Roads Team** 🚦
