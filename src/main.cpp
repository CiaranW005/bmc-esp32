#include <Arduino.h>

#include "connect_to_wifi.h"
#include "send_wal_packet.h"

#include "secrets.h" 

#define BOOT_BUTTON 0

bool lastButtonState = HIGH;

void setup() {
  Serial.begin(115200); // baud rate, communicate over 115200 bits/s

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
}

void loop() {
  bool currentButtonState = digitalRead(BOOT_BUTTON);

  if (lastButtonState == HIGH && currentButtonState == LOW) {
    sendWakeOnLan(PC_MAC);
  }

  lastButtonState = currentButtonState;

  delay(20);
}
