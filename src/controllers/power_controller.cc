#include "controllers/power_controller.h"

#include <Arduino.h>

PowerManagementController::PowerManagementController(): ManagementController("POWER_CTRL") {
    pinMode(OPTO_PIN, OUTPUT);
    digitalWrite(OPTO_PIN, LOW);

    pinMode(POWER_FEEDBACK_PIN, INPUT);
}

ManagementPriority PowerManagementController::priority() const {return ManagementPriority::Power;}

ManagementResult PowerManagementController::pulsePowerButton(unsigned long duration) {
    // Simulate holding the motherboard power button through the optocoupler.
    // Feedback is expected HIGH while asserted and LOW after release.
    // Blocks for approximately duration + 10 ms; duration must be at least 10 ms.
    digitalWrite(OPTO_PIN, HIGH);
    delay(10);

    if (!digitalRead(POWER_FEEDBACK_PIN)) {
        digitalWrite(OPTO_PIN, LOW);

        logger_.error("Power switch signal not detected");
        return ManagementResult::Failed;
    }

    delay(duration - 10);
    digitalWrite(OPTO_PIN, LOW);
    delay(10);

    if (digitalRead(POWER_FEEDBACK_PIN)) {
        logger_.error("POWER_SW remained active");
        return ManagementResult::Failed;
    }

    return ManagementResult::Success;
}

ManagementResult PowerManagementController::powerOn() {
    logger_.info("Powering on PC");
    return pulsePowerButton(300);
}

ManagementResult PowerManagementController::powerOff() {
    logger_.info("Powering off PC");
    return pulsePowerButton(3000);
}

ManagementResult PowerManagementController::reset() {
    logger_.info("Hard Restarting PC");

    ManagementResult result = powerOff();
    if (result != ManagementResult::Success) return result;

    delay(1500);

    result = powerOn();
    if (result != ManagementResult::Success) return result;

    return ManagementResult::Success;
}