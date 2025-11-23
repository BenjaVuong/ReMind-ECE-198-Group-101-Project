#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>

/* ============================================================
   CONFIGURATION
   ============================================================ */

SoftwareSerial espSerial(D2, D3);   // D2=RX, D3=TX
ESP8266WebServer server;

const char* ssid = "benjamin fone";
const char* password = "benjamogus";



// CHANGE THIS to your computer’s LAN IP running server.js
  // Ben's ip on ben hotspot:     http://172.20.10.2:3000/score
  // Ethan's ip on bens hotspot:  
const char* endpoint = "http://172.20.10.2:3000/score";
bool testScoreSent = false;

// For heartbeat
unsigned long lastHeartbeat = 0;

/* ============================================================
   SETUP
   ============================================================ */

void setup() {

  // ---------------- UART ----------------
  Serial.begin(9600);
  espSerial.begin(3200);

  pinMode(D2, INPUT);
  pinMode(D3, OUTPUT);

  Serial.println("\nBooting ESP…");


  // ---------------- WiFi ----------------
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n\nWiFi CONNECTED ✔");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n\nWiFi FAILED ✖");
  }


  // // Webserver route
  // server.on("/", []() {
  //   server.send(200, "text/plain", "ESP8266 is running");
  // });

  // server.begin();
  // Serial.println("Web server started.");
}

/* ============================================================
   SEND SCORE TO BACKEND
   ============================================================ */

void sendScore(int s) {
  Serial.print("Preparing to send score: ");
  Serial.println(s);

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ Cannot send — WiFi NOT connected");
    return;
  }

  Serial.println("WiFi is connected, sending POST...");

  WiFiClient client;
  HTTPClient http;

  http.begin(client, endpoint);
  http.addHeader("Content-Type", "application/json");

  String payload = String("{\"score\":") + s + "}";

  int code = http.POST(payload);

  Serial.print("HTTP response code: ");
  Serial.println(code);

  if (code > 0) {
    Serial.print("Server said: ");
    Serial.println(http.getString());
  } else {
    Serial.println("POST failed.");
  }

  http.end();
}

/* ============================================================
   MAIN LOOP
   ============================================================ */

void loop() {

  server.handleClient();

  // ---------------- Heartbeat ----------------
  if (millis() - lastHeartbeat > 5000) {
    lastHeartbeat = millis();

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("[HEARTBEAT] ESP alive — WiFi OK ✔");
    } else {
      Serial.println("[HEARTBEAT] ESP alive — WiFi LOST ✖");
    }
  }

  if (!testScoreSent && WiFi.status() == WL_CONNECTED) {
    Serial.println("Sending TEST score 8...");
    sendScore(8);
    testScoreSent = true;
  }

  // ---------------- Arduino → ESP scores ----------------
  if (espSerial.available() && (WiFi.status() == WL_CONNECTED) && ( millis()%30 == 0 ) ) {
    Serial.println("UART Connected");
    String msg = espSerial.readStringUntil('\0');
    msg.trim();

    if (msg.startsWith("Score:")) {
      int score = msg.substring(6).toInt();
      Serial.print("Received SCORE: ");
      Serial.println(score);
      sendScore(score);
    }
  }

}
