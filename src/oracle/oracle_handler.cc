#include "oracle_handler.h"
#include "secrets.h"

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
    String authHeader = "Authorization: Bearer ";
    authHeader += DEVICE_TOKEN;
    authHeader += "\r\n";

    webSocket.setExtraHeaders(authHeader.c_str());

    webSocket.begin(
        ORACLE_IP,
        ORACLE_PORT,
        "/device"
    );

    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000);
}

void handle_oracle() {
    webSocket.loop();
}