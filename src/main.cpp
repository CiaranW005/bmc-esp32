#include <Arduino.h>

#include "connect_to_wifi.h"
#include "send_wal_packet.h"
#include "oracle_handler.h"
#include "button_event.h"

#include "secrets.h" 

const int OPTO_PIN = 17;

void setup() {
  Serial.begin(115200);

  pinMode(OPTO_PIN, OUTPUT);
  digitalWrite(OPTO_PIN, LOW);

  connect_wifi(WIFI_SSID, WIFI_PASSWORD);
  connect_to_oracle();
}

void loop() {
  handle_oracle();
  handleBootButton();
}

