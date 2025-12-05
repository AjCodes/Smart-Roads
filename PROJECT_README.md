# 🚦 Smart Traffic Light System

A real-time intelligent traffic control system that uses ultrasonic sensors to detect traffic density and rule-based logic to optimize traffic light duration.

## 📂 Project Structure

```
Smart-Roads/
├── backend/                 # Node.js & Express Server
│   ├── controllers/         # Logic for sensors and decisions
│   │   ├── sensorController.js   # Handles sensor data & runs decision logic
│   │   └── decisionController.js # Helper for decision algorithm
│   ├── config/              # Firebase configuration
│   └── server.js            # Main entry point
├── ESP32/                   # Arduino Firmware
│   └── src/
│       └── main.cpp         # Main firmware logic
├── dashboard/               # Python Flask Dashboard
│   ├── templates/
│   │   └── index.html       # Dashboard UI
│   └── app.py               # Flask server
└── PROJECT_README.md        # This file
```

## 🔄 Data Flow

1.  **Sensing**: ESP32 reads 4 ultrasonic sensors (one for each lane).
2.  **Transmission**: ESP32 sends sensor data (JSON) via HTTP POST to the Backend (`/api/sensor-data`).
3.  **Processing**:
    *   Backend receives data.
    *   Saves raw data to Firebase.
    *   **Immediately** runs decision algorithm (`analyzeTrafficAndDecide`).
    *   Determines the "Active Lane" (shortest distance = most traffic) and "Duration".
    *   Saves decision to Firebase.
4.  **Response**: Backend sends the decision back to ESP32 in the same HTTP response.
5.  **Actuation**: ESP32 parses the response and controls the LEDs (Red/Yellow/Green) for the active lane.
6.  **Visualization**: The Python Dashboard polls the Backend APIs to show live status.

## 🛠️ Setup & Installation

### 1. Backend (Node.js)
*   Navigate to `backend/` folder.
*   Install dependencies: `npm install`
*   Start server: `npm start` (Runs on port 5000)

### 2. ESP32 Firmware
*   Open `ESP32/` in VS Code (PlatformIO) or copy `src/main.cpp` to Arduino IDE.
*   **CRITICAL CONFIGURATION**:
    *   Open `src/main.cpp`.
    *   Update `ssid` and `password` with your WiFi credentials.
    *   Update `serverName` with your PC's local IP address.
        *   **How to find IP**: Open Command Prompt and run `ipconfig`. Look for "IPv4 Address" (e.g., `192.168.1.15`).
        *   Example: `String serverName = "http://192.168.1.15:5000/api/sensor-data";`
*   Upload code to ESP32.

### 3. Dashboard (Python)
*   Navigate to `dashboard/` folder.
*   Install Flask: `pip install flask requests`
*   Start app: `python app.py`
*   Open browser: `http://localhost:5001`

## 🧠 Decision Logic
The system prioritizes the lane with the **most cars waiting**, using a simple rule-based algorithm.
*   **Heavy Traffic** (< 50cm): Green light for 45 seconds.
*   **Moderate Traffic** (< 100cm): Green light for 35 seconds.
*   **Light Traffic** (> 100cm): Green light for 30 seconds.

## 🔌 Pin Configuration (ESP32)

| Lane | Trig | Echo | Red | Yellow | Green |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **1** | 5 | 18 | 13 | 12 | 14 |
| **2** | 19 | 21 | 27 | 33 | 32 |
| **3** | 22 | 23 | 15 | 2 | 4 |
| **4** | 25 | 26 | 16 | 17 | 3 |

---
