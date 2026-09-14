#include <Arduino.h>
#include <WiFi.h>

#include "secrets.h" 

const char* ssid = WIFI_SSID; 
const char* password = WIFI_PASSWORD; 

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // put your main code here, to run repeatedly:
  // digitalWrite(2, HIGH); // Turn the LED on
  // delay(500);                      // Wait for 0.5 seconds
  // digitalWrite(2, LOW);  // Turn the LED off
  // delay(500);  
}
