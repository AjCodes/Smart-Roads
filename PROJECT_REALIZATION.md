# Smart Roads - Project Realization Document

## Project Overview

**Project Name:** Smart Roads - Traffic Management System
**Date Completed:** December 2025
**Team Size:** 5 members

### Project Summary

Smart Roads is an intelligent traffic light management system that uses IoT sensors and cloud computing to optimize traffic flow at intersections. The system counts cars in each lane using ultrasonic sensors, uses a priority-based algorithm to decide which lane should receive a green light, and automatically controls traffic lights to reduce congestion.

---

## System Architecture

### 1. Hardware Layer (ESP32)

**Components:**
- ESP32 microcontroller
- 4x HC-SR04 ultrasonic sensors (one per lane)
- 12x LEDs (3 colors × 4 lanes for traffic lights)
- BME280 environmental sensor (optional)

**Functionality:**
- Detects and counts cars passing through each lane
- Maintains car count state for all 4 lanes
- Sends sensor data to backend API via WiFi
- Receives backend decisions and controls traffic lights accordingly
- Implements non-blocking state machine for smooth operation

### 2. Backend Layer (Node.js)

**Technology Stack:**
- Node.js with Express framework
- Firebase Realtime Database
- ES6 modules

**Key Components:**
- **Routes:** Define API endpoints for sensor data and decisions
- **Controllers:** Handle business logic for data processing and traffic decisions
- **Middleware:** Validates incoming sensor data
- **Firebase Config:** Manages database connection

**Decision Algorithm:**
1. Analyzes car counts from all 4 lanes
2. Filters lanes with waiting cars
3. Prioritizes by car count (most cars wins)
4. If tied, uses first-triggered timestamp (earliest wins)
5. Returns decision with active lane and duration

### 3. Dashboard Layer (Python Flask)

**Technology:**
- Python Flask web server
- HTML/CSS/JavaScript frontend
- Real-time data polling

**Features:**
- Live car count display for all lanes
- Traffic light status visualization
- Environmental data monitoring
- Color-coded traffic density indicators
- Auto-refresh every second

---

## Technical Implementation

### ESP32 Firmware

**Core Features:**
- Car detection using state machine (IDLE → CAR_PRESENT → IDLE)
- Non-blocking traffic light control with phases (GREEN → YELLOW → RED)
- WiFi connectivity with automatic reconnection
- JSON-based communication with backend
- Error handling for sensor timeouts and connection failures

**Key Functions:**
- `readDistance()` - Reads ultrasonic sensor data
- `updateLane()` - Updates car count based on sensor readings
- `startGreen()` - Initiates green light phase for specified lane
- `updateLights()` - Manages traffic light state transitions
- `sendData()` - Sends sensor data to backend and receives decisions

### Backend API

**Endpoints:**
```
POST /api/sensor-data      → Save sensor data & make traffic decision
GET  /api/sensor-data      → Retrieve sensor history
GET  /api/decision/latest  → Get latest traffic decision
GET  /health               → Server health check
```

**Data Flow:**
1. ESP32 sends sensor data via POST request
2. Backend validates data structure
3. Saves data to Firebase
4. Algorithm analyzes traffic and generates decision
5. Decision saved to database
6. Response sent back to ESP32 with decision
7. Dashboard polls for latest data

**Decision Logic:**
- Checks if traffic cycle is active (prevents overlapping decisions)
- Analyzes car counts and waiting times
- Calculates traffic density (light/moderate/heavy)
- Assigns 10-second green light duration
- Marks lane for counter reset after cycle completes

### Dashboard

**Frontend:**
- Responsive grid layout with 3 columns
- Real-time traffic light animation
- Environmental data bar at top
- Color-coded lane status cards
- JavaScript fetch API for data polling

**Backend:**
- Flask route serves main HTML page
- `/api/dashboard-data` endpoint aggregates sensor and decision data
- Error handling for backend connection failures

---

## Development Process

### Phase 1: Hardware Setup
- Wired ultrasonic sensors to ESP32
- Configured LED traffic lights
- Tested sensor accuracy and detection thresholds
- Implemented car counting logic

### Phase 2: Backend Development
- Set up Node.js server with Express
- Configured Firebase Realtime Database
- Implemented API endpoints
- Developed priority-based decision algorithm
- Created validation middleware

### Phase 3: ESP32 Integration
- Implemented WiFi connectivity
- Developed HTTP communication with backend
- Integrated decision parsing and light control
- Added error handling and reconnection logic

### Phase 4: Dashboard Creation
- Built Flask web server
- Designed responsive UI
- Implemented real-time data visualization
- Added auto-refresh functionality

### Phase 5: Testing & Refinement
- Created test scripts for API validation
- Developed ESP32 simulator for dashboard testing
- Tested with real hardware
- Fixed timing issues and state machine bugs

---

## Challenges & Solutions

### Challenge 1: Sensor Reliability
**Problem:** Ultrasonic sensors occasionally returned invalid readings
**Solution:** Implemented timeout handling and distance validation (0-999 cm range)

### Challenge 2: State Machine Conflicts
**Problem:** Multiple green light decisions overlapping
**Solution:** Added cycle tracking to prevent new decisions during active cycles

### Challenge 3: Car Counting Accuracy
**Problem:** Cars sometimes counted multiple times
**Solution:** Implemented state-based detection (enter → exit counting)

### Challenge 4: WiFi Stability
**Problem:** ESP32 occasionally disconnected from WiFi
**Solution:** Added reconnection logic and connection status checking

### Challenge 5: Dashboard Latency
**Problem:** Dashboard updates felt sluggish
**Solution:** Reduced polling interval to 1 second and optimized data fetching

---

## Key Features Achieved

1. **Automated Traffic Control**
   - System operates fully autonomously
   - Makes decisions every 2 seconds based on real-time data
   - Controls traffic lights without manual intervention

2. **Priority-Based Decision Making**
   - Prioritizes lanes with most waiting cars
   - Handles tie-breaking using wait time
   - Adapts to changing traffic conditions

3. **Real-time Monitoring**
   - Live dashboard shows current system state
   - Environmental data tracking
   - Historical data storage in Firebase

4. **Scalable Architecture**
   - Modular design with clear separation of concerns
   - RESTful API allows easy integration
   - Cloud database enables remote monitoring

5. **Robust Error Handling**
   - Validates all sensor inputs
   - Handles network failures gracefully
   - Automatic reconnection and recovery

---

## Testing Results

### Unit Testing
- API endpoints tested with `test-api.js`
- All endpoints returning correct response formats
- Validation middleware catching invalid data

### Integration Testing
- ESP32 simulator testing dashboard updates
- Real-time data flow verified end-to-end
- Traffic light control sequences validated

### Hardware Testing
- Tested with physical traffic model
- Verified sensor accuracy across different distances
- Confirmed LED control and timing

---

## Performance Metrics

- **Response Time:** < 200ms for API requests
- **Decision Making:** < 100ms for algorithm analysis
- **Sensor Reading:** Every 5ms (200 Hz)
- **Data Transmission:** Every 2 seconds
- **Dashboard Refresh:** 1 second interval
- **Traffic Cycle:** 10 seconds green + 3 seconds yellow

---

## Future Enhancements

1. **Enhanced Algorithms**
   - Adaptive timing based on historical data
   - Time-of-day traffic pattern adjustments
   - Multi-intersection coordination

2. **Additional Sensors**
   - Vehicle type detection (car vs truck)
   - Speed measurement
   - Pedestrian detection

3. **Enhanced Dashboard**
   - Historical charts and analytics
   - Traffic pattern visualization
   - Alert system for anomalies

4. **Mobile App**
   - Real-time traffic status for drivers
   - Estimated wait times
   - Navigation suggestions

5. **Energy Optimization**
   - Solar power for ESP32
   - LED efficiency improvements
   - Sleep modes during low traffic

---

## Technologies Used

**Hardware:**
- ESP32 DevKit
- HC-SR04 Ultrasonic Sensors
- LEDs (Red, Yellow, Green)
- BME280 Environmental Sensor
- Breadboard and jumper wires

**Software:**
- C++ (Arduino framework)
- JavaScript (Node.js, Express)
- Python (Flask)
- HTML/CSS/JavaScript

**Cloud Services:**
- Firebase Realtime Database
- Firebase Admin SDK

**Development Tools:**
- PlatformIO / Arduino IDE
- Visual Studio Code
- Git version control
- Postman (API testing)

---

## Team Contributions

**AJ - Backend & Database**
- Developed Node.js API server
- Configured Firebase database
- Implemented decision algorithm
- Created API documentation

**Junior & Ethan - Hardware & ESP32**
- Assembled hardware components and wired sensors/LEDs
- Developed ESP32 firmware
- Implemented sensor reading logic
- Configured WiFi and HTTP communication
- Debugged hardware issues and optimized sensor accuracy
- Tested hardware integration

**Julia - Project Management**
- Coordinated team activities
- Managed project timeline
- Organized testing sessions
- Documented requirements

**Elias - Dashboard Development**
- Built Flask web server
- Designed responsive UI
- Implemented real-time visualization
- Added environmental data display

---

## Lessons Learned

1. **State Machines are Essential**
   - Non-blocking state machines prevent timing conflicts
   - Clear state definitions improve debugging

2. **Validation is Critical**
   - Input validation prevents cascading errors
   - Type checking catches issues early

3. **Real-time Systems Need Buffer**
   - Cycle tracking prevents decision overlap
   - Proper timing between operations is crucial

4. **Testing Early Saves Time**
   - Simulators allow rapid iteration
   - Unit tests catch regressions

5. **Documentation Matters**
   - Clear API docs speed integration
   - Code comments help team collaboration

---

## Conclusion

The Smart Roads project successfully demonstrates an end-to-end IoT solution for intelligent traffic management. By combining hardware sensors, cloud computing, priority-based algorithms, and real-time visualization, we created a system that can actively reduce traffic congestion.

The project achieved all core objectives:
- Accurate car detection and counting
- Priority-based decision making
- Automated traffic light control
- Real-time monitoring and visualization
- Scalable cloud-based architecture

The modular design allows for easy expansion and enhancement, making this a solid foundation for more advanced traffic management systems. The project showcases practical applications of IoT, embedded systems, web development, and algorithms in solving real-world problems.

---

**Project Status:** Completed and Functional
**Demonstration Date:** December 7, 2025
**Final Grade:** Pending

---

*Document prepared by the Smart Roads Team*
