# 🚦 ESP32 Firmware & Simulation

This folder contains the code for the physical traffic light system and a simulation tool for testing.

---

## 🧪 Simulation (Testing Tool)

**Note:** The `simulate_esp32.py` script is a **testing tool** used to verify backend logic without physical hardware.

**Run Simulation:**
```bash
python simulate_esp32.py
```
This script sends random sensor data to the backend to simulate traffic flow.

---

## 🛠️ Hardware Implementation

**⚠️ Status:** Real hardware implementation begins **Monday, Dec 1st**.

### 🔌 Pin Configuration

| Component | Pin (ESP32) | Description |
|-----------|-------------|-------------|
| **Lane 1** | Trig: 5, Echo: 18 | Ultrasonic Sensor |
| **Lane 2** | Trig: 19, Echo: 21 | Ultrasonic Sensor |
| **Lane 3** | Trig: 22, Echo: 23 | Ultrasonic Sensor |
| **Lane 4** | Trig: 25, Echo: 26 | Ultrasonic Sensor |
| **LEDs** | *Various* | Traffic Lights (Red/Yellow/Green) |

### 📥 Upload Firmware

1. Open `src/main.cpp` in **PlatformIO** (VS Code) or **Arduino IDE**.
2. Configure WiFi credentials in the code:
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```
3. Connect ESP32 via USB.
4. Click **Upload**.

---
