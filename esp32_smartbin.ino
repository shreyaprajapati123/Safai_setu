// Safai Setu smart bin: ESP32 + HC-SR04 ultrasonic sensor
// Sends the bin fill level to Firebase Firestore every 15 seconds.
// Board: "ESP32 Dev Module". No extra libraries needed.
//
// Wiring:  HC-SR04 VCC -> 5V (VIN), GND -> GND, TRIG -> GPIO 5, ECHO -> GPIO 18
//          (use a voltage divider on ECHO, or a 3.3V-tolerant sensor, to protect the ESP32)

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// ---- EDIT THESE ----
const char* WIFI_SSID   = "YOUR_WIFI_NAME";
const char* WIFI_PASS   = "YOUR_WIFI_PASSWORD";
const char* PROJECT_ID  = "YOUR_PROJECT_ID";
const char* API_KEY     = "YOUR_API_KEY";
const char* BIN_ID      = "BIN-LIVE";            // document name, must be unique per bin
const char* BIN_NAME    = "Demo Bin, College Gate";
const double BIN_LAT    = 28.6139;
const double BIN_LNG    = 77.2090;
const float  BIN_HEIGHT_CM = 60.0;               // distance from sensor to bottom of an empty bin
// --------------------

const int TRIG_PIN = 5;
const int ECHO_PIN = 18;

float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long us = pulseIn(ECHO_PIN, HIGH, 30000);      // 30 ms timeout
  if (us == 0) return -1;
  return us * 0.0343 / 2.0;
}

float readFillPercent() {
  // average 5 readings to smooth out noise
  float sum = 0; int n = 0;
  for (int i = 0; i < 5; i++) {
    float d = readDistanceCm();
    if (d > 0) { sum += d; n++; }
    delay(60);
  }
  if (n == 0) return -1;
  float d = sum / n;
  float fill = (BIN_HEIGHT_CM - d) / BIN_HEIGHT_CM * 100.0;
  if (fill < 0) fill = 0;
  if (fill > 100) fill = 100;
  return fill;
}

void sendToFirebase(float fill) {
  WiFiClientSecure client;
  client.setInsecure();                          // fine for a demo; use a root CA in production
  HTTPClient http;

  String url = String("https://firestore.googleapis.com/v1/projects/") + PROJECT_ID +
               "/databases/(default)/documents/bins/" + BIN_ID + "?key=" + API_KEY +
               "&updateMask.fieldPaths=name&updateMask.fieldPaths=lat" +
               "&updateMask.fieldPaths=lng&updateMask.fieldPaths=fill";

  String body = String("{\"fields\":{") +
                "\"name\":{\"stringValue\":\"" + BIN_NAME + "\"}," +
                "\"lat\":{\"doubleValue\":" + String(BIN_LAT, 6) + "}," +
                "\"lng\":{\"doubleValue\":" + String(BIN_LNG, 6) + "}," +
                "\"fill\":{\"doubleValue\":" + String(fill, 1) + "}" +
                "}}";

  if (http.begin(client, url)) {
    http.addHeader("Content-Type", "application/json");
    int code = http.sendRequest("PATCH", body);
    Serial.printf("Fill %.1f%% -> HTTP %d\n", fill, code);
    http.end();
  } else {
    Serial.println("HTTP begin failed");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nConnected");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) { WiFi.reconnect(); delay(3000); return; }
  float fill = readFillPercent();
  if (fill >= 0) sendToFirebase(fill);
  else Serial.println("Sensor read failed. Check wiring.");
  delay(15000);
}
