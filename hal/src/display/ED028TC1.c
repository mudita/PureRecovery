/**
 * @file ED028TC1.c
 * @author Lukasz Skrzypczak (l.skrzypczak@mudita.com)
 * @date Sep 6, 2017
 * @brief EInk ED028TC1 electronic paper display driver
 * @copyright Copyright (C) 2017 mudita.com.
 * @details This is hardware specific electronic paper display ED028TC1 driver.
 */

#include <hal/ED028TC1.h>
#include "LUT.h"
#include <stdbool.h>
#include <boot/board.h>
//#include "peripherals.h"
#include "fsl_common.h"
#include "fsl_lpspi.h"
#include <hal/delay.h>
#include <boot/board.h>

/* External variable definitions */
#define _delay_ms(ms) msleep(ms)
//#define ARRAY_SIZE(array)       (sizeof(array)/sizeof(array[0]))
#define EINK_TIMEOUT 1000

#define EINK_WAIT()                                                                             \
    _timeout = EINK_TIMEOUT;                                                                    \
    while ((GPIO_PinRead(BOARD_EINK_BUSY_GPIO, BOARD_EINK_BUSY_GPIO_PIN) == 0) && (--_timeout)) \
    {                                                                                           \
        _delay_ms(1);                                                                           \
    }                                                                                           \
    if (_timeout == 0)                                                                          \
    return (EinkTimeout)

#define EXAMPLE_LPSPI_MASTER_PCS_FOR_INIT (kLPSPI_Pcs0)
#define EXAMPLE_LPSPI_MASTER_PCS_FOR_TRANSFER (kLPSPI_MasterPcs0)
#define TRANSFER_BAUDRATE (2000000U) /*! Transfer baudrate - 500k */

/* Internal variable definitions */
EinkBpp_e BPP = Eink1Bpp;
uint32_t _timeout;
GPIO_Type *base;

static unsigned char EINK_WHITE_SCREEN1_1BPP[2 + (480 * 600 / 8)] = {EinkDataStartTransmission1, 0x00, [2 ...(480 * 600 / 8 - 1)] = 0xFF};

/* Internal function prototypes */
EinkStatus_e
_WriteCommand(uint8_t command);
EinkStatus_e
_WriteData(uint8_t data, uint8_t setCS);
EinkStatus_e
_WriteBuffer(uint8_t *buffer, uint32_t size);
uint8_t
_ReadData(void);

/* Function bodies */

/**
 * @brief Initialize ED028TC1 E-Ink display
 * @param bpp \refEinkBpp_t Bits per pixel
 * @return returns \ref EinkStatus_e.EinkOK if display initialization was OK
 */
EinkStatus_e
EinkInitialize(EinkBpp_e bpp)
{
    unsigned char tmpbuf[10];
    /* Define the init structure for the output LED pin*/
    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
    lpspi_master_config_t masterConfig;
    int i;

    LPSPI_MasterGetDefaultConfig(&masterConfig);

    /*Set clock source for LPSPI*/
    //    CLOCK_SetMux(kCLOCK_LpspiMux, BOARD_EINK_LPSPI_CLOCK_SOURCE_SELECT);
    //    CLOCK_SetDiv(kCLOCK_LpspiDiv, BOARD_EINK_LPSPI_CLOCK_SOURCE_DIVIDER);

    /*Master config*/
    masterConfig.baudRate = TRANSFER_BAUDRATE;
    masterConfig.bitsPerFrame = 8;
    masterConfig.cpol = kLPSPI_ClockPolarityActiveHigh;
    masterConfig.cpha = kLPSPI_ClockPhaseFirstEdge;
    masterConfig.direction = kLPSPI_MsbFirst;

    masterConfig.pcsToSckDelayInNanoSec = 1000000000 / masterConfig.baudRate;
    masterConfig.lastSckToPcsDelayInNanoSec = 1000000000 / masterConfig.baudRate;
    masterConfig.betweenTransferDelayInNanoSec = 1000000000 / masterConfig.baudRate;

    //    masterConfig.whichPcs = EXAMPLE_LPSPI_MASTER_PCS_FOR_INIT;
    //    masterConfig.pcsActiveHighOrLow = kLPSPI_PcsActiveLow;

    masterConfig.pinCfg = kLPSPI_SdiInSdoOut;
    masterConfig.dataOutConfig = kLpspiDataOutRetained;

    LPSPI_MasterInit(BOARD_EINK_LPSPI_BASE, &masterConfig, BOARD_EINK_LPSPI_CLOCK_FREQ);
    LPSPI_Enable(BOARD_EINK_LPSPI_BASE, false);
    BOARD_EINK_LPSPI_BASE->CFGR1 &= (~LPSPI_CFGR1_NOSTALL_MASK);
    LPSPI_Enable(BOARD_EINK_LPSPI_BASE, true);

    /*Flush FIFO , clear status , disable all the inerrupts.*/
    LPSPI_FlushFifo(BOARD_EINK_LPSPI_BASE, true, true);
    LPSPI_ClearStatusFlags(BOARD_EINK_LPSPI_BASE, kLPSPI_AllStatusFlag);
    LPSPI_DisableInterrupts(BOARD_EINK_LPSPI_BASE, kLPSPI_AllInterruptEnable);

    //    BOARD_EINK_LPSPI_BASE->TCR =
    //            (BOARD_EINK_LPSPI_BASE->TCR &
    //                    ~(LPSPI_TCR_CONT_MASK | LPSPI_TCR_CONTC_MASK | LPSPI_TCR_RXMSK_MASK | LPSPI_TCR_PCS_MASK)) |
    //                    LPSPI_TCR_CONT(0) | LPSPI_TCR_CONTC(0) | LPSPI_TCR_RXMSK(0) | LPSPI_TCR_TXMSK(0) | LPSPI_TCR_PCS(EXAMPLE_LPSPI_MASTER_PCS_FOR_INIT);

    GPIO_PinInit(BOARD_EINK_RESET_GPIO, BOARD_EINK_RESET_GPIO_PIN, &gpio_config);
    GPIO_PinWrite(BOARD_EINK_RESET_GPIO, BOARD_EINK_RESET_GPIO_PIN, 1U);

    //CS
    GPIO_PinInit(BOARD_EINK_CS_GPIO, BOARD_EINK_CS_GPIO_PIN, &gpio_config);
    GPIO_PinWrite(BOARD_EINK_CS_GPIO, BOARD_EINK_CS_GPIO_PIN, 1U);

    gpio_config.direction = kGPIO_DigitalInput;
    GPIO_PinInit(BOARD_EINK_BUSY_GPIO, BOARD_EINK_BUSY_GPIO_PIN, &gpio_config);

    // enable frontlight by default
    gpio_config.direction = kGPIO_DigitalOutput;
    GPIO_PinInit(BOARD_EINK_FL_GPIO, BOARD_EINK_FL_GPIO_PIN, &gpio_config);
    GPIO_PinWrite(BOARD_EINK_FL_GPIO, BOARD_EINK_FL_GPIO_PIN, 1U);

    BPP = bpp;

    GPIO_PinWrite(BOARD_EINK_RESET_GPIO, BOARD_EINK_RESET_GPIO_PIN, 0U);
    _delay_ms(100);
    GPIO_PinWrite(BOARD_EINK_RESET_GPIO, BOARD_EINK_RESET_GPIO_PIN, 1U);
    _delay_ms(100);

    EINK_WAIT();

    //fill white screen table
    EINK_WHITE_SCREEN1_1BPP[0] = EinkDataStartTransmission1;
    EINK_WHITE_SCREEN1_1BPP[1] = 0x00;
    for (i = 0; i < (480 * 600 / 8); i++)
        EINK_WHITE_SCREEN1_1BPP[2 + i] = 0xFF;

    //send initialization data

    //    _WriteCommand(EinkPowerON);   //Power ON   0x04
    //    _delay_ms(1);       //1st transmission is somhow corrupted - data is lagging CS. So mak this one dummy and repeat it after some delay

    //    _WriteCommand(EinkPowerON);   //Power ON   0x04
    //    EINK_WAIT();

    tmpbuf[0] = EinkPowerSetting; //0x04
    tmpbuf[1] = 0x03;
    tmpbuf[2] = 0x04;
    tmpbuf[3] = 0x00;
    tmpbuf[4] = 0x00;
    _WriteBuffer(tmpbuf, 5);

    tmpbuf[0] = EinkPanelSetting; //0x00
    tmpbuf[1] = 0x25;             //0x25 -> _XON _RES0 LUT_SEL _DM - SHL _SPIWM RST_N
    tmpbuf[2] = 0x00;
    _WriteBuffer(tmpbuf, 3);

    tmpbuf[0] = 0x26; //Power saving  //0x26
    tmpbuf[1] = 0x82; //B2
    _WriteBuffer(tmpbuf, 2);

    tmpbuf[0] = EinkPowerOFFSequenceSetting; //0x03
    tmpbuf[1] = 0x03;
    _WriteBuffer(tmpbuf, 2);

    tmpbuf[0] = EinkBoosterSoftStart; //0x07
    tmpbuf[1] = 0xEF;
    tmpbuf[2] = 0xEF;
    tmpbuf[3] = 0x28;
    _WriteBuffer(tmpbuf, 4);

    //0xE0, 0x02    GDOS
    tmpbuf[0] = EinkGDOrderSetting; //0xE0
    tmpbuf[1] = 0x02;
    _WriteBuffer(tmpbuf, 2);

    tmpbuf[0] = EinkPLLControl; //0x30
    tmpbuf[1] = 0x0E;
    _WriteBuffer(tmpbuf, 2);

    tmpbuf[0] = 0x41; //Temp. sensor setting TSE
    tmpbuf[1] = 0x00;
    _WriteBuffer(tmpbuf, 2);

    tmpbuf[0] = EinkVcomAndDataIntervalSetting; //0x50
    tmpbuf[1] = 0x0D;
    tmpbuf[2] = 0x22;
    _WriteBuffer(tmpbuf, 3);

    tmpbuf[0] = EinkTCONSetting; //0x60
    tmpbuf[1] = 0x3F;
    tmpbuf[2] = 0x09;
    tmpbuf[3] = 0x2D;
    _WriteBuffer(tmpbuf, 4);

    tmpbuf[0] = EinkResolutionSetting; //0x61
    tmpbuf[1] = 0x02;                  //0x02
    tmpbuf[2] = 0x58;                  //0x60
    tmpbuf[3] = 0x01;                  //0x01
    tmpbuf[4] = 0xE0;                  //0xE0
    _WriteBuffer(tmpbuf, 5);

    //0xE0, 0x02    GDOS
    tmpbuf[0] = EinkGDOrderSetting;
    tmpbuf[1] = 0x02;
    _WriteBuffer(tmpbuf, 2);

    tmpbuf[0] = EinkVCM_DCSetting; //0x82
    tmpbuf[1] = 0x30;
    _WriteBuffer(tmpbuf, 2);

    //LUT
    _WriteBuffer((uint8_t *)EINK_LUTC, sizeof(EINK_LUTC));
    _WriteBuffer((uint8_t *)EINK_LUTD, sizeof(EINK_LUTD));

    //_WriteCommand(EinkPowerON);    //0x04
    EINK_WAIT();

    //0xE0, 0x02    GDOS
    tmpbuf[0] = EinkGDOrderSetting;
    tmpbuf[1] = 0x02;
    _WriteBuffer(tmpbuf, 2);

    //refresh
    tmpbuf[0] = EinkDisplayRefresh;
    tmpbuf[1] = 0x08;
    tmpbuf[2] = 0x00;
    tmpbuf[3] = 0x00;
    tmpbuf[4] = 0x00;
    tmpbuf[5] = 0x00;
    tmpbuf[6] = 0x02;
    tmpbuf[7] = 0x58;
    tmpbuf[8] = 0x01;
    tmpbuf[9] = 0xE0;
    _WriteBuffer(tmpbuf, 10);
    EINK_WAIT();

    //_WriteCommand(EinkPowerOFF);    //0x02
    EINK_WAIT();

    //EinkClearScreen();

    _delay_ms(200);

    return EinkOK;
} //EinkInitialize

/**
 * @brief Display image
 * @param X image start position X
 * @param Y image start position Y
 * @param W image width
 * @param H image height
 * @param buffer pointer to image encoded according to BPP set in initialization
 */
EinkStatus_e
EinkDisplayImage(uint16_t X, uint16_t Y, uint16_t W, uint16_t H,
                 uint8_t *buffer)
{
    uint8_t buf[10];
    uint32_t tmp;

    buf[0] = EinkDataStartTransmissionWindow; //set display window
    buf[1] = (uint8_t)(X >> 8);               //MSB
    buf[2] = (uint8_t)X;                      //LSB
    buf[3] = (uint8_t)(Y >> 8);               //MSB
    buf[4] = (uint8_t)Y;                      //LSB
    buf[5] = (uint8_t)(W >> 8);               //MSB
    buf[6] = (uint8_t)W;                      //LSB
    buf[7] = (uint8_t)(H >> 8);               //MSB
    buf[8] = (uint8_t)H;                      //LSB
    _WriteBuffer(buf, 9);
    EINK_WAIT();

    tmp = ((uint32_t)W * (uint32_t)H / 8) + 2;
    _WriteBuffer(buffer, tmp);

    EINK_WAIT();

    _WriteCommand(EinkPowerON); //0x04
    EINK_WAIT();

    //0xE0, 0x02    GDOS
    buf[0] = EinkGDOrderSetting;
    buf[1] = 0x02;
    _WriteBuffer(buf, 2);

    EinkRefreshImage(X, Y, W, H);

    _WriteCommand(EinkPowerOFF); //0x02
    EINK_WAIT();

    return (EinkOK);
}

/**
 * @brief Clear screen
 */
EinkStatus_e
EinkClearScreen(void)
{
    uint8_t buf[10];
    //display white
    //DTMW
    buf[0] = EinkDataStartTransmissionWindow; //0x83
    buf[1] = 0x00;
    buf[2] = 0x00;
    buf[3] = 0x00;
    buf[4] = 0x00;
    buf[5] = 0x02;
    buf[6] = 0x58;
    buf[7] = 0x01;
    buf[8] = 0xE0;
    _WriteBuffer(buf, 9);
    EINK_WAIT();

    //image
    _WriteBuffer((uint8_t *)EINK_WHITE_SCREEN1_1BPP, sizeof(EINK_WHITE_SCREEN1_1BPP));
    EINK_WAIT();

    _WriteCommand(EinkPowerON); //0x04
    EINK_WAIT();

    //0xE0, 0x02    GDOS
    buf[0] = EinkGDOrderSetting;
    buf[1] = 0x02;
    _WriteBuffer(buf, 2);

    //refresh
    buf[0] = EinkDisplayRefresh;
    buf[1] = 0x08;
    buf[2] = 0x00;
    buf[3] = 0x00;
    buf[4] = 0x00;
    buf[5] = 0x00;
    buf[6] = 0x02;
    buf[7] = 0x58;
    buf[8] = 0x01;
    buf[9] = 0xE0;
    _WriteBuffer(buf, 10);
    EINK_WAIT();

    _WriteCommand(EinkPowerOFF); //0x02
    EINK_WAIT();

    /*
     * 0x83, 0, 0, 0, 0, 0x02, 0x58, 0x01, 0xE0
     * 0x10, 0x03 ... obrazek
     * 0x04
     * 0xE0, 0x02
     * 0x12, 0x08, 0x00, 0x00, 0x00, 0x00, 0x02, 0x58, 0x01, 0xE0
     * 0x02
     */

    return EinkOK;
}

/**
 * @brief Refresh window on the screen. E-paper display tends to loose contrast over time. To Keep the image sharp refresh is needed.
 * @param X refresh window position X
 * @param Y refresh window position Y
 * @param W refresh window width
 * @param H refresh window height
 */
EinkStatus_e
EinkRefreshImage(uint16_t X, uint16_t Y, uint16_t W, uint16_t H)
{
    uint8_t buf[10];

    buf[0] = EinkDisplayRefresh; //refresh image
    buf[1] = 0x08;               //for now - magic number

    buf[2] = (uint8_t)(X >> 8); //MSB
    buf[3] = (uint8_t)X;        //LSB
    buf[4] = (uint8_t)(Y >> 8); //MSB
    buf[5] = (uint8_t)Y;        //LSB
    buf[6] = (uint8_t)(W >> 8); //MSB
    buf[7] = (uint8_t)W;        //LSB
    buf[8] = (uint8_t)(H >> 8); //MSB
    buf[9] = (uint8_t)H;        //LSB

    _WriteBuffer(buf, sizeof(buf));
    /* this delay is needed because without it data sent trhough SPI get corrupted. No idea why :/ */
    _delay_ms(1);
    EINK_WAIT();

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

/**
 * @brief Internal function. Write command to display controller
 * @param command command to be written
 * @return \ref EinkStatus_e.EinkOK if command was sent
 */
EinkStatus_e
_WriteCommand(uint8_t command)
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
 * @brief Internal function. Write data to display controller
 * @param data data byte to be written
 * @return \ref EinkStatus_e.EinkOK if data was sent
 */
EinkStatus_e
_WriteData(uint8_t data, uint8_t setCS)
{
    (void)setCS;
    lpspi_transfer_t transfer;
    GPIO_PinWrite(BOARD_EINK_CS_GPIO, BOARD_EINK_CS_GPIO_PIN, 0U);
    transfer.configFlags = kLPSPI_MasterPcs0 | kLPSPI_MasterPcsContinuous;
    transfer.dataSize = 1;
    transfer.txData = &data;
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
EinkStatus_e
_WriteBuffer(uint8_t *buffer, uint32_t size)
{
    lpspi_transfer_t transfer;
    GPIO_PinWrite(BOARD_EINK_CS_GPIO, BOARD_EINK_CS_GPIO_PIN, 0U);
    //    LPSPI_WriteData(BOARD_EINK_LPSPI_BASE, (uint32_t)command);
    //    while (!(LPSPI_GetStatusFlags(BOARD_EINK_LPSPI_BASE) & kLPSPI_TransferCompleteFlag));
    transfer.configFlags = kLPSPI_MasterPcs0 | kLPSPI_MasterPcsContinuous;
    transfer.dataSize = size;
    transfer.txData = buffer;
    transfer.rxData = NULL;
    LPSPI_MasterTransferBlocking(BOARD_EINK_LPSPI_BASE, &transfer);
    GPIO_PinWrite(BOARD_EINK_CS_GPIO, BOARD_EINK_CS_GPIO_PIN, 1U);

    return EinkOK;
}

/**
 * @brief Read data from display
 * @return value of data read
 */
uint8_t
_ReadData(void)
{
    uint8_t data = 0;
    //if (SPIXfer (EinkSPIHandle, (void*) &dummy, (void*) &data, sizeof(uint8_t),
    //             1) != SPIOK)
    //    return EinkSPIErr;
    data = (uint8_t)LPSPI_ReadData(BOARD_EINK_LPSPI_BASE);

    return data;
}
