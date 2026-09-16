#include <Arduino.h>

#include "connect_to_wifi.h"
#include "send_wal_packet.h"
#include "oracle_handler.h"
#include "button_event.h"

#include "secrets.h" 

void setup() {
  Serial.begin(115200);

  connect_wifi(WIFI_SSID, WIFI_PASSWORD);
  connect_to_oracle();
}

void loop() {
  handle_oracle();
  handleBootButton();
}
