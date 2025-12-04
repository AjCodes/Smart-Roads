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
#define ENABLE_WIFI true  // Set to true when power is stable

const char* ssid = "AJ";
const char* password = "#AJ787878";
String serverName = "http://10.210.43.197:5000/api/sensor-data";

// Static IP Configuration (optional)
#define USE_STATIC_IP false
IPAddress local_IP(192, 168, 1, 184);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

// Ultrasonic Sensors - ALL 4 LANES
// Note: Echo pins moved to Input-Only pins to free up Output pins for LEDs
const int LANE1_TRIG = 5;
const int LANE1_ECHO = 34; 
const int LANE2_TRIG = 19;
const int LANE2_ECHO = 35; 
const int LANE3_TRIG = 25;
const int LANE3_ECHO = 36; // (VP)
const int LANE4_TRIG = 32;
const int LANE4_ECHO = 39; // (VN)

// BME280 Sensor 
Adafruit_BME280 bme;
bool bmeFound = false;

// Traffic Light LEDs - ALL 4 LANES
// Lane 1
const int L1_R = 13;
const int L1_Y = 12;
const int L1_G = 14;
// Lane 2
const int L2_R = 15;
const int L2_Y = 2;
const int L2_G = 4;
// Lane 3 (New)
const int L3_R = 18;
const int L3_Y = 23;
const int L3_G = 26;
// Lane 4 (New)
const int L4_R = 33;
const int L4_Y = 27;
const int L4_G = 16; // RX2 pin

// Timing
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;  // Send data every 10 seconds

// ===========================================================
// FUNCTION DECLARATIONS
// ===========================================================
long readDistanceCM(int trigPin, int echoPin);
void initWiFi();
bool sendDataToBackend(long lane1, long lane2, long lane3, long lane4, float temp, float humidity, float pressure);
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
  Serial.println("   SMART ROADS - 4 LANE SYSTEM");
  Serial.println("========================================");
  Serial.println("⚠️  Brownout Detector: DISABLED");
  Serial.println("========================================\n");

  // Initialize ALL 4 sensor pins
  pinMode(LANE1_TRIG, OUTPUT);
  pinMode(LANE1_ECHO, INPUT);
  pinMode(LANE2_TRIG, OUTPUT);
  pinMode(LANE2_ECHO, INPUT);
  pinMode(LANE3_TRIG, OUTPUT);
  pinMode(LANE3_ECHO, INPUT);
  pinMode(LANE4_TRIG, OUTPUT);
  pinMode(LANE4_ECHO, INPUT);

  // Initialize LED pins - ALL 4 LANES
  // Lane 1
  pinMode(L1_R, OUTPUT);
  pinMode(L1_Y, OUTPUT);
  pinMode(L1_G, OUTPUT);
  // Lane 2
  pinMode(L2_R, OUTPUT);
  pinMode(L2_Y, OUTPUT);
  pinMode(L2_G, OUTPUT);
  // Lane 3
  pinMode(L3_R, OUTPUT);
  pinMode(L3_Y, OUTPUT);
  pinMode(L3_G, OUTPUT);
  // Lane 4
  pinMode(L4_R, OUTPUT);
  pinMode(L4_Y, OUTPUT);
  pinMode(L4_G, OUTPUT);

  // Set all lights to RED
  setAllRed();
  Serial.println("✅ ALL 4 ultrasonic sensors initialized");
  Serial.println("✅ ALL 4 Traffic Lights enabled");

  // 🚦 LED TEST SEQUENCE 🚦
  Serial.println("🚦 Testing LEDs...");
  Serial.println("   🔴 ALL RED ON");
  setAllRed();
  delay(1000);

  Serial.println("   🟡 ALL YELLOW ON");
  digitalWrite(L1_R, LOW); digitalWrite(L2_R, LOW); digitalWrite(L3_R, LOW); digitalWrite(L4_R, LOW);
  digitalWrite(L1_Y, HIGH); digitalWrite(L2_Y, HIGH); digitalWrite(L3_Y, HIGH); digitalWrite(L4_Y, HIGH);
  delay(1000);

  Serial.println("   🟢 ALL GREEN ON");
  digitalWrite(L1_Y, LOW); digitalWrite(L2_Y, LOW); digitalWrite(L3_Y, LOW); digitalWrite(L4_Y, LOW);
  digitalWrite(L1_G, HIGH); digitalWrite(L2_G, HIGH); digitalWrite(L3_G, HIGH); digitalWrite(L4_G, HIGH);
  delay(1000);

  // Reset to RED
  setAllRed();
  Serial.println("   ✅ LED Test Complete (Reset to RED)");

  // Initialize BME280
  Wire.begin(21, 22); // SDA=21, SCL=22
  delay(100);

  // 🔍 I2C SCANNER 🔍
  Serial.println("\nScanning I2C bus...");
  byte error, address;
  int nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.printf("   ✅ I2C device found at address 0x%02X\n", address);
      nDevices++;
    } else if (error == 4) {
      Serial.printf("   ❌ Unknown error at address 0x%02X\n", address);
    }
  }
  if (nDevices == 0) Serial.println("   ❌ No I2C devices found! Check wiring (SDA=21, SCL=22)\n");
  else Serial.println("   Scan complete.\n");
  
  if (bme.begin(0x76) || bme.begin(0x77)) {
    bmeFound = true;
    Serial.println("✅ BME280 sensor found!");
  } else {
    Serial.println("⚠️  BME280 not found (will send default values)");
  }

  delay(500);

  // Initialize WiFi (only if enabled)
  #if ENABLE_WIFI
    Serial.println("\n⚡ POWER STABILIZATION (5 seconds)...");
    Serial.println("   This delay prevents WiFi brownout");
    delay(5000);  // Extra-long wait before WiFi
    initWiFi();
    Serial.println("\n✅ System ready! Starting data transmission...\n");
  #else
    Serial.println("\n⚠️  WiFi DISABLED (USB power only mode)");
    Serial.println("   Use external 5V power + capacitor to enable WiFi");
    Serial.println("✅ System ready! Sensors working in test mode...\n");
  #endif
}

// ===========================================================
// MAIN LOOP
// ===========================================================
void loop() {
  if ((millis() - lastTime) > timerDelay) {

    // Read ALL 4 ultrasonic sensors
    long lane1 = readDistanceCM(LANE1_TRIG, LANE1_ECHO);
    delay(100);
    long lane2 = readDistanceCM(LANE2_TRIG, LANE2_ECHO);
    delay(100);
    long lane3 = readDistanceCM(LANE3_TRIG, LANE3_ECHO);
    delay(100);
    long lane4 = readDistanceCM(LANE4_TRIG, LANE4_ECHO);

    // Read BME280 data
    float temperature = 0.0;
    float humidity = 0.0;
    float pressure = 0.0;
    
    if (bmeFound) {
      temperature = bme.readTemperature();
      humidity = bme.readHumidity();
      pressure = bme.readPressure() / 100.0F; // Convert to hPa
    }

    // Print ALL 4 lanes sensor data
    Serial.println("=====================================");
    Serial.printf("📊 Lane 1: %ld cm %s\n", lane1, lane1 == 999 ? "(clear)" : "");
    Serial.printf("📊 Lane 2: %ld cm %s\n", lane2, lane2 == 999 ? "(clear)" : "");
    Serial.printf("📊 Lane 3: %ld cm %s\n", lane3, lane3 == 999 ? "(clear)" : "");
    Serial.printf("📊 Lane 4: %ld cm %s\n", lane4, lane4 == 999 ? "(clear)" : "");
    
    if (bmeFound) {
      Serial.printf("🌡️  Temperature: %.1f°C\n", temperature);
      Serial.printf("💧 Humidity: %.1f%%\n", humidity);
      Serial.printf("🔘 Pressure: %.1f hPa\n", pressure);
    }
    Serial.println("=====================================");

    // Send to backend if WiFi enabled and connected
    #if ENABLE_WIFI
      if (WiFi.status() == WL_CONNECTED) {
        sendDataToBackend(lane1, lane2, lane3, lane4, temperature, humidity, pressure);
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

// Initialize WiFi with POWER-OPTIMIZED settings
void initWiFi() {
  Serial.println("🔌 Initializing WiFi with POWER OPTIMIZATIONS...");
  
  // CRITICAL: Disconnect any previous connections
  WiFi.disconnect(true);
  delay(100);
  
  // Set to Station mode FIRST (before power settings)
  WiFi.mode(WIFI_STA);
  delay(100);
  
  // Set to ABSOLUTE MINIMUM power
  WiFi.setTxPower(WIFI_POWER_2dBm);
  Serial.println("   Power: 2dBm (minimum)");
  
  // Disable sleep to avoid reconnection issues
  WiFi.setSleep(false);
  
  // Enable persistent WiFi credentials (faster reconnect)
  WiFi.persistent(true);
  
  // Configure static IP if enabled (faster connection)
  #if USE_STATIC_IP
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
      Serial.println("⚠️  Static IP config failed, using DHCP");
    } else {
      Serial.println("   Using Static IP");
    }
  #endif
  
  delay(500);  // Allow power to stabilize
  
  Serial.printf("🔗 Connecting to: %s\n", ssid);
  WiFi.begin(ssid, password);
  
  // Progressive connection attempts with visual feedback
  int attempts = 0;
  int maxAttempts = 30;
  
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    delay(500);
    
    // Print progress every 5 attempts
    if (attempts % 5 == 0) {
      Serial.printf(" [%d/%d]", attempts, maxAttempts);
    } else {
      Serial.print(".");
    }
    
    attempts++;
    
    // If halfway through and still not connected, try reconnecting
    if (attempts == maxAttempts / 2) {
      Serial.println("\n   Retrying connection...");
      WiFi.disconnect();
      delay(100);
      WiFi.begin(ssid, password);
    }
  }
  
  Serial.println();  // New line after dots
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("✅ WiFi Connected Successfully!");
    Serial.printf("   IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("   Signal: %d dBm\n", WiFi.RSSI());
    Serial.printf("   Server: %s\n", serverName.c_str());
    Serial.println("   ⚡ Low power mode active");
  } else {
    Serial.println("❌ WiFi Connection FAILED!");
    Serial.println("   Possible causes:");
    Serial.println("   - Insufficient power (add capacitor)");
    Serial.println("   - Wrong password");
    Serial.println("   - Router too far (low TX power = short range)");
    Serial.println("   System will continue without WiFi...");
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

// Send data to backend - ALL 4 LANES
bool sendDataToBackend(long lane1, long lane2, long lane3, long lane4, float temp, float humidity, float pressure) {
  HTTPClient http;
  http.begin(serverName);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);

  // Create JSON payload with ALL 4 lanes
  StaticJsonDocument<512> doc;
  doc["lane1"] = lane1;
  doc["lane2"] = lane2;
  doc["lane3"] = lane3;  // REAL sensor data
  doc["lane4"] = lane4;  // REAL sensor data
  
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

    // Parse decision from response - Increased buffer size for safety
    StaticJsonDocument<2048> responseDoc;
    DeserializationError error = deserializeJson(responseDoc, response);

    if (!error) {
      if (responseDoc.containsKey("decision")) {
        const char* activeLane = responseDoc["decision"]["activeLane"];
        int duration = responseDoc["decision"]["duration"];

        Serial.printf("🚦 Traffic Decision: %s for %ds\n", activeLane, duration);
        
        // Control Traffic Lights
        controlTrafficLights(String(activeLane), duration);
      } else {
        Serial.println("⚠️ Response missing 'decision' field");
      }
    } else {
      Serial.print("❌ JSON Parsing Failed: ");
      Serial.println(error.c_str());
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
  // Lane 1
  digitalWrite(L1_R, HIGH);
  digitalWrite(L1_Y, LOW);
  digitalWrite(L1_G, LOW);
  // Lane 2
  digitalWrite(L2_R, HIGH);
  digitalWrite(L2_Y, LOW);
  digitalWrite(L2_G, LOW);
  // Lane 3
  digitalWrite(L3_R, HIGH);
  digitalWrite(L3_Y, LOW);
  digitalWrite(L3_G, LOW);
  // Lane 4
  digitalWrite(L4_R, HIGH);
  digitalWrite(L4_Y, LOW);
  digitalWrite(L4_G, LOW);
}

// Control traffic light sequence
void controlTrafficLights(String activeLane, int duration) {
  setAllRed();

  int rPin, yPin, gPin;

  if (activeLane == "lane1") {
    rPin = L1_R; yPin = L1_Y; gPin = L1_G;
  } else if (activeLane == "lane2") {
    rPin = L2_R; yPin = L2_Y; gPin = L2_G;
  } else if (activeLane == "lane3") {
    rPin = L3_R; yPin = L3_Y; gPin = L3_G;
  } else if (activeLane == "lane4") {
    rPin = L4_R; yPin = L4_Y; gPin = L4_G;
  } else {
    Serial.printf("⚠️ Unknown lane: %s\n", activeLane.c_str());
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