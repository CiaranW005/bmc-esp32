#pragma once

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length);
void connect_to_oracle();
void handle_oracle();