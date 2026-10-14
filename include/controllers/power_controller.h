#pragma once

#include "management_controller.h"

class PowerManagementController: public ManagementController {
public:
    PowerManagementController();
    
    ManagementPriority priority() const override;
    
    ManagementResult pulsePowerButton(unsigned long duration);

    ManagementResult powerOn() override;
    ManagementResult powerOff() override;
    ManagementResult reset() override;

private:
    static constexpr int OPTO_PIN = 16;
    static constexpr int POWER_FEEDBACK_PIN = 17;
};