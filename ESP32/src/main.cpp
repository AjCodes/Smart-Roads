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

// Traffic Light LEDs - ALL 4 LANES
const int L1_R = 13, L1_Y = 12, L1_G = 14;
const int L2_R = 15, L2_Y = 2, L2_G = 4;
const int L3_R = 18, L3_Y = 23, L3_G = 26;
const int L4_R = 33, L4_Y = 27, L4_G = 16;

// Timing
unsigned long lastTime = 0;
unsigned long timerDelay = 3000;

// Function declarations
long readDistanceCM(int trigPin, int echoPin);
void initWiFi();
bool sendDataToBackend(long lane1, long lane2, long lane3, long lane4, float temp, float humidity, float pressure);
void setAllRed();
void controlTrafficLights(String activeLane, int duration);

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n========================================");
  Serial.println("   SMART ROADS - 4 LANE SYSTEM");
  Serial.println("   Sensor Range: 0-18cm");
  Serial.println("========================================\n");

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

  // Initialize BME280
  Wire.begin(21, 22);
  if (bme.begin(0x76) || bme.begin(0x77)) {
    bmeFound = true;
    Serial.println("✅ BME280 sensor found!");
  } else {
    Serial.println("⚠️ BME280 not found");
  }

  #if ENABLE_WIFI
    delay(3000);
    initWiFi();
  #endif
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    long lane1 = readDistanceCM(LANE1_TRIG, LANE1_ECHO);
    delay(50);
    long lane2 = readDistanceCM(LANE2_TRIG, LANE2_ECHO);
    delay(50);
    long lane3 = readDistanceCM(LANE3_TRIG, LANE3_ECHO);
    delay(50);
    long lane4 = readDistanceCM(LANE4_TRIG, LANE4_ECHO);

    float temperature = bmeFound ? bme.readTemperature() : 0;
    float humidity = bmeFound ? bme.readHumidity() : 0;
    float pressure = bmeFound ? bme.readPressure() / 100.0F : 0;

    Serial.println("=====================================");
    Serial.printf("📊 Lane 1: %ld cm %s\n", lane1, lane1 == 999 ? "(clear)" : "");
    Serial.printf("📊 Lane 2: %ld cm %s\n", lane2, lane2 == 999 ? "(clear)" : "");
    Serial.printf("📊 Lane 3: %ld cm %s\n", lane3, lane3 == 999 ? "(clear)" : "");
    Serial.printf("📊 Lane 4: %ld cm %s\n", lane4, lane4 == 999 ? "(clear)" : "");
    Serial.println("=====================================");

    #if ENABLE_WIFI
      if (WiFi.status() == WL_CONNECTED) {
        sendDataToBackend(lane1, lane2, lane3, lane4, temperature, humidity, pressure);
      } else {
        WiFi.reconnect();
      }
    #endif

    lastTime = millis();
  }
  delay(10);
}

// Read ultrasonic distance (0-18cm range)
long readDistanceCM(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  long distance = duration * 0.034 / 2;

  if (duration == 0) {
    return 999;
  }

  // If distance > 18cm, treat as clear (no vehicle)
  if (distance > 18) {
    return 999;
  }

  return distance;
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
    Serial.printf("✅ Response [%d]\n", httpResponseCode);

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