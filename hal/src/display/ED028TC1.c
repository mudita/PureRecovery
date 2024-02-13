/**
 * @file ED028TC1.c
 * @author Lukasz Skrzypczak (l.skrzypczak@mudita.com)
 * @date Sep 6, 2017
 * @brief EInk ED028TC1 electronic paper display driver
 * @copyright Copyright (C) 2017 mudita.com.
 * @details This is hardware specific electronic paper display ED028TC1 driver.
 */

#include <hal/ED028TC1.h>
#include "LUTS.h"
#include <stdbool.h>
#include <boot/board.h>
#include "fsl_common.h"
#include "fsl_lpspi.h"
#include <hal/delay.h>

/* Defines */
#define EINK_TIMEOUT 1000
#define TRANSFER_BAUDRATE 2000000U /*! Transfer baudrate - 500k */

#define IMAGE_BUFFER_SIZE ((BOARD_EINK_DISPLAY_RES_X * BOARD_EINK_DISPLAY_RES_Y) / PIXELS_PER_BYTE)
#define EMPTY_SCREEN_BUFFER_SIZE (IMAGE_BUFFER_COMMAND_SIZE + IMAGE_BUFFER_SIZE)

static uint8_t eink_screen_1bpp[EMPTY_SCREEN_BUFFER_SIZE];
static const EinkFrame_t full_frame = {
        .x = 0,
        .y = 0,
        .w = BOARD_EINK_DISPLAY_RES_Y,
        .h = BOARD_EINK_DISPLAY_RES_X
};

/* Function bodies */
/**
 * @brief Internal function. Wait for display controller to finish last operation.
 * @return \ref EinkStatus_e.EinkOK if command was sent, \ref EinkStatus_e.EinkTimeout if timeout occurred
 */
static EinkStatus_e eink_wait(void) {
    uint32_t timeout = EINK_TIMEOUT;
    while ((GPIO_PinRead(BOARD_EINK_BUSY_GPIO, BOARD_EINK_BUSY_GPIO_PIN) == 0) && (--timeout)) {
        msleep(1);
    }
    if (timeout == 0) {
        return EinkTimeout;
    }
    return EinkOK;
}

/**
 * @brief Internal function. Write command to display controller
 * @param command command to be written
 * @return \ref EinkStatus_e.EinkOK if command was sent
 */
static EinkStatus_e WriteCommand(uint8_t command)
{
    lpspi_transfer_t transfer;
    GPIO_PinWrite(BOARD_EINK_CS_GPIO, BOARD_EINK_CS_GPIO_PIN, 0U);
    transfer.configFlags = kLPSPI_MasterPcs0 | kLPSPI_MasterPcsContinuous;
    transfer.dataSize = 1;
    transfer.txData = &command;
    transfer.rxData = NULL;
    LPSPI_MasterTransferBlocking(BOARD_EINK_LPSPI_BASE, &transfer);
    GPIO_PinWrite(BOARD_EINK_CS_GPIO, BOARD_EINK_CS_GPIO_PIN, 1U);

    return EinkOK;
}

/**
 * @brief Internal function. Send buffer to the display
 * @param buffer pointer to image buffer
 * @param size size of image buffer
 * @return \ref EinkStatus_e.EinkOK if buffer was sent
 */
static EinkStatus_e WriteBuffer(uint8_t *buffer, uint32_t size)
{
    lpspi_transfer_t transfer;
    GPIO_PinWrite(BOARD_EINK_CS_GPIO, BOARD_EINK_CS_GPIO_PIN, 0U);
    transfer.configFlags = kLPSPI_MasterPcs0 | kLPSPI_MasterPcsContinuous;
    transfer.dataSize = size;
    transfer.txData = buffer;
    transfer.rxData = NULL;
    LPSPI_MasterTransferBlocking(BOARD_EINK_LPSPI_BASE, &transfer);
    GPIO_PinWrite(BOARD_EINK_CS_GPIO, BOARD_EINK_CS_GPIO_PIN, 1U);

    return EinkOK;
}

/**
 * @brief Initialize ED028TC1 E-Ink display
 * @return returns \ref EinkStatus_e.EinkOK if display initialization was OK
 */
EinkStatus_e EinkInitialize()
{
    uint8_t buf[10];

    /* Define the init structure for the output pins */
    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
    lpspi_master_config_t masterConfig;

    /* Master config */
    masterConfig.baudRate = TRANSFER_BAUDRATE;
    masterConfig.bitsPerFrame = 8;
    masterConfig.cpol = kLPSPI_ClockPolarityActiveHigh;
    masterConfig.cpha = kLPSPI_ClockPhaseFirstEdge;
    masterConfig.direction = kLPSPI_MsbFirst;

    masterConfig.pcsToSckDelayInNanoSec = 1000000000 / masterConfig.baudRate;
    masterConfig.lastSckToPcsDelayInNanoSec = 1000000000 / masterConfig.baudRate;
    masterConfig.betweenTransferDelayInNanoSec = 1000000000 / masterConfig.baudRate;

    masterConfig.pinCfg = kLPSPI_SdiInSdoOut;
    masterConfig.dataOutConfig = kLpspiDataOutRetained;

    LPSPI_MasterInit(BOARD_EINK_LPSPI_BASE, &masterConfig, BOARD_EINK_LPSPI_CLOCK_FREQ);
    LPSPI_Enable(BOARD_EINK_LPSPI_BASE, false);
    BOARD_EINK_LPSPI_BASE->CFGR1 |= LPSPI_CFGR1_AUTOPCS(0);
    BOARD_EINK_LPSPI_BASE->CFGR1 &= (~LPSPI_CFGR1_NOSTALL_MASK);
    LPSPI_SetMasterSlaveMode(BOARD_EINK_LPSPI_BASE, kLPSPI_Master);
    LPSPI_Enable(BOARD_EINK_LPSPI_BASE, true);

    /* Flush FIFO, clear status, disable all the interrupts */
    LPSPI_FlushFifo(BOARD_EINK_LPSPI_BASE, true, true);
    LPSPI_ClearStatusFlags(BOARD_EINK_LPSPI_BASE, kLPSPI_AllStatusFlag);
    LPSPI_DisableInterrupts(BOARD_EINK_LPSPI_BASE, kLPSPI_AllInterruptEnable);

    /* Put display in reset state */
    GPIO_PinInit(BOARD_EINK_RESET_GPIO, BOARD_EINK_RESET_GPIO_PIN, &gpio_config);
    GPIO_PinWrite(BOARD_EINK_RESET_GPIO, BOARD_EINK_RESET_GPIO_PIN, 0U);

    /* Deselect device */
    GPIO_PinInit(BOARD_EINK_CS_GPIO, BOARD_EINK_CS_GPIO_PIN, &gpio_config);
    GPIO_PinWrite(BOARD_EINK_CS_GPIO, BOARD_EINK_CS_GPIO_PIN, 1U);

    /* Configure busy pin */
    gpio_config.direction = kGPIO_DigitalInput;
    GPIO_PinInit(BOARD_EINK_BUSY_GPIO, BOARD_EINK_BUSY_GPIO_PIN, &gpio_config);

    /* Enable frontlight by default */
    gpio_config.direction = kGPIO_DigitalOutput;
    GPIO_PinInit(BOARD_EINK_FL_GPIO, BOARD_EINK_FL_GPIO_PIN, &gpio_config);
    GPIO_PinWrite(BOARD_EINK_FL_GPIO, BOARD_EINK_FL_GPIO_PIN, 1U);

    /* Release reset condition */
    GPIO_PinWrite(BOARD_EINK_RESET_GPIO, BOARD_EINK_RESET_GPIO_PIN, 1U);
    msleep(100);
    eink_wait();

    /* Configure registers */
    buf[0] = EinkPanelSetting;
    buf[1] = 0x25;
    buf[2] = 0x00;
    WriteBuffer(buf, 3);

    buf[0] = EinkPowerSetting;
    buf[1] = 0x03;
    buf[2] = 0x04;
    buf[3] = 0x00;
    buf[4] = 0x00;
    WriteBuffer(buf, 5);

    buf[0] = EinkPowerOFFSequenceSetting;
    buf[1] = 0x03;
    WriteBuffer(buf, 2);

    buf[0] = EinkBoosterSoftStart;
    buf[1] = 0xEF;
    buf[2] = 0xEF;
    buf[3] = 0x28;
    WriteBuffer(buf, 4);

    buf[0] = EinkPLLControl;
    buf[1] = 0x0E;
    WriteBuffer(buf, 2);

    buf[0] = 0x41; //Temp. sensor setting TSE
    buf[1] = 0x00;
    WriteBuffer(buf, 2);

    buf[0] = EinkVcomAndDataIntervalSetting;
    buf[1] = 0x0D;
    buf[2] = 0x22;
    WriteBuffer(buf, 3);

    buf[0] = EinkTCONSetting;
    buf[1] = 0x3F;
    buf[2] = 0x09;
    buf[3] = 0x2D;
    WriteBuffer(buf, 4);

    buf[0] = EinkResolutionSetting;
    buf[1] = 0x02;
    buf[2] = 0x60;
    buf[3] = 0x01;
    buf[4] = 0xE0;
    WriteBuffer(buf, 5);

    buf[0] = EinkVCM_DCSetting;
    buf[1] = 0x30;
    WriteBuffer(buf, 2);

    buf[0] = EinkGDOrderSetting;
    buf[1] = 0x02;
    buf[2] = 0x00;
    WriteBuffer(buf, 3);

    /* Create white screen array, overwrite both image buffers and refresh (later you can use EinkClearScreen()) */
    memset(eink_screen_1bpp, 0xFF, sizeof(eink_screen_1bpp));
    eink_screen_1bpp[0] = EinkDataStartTransmission2;
    eink_screen_1bpp[1] = 0x00;
    EinkDisplayImage(&full_frame, (uint8_t *) eink_screen_1bpp, REFRESH_NONE);
    eink_screen_1bpp[0] = EinkDataStartTransmission1;
    EinkDisplayImage(&full_frame, (uint8_t *) eink_screen_1bpp, REFRESH_DEEP);

    return EinkOK;
}

/**
 * @brief Display image
 * @param frame pointer to structure containing image frame parameters
 * @param buffer pointer to image encoded according to BPP set in initialization
 * @param mode eink refresh mode; fast, deep or no refresh
 */
EinkStatus_e EinkDisplayImage(const EinkFrame_t *frame, uint8_t *buffer, EinkRefreshMode_e mode)
{
    if ((frame == NULL) || (buffer == NULL)) {
        return EinkInitErr;
    }

    uint8_t buf[10];

    buf[0] = EinkDataStartTransmissionWindow; // Set display window
    buf[1] = (uint8_t) (frame->x >> 8); //MSB
    buf[2] = (uint8_t) frame->x;		//LSB
    buf[3] = (uint8_t) (frame->y >> 8); //MSB
    buf[4] = (uint8_t) frame->y;		//LSB
    buf[5] = (uint8_t) (frame->w >> 8); //MSB
    buf[6] = (uint8_t) frame->w;		//LSB
    buf[7] = (uint8_t) (frame->h >> 8); //MSB
    buf[8] = (uint8_t) frame->h;		//LSB
    WriteBuffer(buf, 9);
    eink_wait();

    const uint32_t img_size = (((uint32_t)frame->w * (uint32_t)frame->w) / PIXELS_PER_BYTE) + IMAGE_BUFFER_COMMAND_SIZE;
    WriteBuffer(buffer, img_size);
    eink_wait();

    if (mode != REFRESH_NONE) {
        WriteCommand(EinkPowerON);
        eink_wait();

        EinkRefreshImage(frame, mode);

        WriteCommand(EinkPowerOFF);
        eink_wait();
    }

    return EinkOK;
}

/**
 * @brief Clear screen
 */
EinkStatus_e EinkClearScreen(void)
{
    return EinkDisplayImage(&full_frame, (uint8_t *)eink_screen_1bpp, REFRESH_DEEP);
}

/**
 * @brief Refresh image on the screen.
 * @param frame pointer to structure containing image frame parameters
 * @param mode eink refresh mode; fast, deep or no refresh
 */
EinkStatus_e EinkRefreshImage(const EinkFrame_t *frame, EinkRefreshMode_e mode)
{
    uint8_t buf[10];

    /* Load desired LUT */
    switch (mode) {
        case REFRESH_FAST:
            WriteBuffer((uint8_t *)LUTC_DU2, sizeof(LUTC_DU2));
            WriteBuffer((uint8_t *)LUTD_DU2, sizeof(LUTD_DU2));
            eink_wait();
            break;
        case REFRESH_DEEP:
            WriteBuffer((uint8_t *)LUTC_GC16, sizeof(LUTC_GC16));
            WriteBuffer((uint8_t *)LUTD_GC16, sizeof(LUTD_GC16));
            eink_wait();
            break;
        default:
            break;
    }

    buf[0] = EinkDisplayRefresh;
    buf[1] = 0x08; //Waveform mode 0, clear previous frame, disable update optimization (needs VCOM LUT)

    buf[2] = (uint8_t) (frame->x >> 8); //MSB
    buf[3] = (uint8_t) frame->x;		//LSB
    buf[4] = (uint8_t) (frame->y >> 8); //MSB
    buf[5] = (uint8_t) frame->y;		//LSB
    buf[6] = (uint8_t) (frame->w >> 8); //MSB
    buf[7] = (uint8_t) frame->w;		//LSB
    buf[8] = (uint8_t) (frame->h >> 8); //MSB
    buf[9] = (uint8_t) frame->h;		//LSB

    WriteBuffer(buf, sizeof(buf));
    eink_wait();

    return EinkOK;
}

/**
 * @brief Enable or disable the frontlight
 * @param enable enable (true) or disable (false)
 */
void EinkEnableFrontlight(bool enable)
{
    GPIO_PinWrite(BOARD_EINK_FL_GPIO, BOARD_EINK_FL_GPIO_PIN, (uint8_t)enable);
}
