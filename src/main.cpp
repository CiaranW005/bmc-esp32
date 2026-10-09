#include <Arduino.h>

#include "wifi/connect_to_wifi.h"
#include "board/network_controller.h"
#include "oracle/oracle_handler.h"
#include "board/button_event.h"

#include "config/secrets.h" 

const int OPTO_PIN = 17;

NetworkManagementController netManager;
OracleHandler handler(netManager);

void setup() {
  Serial.begin(115200);

  pinMode(OPTO_PIN, OUTPUT);
  digitalWrite(OPTO_PIN, LOW);

  connect_wifi(WIFI_SSID, WIFI_PASSWORD);

  netManager.connect();
  handler.connect();
}

void loop() {
  handler.loop();
  handleBootButton();
}

