#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// ===========================================================
// CONFIGURATION
// ===========================================================
#define ENABLE_WIFI true

// Per-lane detection thresholds (cm)
const int DETECTION_THRESHOLD[4] = {8, 5, 5, 5};  // Lane1=8cm, Lane2-4=5cm

const char* ssid = "AJ";
const char* password = "#AJ787878";
String serverName = "http://10.210.43.197:5000/api/sensor-data";

// Ultrasonic Sensors - ALL 4 LANES
const int TRIG_PINS[4] = {5, 19, 25, 32};
const int ECHO_PINS[4] = {34, 35, 36, 39};

// BME280 Sensor
Adafruit_BME280 bme;
bool bmeFound = false;

// Traffic Light LEDs - ALL 4 LANES
const int LED_R[4] = {13, 15, 18, 33};
const int LED_Y[4] = {12, 2, 23, 27};
const int LED_G[4] = {14, 4, 26, 16};

// ===========================================================
// CAR COUNTING STATE MACHINE
// ===========================================================
enum DetectionState { IDLE, CAR_PRESENT };
DetectionState laneState[4] = {IDLE, IDLE, IDLE, IDLE};

// Car counts and timestamps per lane
int carCount[4] = {0, 0, 0, 0};
unsigned long firstTriggered[4] = {0, 0, 0, 0};

// ===========================================================
// TRAFFIC LIGHT STATE MACHINE (Non-blocking!)
// ===========================================================
enum LightPhase { PHASE_IDLE, PHASE_GREEN, PHASE_YELLOW, PHASE_RED };
LightPhase currentPhase = PHASE_IDLE;
int activeLane = -1;
int laneToReset = -1;  // Reset this lane AFTER cycle completes
unsigned long phaseStartTime = 0;
int greenDuration = 0;
const int YELLOW_DURATION = 3000;  // 3 seconds yellow

// Timing
unsigned long lastSendTime = 0;
unsigned long sendInterval = 2000;  // Send data every 2 seconds
unsigned long lastReadTime = 0;
unsigned long readInterval = 5;     // Read sensors every 5ms - ALWAYS!

// Function declarations
long readDistanceCM(int laneIndex);
void updateLaneState(int laneIndex, long distance);
void initWiFi();
bool sendDataToBackend(float temp, float humidity, float pressure);
void setAllRed();
void updateTrafficLights();  // Non-blocking traffic light update
void startGreenPhase(int lane, int duration);
void printStatus();

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n========================================");
  Serial.println("   SMART ROADS - CAR COUNTING MODE");
  Serial.println("   NON-BLOCKING LED CONTROL");
  Serial.println("   Thresholds: L1=8cm, L2-4=5cm");
  Serial.println("   Scan interval: 5ms (always scanning!)");
  Serial.println("========================================\n");

  // Initialize sensor pins
  for (int i = 0; i < 4; i++) {
    pinMode(TRIG_PINS[i], OUTPUT);
    pinMode(ECHO_PINS[i], INPUT);
    pinMode(LED_R[i], OUTPUT);
    pinMode(LED_Y[i], OUTPUT);
    pinMode(LED_G[i], OUTPUT);
  }

  setAllRed();
  Serial.println("✅ Sensors and LEDs initialized");

  // Initialize BME280
  Wire.begin(21, 22);
  if (bme.begin(0x76) || bme.begin(0x77)) {
    bmeFound = true;
    Serial.println("✅ BME280 sensor found!");
  } else {
    Serial.println("⚠️ BME280 not found");
  }

  #if ENABLE_WIFI
    delay(2000);
    initWiFi();
  #endif
}

void loop() {
  // ===== ALWAYS READ SENSORS - NEVER BLOCKED! =====
  if ((millis() - lastReadTime) >= readInterval) {
    for (int i = 0; i < 4; i++) {
      long distance = readDistanceCM(i);
      updateLaneState(i, distance);
    }
    lastReadTime = millis();
  }

  // ===== Update traffic lights (non-blocking) =====
  updateTrafficLights();

  // ===== Send data to backend periodically =====
  if ((millis() - lastSendTime) >= sendInterval) {
    // Only send if we're not mid-cycle or always send
    float temperature = bmeFound ? bme.readTemperature() : 0;
    float humidity = bmeFound ? bme.readHumidity() : 0;
    float pressure = bmeFound ? bme.readPressure() / 100.0F : 0;

    printStatus();

    #if ENABLE_WIFI
      if (WiFi.status() == WL_CONNECTED) {
        sendDataToBackend(temperature, humidity, pressure);
      } else {
        Serial.println("📶 WiFi: DISCONNECTED - Reconnecting...");
        WiFi.reconnect();
      }
    #endif

    lastSendTime = millis();
  }
}

// ===== NON-BLOCKING traffic light state machine =====
void updateTrafficLights() {
  if (currentPhase == PHASE_IDLE) {
    return;  // Nothing to do
  }

  unsigned long elapsed = millis() - phaseStartTime;

  if (currentPhase == PHASE_GREEN) {
    if (elapsed >= greenDuration) {
      // Switch to yellow
      currentPhase = PHASE_YELLOW;
      phaseStartTime = millis();
      digitalWrite(LED_G[activeLane], LOW);
      digitalWrite(LED_Y[activeLane], HIGH);
      Serial.printf("🟡 Lane %d YELLOW for 3s\n", activeLane + 1);
    }
  }
  else if (currentPhase == PHASE_YELLOW) {
    if (elapsed >= YELLOW_DURATION) {
      // Switch to red, cycle complete
      currentPhase = PHASE_IDLE;
      digitalWrite(LED_Y[activeLane], LOW);
      digitalWrite(LED_R[activeLane], HIGH);
      Serial.printf("🔴 Lane %d RED - Cycle complete\n", activeLane + 1);
      activeLane = -1;
      
      // Reset the winning lane counter after cycle completes
      if (laneToReset >= 0 && laneToReset < 4) {
        Serial.printf("🔄 Resetting Lane %d count (was %d)\n", laneToReset + 1, carCount[laneToReset]);
        carCount[laneToReset] = 0;
        firstTriggered[laneToReset] = 0;
        laneToReset = -1;
      }
    }
  }
}

// Start a new green phase (called from backend response)
void startGreenPhase(int lane, int duration) {
  if (lane < 0 || lane >= 4) return;
  
  // If already in a cycle, ignore new requests
  if (currentPhase != PHASE_IDLE) {
    Serial.println("⚠️ Already in traffic cycle, ignoring new request");
    return;
  }

  activeLane = lane;
  greenDuration = (duration - 3) * 1000;  // Convert to ms, subtract yellow time
  currentPhase = PHASE_GREEN;
  phaseStartTime = millis();

  setAllRed();
  digitalWrite(LED_R[lane], LOW);
  digitalWrite(LED_G[lane], HIGH);
  Serial.printf("🟢 Lane %d GREEN for %ds\n", lane + 1, duration - 3);
}

// Read ultrasonic distance for a lane
long readDistanceCM(int laneIndex) {
  digitalWrite(TRIG_PINS[laneIndex], LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PINS[laneIndex], HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PINS[laneIndex], LOW);

  long duration = pulseIn(ECHO_PINS[laneIndex], HIGH, 30000);
  
  if (duration == 0) {
    return 999;  // Timeout
  }

  return duration * 0.034 / 2;
}

// State machine: Update lane state and count cars
void updateLaneState(int laneIndex, long distance) {
  int threshold = DETECTION_THRESHOLD[laneIndex];
  
  if (laneState[laneIndex] == IDLE && distance <= threshold && distance > 0) {
    // Car just entered detection zone
    laneState[laneIndex] = CAR_PRESENT;
    
    // Record first triggered time if this is the first car
    if (carCount[laneIndex] == 0 && firstTriggered[laneIndex] == 0) {
      firstTriggered[laneIndex] = millis();
    }
    
    Serial.printf("🚗 Lane %d: Car ENTERED (dist: %ld cm)\n", laneIndex + 1, distance);
  } 
  else if (laneState[laneIndex] == CAR_PRESENT && distance > threshold) {
    // Car just left detection zone - COUNT IT!
    laneState[laneIndex] = IDLE;
    carCount[laneIndex]++;
    
    Serial.printf("✅ Lane %d: Car EXITED - Count: %d\n", laneIndex + 1, carCount[laneIndex]);
  }
}

// Print current status
void printStatus() {
  Serial.println("\n=====================================");
  Serial.println("        CAR COUNTS");
  Serial.println("=====================================");
  
  for (int i = 0; i < 4; i++) {
    const char* state = laneState[i] == CAR_PRESENT ? "🚗 CAR" : "✅ CLR";
    const char* traffic = carCount[i] >= 5 ? "HEAVY" : carCount[i] >= 3 ? "MOD" : carCount[i] > 0 ? "LIGHT" : "NONE";
    Serial.printf("L%d: %d cars [%s] %s\n", i + 1, carCount[i], traffic, state);
  }
  
  if (currentPhase != PHASE_IDLE) {
    Serial.printf("🚦 Active: Lane %d (%s)\n", activeLane + 1, 
                  currentPhase == PHASE_GREEN ? "GREEN" : "YELLOW");
  }
  Serial.println("=====================================\n");
}

void initWiFi() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_2dBm);
  WiFi.setSleep(false);
  
  Serial.printf("🔗 Connecting to: %s\n", ssid);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n✅ Connected! IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\n❌ WiFi Failed");
  }
}

bool sendDataToBackend(float temp, float humidity, float pressure) {
  HTTPClient http;
  http.begin(serverName);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);

  // Build JSON with nested lane objects
  StaticJsonDocument<1024> doc;
  
  for (int i = 0; i < 4; i++) {
    String laneName = "lane" + String(i + 1);
    doc[laneName]["carCount"] = carCount[i];
    doc[laneName]["firstTriggered"] = firstTriggered[i];
  }
  
  if (bmeFound) {
    doc["temperature"] = temp;
    doc["humidity"] = humidity;
    doc["pressure"] = pressure;
  }

  String requestBody;
  serializeJson(doc, requestBody);

  int httpResponseCode = http.POST(requestBody);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.printf("✅ Response [%d]\n", httpResponseCode);

    StaticJsonDocument<2048> responseDoc;
    if (!deserializeJson(responseDoc, response)) {
      // Check for decision
      if (responseDoc.containsKey("decision")) {
        const char* activeLaneStr = responseDoc["decision"]["activeLane"];
        int duration = responseDoc["decision"]["duration"];
        
        if (strcmp(activeLaneStr, "none") != 0) {
          int laneNum = activeLaneStr[4] - '1';  // "lane1" -> 0
          Serial.printf("🚦 Decision: %s for %ds\n", activeLaneStr, duration);
          startGreenPhase(laneNum, duration);  // Non-blocking!
        }
      }
      
      // Check for reset command
      if (responseDoc.containsKey("decision") && responseDoc["decision"].containsKey("resetLane")) {
        int resetLane = responseDoc["decision"]["resetLane"];
        if (resetLane >= 1 && resetLane <= 4) {
          laneToReset = resetLane - 1;  // Store for reset after cycle
          Serial.printf("� Lane %d marked for reset after cycle\n", resetLane);
        }
      }
    }
    http.end();
    return true;
  } else {
    Serial.printf("❌ Backend Failed! Error: %d\n", httpResponseCode);
  }
  http.end();
  return false;
}

void setAllRed() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(LED_R[i], HIGH);
    digitalWrite(LED_Y[i], LOW);
    digitalWrite(LED_G[i], LOW);
  }
}