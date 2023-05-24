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
  - {pin_num: L14, peripheral: LPUART1, signal: RX, pin_signal: GPIO_AD_B0_13, software_input_on: Disable, hysteresis_enable: Disable, pull_up_down_config: Pull_Down_100K_Ohm,
    pull_keeper_select: Keeper, pull_keeper_enable: Enable, open_drain: Disable, speed: MHZ_100, drive_strength: R0_6, slew_rate: Slow}
  - {pin_num: K14, peripheral: LPUART1, signal: TX, pin_signal: GPIO_AD_B0_12, software_input_on: Disable, hysteresis_enable: Disable, pull_up_down_config: Pull_Down_100K_Ohm,
    pull_keeper_select: Keeper, pull_keeper_enable: Enable, open_drain: Disable, speed: MHZ_100, drive_strength: R0_6, slew_rate: Slow}
  - {pin_num: K10, peripheral: KPP, signal: 'kpp_col, 4', pin_signal: GPIO_AD_B1_07, software_input_on: Disable, hysteresis_enable: Disable, pull_up_down_config: Pull_Down_100K_Ohm,
    pull_keeper_select: Keeper, pull_keeper_enable: Enable, open_drain: Disable, speed: MHZ_100, drive_strength: R0_6, slew_rate: Slow}
  - {pin_num: K12, peripheral: KPP, signal: 'kpp_col, 5', pin_signal: GPIO_AD_B1_05, software_input_on: Disable, hysteresis_enable: Disable, pull_up_down_config: Pull_Down_100K_Ohm,
    pull_keeper_select: Keeper, pull_keeper_enable: Enable, open_drain: Disable, speed: MHZ_100, drive_strength: R0_6, slew_rate: Slow}
  - {pin_num: M12, peripheral: KPP, signal: 'kpp_col, 6', pin_signal: GPIO_AD_B1_03, software_input_on: Disable, hysteresis_enable: Disable, pull_up_down_config: Pull_Down_100K_Ohm,
    pull_keeper_select: Keeper, pull_keeper_enable: Enable, open_drain: Disable, speed: MHZ_100, drive_strength: R0_6, slew_rate: Slow}
  - {pin_num: K11, peripheral: KPP, signal: 'kpp_col, 7', pin_signal: GPIO_AD_B1_01, software_input_on: Disable, hysteresis_enable: Disable, pull_up_down_config: Pull_Down_100K_Ohm,
    pull_keeper_select: Keeper, pull_keeper_enable: Enable, open_drain: Disable, speed: MHZ_100, drive_strength: R0_6, slew_rate: Slow}
  - {pin_num: J12, peripheral: KPP, signal: 'kpp_row, 4', pin_signal: GPIO_AD_B1_06, software_input_on: Disable, hysteresis_enable: Disable, pull_up_down_config: Pull_Up_47K_Ohm,
    pull_keeper_select: Pull, pull_keeper_enable: Enable, open_drain: Disable, speed: MHZ_100, drive_strength: R0_6, slew_rate: Slow}
  - {pin_num: L12, peripheral: KPP, signal: 'kpp_row, 5', pin_signal: GPIO_AD_B1_04, software_input_on: Disable, hysteresis_enable: Disable, pull_up_down_config: Pull_Up_47K_Ohm,
    pull_keeper_select: Pull, pull_keeper_enable: Enable, open_drain: Disable, speed: MHZ_100, drive_strength: R0_6, slew_rate: Slow}
  - {pin_num: L11, peripheral: KPP, signal: 'kpp_row, 6', pin_signal: GPIO_AD_B1_02, software_input_on: Disable, hysteresis_enable: Disable, pull_up_down_config: Pull_Up_47K_Ohm,
    pull_keeper_select: Pull, pull_keeper_enable: Enable, open_drain: Disable, speed: MHZ_100, drive_strength: R0_6, slew_rate: Slow}
  - {pin_num: J11, peripheral: KPP, signal: 'kpp_row, 7', pin_signal: GPIO_AD_B1_00, software_input_on: Disable, hysteresis_enable: Disable, pull_up_down_config: Pull_Up_47K_Ohm,
    pull_keeper_select: Pull, pull_keeper_enable: Enable, open_drain: Disable, speed: MHZ_100, drive_strength: R0_6, slew_rate: Slow}
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
  IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_00_KPP_ROW07, 0U); 
  IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_01_KPP_COL07, 0U); 
  IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_02_KPP_ROW06, 0U); 
  IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_03_KPP_COL06, 0U); 
  IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_04_KPP_ROW05, 0U); 
  IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_05_KPP_COL05, 0U); 
  IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_06_KPP_ROW04, 0U); 
  IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_07_KPP_COL04, 0U); 
  IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_12_LPUART1_TXD, 0x10B0U); 
  IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_13_LPUART1_RXD, 0x10B0U); 
  IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_00_KPP_ROW07, 0x70B0U); 
  IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_01_KPP_COL07, 0x10B0U); 
  IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_02_KPP_ROW06, 0x70B0U); 
  IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_03_KPP_COL06, 0x10B0U); 
  IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_04_KPP_ROW05, 0x70B0U); 
  IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_05_KPP_COL05, 0x10B0U); 
  IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_06_KPP_ROW04, 0x70B0U); 
  IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_07_KPP_COL04, 0x10B0U); 
}

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
