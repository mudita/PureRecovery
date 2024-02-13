/**
 * @file ED028TC1.h
 * @author Lukasz Skrzypczak (l.skrzypczak@mudita.com)
 * @date Sep 6, 2017
 * @brief Header for EInk ED028TC1 electronic paper display driver
 * @copyright Copyright (C) 2017 mudita.com.
 * @details This is hardware specific electronic paper display ED028TC1 driver.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ED028TC1_H
#define __ED028TC1_H

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/
/**
 * @enum EinkStatus_e
 */
typedef enum
{
  EinkOK,                  //!< EinkOK
  EinkSPIErr,              //!< EinkSPIErr
  EinkSPINotInitializedErr,//!< EinkSPINotInitializedErr
  EinkDMAErr,              //!< EinkDMAErr
  EinkInitErr,             //!< EinkInitErr
  EinkTimeout              //!< Timeout occured while waiting for nWait signale from EINK
} EinkStatus_e;

typedef enum {
    REFRESH_FAST,
    REFRESH_DEEP,
    REFRESH_NONE
} EinkRefreshMode_e;

typedef struct
{
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
} EinkFrame_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/**
 * @brief ED028TC1 register definitions
 */
#define EinkPanelSetting                  (0x00U)
#define EinkPowerSetting                  (0x01U)
#define EinkPowerOFF                      (0x02U)
#define EinkPowerOFFSequenceSetting       (0x03U)
#define EinkPowerON                       (0x04U)
#define EinkBoosterSoftStart              (0x07U)
#define EinkDataStartTransmission1        (0x10U)
#define EinkDisplayRefresh                (0x12U)
#define EinkDataStartTransmission2        (0x13U)
#define EinkPLLControl                    (0x30U)
#define EinkVcomAndDataIntervalSetting    (0x50U)
#define EinkTCONSetting                   (0x60U)
#define EinkResolutionSetting             (0x61U)
#define EinkAutoMeasurementVcom           (0x80U)
#define EinkReadVcomValue                 (0x81U)
#define EinkVCM_DCSetting                 (0x82U)
#define EinkDataStartTransmissionWindow   (0x83U)
#define EinkGDOrderSetting                (0xE0U)

/**
 * @brief ED028TC1 register bit definitions
 */
//PanelSetting
#define XON             (1<<7)
#define RES0            (1<<6)
#define LUT_SEL         (1<<5)
#define DM              (1<<4)
#define SHL             (1<<2)
#define SPIWM           (1<<1)
#define RST_N           (1<<0)
#define SFT1PX          (0<<0)
#define SFT2PX          (1<<0)
#define SFT3PX          (2<<0)
#define SFT4PX          (3<<0)

//PowerSetting
#define VSource_EN      (1<<0)
#define VGate_EN        (1<<1)
#define VG_LVL17V       (0<<0)
#define VG_LVL18V       (1<<0)
#define VG_LVL19V       (2<<0)
#define VG_LVL20V       (3<<0)
#define VG_LVL21V       (4<<0)
#define VSLV_LVL        (0)     //this is: 7-bit 00h=2.4V, 7Fh=15.0V
#define VSL_LVL15V      (0<<2)
#define VSL_LVL14V      (1<<2)
#define VSL_LVL13V      (2<<2)
#define VSL_LVL12V      (3<<2)
#define VSH_LVL15V      (0<<0)
#define VSH_LVL14V      (1<<0)
#define VSH_LVL13V      (2<<0)
#define VSH_LVL12V      (3<<0)

//PowerOFFSequenceSetting
#define T_VDS_OFF_1F    (0<<4)
#define T_VDS_OFF_2F    (1<<4)
#define T_VDS_OFF_3F    (2<<4)
#define T_VDS_OFF_4F    (3<<4)

//BoosterSoftStart
#define  BTPHx_SSP_10ms (0<<6)  //soft start period of phase A,B
#define  BTPHx_SSP_20ms (1<<6)
#define  BTPHx_SSP_30ms (2<<6)
#define  BTPHx_SSP_40ms (3<<6)
#define  BTPHx_DS_1     (0<<3)  //driving strength of phase A,B,C
#define  BTPHx_DS_2     (1<<3)
#define  BTPHx_DS_3     (2<<3)
#define  BTPHx_DS_4     (3<<3)
#define  BTPHx_DS_5     (4<<3)
#define  BTPHx_DS_6     (5<<3)
#define  BTPHx_DS_7     (6<<3)
#define  BTPHx_DS_8     (7<<3)
#define  BTPHx_OT_027   (0<<0)  //minimum OFF time setting of phase A,B,C
#define  BTPHx_OT_034   (1<<0)
#define  BTPHx_OT_040   (2<<0)
#define  BTPHx_OT_054   (3<<0)
#define  BTPHx_OT_080   (4<<0)
#define  BTPHx_OT_154   (5<<0)
#define  BTPHx_OT_334   (6<<0)
#define  BTPHx_OT_658   (7<<0)

//DataStartTransmission1
#define Cur_BPP1        (0<<0)
#define Cur_BPP2        (1<<0)
#define Cur_BPP3        (2<<0)
#define Cur_BPP4        (3<<0)

//DisplayRefresh
#define AC_DCVCOM       (1<<7)
#define WFMode0         (0<<4)
#define WFMode1         (1<<4)
#define WFMode2         (2<<4)
#define WFMode3         (3<<4)
#define WFMode4         (4<<4)
#define UPD_CPY_TO_PRE  (1<<3)
#define DN_EN           (1<<2)
#define Regal_EN_DIS    (0<<0)
#define Regal_EN_K      (1<<0)
#define Regal_EN_W      (2<<0)
#define Regal_EN_KW     (3<<0)

//PLLControl
#define OSC_RATE_SEL2_5 (0<<1)
#define OSC_RATE_SEL5   (1<<1)
#define OSC_RATE_SEL8   (2<<1)
#define OSC_RATE_SEL10  (3<<1)
#define OSC_RATE_SEL16  (4<<1)
#define OSC_RATE_SEL18  (5<<1)
#define OSC_RATE_SEL19  (6<<1)
#define OSC_RATE_SEL20  (7<<1)

//VcomAndDataIntervalSetting
#define VBD_CON         (1<<3)
#define VBD_OT_G0_G0    (0<<1)
#define VBD_OT_G0_G15   (2<<1)
#define VBD_OT_G15_G0   (3<<1)
#define VBD_OT_G15_G15  (4<<1)
#define DDX             (1<<0)
#define CDI             (4)     //Vcom data interval: 0h=2hsync -> Fh=32hsync, step=2
#define DCI             (0)     //Data to Vcom interval: 0h=1hsync -> Fh=16hsync, step=1

//AutoMeasurementVcom
#define VCM_EN          (1<<6)
#define AMVT3s          (0<<4)
#define AMVT5s          (1<<4)
#define AMVT6s          (2<<4)
#define AMVT10s         (3<<4)
#define AMVX            (1<<3)
#define AMVS            (1<<2)
#define AMV             (1<<1)
#define AMVE            (1<<0)

//GDOrderSetting
#define VBD_EN_SEL      (1<<3)
#define GDOS_M0         (0<<0)
#define GDOS_M1         (1<<0)
#define GDOS_M2         (2<<0)
#define GDOS_M3         (3<<0)
#define GDOS_M4         (4<<0)
#define GDOS_M5         (5<<0)
#define GDOS_M6         (6<<0)
#define GDOS_M7         (7<<0)
#define VBD_FN          (0)     //VBorder frame number seting: 0-VBD disabled, 1-VBD=8, ... 1Fh-VBD=248

#define PIXELS_PER_BYTE           8 // In 1bpp mode
#define IMAGE_BUFFER_COMMAND_SIZE 2 // One for data transmission start command, one for mode (1bpp)

/* Exported functions ------------------------------------------------------- */
EinkStatus_e EinkInitialize(void);
EinkStatus_e EinkDisplayImage(const EinkFrame_t *frame, uint8_t *buffer, EinkRefreshMode_e mode);
EinkStatus_e EinkClearScreen(void);
EinkStatus_e EinkRefreshImage(const EinkFrame_t *frame, EinkRefreshMode_e mode);
void EinkEnableFrontlight(bool enable);

#endif /* __ED028TC1_H */

/******************* (C) COPYRIGHT 2017 mudita *****END OF FILE****/
