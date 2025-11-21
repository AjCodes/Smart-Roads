#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// --- CONFIGURATION ---
const char* ssid = "AJ";
const char* password = "#AJ787878";

// BACKEND URL - REPLACE WITH YOUR PC'S IP ADDRESS
// Example: "http://192.168.1.15:5000/api/sensor-data"
String serverName = "http://192.168.1.15:5000/api/sensor-data"; 

// --- PIN DEFINITIONS ---
// Ultrasonic Sensors (Trig, Echo)
const int LANE1_TRIG = 5;
const int LANE1_ECHO = 18;
const int LANE2_TRIG = 19;
const int LANE2_ECHO = 21;
const int LANE3_TRIG = 22;
const int LANE3_ECHO = 23;
const int LANE4_TRIG = 25;
const int LANE4_ECHO = 26;

// LEDs (Red, Yellow, Green)
// Lane 1
const int L1_R = 13;
const int L1_Y = 12;
const int L1_G = 14;
// Lane 2
const int L2_R = 27;
const int L2_Y = 33;
const int L2_G = 32;
// Lane 3
const int L3_R = 15;
const int L3_Y = 2;
const int L3_G = 4;
// Lane 4
const int L4_R = 16;
const int L4_Y = 17;
const int L4_G = 3;

// --- VARIABLES ---
unsigned long lastTime = 0;
unsigned long timerDelay = 10000; // 10 seconds

void setup() {
  Serial.begin(115200);
  
  // Sensor Pins
  pinMode(LANE1_TRIG, OUTPUT); pinMode(LANE1_ECHO, INPUT);
  pinMode(LANE2_TRIG, OUTPUT); pinMode(LANE2_ECHO, INPUT);
  pinMode(LANE3_TRIG, OUTPUT); pinMode(LANE3_ECHO, INPUT);
  pinMode(LANE4_TRIG, OUTPUT); pinMode(LANE4_ECHO, INPUT);

  // LED Pins
  pinMode(L1_R, OUTPUT); pinMode(L1_Y, OUTPUT); pinMode(L1_G, OUTPUT);
  pinMode(L2_R, OUTPUT); pinMode(L2_Y, OUTPUT); pinMode(L2_G, OUTPUT);
  pinMode(L3_R, OUTPUT); pinMode(L3_Y, OUTPUT); pinMode(L3_G, OUTPUT);
  pinMode(L4_R, OUTPUT); pinMode(L4_Y, OUTPUT); pinMode(L4_G, OUTPUT);

  // Initialize LEDs (All Red)
  setAllRed();

  // WiFi Connection
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Send data every 10 seconds
  if ((millis() - lastTime) > timerDelay) {
    if(WiFi.status() == WL_CONNECTED) {
      
      // 1. Read Sensors
      long d1 = readDistance(LANE1_TRIG, LANE1_ECHO);
      long d2 = readDistance(LANE2_TRIG, LANE2_ECHO);
      long d3 = readDistance(LANE3_TRIG, LANE3_ECHO);
      long d4 = readDistance(LANE4_TRIG, LANE4_ECHO);

      Serial.printf("Sensors: L1=%ld, L2=%ld, L3=%ld, L4=%ld\n", d1, d2, d3, d4);

      // 2. Send Data to Backend
      HTTPClient http;
      http.begin(serverName);
      http.addHeader("Content-Type", "application/json");

      // Create JSON payload
      StaticJsonDocument<200> doc;
      doc["lane1"] = d1;
      doc["lane2"] = d2;
      doc["lane3"] = d3;
      doc["lane4"] = d4;
      
      String requestBody;
      serializeJson(doc, requestBody);

      int httpResponseCode = http.POST(requestBody);

      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Response Code: " + String(httpResponseCode));
        Serial.println("Response: " + response);

        // 3. Parse Decision
        StaticJsonDocument<1024> responseDoc;
        DeserializationError error = deserializeJson(responseDoc, response);

        if (!error) {
          const char* activeLane = responseDoc["decision"]["activeLane"];
          int duration = responseDoc["decision"]["duration"];
          
          Serial.printf("Active Lane: %s, Duration: %d\n", activeLane, duration);
          
          // 4. Control LEDs
          controlTrafficLights(activeLane, duration);
        } else {
          Serial.print("deserializeJson() failed: ");
          Serial.println(error.c_str());
        }
      } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
      }

      http.end();
    } else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}

// Helper: Read Ultrasonic Distance
long readDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH);
  long distance = duration * 0.034 / 2;
  
  if (distance > 400 || distance == 0) return 400; // Cap at 400cm
  return distance;
}

// Helper: Set all lanes to RED
void setAllRed() {
  digitalWrite(L1_R, HIGH); digitalWrite(L1_Y, LOW); digitalWrite(L1_G, LOW);
  digitalWrite(L2_R, HIGH); digitalWrite(L2_Y, LOW); digitalWrite(L2_G, LOW);
  digitalWrite(L3_R, HIGH); digitalWrite(L3_Y, LOW); digitalWrite(L3_G, LOW);
  digitalWrite(L4_R, HIGH); digitalWrite(L4_Y, LOW); digitalWrite(L4_G, LOW);
}

// Helper: Control Traffic Lights Sequence
void controlTrafficLights(String activeLane, int duration) {
  // Reset all to Red first
  setAllRed();
  
  // Determine which lane gets Green
  int rPin, yPin, gPin;
  
  if (activeLane == "lane1") { rPin = L1_R; yPin = L1_Y; gPin = L1_G; }
  else if (activeLane == "lane2") { rPin = L2_R; yPin = L2_Y; gPin = L2_G; }
  else if (activeLane == "lane3") { rPin = L3_R; yPin = L3_Y; gPin = L3_G; }
  else if (activeLane == "lane4") { rPin = L4_R; yPin = L4_Y; gPin = L4_G; }
  else return; // Unknown lane

  // Sequence: Red -> Red+Yellow (optional) -> Green -> Yellow -> Red
  
  // 1. Green ON (Red OFF)
  digitalWrite(rPin, LOW);
  digitalWrite(gPin, HIGH);
  
  // Wait for duration (minus yellow time)
  // Note: This blocks the loop, which is fine for this simple logic
  // For non-blocking, we'd need a state machine, but user asked for simple logic
  delay((duration - 3) * 1000); 
  
  // 2. Yellow ON (Green OFF)
  digitalWrite(gPin, LOW);
  digitalWrite(yPin, HIGH);
  delay(3000); // 3 seconds yellow
  
  // 3. Red ON (Yellow OFF)
  digitalWrite(yPin, LOW);
  digitalWrite(rPin, HIGH);
}
