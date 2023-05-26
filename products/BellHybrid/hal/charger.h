#pragma once

enum charging_state_e {
    CHARGING_DISABLE,
    CHARGING_ENABLE
};

void charger_init();
void charger_ctrl(enum charging_state_e charging);
