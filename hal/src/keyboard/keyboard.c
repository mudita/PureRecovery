#include <hal/keyboard.h>
#include <hal/delay.h>
#include <drivers/fsl_common.h>
#include <drivers/fsl_gpio.h>
#include <boot/board.h>
#include <hal/system.h>
#include <hal/i2c_host.h>
#include "TCA8418.h"
#include <errno.h>

static kbd_event_t events[16] = {};

/** Initialize the keyboard engine */
int kbd_init() {

    // Define the init structure for the output RST pin
    gpio_pin_config_t rstpin_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
    // Init output CS GPIO
    GPIO_PinInit(BOARD_KEYBOARD_RESET_GPIO, BOARD_KEYBOARD_RESET_GPIO_PIN, &rstpin_config);
    // Define the init structure for the input IRQ pin
    gpio_pin_config_t irqpin_config = {kGPIO_DigitalInput, 0, kGPIO_IntFallingEdge};
    // Init input KEYBOARD IRQ
    GPIO_PinInit(BOARD_KEYBOARD_IRQ_GPIO, BOARD_KEYBOARD_IRQ_GPIO_PIN, &irqpin_config);

    // Reset keyboard controller
    GPIO_PinWrite(BOARD_KEYBOARD_RESET_GPIO, BOARD_KEYBOARD_RESET_GPIO_PIN, 0);
    msleep(100);
    GPIO_PinWrite(BOARD_KEYBOARD_RESET_GPIO, BOARD_KEYBOARD_RESET_GPIO_PIN, 1);
    msleep(100);

    uint32_t reg = 0;
    status_t error = 0;

    // Assemble a mask for row and column registers
    reg = ~(~0U << TCA8418_ROWS_COUNT);
    reg += (~(~0U << TCA8418_COL_COUNT)) << 8;

    hal_i2c_dev_t *const i2c_inst = get_i2c_controller();

    // Set registers to keypad mode
    error |= TCA8418_SendByte(i2c_inst, REG_KP_GPIO1, reg);
    reg = reg >> 8;
    error |= TCA8418_SendByte(i2c_inst, REG_KP_GPIO2, reg);
    reg = reg >> 16;
    error |= TCA8418_SendByte(i2c_inst, REG_KP_GPIO3, reg);

    // Enable column debouncing
    error |= TCA8418_SendByte(i2c_inst, REG_DEBOUNCE_DIS1, reg);
    reg = reg >> 8;
    error |= TCA8418_SendByte(i2c_inst, REG_DEBOUNCE_DIS2, reg);
    reg = reg >> 16;
    error |= TCA8418_SendByte(i2c_inst, REG_DEBOUNCE_DIS3, reg);

    if (error != kStatus_Success) {
        return -EIO;
    }

    reg = CFG_INT_CFG | CFG_OVR_FLOW_IEN | CFG_KE_IEN;
    // Enable interrupts
    error = TCA8418_SendByte(i2c_inst, REG_CFG, reg);

    // Get key pressed/released count
    uint8_t val = 0;
    TCA8418_ReceiveByte(i2c_inst, REG_KEY_LCK_EC, &val);

    uint8_t key_count = val & 0xF;
    for (uint8_t i = 0; i < key_count; i++) {
        TCA8418_ReceiveByte(i2c_inst, REG_KEY_EVENT_A, &val);
    }

    // Clear all interrupts, even IRQs we didn't check (GPI, CAD, LCK)
    TCA8418_SendByte(i2c_inst, REG_INT_STAT, 0xFF);

    return 0;
}

int kbd_lock(kbd_lock_status_t lock) {
    uint8_t mask = 0x40; // Set/clr 6th bit
    int ret = TCA8418_ModifyReg(get_i2c_controller(), REG_KEY_LCK_EC, mask, lock);
    return (ret != kStatus_Success) ? (-EIO) : (0);
}

kbd_event_t *kbd_read_events() {
    memset(&events[0], 0, sizeof(events));
    hal_i2c_dev_t *const i2c_inst = get_i2c_controller();
    uint8_t retval = 0;

    // Read how many key events has been registered
    TCA8418_ReceiveByte(i2c_inst, REG_KEY_LCK_EC, &retval);

    const uint8_t events_count = retval & 0xF;
    // Iterate over and parse each event
    for (uint8_t i = 0; i < events_count; i++) {
        TCA8418_ReceiveByte(i2c_inst, REG_KEY_EVENT_A, &retval);

        events[i].key = retval & 0x7F;
        // rel_pres: 0 - key release, 1 - key press
        events[i].event = (retval & 0x80) >> 7; // key release/pressed is stored on the last bit
    }
    // Clear all interrupts
    TCA8418_SendByte(i2c_inst, REG_INT_STAT, 0xFF);
    return events_count == 0 ? NULL : &events[0];
}