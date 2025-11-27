# 🚦 Smart Roads - AI Traffic Management System

[![Latest Release](https://img.shields.io/github/v/release/AjCodes/Smart-Roads?cache=none)](https://github.com/AjCodes/Smart-Roads/releases)
[![Backend](https://img.shields.io/badge/Backend-Complete-success)](https://github.com/AjCodes/Smart-Roads/tree/main/backend)
[![Demo Date](https://img.shields.io/badge/Demo-Dec%207%2C%202025-blue)](https://github.com/AjCodes/Smart-Roads)

An intelligent traffic light system using ESP32 microcontroller, AI decision-making, and real-time data to optimize traffic flow.

---

## 📋 Overview

Smart Roads uses **ultrasonic sensors** to detect traffic density, analyzes the data with **AI algorithms**, and automatically adjusts traffic light timing to reduce congestion.

**How it works:**
1. Sensors detect cars in each lane
2. ESP32 sends data to backend API
3. AI analyzes and decides which lane gets green light
4. ESP32 controls the traffic lights
5. Python dashboard visualizes real-time data

---

## 🛠️ Tech Stack

- **Hardware:** ESP32 + Ultrasonic Sensors + LEDs
- **Backend:** Node.js + Express + Firebase
- **Database:** Firebase Realtime Database
- **Dashboard:** Python (Flask/Streamlit)
- **Deployment:** Railway / Render

---

## 📁 Project Structure

```
Smart-Roads/
│
├── ESP32/          # ESP32 firmware (see ESP32/README.md)
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
python app.py
```
See [dashboard/README.md](dashboard/README.md) for details.

### ESP32
Upload `ESP32/traffic_light.ino` using ESP32 IDE. 
Simulation `python simulate_esp32.py`
See [ESP32/README.md](ESP32/README.md) for details.

---

## 👥 Team

| Name | Role |
|------|------|
| AJ | Backend & Database |
| Junior | AI/ML Engineer |
| Ethan | Hardware (ESP32) |
| Julia | Project Management & Dashboard |
| Elias | Dashboard (Python) |

---

## 📊 Current Status

- ✅ Backend API (100%)
- ✅ AI Decision Engine (100%)
- ✅ Firebase Database (100%)
- 🚧 ESP32 Integration (In Progress)
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
- **ESP32:** [ESP32/README.md](ESP32/README.md)

---

## 📄 License

MIT License - See [LICENSE](LICENSE) for details.

---

**Built by the Smart Roads Team** 🚦
