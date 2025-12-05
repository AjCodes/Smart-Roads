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
const int DETECTION_THRESHOLD[4] = {8, 5, 5, 5};

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
// Lane 1: R=13, Y=12, G=14
// Lane 2: R=15, Y=2,  G=4
// Lane 3: R=18, Y=23, G=26
// Lane 4: R=33, Y=27, G=16
const int LED_R[4] = {13, 15, 18, 33};
const int LED_Y[4] = {12, 2, 23, 27};
const int LED_G[4] = {14, 4, 26, 16};

// ===========================================================
// CAR COUNTING STATE MACHINE
// ===========================================================
enum DetectionState { IDLE, CAR_PRESENT };
DetectionState laneState[4] = {IDLE, IDLE, IDLE, IDLE};

int carCount[4] = {0, 0, 0, 0};
unsigned long firstTriggered[4] = {0, 0, 0, 0};

// ===========================================================
// TRAFFIC LIGHT STATE MACHINE (Non-blocking!)
// ===========================================================
enum LightPhase { PHASE_IDLE, PHASE_GREEN, PHASE_YELLOW };
LightPhase currentPhase = PHASE_IDLE;
int activeLane = -1;
int laneToReset = -1;
unsigned long phaseStartTime = 0;
int greenDuration = 0;
const int YELLOW_DURATION = 3000;

// Timing
unsigned long lastSendTime = 0;
unsigned long sendInterval = 2000;
unsigned long lastReadTime = 0;
unsigned long readInterval = 5;

// Forward declarations
long readDistanceCM(int laneIndex);
void updateLaneState(int laneIndex, long distance);
void initWiFi();
bool sendDataToBackend(float temp, float humidity, float pressure);
void setAllRed();
void updateTrafficLights();
void startGreenPhase(int lane, int duration);
void printStatus();
void testLEDs();

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n========================================");
  Serial.println("   SMART ROADS - CAR COUNTING MODE");
  Serial.println("========================================\n");

  // Initialize all pins
  for (int i = 0; i < 4; i++) {
    pinMode(TRIG_PINS[i], OUTPUT);
    pinMode(ECHO_PINS[i], INPUT);
    pinMode(LED_R[i], OUTPUT);
    pinMode(LED_Y[i], OUTPUT);
    pinMode(LED_G[i], OUTPUT);
  }

  // Test all LEDs at startup
  testLEDs();

  // Initialize BME280
  Wire.begin(21, 22);
  if (bme.begin(0x76) || bme.begin(0x77)) {
    bmeFound = true;
    Serial.println("BME280 found!");
  }

  #if ENABLE_WIFI
    delay(1000);
    initWiFi();
  #endif
}

void testLEDs() {
  Serial.println("Testing LEDs for all 4 lanes...");
  
  for (int i = 0; i < 4; i++) {
    Serial.printf("Lane %d: Testing R(pin%d) Y(pin%d) G(pin%d)\n", 
                  i+1, LED_R[i], LED_Y[i], LED_G[i]);
    
    // Test RED
    digitalWrite(LED_R[i], HIGH);
    delay(200);
    digitalWrite(LED_R[i], LOW);
    
    // Test YELLOW
    digitalWrite(LED_Y[i], HIGH);
    delay(200);
    digitalWrite(LED_Y[i], LOW);
    
    // Test GREEN
    digitalWrite(LED_G[i], HIGH);
    delay(200);
    digitalWrite(LED_G[i], LOW);
  }
  
  setAllRed();
  Serial.println("LED test complete - all RED now\n");
}

void loop() {
  // ALWAYS read sensors
  if ((millis() - lastReadTime) >= readInterval) {
    for (int i = 0; i < 4; i++) {
      long distance = readDistanceCM(i);
      updateLaneState(i, distance);
    }
    lastReadTime = millis();
  }

  // Update traffic lights (non-blocking)
  updateTrafficLights();

  // Send data periodically
  if ((millis() - lastSendTime) >= sendInterval) {
    float temperature = bmeFound ? bme.readTemperature() : 0;
    float humidity = bmeFound ? bme.readHumidity() : 0;
    float pressure = bmeFound ? bme.readPressure() / 100.0F : 0;

    printStatus();

    #if ENABLE_WIFI
      if (WiFi.status() == WL_CONNECTED) {
        sendDataToBackend(temperature, humidity, pressure);
      } else {
        Serial.println("WiFi disconnected, reconnecting...");
        WiFi.reconnect();
      }
    #endif

    lastSendTime = millis();
  }
}

void updateTrafficLights() {
  if (currentPhase == PHASE_IDLE) return;

  unsigned long elapsed = millis() - phaseStartTime;

  if (currentPhase == PHASE_GREEN) {
    if (elapsed >= greenDuration) {
      currentPhase = PHASE_YELLOW;
      phaseStartTime = millis();
      digitalWrite(LED_G[activeLane], LOW);
      digitalWrite(LED_Y[activeLane], HIGH);
      Serial.printf("YELLOW Lane %d for 3s\n", activeLane + 1);
    }
  }
  else if (currentPhase == PHASE_YELLOW) {
    if (elapsed >= YELLOW_DURATION) {
      currentPhase = PHASE_IDLE;
      digitalWrite(LED_Y[activeLane], LOW);
      digitalWrite(LED_R[activeLane], HIGH);
      Serial.printf("RED Lane %d - Cycle complete\n", activeLane + 1);
      
      // Reset the lane counter now
      if (laneToReset >= 0 && laneToReset < 4) {
        Serial.printf("Resetting Lane %d count (was %d)\n", laneToReset + 1, carCount[laneToReset]);
        carCount[laneToReset] = 0;
        firstTriggered[laneToReset] = 0;
        laneToReset = -1;
      }
      activeLane = -1;
    }
  }
}

void startGreenPhase(int lane, int duration) {
  if (lane < 0 || lane >= 4) {
    Serial.printf("Invalid lane: %d\n", lane);
    return;
  }
  
  if (currentPhase != PHASE_IDLE) {
    Serial.println("Already in cycle, ignoring");
    return;
  }

  Serial.printf(">>> Starting GREEN for Lane %d, duration %ds <<<\n", lane + 1, duration);

  activeLane = lane;
  greenDuration = (duration - 3) * 1000;
  currentPhase = PHASE_GREEN;
  phaseStartTime = millis();

  // Set all RED first
  setAllRed();
  
  // Then set the active lane to GREEN
  digitalWrite(LED_R[lane], LOW);
  digitalWrite(LED_G[lane], HIGH);
  
  Serial.printf("GREEN Lane %d for %ds (pin %d HIGH)\n", lane + 1, duration - 3, LED_G[lane]);
}

long readDistanceCM(int laneIndex) {
  digitalWrite(TRIG_PINS[laneIndex], LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PINS[laneIndex], HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PINS[laneIndex], LOW);

  long duration = pulseIn(ECHO_PINS[laneIndex], HIGH, 30000);
  if (duration == 0) return 999;
  return duration * 0.034 / 2;
}

void updateLaneState(int laneIndex, long distance) {
  int threshold = DETECTION_THRESHOLD[laneIndex];
  
  if (laneState[laneIndex] == IDLE && distance <= threshold && distance > 0) {
    laneState[laneIndex] = CAR_PRESENT;
    if (carCount[laneIndex] == 0 && firstTriggered[laneIndex] == 0) {
      firstTriggered[laneIndex] = millis();
    }
    Serial.printf("Car ENTERED Lane %d (dist: %ld cm)\n", laneIndex + 1, distance);
  } 
  else if (laneState[laneIndex] == CAR_PRESENT && distance > threshold) {
    laneState[laneIndex] = IDLE;
    carCount[laneIndex]++;
    Serial.printf("Car EXITED Lane %d - Count: %d\n", laneIndex + 1, carCount[laneIndex]);
  }
}

void printStatus() {
  Serial.println("\n--- CAR COUNTS ---");
  for (int i = 0; i < 4; i++) {
    Serial.printf("L%d: %d cars\n", i + 1, carCount[i]);
  }
  if (currentPhase != PHASE_IDLE) {
    Serial.printf("Active: Lane %d (%s)\n", activeLane + 1, 
                  currentPhase == PHASE_GREEN ? "GREEN" : "YELLOW");
  }
  Serial.println("------------------\n");
}

void initWiFi() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_2dBm);
  WiFi.setSleep(false);
  
  Serial.printf("Connecting to %s", ssid);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\nWiFi Failed");
  }
}

bool sendDataToBackend(float temp, float humidity, float pressure) {
  HTTPClient http;
  http.begin(serverName);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);

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

    StaticJsonDocument<2048> responseDoc;
    if (!deserializeJson(responseDoc, response)) {
      if (responseDoc.containsKey("decision")) {
        const char* activeLaneStr = responseDoc["decision"]["activeLane"];
        int duration = responseDoc["decision"]["duration"];
        
        if (strcmp(activeLaneStr, "none") != 0) {
          int laneNum = activeLaneStr[4] - '1';
          Serial.printf("Decision: %s for %ds\n", activeLaneStr, duration);
          startGreenPhase(laneNum, duration);
        }
      }
      
      if (responseDoc.containsKey("decision") && responseDoc["decision"].containsKey("resetLane")) {
        int resetLane = responseDoc["decision"]["resetLane"];
        if (resetLane >= 1 && resetLane <= 4) {
          laneToReset = resetLane - 1;
          Serial.printf("Lane %d marked for reset\n", resetLane);
        }
      }
    }
    http.end();
    return true;
  } else {
    Serial.printf("Backend error: %d\n", httpResponseCode);
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