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
            String message = String((char*)payload);

            Serial.println("Oracle command: ");
            Serial.print(message);

            if (message == "wake") {
                sendWakeOnLan(PC_MAC);
            }
            break;
        }
    }
}

void connect_to_oracle() {
    webSocket.begin(
        ORACLE_IP,
        ORACLE_PORT,
        "/device"
    );
}

void handle_oracle() {
    webSocket.loop();
}