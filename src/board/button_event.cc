#include "board/button_event.h"
#include "config/secrets.h"

#include <Arduino.h>

#include "controllers/power_controller.h"
#include "controllers/network_controller.h"

#define BOOT_BUTTON 0

bool lastButtonState = HIGH;
unsigned long lastButtonCheck = 0;

void handleBootButton() {
    if (millis() - lastButtonCheck < 20) {
        return;
    }

    lastButtonCheck = millis();

    bool currentButtonState = digitalRead(BOOT_BUTTON);

    if (lastButtonState == HIGH && currentButtonState == LOW) {
        // pulsePowerButton();
        // sendWakeOnLan(PC_MAC);
    }

    lastButtonState = currentButtonState;
}