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
#define ENABLE_DIAGNOSTICS true  // Set to false after debugging

const char* ssid = "AJ";
const char* password = "#AJ787878";
String serverName = "http://10.210.43.197:5000/api/sensor-data";

#define USE_STATIC_IP false
IPAddress local_IP(192, 168, 1, 184);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

// Ultrasonic Sensors - ALL 4 LANES
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
uint8_t bmeAddress = 0;  // Store found address

// Traffic Light LEDs - ALL 4 LANES
const int L1_R = 13, L1_Y = 12, L1_G = 14;
const int L2_R = 15, L2_Y = 2, L2_G = 4;
const int L3_R = 18, L3_Y = 23, L3_G = 26;
const int L4_R = 33, L4_Y = 27, L4_G = 16;

// Timing
unsigned long lastTime = 0;
unsigned long timerDelay = 3000;

// Diagnostic counters
int lane1_timeouts = 0, lane2_timeouts = 0, lane3_timeouts = 0, lane4_timeouts = 0;
int lane1_success = 0, lane2_success = 0, lane3_success = 0, lane4_success = 0;

// Function declarations
long readDistanceCM(int trigPin, int echoPin, const char* laneName);
void initWiFi();
bool sendDataToBackend(long lane1, long lane2, long lane3, long lane4, float temp, float humidity, float pressure);
void setAllRed();
void controlTrafficLights(String activeLane, int duration);
void scanI2C();
void runSensorDiagnostics();
void testSingleSensor(int trigPin, int echoPin, const char* name);
void diagnoseBME280();
void printLaneStatus(const char* name, long distance, int trigPin, int echoPin, int success, int timeouts);

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n========================================");
  Serial.println("   SMART ROADS - 4 LANE SYSTEM");
  Serial.println("   DIAGNOSTIC MODE ENABLED");
  Serial.println("========================================\n");
  Serial.println("Error Codes:");
  Serial.println("  -1 = TIMEOUT (Check WIRING!)");
  Serial.println("  -2 = ECHO stuck HIGH (Bad sensor/wiring)");
  Serial.println("  999 = Clear lane (>18cm, normal)");
  Serial.println("  0-18 = Vehicle detected\n");

  // Initialize sensor pins
  pinMode(LANE1_TRIG, OUTPUT); pinMode(LANE1_ECHO, INPUT);
  pinMode(LANE2_TRIG, OUTPUT); pinMode(LANE2_ECHO, INPUT);
  pinMode(LANE3_TRIG, OUTPUT); pinMode(LANE3_ECHO, INPUT);
  pinMode(LANE4_TRIG, OUTPUT); pinMode(LANE4_ECHO, INPUT);

  // Initialize LED pins
  pinMode(L1_R, OUTPUT); pinMode(L1_Y, OUTPUT); pinMode(L1_G, OUTPUT);
  pinMode(L2_R, OUTPUT); pinMode(L2_Y, OUTPUT); pinMode(L2_G, OUTPUT);
  pinMode(L3_R, OUTPUT); pinMode(L3_Y, OUTPUT); pinMode(L3_G, OUTPUT);
  pinMode(L4_R, OUTPUT); pinMode(L4_Y, OUTPUT); pinMode(L4_G, OUTPUT);

  setAllRed();
  Serial.println("✅ Sensors and LEDs initialized");

  // ===== I2C SCAN for BME280 =====
  Serial.println("\n--- I2C Bus Scan ---");
  Wire.begin(21, 22);
  scanI2C();

  // Initialize BME280 with better diagnostics
  Serial.println("\n--- BME280 Initialization ---");
  diagnoseBME280();

  // ===== Run initial sensor diagnostics =====
  #if ENABLE_DIAGNOSTICS
    Serial.println("\n--- Ultrasonic Sensor Diagnostics ---");
    runSensorDiagnostics();
  #endif

  #if ENABLE_WIFI
    delay(3000);
    initWiFi();
  #endif
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    long lane1 = readDistanceCM(LANE1_TRIG, LANE1_ECHO, "Lane1");
    delay(50);
    long lane2 = readDistanceCM(LANE2_TRIG, LANE2_ECHO, "Lane2");
    delay(50);
    long lane3 = readDistanceCM(LANE3_TRIG, LANE3_ECHO, "Lane3");
    delay(50);
    long lane4 = readDistanceCM(LANE4_TRIG, LANE4_ECHO, "Lane4");

    // Read BME280 with fallback values
    float temperature = 0;
    float humidity = 0;
    float pressure = 0;
    
    if (bmeFound) {
      temperature = bme.readTemperature();
      humidity = bme.readHumidity();
      pressure = bme.readPressure() / 100.0F;
      
      // Check for invalid readings (NaN or unrealistic values)
      if (isnan(temperature) || temperature < -40 || temperature > 85) {
        Serial.println("⚠️ BME280: Invalid temperature reading!");
        temperature = -999;  // Error code
      }
      if (isnan(humidity) || humidity < 0 || humidity > 100) {
        Serial.println("⚠️ BME280: Invalid humidity reading!");
        humidity = -999;
      }
      if (isnan(pressure) || pressure < 300 || pressure > 1100) {
        Serial.println("⚠️ BME280: Invalid pressure reading!");
        pressure = -999;
      }
    }

    Serial.println("\n=====================================");
    Serial.println("        SENSOR READINGS");
    Serial.println("=====================================");
    
    // Lane status with diagnostic info
    printLaneStatus("Lane 1", lane1, LANE1_TRIG, LANE1_ECHO, lane1_success, lane1_timeouts);
    printLaneStatus("Lane 2", lane2, LANE2_TRIG, LANE2_ECHO, lane2_success, lane2_timeouts);
    printLaneStatus("Lane 3", lane3, LANE3_TRIG, LANE3_ECHO, lane3_success, lane3_timeouts);
    printLaneStatus("Lane 4", lane4, LANE4_TRIG, LANE4_ECHO, lane4_success, lane4_timeouts);
    
    // BME280 readings
    Serial.println("-------------------------------------");
    if (bmeFound) {
      Serial.printf("🌡️ Temp: %.1f°C | 💧 Humidity: %.1f%% | 🌀 Pressure: %.1f hPa\n", 
                    temperature, humidity, pressure);
    } else {
      Serial.println("❌ BME280 not connected - Check wiring: SDA->21, SCL->22, VCC->3.3V, GND->GND");
    }
    Serial.println("=====================================");
    
    // WiFi Status
    #if ENABLE_WIFI
      if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("📶 WiFi: CONNECTED | IP: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("📤 Backend: %s\n", serverName.c_str());
      } else {
        Serial.println("📶 WiFi: ❌ DISCONNECTED - Reconnecting...");
      }
    #else
      Serial.println("📶 WiFi: DISABLED");
    #endif
    Serial.println("=====================================\n");

    // For backend, convert error codes to 999 to maintain compatibility
    long l1_send = (lane1 < 0) ? 999 : lane1;
    long l2_send = (lane2 < 0) ? 999 : lane2;
    long l3_send = (lane3 < 0) ? 999 : lane3;
    long l4_send = (lane4 < 0) ? 999 : lane4;

    #if ENABLE_WIFI
      if (WiFi.status() == WL_CONNECTED) {
        sendDataToBackend(l1_send, l2_send, l3_send, l4_send, temperature, humidity, pressure);
      } else {
        WiFi.reconnect();
      }
    #endif

    lastTime = millis();
  }
  delay(10);
}

// Print lane status with diagnostic information
void printLaneStatus(const char* name, long distance, int trigPin, int echoPin, int success, int timeouts) {
  if (distance == -1) {
    Serial.printf("❌ %s: TIMEOUT (TRIG:%d, ECHO:%d) - CHECK WIRING! [OK:%d, FAIL:%d]\n", 
                  name, trigPin, echoPin, success, timeouts);
  } else if (distance == -2) {
    Serial.printf("❌ %s: ECHO STUCK HIGH - Sensor/wiring issue [OK:%d, FAIL:%d]\n", 
                  name, success, timeouts);
  } else if (distance == 999) {
    Serial.printf("✅ %s: %ld cm (clear) [OK:%d, FAIL:%d]\n", name, distance, success, timeouts);
  } else {
    Serial.printf("🚗 %s: %ld cm (VEHICLE) [OK:%d, FAIL:%d]\n", name, distance, success, timeouts);
  }
}

// Read ultrasonic distance with detailed diagnostics
long readDistanceCM(int trigPin, int echoPin, const char* laneName) {
  // Check if echo is stuck HIGH before triggering
  if (digitalRead(echoPin) == HIGH) {
    if (strcmp(laneName, "Lane1") == 0) lane1_timeouts++;
    else if (strcmp(laneName, "Lane2") == 0) lane2_timeouts++;
    else if (strcmp(laneName, "Lane3") == 0) lane3_timeouts++;
    else if (strcmp(laneName, "Lane4") == 0) lane4_timeouts++;
    return -2;  // Echo stuck HIGH - bad sensor/wiring
  }

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  
  if (duration == 0) {
    // Timeout - no echo received
    if (strcmp(laneName, "Lane1") == 0) lane1_timeouts++;
    else if (strcmp(laneName, "Lane2") == 0) lane2_timeouts++;
    else if (strcmp(laneName, "Lane3") == 0) lane3_timeouts++;
    else if (strcmp(laneName, "Lane4") == 0) lane4_timeouts++;
    return -1;  // Timeout - wiring issue
  }

  // Success counter
  if (strcmp(laneName, "Lane1") == 0) lane1_success++;
  else if (strcmp(laneName, "Lane2") == 0) lane2_success++;
  else if (strcmp(laneName, "Lane3") == 0) lane3_success++;
  else if (strcmp(laneName, "Lane4") == 0) lane4_success++;

  long distance = duration * 0.034 / 2;

  // If distance > 5cm, treat as clear (no vehicle)
  if (distance > 5) {
    return 999;
  }

  return distance;
}

// Scan I2C bus for any connected devices
void scanI2C() {
  byte error, address;
  int nDevices = 0;
  
  Serial.println("Scanning I2C bus (SDA=21, SCL=22)...");
  
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.printf("✅ I2C device found at 0x%02X", address);
      if (address == 0x76 || address == 0x77) {
        Serial.print(" (BME280/BMP280)");
        bmeAddress = address;
      }
      Serial.println();
      nDevices++;
    }
  }
  
  if (nDevices == 0) {
    Serial.println("❌ No I2C devices found!");
    Serial.println("   Check wiring:");
    Serial.println("   - SDA -> GPIO 21");
    Serial.println("   - SCL -> GPIO 22");
    Serial.println("   - VCC -> 3.3V (NOT 5V!)");
    Serial.println("   - GND -> GND");
  } else {
    Serial.printf("Found %d I2C device(s)\n", nDevices);
  }
}

// Detailed BME280 diagnostics
void diagnoseBME280() {
  // Try address 0x76 first (default for most BME280)
  Serial.println("Trying BME280 at address 0x76...");
  if (bme.begin(0x76)) {
    bmeFound = true;
    bmeAddress = 0x76;
    Serial.println("✅ BME280 found at 0x76!");
  } else {
    // Try address 0x77 (alternate address)
    Serial.println("Trying BME280 at address 0x77...");
    if (bme.begin(0x77)) {
      bmeFound = true;
      bmeAddress = 0x77;
      Serial.println("✅ BME280 found at 0x77!");
    }
  }
  
  if (bmeFound) {
    // Read and display initial values
    float temp = bme.readTemperature();
    float hum = bme.readHumidity();
    float pres = bme.readPressure() / 100.0F;
    
    Serial.printf("Initial readings: Temp=%.1f°C, Humidity=%.1f%%, Pressure=%.1f hPa\n", temp, hum, pres);
    
    // Check for invalid readings
    if (isnan(temp) || isnan(hum) || isnan(pres)) {
      Serial.println("⚠️ Warning: Some readings are NaN - sensor may be damaged");
    }
    if (hum == 0 && pres == 0) {
      Serial.println("⚠️ Warning: Humidity and Pressure are 0 - might be BMP280 (no humidity)");
    }
  } else {
    Serial.println("❌ BME280 NOT found at either 0x76 or 0x77");
    Serial.println("   Possible causes:");
    Serial.println("   1. Wrong wiring - check SDA/SCL connections");
    Serial.println("   2. Wrong voltage - BME280 needs 3.3V");
    Serial.println("   3. Damaged sensor");
    Serial.println("   4. SDO pin not connected (try connecting SDO to GND for 0x76 or VCC for 0x77)");
  }
}

// Run diagnostics on all ultrasonic sensors
void runSensorDiagnostics() {
  Serial.println("Testing each ultrasonic sensor individually...\n");
  
  testSingleSensor(LANE1_TRIG, LANE1_ECHO, "Lane 1");
  delay(100);
  testSingleSensor(LANE2_TRIG, LANE2_ECHO, "Lane 2");
  delay(100);
  testSingleSensor(LANE3_TRIG, LANE3_ECHO, "Lane 3");
  delay(100);
  testSingleSensor(LANE4_TRIG, LANE4_ECHO, "Lane 4");
  
  Serial.println("\n--- Wiring Reference ---");
  Serial.println("Lane 1: TRIG=5,  ECHO=34");
  Serial.println("Lane 2: TRIG=19, ECHO=35");
  Serial.println("Lane 3: TRIG=25, ECHO=36 (VP)");
  Serial.println("Lane 4: TRIG=32, ECHO=39 (VN)");
  Serial.println("\n⚠️ Note: GPIO 34,35,36,39 are INPUT ONLY!");
  Serial.println("   Make sure ECHO pins go to these, not TRIG pins.\n");
}

// Test a single sensor with detailed output
void testSingleSensor(int trigPin, int echoPin, const char* name) {
  Serial.printf("Testing %s (TRIG=%d, ECHO=%d)... ", name, trigPin, echoPin);
  
  // Check ECHO pin state before triggering
  int echoState = digitalRead(echoPin);
  if (echoState == HIGH) {
    Serial.println("❌ FAIL - ECHO stuck HIGH before trigger!");
    Serial.println("   -> Check if ECHO is actually connected to TRIG by mistake");
    return;
  }
  
  // Send trigger pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Wait for echo
  long duration = pulseIn(echoPin, HIGH, 30000);
  
  if (duration == 0) {
    Serial.println("❌ FAIL - No echo received (TIMEOUT)");
    Serial.println("   -> Check: Is VCC connected? Is TRIG wired correctly?");
  } else {
    long distance = duration * 0.034 / 2;
    Serial.printf("✅ OK - Distance: %ld cm (raw duration: %ld µs)\n", distance, duration);
  }
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

bool sendDataToBackend(long lane1, long lane2, long lane3, long lane4, float temp, float humidity, float pressure) {
  HTTPClient http;
  http.begin(serverName);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);

  StaticJsonDocument<512> doc;
  doc["lane1"] = lane1;
  doc["lane2"] = lane2;
  doc["lane3"] = lane3;
  doc["lane4"] = lane4;
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
    Serial.printf("✅ Data Sent! Response [%d] - Saved to Firebase\n", httpResponseCode);

    StaticJsonDocument<2048> responseDoc;
    if (!deserializeJson(responseDoc, response)) {
      if (responseDoc.containsKey("decision")) {
        const char* activeLane = responseDoc["decision"]["activeLane"];
        int duration = responseDoc["decision"]["duration"];
        Serial.printf("🚦 Decision: %s for %ds\n", activeLane, duration);
        controlTrafficLights(String(activeLane), duration);
      }
    }
    http.end();
    return true;
  }
  http.end();
  Serial.printf("❌ Backend Failed! Error code: %d\n", httpResponseCode);
  return false;
}

void setAllRed() {
  digitalWrite(L1_R, HIGH); digitalWrite(L1_Y, LOW); digitalWrite(L1_G, LOW);
  digitalWrite(L2_R, HIGH); digitalWrite(L2_Y, LOW); digitalWrite(L2_G, LOW);
  digitalWrite(L3_R, HIGH); digitalWrite(L3_Y, LOW); digitalWrite(L3_G, LOW);
  digitalWrite(L4_R, HIGH); digitalWrite(L4_Y, LOW); digitalWrite(L4_G, LOW);
}

void controlTrafficLights(String activeLane, int duration) {
  setAllRed();
  int rPin, yPin, gPin;

  if (activeLane == "lane1") { rPin = L1_R; yPin = L1_Y; gPin = L1_G; }
  else if (activeLane == "lane2") { rPin = L2_R; yPin = L2_Y; gPin = L2_G; }
  else if (activeLane == "lane3") { rPin = L3_R; yPin = L3_Y; gPin = L3_G; }
  else if (activeLane == "lane4") { rPin = L4_R; yPin = L4_Y; gPin = L4_G; }
  else return;

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