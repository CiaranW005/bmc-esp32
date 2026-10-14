#include "tasks/tasks.h"

#include <Arduino.h>

#include "network/manager.h"
#include "controllers/network_controller.h"
#include "oracle/oracle_handler.h"

#include "logger/logger.h"
#include "config/secrets.h"

namespace {
    bmc::NetworkManager networkManager(WIFI_SSID, WIFI_PASSWORD);
    NetworkManagementController controllerManager;
    OracleHandler handler(controllerManager);

    void networkTask(void* parameter) {
        auto* manager = static_cast<bmc::NetworkManager*>(parameter);

        manager->begin();

        while (true) {
            manager->update();

            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }

    void oracleTask(void* parameter) {
        while (!networkManager.isConnected()) {
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        uint32_t lastGen = networkManager.generation();
        handler.begin();

        while (true) {
            uint32_t currentGen = networkManager.generation();

            if (currentGen != lastGen) {
                lastGen = currentGen;

                handler.disconnect();

                if (networkManager.isConnected()) {
                    handler.begin();
                }
            }

            if (networkManager.isConnected()) {
                handler.loop();
            }

            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }


    void controllerTask(void* parameter) {
        while (!networkManager.isConnected()) {
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        uint32_t lastGen = networkManager.generation();
        controllerManager.connect();
        
        while (true) {
            uint32_t currentGen = networkManager.generation();

            if (currentGen != lastGen) {
                lastGen = currentGen;
                controllerManager.disconnect();
            }

            if (!networkManager.isConnected()) {
                vTaskDelay(pdMS_TO_TICKS(500));
                continue;
            }

            if (!controllerManager.isConnected()) {
                controllerManager.connect();
            }

            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

void startTasks() {
    Logger::begin(networkManager);

    xTaskCreate(networkTask, "NetworkTask", 4096, &networkManager, 2, nullptr);
    xTaskCreate(oracleTask, "OracleTask", 8192, nullptr, 1, nullptr);
    xTaskCreate(controllerTask, "ControllerTask", 4096, nullptr, 1, nullptr);
}