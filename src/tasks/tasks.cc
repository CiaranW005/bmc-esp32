#include "tasks/tasks.h"

#include <Arduino.h>

#include "network/manager.h"
#include "controllers/network_controller.h"
#include "oracle/oracle_handler.h"

#include "logger/logger.h"
#include "config/secrets.h"

// The network task monitors connectivity, the Oracle task services WebSocket
// traffic, and the controller task executes PC commands.
// Commands and replies pass between the Oracle and controller tasks via
// mutex-protected state in OracleHandler.
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
        // An interface change invalidates existing connections, even if the network remains available. 
        uint32_t lastGen = networkManager.generation();
        handler.begin();

        while (true) {
            uint32_t currentGen = networkManager.generation();

            // Recreate the Oracle connection when the selected network interface changes.
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
            
            handler.processCommand();
            if (auto request = handler.takeTelemetryRequest()) {
                ManagementResult started = controllerManager.sendCommand(
                    MessageType::Telemetry,
                    1500,
                    [request = *request](const ResponseResult& result) {
                        // Setup/send failures are handled by the immediate return below.
                        if (result.err == ResponseError::Canceled) {
                            return;
                        }

                        handler.telemetryCompleted(request, result);
                    }
                );

                if (started != ManagementResult::Success) {
                    handler.telemetryCompleted(
                        *request,
                        {std::nullopt, ResponseError::DeliveryFailed}
                    );
                }
            }

            if (networkManager.isConnected()) {
                if (!controllerManager.isConnected()) {
                    controllerManager.connect();
                } else {
                    controllerManager.update();
                }
            }

            controllerManager.tick();
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void startTasks() {
    Logger::begin(networkManager);

    controllerManager.setCompletionHandler(
        [](MessageType type, const ResponseResult& result) {
            handler.commandCompleted(type, result);
        }
    );

    xTaskCreate(networkTask, "NetworkTask", 4096, &networkManager, 2, nullptr);
    xTaskCreate(oracleTask, "OracleTask", 8192, nullptr, 1, nullptr);
    xTaskCreate(controllerTask, "ControllerTask", 4096, nullptr, 1, nullptr);
}
