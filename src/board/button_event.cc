#include "button_event.h"
#include "secrets.h"

#include <Arduino.h>

#include "send_wal_packet.h"

#define BOOT_BUTTON 0

bool lastButtonState = HIGH;
unsigned long lastButtonCheck = 0;

const int OPTO_PIN = 17;

void pulsePowerButton() {
    digitalWrite(OPTO_PIN, HIGH);
    Serial.println("GPIO17 HIGH");

    delay(300);

    digitalWrite(OPTO_PIN, LOW);
}

// void handleBootButton() {
//     if (millis() - lastButtonCheck < 20) {
//         return;
//     }

//     lastButtonCheck = millis();

//     bool currentButtonState = digitalRead(BOOT_BUTTON);

//     if (lastButtonState == HIGH && currentButtonState == LOW) {
//         sendWakeOnLan(PC_MAC);
//     }

//     lastButtonState = currentButtonState;
// }

void handleBootButton() {
    if (millis() - lastButtonCheck < 20) {
        return;
    }

    lastButtonCheck = millis();

    bool currentButtonState = digitalRead(BOOT_BUTTON);

    if (lastButtonState == HIGH && currentButtonState == LOW) {
        pulsePowerButton();
    }

    lastButtonState = currentButtonState;
}