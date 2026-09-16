#include <Arduino.h>

#include "connect_to_wifi.h"
#include "send_wal_packet.h"

#include "secrets.h" 

#define BOOT_BUTTON 0

bool lastButtonState = HIGH;

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
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
  bool currentButtonState = digitalRead(BOOT_BUTTON);

  if (lastButtonState == HIGH && currentButtonState == LOW) {
    sendWakeOnLan(PC_MAC);
  }

  lastButtonState = currentButtonState;

  delay(20);
}
