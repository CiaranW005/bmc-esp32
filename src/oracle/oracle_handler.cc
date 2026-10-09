#include "oracle/oracle_handler.h"
#include "config/secrets.h"
#include "config/config.h"
#include "config/device_credentials.h"
#include "board/management_controller.h"

#include  <WiFi.h>
#include <WebSocketsClient.h>

OracleHandler::OracleHandler(ManagementController& manager): manager_(manager) {};

void OracleHandler::connect() {
    webSocket_.beginSslWithClientKey(
        ORACLE_HOST,
        ORACLE_PORT,
        "/device",
        BMC_ROOT_CA,
        CLIENT_CERT,
        CLIENT_KEY
    );

    String headers =
        "Origin: https://device.corasfein.com:443\r\n"
        "Authorization: Bearer " + String(DEVICE_TOKEN) + "\r\n";

    webSocket_.setExtraHeaders(headers.c_str());

    webSocket_.onEvent(
        [this](WStype_t type, uint8_t* payload, size_t length) {
            this->handleEvent(type, payload, length);
        }
    );
    
    webSocket_.setReconnectInterval(5000);
}

void OracleHandler::loop() {
    webSocket_.loop();
}

bool OracleHandler::isConnected() {
    return webSocket_.isConnected();
}

bool OracleHandler::send(const String& message) {
    if (!this->isConnected()) {
        return false;
    }

    return webSocket_.sendTXT(message.c_str());
}


void OracleHandler::handleEvent(WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED:
            Serial.println("Connected to Oracle");
            break;
        
        case WStype_DISCONNECTED:
            Serial.println("Disconnected from Oracle");
            break;
        
        case WStype_TEXT: {
            String message = String((char*)payload, length);

            Serial.print("Oracle command: ");
            Serial.println(message);

            if (message == "wake") {
                manager_.powerOn();
            }
            else if (message == "reset") {
                manager_.reset();
            } 
            else if (message == "power_off") {
                manager_.powerOff();
            }
            else {
                Serial.println("Unknown Oracle Command");
            }
            break;
        }

        case WStype_ERROR:
            Serial.println("Websocket error");
            break;
        
        default:
            break;
    }
}
