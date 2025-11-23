#include <SoftwareSerial.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

/* ------------------------------------------------------------------
                                SETUP
------------------------------------------------------------------- */ 
// UART Serial Object Setup
SoftwareSerial espSerial(D2, D3);   // D2 rx, D3 tx

// Webserver setup
ESP8266WebServer server;
const char* ssid = "benjamin fone";
const char* password = "benjamogus";

// Timing
unsigned long timeInterval = 0; 


void setup() {

// -------------------- UART CONNECTIONS ---------------------
  // Setup ESP_UART -> USB_UART Serial
  Serial.begin(9600);
  // Setup ESP_UART -> Arduino_UART Serial
  espSerial.begin(3200);
  pinMode(D2, INPUT);
  pinMode(D3, OUTPUT);

  // STATUS: ESP to Arduino UART is online.
  Serial.println("Listening...");

// ---------------- WEBSERVER CONNECTIONS --------------------
  // Connect to WiFi (probably hotspot)
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  // Display IP Address to Connect to
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.print( WiFi.localIP() );
  // Enable the webserver
  server.on(
    "/",
    [](){ server.send(200, "text/html", "<h1>ReMind Database</h1>"); }
  );
  server.begin();
}



/* ------------------------------------------------------------------
                            CONTINUOUS
------------------------------------------------------------------- */ 
void loop() {

  timeInterval = millis();
  server.handleClient();
  
  if ( (espSerial.available()) && (timeInterval%30 == 0) ) {
    // read incoming string
    String incomingString = espSerial.readStringUntil('\0');
    incomingString.trim();
    // print to serial (to check)
    Serial.print("You received: ");
    Serial.println( incomingString );
    // send to webserver
    server.send(200, "text/html", "<p>"+incomingString+"</p>");
  }

}