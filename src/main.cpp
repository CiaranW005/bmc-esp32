#include <Arduino.h>

#include "wifi/connect_to_wifi.h"
#include "board/network_controller.h"
#include "oracle/oracle_handler.h"
#include "board/button_event.h"

#include "config/secrets.h" 

const int OPTO_PIN = 17;

NetworkManagementController netManager;
OracleHandler handler(netManager);

void oracleTask(void* parameter) {
    handler.connect();

    while (true) {
        handler.loop();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void networkTask(void* parameter) {
    while (true) {
        if (!netManager.isConnected()) {
            netManager.connect();
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void setup() {
  Serial.begin(115200);

  connect_wifi(WIFI_SSID, WIFI_PASSWORD);

    xTaskCreate(
        oracleTask,
        "OracleTask",
        8192,
        nullptr,
        1,
        nullptr
    );

    xTaskCreate(
        networkTask,
        "NetworkTask",
        4096,
        nullptr,
        1,
        nullptr
    );
}

void loop() {
  handleBootButton();
}
