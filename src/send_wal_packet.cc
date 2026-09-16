#include "send_wal_packet.h"

void sendWakeOnLan(const uint8_t mac[6]) {
    uint8_t packet[102];

    for (int i = 0; i < 6; i++) {
        packet[i] = 0xFF;
    }

    for (int i = 1; i <=  16; i++) {
        memcpy(&packet[i * 6], mac, 6);
    }

    udp.beginPacket("255.255.255.255", 9);
    udp.write(packet, sizeof(packet)),
    udp.endPacket();
}