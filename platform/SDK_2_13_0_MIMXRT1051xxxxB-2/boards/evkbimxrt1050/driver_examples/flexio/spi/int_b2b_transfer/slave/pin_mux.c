/*
 * Copyright 2017-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Pins v9.0
processor: MIMXRT1052xxxxB
package_id: MIMXRT1052DVL6B
mcu_data: ksdk2_0
processor_version: 0.0.0
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "pin_mux.h"

/* FUNCTION ************************************************************************************************************
 * 
 * Function Name : BOARD_InitBootPins
 * Description   : Calls initialization functions.
 * 
 * END ****************************************************************************************************************/
void BOARD_InitBootPins(void) {
    BOARD_InitPins();
}

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitPins:
- options: {callFromInitBoot: 'true', coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: L14, peripheral: LPUART1, signal: RX, pin_signal: GPIO_AD_B0_13}
  - {pin_num: K14, peripheral: LPUART1, signal: TX, pin_signal: GPIO_AD_B0_12}
  - {pin_num: B8, peripheral: FLEXIO2, signal: 'IO, 05', pin_signal: GPIO_B0_05, direction: INPUT, software_input_on: Enable, slew_rate: Fast}
  - {pin_num: A8, peripheral: FLEXIO2, signal: 'IO, 06', pin_signal: GPIO_B0_06, direction: OUTPUT, slew_rate: Fast}
  - {pin_num: A9, peripheral: FLEXIO2, signal: 'IO, 07', pin_signal: GPIO_B0_07, direction: INPUT, software_input_on: Enable, slew_rate: Fast}
  - {pin_num: B9, peripheral: FLEXIO2, signal: 'IO, 08', pin_signal: GPIO_B0_08, direction: INPUT, software_input_on: Enable, slew_rate: Fast}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitPins(void) {
  CLOCK_EnableClock(kCLOCK_Iomuxc);           

  IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_12_LPUART1_TXD, 0U); 
  IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_13_LPUART1_RXD, 0U); 
  IOMUXC_SetPinMux(IOMUXC_GPIO_B0_05_FLEXIO2_D05, 1U); 
  IOMUXC_SetPinMux(IOMUXC_GPIO_B0_06_FLEXIO2_D06, 0U); 
  IOMUXC_SetPinMux(IOMUXC_GPIO_B0_07_FLEXIO2_D07, 1U); 
  IOMUXC_SetPinMux(IOMUXC_GPIO_B0_08_FLEXIO2_D08, 1U); 
  IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_05_FLEXIO2_D05, 0x10B1U); 
  IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_06_FLEXIO2_D06, 0x10B1U); 
  IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_07_FLEXIO2_D07, 0x10B1U); 
  IOMUXC_SetPinConfig(IOMUXC_GPIO_B0_08_FLEXIO2_D08, 0x10B1U); 
}

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
