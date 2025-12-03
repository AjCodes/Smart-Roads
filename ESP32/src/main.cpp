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
#define ENABLE_WIFI true  // Testing with LEDs disabled

const char* ssid = "AJ";
const char* password = "#AJ787878";
String serverName = "http://145.93.84.104:5000/api/sensor-data";

// Ultrasonic Sensors
const int LANE1_TRIG = 5;
const int LANE1_ECHO = 18;
const int LANE2_TRIG = 19;
const int LANE2_ECHO = 23;

// BME280 Sensor (I2C: SDA=21, SCL=22)
Adafruit_BME280 bme;
bool bmeFound = false;

// Traffic Light LEDs
const int L1_R = 13;
const int L1_Y = 12;
const int L1_G = 14;
const int L2_R = 15;
const int L2_Y = 2;
const int L2_G = 4;

// Timing
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;  // Send data every 5 seconds

// ===========================================================
// FUNCTION DECLARATIONS
// ===========================================================
long readDistanceCM(int trigPin, int echoPin);
void initWiFi();
bool sendDataToBackend(long lane1, long lane2, float temp, float humidity, float pressure);
void setAllRed();
void controlTrafficLights(String activeLane, int duration);

// ===========================================================
// SETUP
// ===========================================================
void setup() {
  // CRITICAL: Disable brownout detector FIRST
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  delay(2000);

  Serial.println("\n\n========================================");
  Serial.println("   SMART ROADS - FULL SYSTEM");
  Serial.println("========================================");
  Serial.println("⚠️  Brownout Detector: DISABLED");
  Serial.println("========================================\n");

  // Initialize sensor pins
  pinMode(LANE1_TRIG, OUTPUT);
  pinMode(LANE1_ECHO, INPUT);
  pinMode(LANE2_TRIG, OUTPUT);
  pinMode(LANE2_ECHO, INPUT);

  // Initialize LED pins (DISABLED FOR POWER TESTING)
  // pinMode(L1_R, OUTPUT);
  // pinMode(L1_Y, OUTPUT);
  // pinMode(L1_G, OUTPUT);
  // pinMode(L2_R, OUTPUT);
  // pinMode(L2_Y, OUTPUT);
  // pinMode(L2_G, OUTPUT);

  // Set all lights to RED
  // setAllRed();
  Serial.println("✅ Ultrasonic sensors initialized");
  Serial.println("⚠️  LEDs disabled (power testing mode)");

  // Initialize BME280
  Wire.begin(21, 22); // SDA=21, SCL=22
  delay(100);
  
  if (bme.begin(0x76) || bme.begin(0x77)) {
    bmeFound = true;
    Serial.println("✅ BME280 sensor found!");
  } else {
    Serial.println("⚠️  BME280 not found (will send default values)");
  }

  delay(500);

  // Initialize WiFi (only if enabled)
  #if ENABLE_WIFI
    delay(1000);  // Wait before WiFi
    initWiFi();
    Serial.println("\n✅ System ready! Starting data transmission...\n");
  #else
    Serial.println("\n⚠️  WiFi DISABLED (USB power only mode)");
    Serial.println("   Use external 5V power to enable WiFi");
    Serial.println("✅ System ready! Sensors working in test mode...\n");
  #endif
}

// ===========================================================
// MAIN LOOP
// ===========================================================
void loop() {
  if ((millis() - lastTime) > timerDelay) {

    // Read ultrasonic sensors
    long lane1 = readDistanceCM(LANE1_TRIG, LANE1_ECHO);
    delay(100);
    long lane2 = readDistanceCM(LANE2_TRIG, LANE2_ECHO);

    // Read BME280 data
    float temperature = 0.0;
    float humidity = 0.0;
    float pressure = 0.0;
    
    if (bmeFound) {
      temperature = bme.readTemperature();
      humidity = bme.readHumidity();
      pressure = bme.readPressure() / 100.0F; // Convert to hPa
    }

    // Print sensor data
    Serial.println("=====================================");
    Serial.printf("📊 Lane 1: %ld cm %s\n", lane1, lane1 == 999 ? "(clear)" : "");
    Serial.printf("📊 Lane 2: %ld cm %s\n", lane2, lane2 == 999 ? "(clear)" : "");
    
    if (bmeFound) {
      Serial.printf("🌡️  Temperature: %.1f°C\n", temperature);
      Serial.printf("💧 Humidity: %.1f%%\n", humidity);
      Serial.printf("🔘 Pressure: %.1f hPa\n", pressure);
    }
    Serial.println("=====================================");

    // Send to backend if WiFi enabled and connected
    #if ENABLE_WIFI
      if (WiFi.status() == WL_CONNECTED) {
        sendDataToBackend(lane1, lane2, temperature, humidity, pressure);
      } else {
        Serial.println("⚠️  WiFi disconnected! Reconnecting...");
        WiFi.reconnect();
      }
    #else
      Serial.println("   (WiFi disabled - no data sent to backend)\n");
    #endif

    lastTime = millis();
  }

  delay(10);  // Small delay
}

// ===========================================================
// HELPER FUNCTIONS
// ===========================================================

// Initialize WiFi with reduced power
void initWiFi() {
  Serial.println("Initializing WiFi with reduced power...");

  // Reduce WiFi power to prevent brownout
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

  delay(200);

  Serial.printf("Connecting to: %s\n", ssid);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connected!");
    Serial.print("   IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.printf("   Server: %s\n", serverName.c_str());
  } else {
    Serial.println("\n❌ WiFi failed!");
    Serial.println("   Use external 5V power if brownout occurs.");
  }
}

// Read ultrasonic distance
long readDistanceCM(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  long distance = duration * 0.034 / 2;

  if (duration == 0) {
    Serial.printf("⚠️ Sensor Timeout (Trig %d, Echo %d): Check wiring/power!\n", trigPin, echoPin);
    return 999;
  }

  if (distance > 400) {
    Serial.printf("⚠️ Sensor Out of Range (Trig %d): %ld cm\n", trigPin, distance);
    return 999;
  }
  
  return distance;
}

// Send data to backend
bool sendDataToBackend(long lane1, long lane2, float temp, float humidity, float pressure) {
  HTTPClient http;
  http.begin(serverName);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);

  // Create JSON payload
  StaticJsonDocument<512> doc;
  doc["lane1"] = lane1;
  doc["lane2"] = lane2;
  doc["lane3"] = 999;  // Simulated
  doc["lane4"] = 999;  // Simulated
  
  // Add environmental data
  if (bmeFound) {
    doc["temperature"] = temp;
    doc["humidity"] = humidity;
    doc["pressure"] = pressure;
  }

  String requestBody;
  serializeJson(doc, requestBody);

  Serial.println("📤 Sending to backend...");
  Serial.printf("   Data: %s\n", requestBody.c_str());

  int httpResponseCode = http.POST(requestBody);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.printf("✅ Response [%d]: ", httpResponseCode);
    Serial.println(response);

    // Parse decision from response
    StaticJsonDocument<512> responseDoc;
    DeserializationError error = deserializeJson(responseDoc, response);

    if (!error && responseDoc.containsKey("decision")) {
      const char* activeLane = responseDoc["decision"]["activeLane"];
      int duration = responseDoc["decision"]["duration"];

      Serial.printf("🚦 Traffic Decision: %s for %ds\n", activeLane, duration);
      // LEDs disabled for power testing - uncomment below to re-enable
      // controlTrafficLights(String(activeLane), duration);
    }

    http.end();
    return true;
  } else {
    Serial.printf("❌ HTTP Error: %d\n", httpResponseCode);
    http.end();
    return false;
  }
}

// Set all traffic lights to RED
void setAllRed() {
  digitalWrite(L1_R, HIGH);
  digitalWrite(L1_Y, LOW);
  digitalWrite(L1_G, LOW);
  digitalWrite(L2_R, HIGH);
  digitalWrite(L2_Y, LOW);
  digitalWrite(L2_G, LOW);
}

// Control traffic light sequence
void controlTrafficLights(String activeLane, int duration) {
  setAllRed();

  int rPin, yPin, gPin;

  if (activeLane == "lane1") {
    rPin = L1_R;
    yPin = L1_Y;
    gPin = L1_G;
  } else if (activeLane == "lane2") {
    rPin = L2_R;
    yPin = L2_Y;
    gPin = L2_G;
  } else {
    Serial.println("⚠️ Unknown lane");
    return;
  }

  // GREEN phase
  digitalWrite(rPin, LOW);
  digitalWrite(gPin, HIGH);
  Serial.printf("🟢 %s GREEN for %ds\n", activeLane.c_str(), duration - 3);
  delay((duration - 3) * 1000);

  // YELLOW phase
  digitalWrite(gPin, LOW);
  digitalWrite(yPin, HIGH);
  Serial.printf("🟡 %s YELLOW for 3s\n", activeLane.c_str());
  delay(3000);

  // Back to RED
  digitalWrite(yPin, LOW);
  digitalWrite(rPin, HIGH);
  Serial.printf("🔴 %s RED\n", activeLane.c_str());
}