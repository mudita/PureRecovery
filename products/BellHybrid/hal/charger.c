#include "charger.h"
#include <fsl_gpio.h>

/**
 * Charger
 */

#define BOARD_BATTERY_CHARGER_GPIO   GPIO2
#define BOARD_BATTERY_CHARGER_ACOK   22U // GPIO_B1_06
#define BOARD_BATTERY_CHARGER_CHGEN  23U // GPIO_B1_07

void charger_init() {
    gpio_pin_config_t gpio_config = {
            .direction = kGPIO_DigitalOutput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode
    };

    GPIO_PinInit(BOARD_BATTERY_CHARGER_GPIO, BOARD_BATTERY_CHARGER_CHGEN, &gpio_config);
    charger_ctrl(CHARGING_DISABLE);
}

void charger_ctrl(enum charging_state_e charging) {
    switch (charging) {
        case CHARGING_DISABLE:
            GPIO_PinWrite(BOARD_BATTERY_CHARGER_GPIO, BOARD_BATTERY_CHARGER_CHGEN, 1U);
            break;
        case CHARGING_ENABLE:
            GPIO_PinWrite(BOARD_BATTERY_CHARGER_GPIO, BOARD_BATTERY_CHARGER_CHGEN, 0U);
            break;
        default:
            break;
    }
}
