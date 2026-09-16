#pragma once

#include <WiFi.h>
#include <WiFiUdp.h>

WiFiUDP udp;

void sendWakeOnLan(const uint8_t mac[6]);