#include "oracle_handler.h"
#include "secrets.h"
#include "config.h"
#include "device_credentials.h"

#include  <WiFi.h>
#include <WebSocketsClient.h>

#include "send_wal_packet.h"

WebSocketsClient webSocket;

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
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
                sendWakeOnLan(PC_MAC);
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

void connect_to_oracle() {
    webSocket.beginSslWithClientKey(
        ORACLE_HOST,
        ORACLE_PORT,
        "/device",
        BMC_ROOT_CA,
        CLIENT_CERT,
        CLIENT_KEY
    );

    // String bearer = "Bearer ";
    // bearer += DEVICE_TOKEN;

    // // webSocket.setAuthorization(bearer.c_str());

    String headers =
        "Origin: https://device.corasfein.com:443\r\n"
        "Authorization: Bearer " + String(DEVICE_TOKEN) + "\r\n";

    webSocket.setExtraHeaders(headers.c_str());

    
    webSocket.setExtraHeaders(
        "Origin: https://device.corasfein.com\r\n"
    );

    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000);
}

void handle_oracle() {
    webSocket.loop();
}