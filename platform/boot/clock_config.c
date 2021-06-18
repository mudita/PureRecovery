/*
 * The Clear BSD License
 * Copyright 2017 NXP
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 *  that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "fsl_common.h"
#include "boot/clock_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* ARM PLL configuration for RUN mode */
const clock_arm_pll_config_t armPllConfig = {.loopDivider = 100U};

/* SYS PLL configuration for RUN mode */
const clock_sys_pll_config_t sysPllConfig = {.loopDivider = 1U};

/* USB1 PLL configuration for RUN mode */
const clock_usb_pll_config_t usb1PllConfig = {.loopDivider = 0U};

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* System clock frequency. */
extern uint32_t SystemCoreClock;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*******************************************************************************
 ************************ BOARD_InitBootClocks function ************************
 ******************************************************************************/
void BOARD_InitBootClocks(void)
{
    BOARD_BootClockRUN();
}

const clock_arm_pll_config_t armPllConfig_BOARD_BootClockRUN =
    {
        .loopDivider = 100,                       /* PLL loop divider, Fout = Fin * 50 */
        .src = 0,                                 /* Bypass clock source, 0 - OSC 24M, 1 - CLK1_P and CLK1_N */
    };
const clock_sys_pll_config_t sysPllConfig_BOARD_BootClockRUN =
    {
        .loopDivider = 1,                         /* PLL loop divider, Fout = Fin * ( 20 + loopDivider*2 + numerator / denominator ) */
        .numerator = 0,                           /* 30 bit numerator of fractional loop divider */
        .denominator = 1,                         /* 30 bit denominator of fractional loop divider */
        .src = 0,                                 /* Bypass clock source, 0 - OSC 24M, 1 - CLK1_P and CLK1_N */
    };
const clock_usb_pll_config_t usb1PllConfig_BOARD_BootClockRUN =
    {
        .loopDivider = 0,                         /* PLL loop divider, Fout = Fin * 20 */
        .src = 0,                                 /* Bypass clock source, 0 - OSC 24M, 1 - CLK1_P and CLK1_N */
    }; 

static void BOARD_BootClockGate(void)
{
    /* Disable all unused peripheral clock */
//    CCM->CCGR0 = 0x00C0000FU;
//    CCM->CCGR1 = 0x30000000U | (0x3<<12);
//    CCM->CCGR2 = 0xFF3F303FU;
//    CCM->CCGR3 = 0xF0000330U;
//    CCM->CCGR4 = 0x0000FF3CU;
//    CCM->CCGR5 = 0xF003330FU;
//    CCM->CCGR6 = 0x00FC0F00U;

    CCM->CCGR0 = 0xFFFFFFFFU;
    CCM->CCGR1 = 0xFFFFFFFFU;
    CCM->CCGR2 = 0xFFFFFFFFU;
    CCM->CCGR3 = 0xFFFFFFFFU;
    CCM->CCGR4 = 0xFFFFFFFFU;
    CCM->CCGR5 = 0xFFFFFFFFU;
    CCM->CCGR6 = 0xFFFFFFFFU;
}

#if 1
void BOARD_BootClockRUN(void)
{
    /* Boot ROM did initialize the XTAL, here we only sets external XTAL OSC freq */
    CLOCK_SetXtalFreq(24000000U);
    CLOCK_SetRtcXtalFreq(32768U);

    CLOCK_SetMux(kCLOCK_PeriphClk2Mux, 0x1); /* Set PERIPH_CLK2 MUX to OSC */
    CLOCK_SetMux(kCLOCK_PeriphMux, 0x1);     /* Set PERIPH_CLK MUX to PERIPH_CLK2 */

    /* Setting the VDD_SOC to 1.5V. It is necessary to config AHB to 600Mhz */
    DCDC->REG3 = (DCDC->REG3 & (~DCDC_REG3_TRG_MASK)) | DCDC_REG3_TRG(0x12);

    CLOCK_InitArmPll(&armPllConfig); /* Configure ARM PLL to 1200M */
#ifndef SKIP_SYSCLK_INIT
    CLOCK_InitSysPll(&sysPllConfig); /* Configure SYS PLL to 528M */
#endif
#ifndef XIP_EXTERNAL_FLASH
    CLOCK_InitUsb1Pll(&usb1PllConfig); /* Configure USB1 PLL to 480M */
#endif
    CLOCK_SetDiv(kCLOCK_ArmDiv, 0x1); /* Set ARM PODF to 0, divide by 2 */
    CLOCK_SetDiv(kCLOCK_AhbDiv, 0x0); /* Set AHB PODF to 0, divide by 1 */
    CLOCK_SetDiv(kCLOCK_IpgDiv, 0x3); /* Set IPG PODF to 3, divede by 4 */

    CLOCK_SetMux(kCLOCK_PrePeriphMux, 0x3); /* Set PRE_PERIPH_CLK to PLL1, 1200M */
    CLOCK_SetMux(kCLOCK_PeriphMux, 0x0);    /* Set PERIPH_CLK MUX to PRE_PERIPH_CLK */
    //CLOCK_SetMux(kCLOCK_PerclkMux, 0x0);

    /* Disable unused clock */
    BOARD_BootClockGate();

    /* Power down all unused PLL */
    CLOCK_DeinitAudioPll();
    CLOCK_DeinitVideoPll();
    CLOCK_DeinitEnetPll();
    CLOCK_DeinitUsb2Pll();

    /* Configure UART divider to default */
    CLOCK_SetMux(kCLOCK_UartMux, 1); /* Set UART source to OSC */
    CLOCK_SetDiv(kCLOCK_UartDiv, 0); /* Set UART divider to 1 */

    /* Init System pfd2. */
    CLOCK_InitSysPfd(kCLOCK_Pfd2, 29);	

    /* Deinit system pfd0, pfd1, pfd3*/
    CLOCK_DeinitSysPfd(kCLOCK_Pfd0);
    CLOCK_DeinitSysPfd(kCLOCK_Pfd1);
    CLOCK_DeinitSysPfd(kCLOCK_Pfd3);

    /* Set SEMC_PODF. */
    CLOCK_SetDiv(kCLOCK_SemcDiv, 1);	//divide by 2
    /* Set Semc alt clock source. */
    CLOCK_SetMux(kCLOCK_SemcAltMux, 0);	//PLL2 PFD2
    /* Set Semc clock source. */
    CLOCK_SetMux(kCLOCK_SemcMux, 1);	//SEMC_ALT

#if 0
    /* Init System pfd2. */
    CLOCK_InitSysPfd(kCLOCK_Pfd2, 29);	//
    /* Set SEMC_PODF. */
    CLOCK_SetDiv(kCLOCK_SemcDiv, 1);	//divide by 2
    /* Set Semc alt clock source. */
    CLOCK_SetMux(kCLOCK_SemcAltMux, 0);	//PLL2 PFD2
    /* Set Semc clock source. */
    CLOCK_SetMux(kCLOCK_SemcMux, 1);	//SEMC_ALT

    /* Set AHB_PODF. */
    CLOCK_SetDiv(kCLOCK_AhbDiv, 3);
    /* Set IPG_PODF. */
    CLOCK_SetDiv(kCLOCK_IpgDiv, 1);
#else
    #ifndef SKIP_SYSCLK_INIT
        /* Set SEMC_PODF. */
        CLOCK_SetDiv(kCLOCK_SemcDiv, 3);//7
        /* Set Semc alt clock source. */
        CLOCK_SetMux(kCLOCK_SemcAltMux, 0);//0
        /* Set Semc clock source. */
        CLOCK_SetMux(kCLOCK_SemcMux, 0);//0
    #endif
#endif
    #if !(defined(XIP_EXTERNAL_FLASH) && (XIP_EXTERNAL_FLASH == 1))
    /* Init Usb1 PLL. */
    CLOCK_InitUsb1Pll(&usb1PllConfig_BOARD_BootClockRUN);
    /* Init Usb1 pfd0. */
    CLOCK_InitUsb1Pfd(kCLOCK_Pfd0, 33);
    /* Init Usb1 pfd1. */
    CLOCK_InitUsb1Pfd(kCLOCK_Pfd1, 16);
    /* Init Usb1 pfd2. */
    CLOCK_InitUsb1Pfd(kCLOCK_Pfd2, 17);
    /* Init Usb1 pfd3. */
    CLOCK_InitUsb1Pfd(kCLOCK_Pfd3, 19);
    /* Disable Usb1 PLL output for USBPHY1. */
    CCM_ANALOG->PLL_USB1 &= ~CCM_ANALOG_PLL_USB1_EN_USB_CLKS_MASK;
#endif 
    /* Set LPSPI_PODF. */
    CLOCK_SetDiv(kCLOCK_LpspiDiv, 7);
    /* Set Lpspi clock source. */
    CLOCK_SetMux(kCLOCK_LpspiMux, 3);	//1

    /* Update core clock */
    SystemCoreClockUpdate();
}
#else
/*
 Target.WriteU32(0x400D8030,0x00002001); //CCM_ANALOG->PLL_SYS
  //enable, DIV_SELECT=Fout=Fref*22
  Target.WriteU32(0x400D8100,0x001d0000); //CCM_ANALOG->PFD_528
  //PFD2_FRAC=29
  Target.WriteU32(0x400FC014,0x00010D40); //CCM->CBCDR
  //SEMC_PODF=div2, AHB_PODF=div4, IPG_PODF=div2, SEMC_CLK_SEL=alt
 */

#define BOARD_KEYBOARD_I2C_CLOCK_SOURCE_SELECT (0U)
#define BOARD_KEYBOARD_I2C_CLOCK_SOURCE_DIVIDER (5U)
#define BOARD_BOOTCLOCKRUN_CORE_CLOCK             516000000U  /*!< Core clock frequency: 516000000Hz */ 
/*******************************************************************************
 * Variables for BOARD_BootClockRUN configuration
 ******************************************************************************/
const clock_arm_pll_config_t armPllConfig_BOARD_BootClockRUN =
    {
        .loopDivider = 86,                       /* PLL loop divider, Fout = Fin * 43 */
        .src = 0,                                 /* Bypass clock source, 0 - OSC 24M, 1 - CLK1_P and CLK1_N */
    };
const clock_sys_pll_config_t sysPllConfig_BOARD_BootClockRUN =
    {
        .loopDivider = 1,                         /* PLL loop divider, Fout = Fin * ( 20 + loopDivider*2 + numerator / denominator ) */
        .numerator = 0,                           /* 30 bit numerator of fractional loop divider */
        .denominator = 1,                         /* 30 bit denominator of fractional loop divider */
        .src = 0,                                 /* Bypass clock source, 0 - OSC 24M, 1 - CLK1_P and CLK1_N */
    };
const clock_usb_pll_config_t usb1PllConfig_BOARD_BootClockRUN =
    {
        .loopDivider = 0,                         /* PLL loop divider, Fout = Fin * 20 */
        .src = 0,                                 /* Bypass clock source, 0 - OSC 24M, 1 - CLK1_P and CLK1_N */
    }; 
void BOARD_BootClockRUN(void)
{
    /* Init RTC OSC clock frequency. */
    CLOCK_SetRtcXtalFreq(32768U);
    /* Enable 1MHz clock output. */
    //XTALOSC24M->OSC_CONFIG2 |= XTALOSC24M_OSC_CONFIG2_ENABLE_1M_MASK;
    /* Use free 1MHz clock output. */
    //XTALOSC24M->OSC_CONFIG2 &= ~XTALOSC24M_OSC_CONFIG2_MUX_1M_MASK;
    /* Set XTAL 24MHz clock frequency. */
    CLOCK_SetXtalFreq(24000000U);
    /* Enable XTAL 24MHz clock source. */
    CLOCK_InitExternalClk(0);
    /* Switch clock source to external OSC. */
    CLOCK_SwitchOsc(kCLOCK_XtalOsc);

    /* Set Oscillator ready counter value. */
    CCM->CCR = (CCM->CCR & (~CCM_CCR_OSCNT_MASK)) | CCM_CCR_OSCNT(127);
    /* Setting PeriphClk2Mux and PeriphMux to provide stable clock before PLLs are initialed */
    CLOCK_SetMux(kCLOCK_PeriphClk2Mux, 1); /* Set PERIPH_CLK2 MUX to OSC */
    CLOCK_SetMux(kCLOCK_PeriphMux, 0);     /* Set PERIPH_CLK MUX to pre_periph_clk_sel */

    /* Setting the VDD_SOC to 1.15V. It is necessary to config AHB to 516Mhz. */
    DCDC->REG3 = (DCDC->REG3 & (~DCDC_REG3_TRG_MASK)) | DCDC_REG3_TRG(0xE);
    /* Waiting for DCDC_STS_DC_OK bit is asserted */
    while (DCDC_REG0_STS_DC_OK_MASK != (DCDC_REG0_STS_DC_OK_MASK & DCDC->REG0))
    {
    }

    /* Init ARM PLL. */
    CLOCK_InitArmPll(&armPllConfig_BOARD_BootClockRUN);

    /* Init System PLL. */
    CLOCK_InitSysPll(&sysPllConfig_BOARD_BootClockRUN);
    /* Init System pfd0. */
    CLOCK_InitSysPfd(kCLOCK_Pfd0, 27);
    /* Init System pfd1. */
    CLOCK_DeinitSysPfd(kCLOCK_Pfd1);
    //CLOCK_InitSysPfd(kCLOCK_Pfd1, 16);
    /* Init System pfd2. */
    CLOCK_InitSysPfd(kCLOCK_Pfd2, 29);
    /* Init System pfd3. */
    CLOCK_DeinitSysPfd(kCLOCK_Pfd3);
    //CLOCK_InitSysPfd(kCLOCK_Pfd3, 32);
    /* Disable pfd offset. */
    CCM_ANALOG->PLL_SYS &= ~CCM_ANALOG_PLL_SYS_PFD_OFFSET_EN_MASK;

    /* Init Usb1 PLL. */
    //CLOCK_DeinitUsb1Pll();
    CLOCK_InitUsb1Pll(&usb1PllConfig_BOARD_BootClockRUN);
    /* Init Usb1 pfd0. */
    //CLOCK_DeinitUsb1Pfd(kCLOCK_Pfd0);
    CLOCK_InitUsb1Pfd(kCLOCK_Pfd0, 33);
    /* Init Usb1 pfd1. */
    //CLOCK_DeinitUsb1Pfd(kCLOCK_Pfd1);
    //CLOCK_InitUsb1Pfd(kCLOCK_Pfd1, 16);
    /* Init Usb1 pfd2. */
    //CLOCK_DeinitUsb1Pfd(kCLOCK_Pfd2);
    //CLOCK_InitUsb1Pfd(kCLOCK_Pfd2, 17);
    /* Init Usb1 pfd3. */
    //CLOCK_DeinitUsb1Pfd(kCLOCK_Pfd3);
    //CLOCK_InitUsb1Pfd(kCLOCK_Pfd3, 19);
    /* Disable Usb1 PLL output for USBPHY1. */
    CCM_ANALOG->PLL_USB1 &= ~CCM_ANALOG_PLL_USB1_EN_USB_CLKS_MASK;

    /* DeInit Audio PLL. */
    CLOCK_DeinitAudioPll();
#if 0
    /* Bypass Audio PLL. */
    CLOCK_SetPllBypass(CCM_ANALOG, kCLOCK_PllAudio, 1);
    /* Set divider for Audio PLL. */
    CCM_ANALOG->MISC2 &= ~CCM_ANALOG_MISC2_AUDIO_DIV_LSB_MASK;
    CCM_ANALOG->MISC2 &= ~CCM_ANALOG_MISC2_AUDIO_DIV_MSB_MASK;
    /* Enable Audio PLL output. */
    CCM_ANALOG->PLL_AUDIO |= CCM_ANALOG_PLL_AUDIO_ENABLE_MASK;
#endif
    /* DeInit Video PLL. */
    CLOCK_DeinitVideoPll();
    /* Bypass Video PLL. */
    //CCM_ANALOG->PLL_VIDEO |= CCM_ANALOG_PLL_VIDEO_BYPASS_MASK;
    /* Set divider for Video PLL. */
    //CCM_ANALOG->MISC2 = (CCM_ANALOG->MISC2 & (~CCM_ANALOG_MISC2_VIDEO_DIV_MASK)) | CCM_ANALOG_MISC2_VIDEO_DIV(0);
    /* Enable Video PLL output. */
    //CCM_ANALOG->PLL_VIDEO |= CCM_ANALOG_PLL_VIDEO_ENABLE_MASK;
    /* DeInit Enet PLL. */
    CLOCK_DeinitEnetPll();
    /* Bypass Enet PLL. */
    //CLOCK_SetPllBypass(CCM_ANALOG, kCLOCK_PllEnet, 1);
    /* Set Enet output divider. */
    //CCM_ANALOG->PLL_ENET = (CCM_ANALOG->PLL_ENET & (~CCM_ANALOG_PLL_ENET_DIV_SELECT_MASK)) | CCM_ANALOG_PLL_ENET_DIV_SELECT(1);
    /* Enable Enet output. */
    //CCM_ANALOG->PLL_ENET |= CCM_ANALOG_PLL_ENET_ENABLE_MASK;
    /* Enable Enet25M output. */
    //CCM_ANALOG->PLL_ENET |= CCM_ANALOG_PLL_ENET_ENET_25M_REF_EN_MASK;
    /* DeInit Usb2 PLL. */
    CLOCK_DeinitUsb2Pll();
    /* Bypass Usb2 PLL. */
    //CLOCK_SetPllBypass(CCM_ANALOG, kCLOCK_PllUsb2, 1);
    /* Enable Usb2 PLL output. */
    //CCM_ANALOG->PLL_USB2 |= CCM_ANALOG_PLL_USB2_ENABLE_MASK;
    /* Set AHB_PODF. */
    CLOCK_SetDiv(kCLOCK_AhbDiv, 0);
    /* Set IPG_PODF. */
    CLOCK_SetDiv(kCLOCK_IpgDiv, 3);
    /* Set ARM_PODF. */
    CLOCK_SetDiv(kCLOCK_ArmDiv, 1);
    /* Set preperiph clock source. */
    CLOCK_SetMux(kCLOCK_PrePeriphMux, 3);	//PLL1
    /* Set periph clock source. */
    CLOCK_SetMux(kCLOCK_PeriphMux, 0);	//pre_periph_clk_sel
    /* Set PERIPH_CLK2_PODF. */
    CLOCK_SetDiv(kCLOCK_PeriphClk2Div, 0);
    /* Set periph clock2 clock source. */
    CLOCK_SetMux(kCLOCK_PeriphClk2Mux, 0);	//PLL3
    /* Set PERCLK_PODF. */
    CLOCK_SetDiv(kCLOCK_PerclkDiv, 0);	// /1
    /* Set per clock source. */
    CLOCK_SetMux(kCLOCK_PerclkMux, 1);	//0 //OSC_24m
    /* Set USDHC1_PODF. */
    CLOCK_SetDiv(kCLOCK_Usdhc1Div, 2);	// /3
    /* Set Usdhc1 clock source. */
    CLOCK_SetMux(kCLOCK_Usdhc1Mux, 0);	//PLL2_PFD0
    CLOCK_DisableClock(kCLOCK_Usdhc2);
    /* Set USDHC2_PODF. */
    //CLOCK_SetDiv(kCLOCK_Usdhc2Div, 1);	// /2
    /* Set Usdhc2 clock source. */
    //CLOCK_SetMux(kCLOCK_Usdhc2Mux, 0);	//PLL2_PFD0
    CLOCK_DisableClock(kCLOCK_FlexSpi);
    /* Set FLEXSPI_PODF. */
    //CLOCK_SetDiv(kCLOCK_FlexspiDiv, 0);
    /* Set Flexspi clock source. */
    //CLOCK_SetMux(kCLOCK_FlexspiMux, 3);
    CLOCK_DisableClock(kCLOCK_Csi);
    /* Set CSI_PODF. */
    //CLOCK_SetDiv(kCLOCK_CsiDiv, 1);
    /* Set Csi clock source. */
    //CLOCK_SetMux(kCLOCK_CsiMux, 0);

    /* Set LPSPI_PODF. */
    CLOCK_SetDiv(kCLOCK_LpspiDiv, 7);
    /* Set Lpspi clock source. */
    CLOCK_SetMux(kCLOCK_LpspiMux, 3);	//1
    CLOCK_DisableClock(kCLOCK_Trace);
    /* Set TRACE_PODF. */
    //CLOCK_SetDiv(kCLOCK_TraceDiv, 2);
    /* Set Trace clock source. */
    //CLOCK_SetMux(kCLOCK_TraceMux, 2);
    /* Set SAI1_CLK_PRED. */
    CLOCK_SetDiv(kCLOCK_Sai1PreDiv, 1);
    /* Set SAI1_CLK_PODF. */
    CLOCK_SetDiv(kCLOCK_Sai1Div, 63);
    /* Set Sai1 clock source. */
    CLOCK_SetMux(kCLOCK_Sai1Mux, 2);
    /* Set SAI2_CLK_PRED. */
    CLOCK_SetDiv(kCLOCK_Sai2PreDiv, 1);
    /* Set SAI2_CLK_PODF. */
    CLOCK_SetDiv(kCLOCK_Sai2Div, 63);
    /* Set Sai2 clock source. */
    CLOCK_SetMux(kCLOCK_Sai2Mux, 2);
    CLOCK_DisableClock(kCLOCK_Sai3);
    /* Set SAI3_CLK_PRED. */
    //CLOCK_SetDiv(kCLOCK_Sai3PreDiv, 3);
    /* Set SAI3_CLK_PODF. */
    //CLOCK_SetDiv(kCLOCK_Sai3Div, 1);
    /* Set Sai3 clock source. */
    //CLOCK_SetMux(kCLOCK_Sai3Mux, 0);
    /* Set LPI2C_CLK_PODF. */
    CLOCK_SetDiv(kCLOCK_Lpi2cDiv, BOARD_KEYBOARD_I2C_CLOCK_SOURCE_DIVIDER);	//5
    /* Set Lpi2c clock source. */
    CLOCK_SetMux(kCLOCK_Lpi2cMux, BOARD_KEYBOARD_I2C_CLOCK_SOURCE_SELECT);	//0 1-OSC
    CLOCK_DisableClock(kCLOCK_Can1);
    CLOCK_DisableClock(kCLOCK_Can2);
    /* Set CAN_CLK_PODF. */
    //CLOCK_SetDiv(kCLOCK_CanDiv, 1);
    /* Set Can clock source. */
    CLOCK_SetMux(kCLOCK_CanMux, 0);
    /* Set UART_CLK_PODF. */
    CLOCK_SetDiv(kCLOCK_UartDiv, 0);	//0
    /* Set Uart clock source. */
    CLOCK_SetMux(kCLOCK_UartMux, 1);	//0-pll3_80m, 1-OSC
    /* Set LCDIF_PRED. */
    //CLOCK_SetDiv(kCLOCK_LcdifPreDiv, 1);
    /* Set LCDIF_CLK_PODF. */
    //CLOCK_SetDiv(kCLOCK_LcdifDiv, 3);
    /* Set Lcdif pre clock source. */
    //CLOCK_SetMux(kCLOCK_LcdifPreMux, 5);
    /* Set SPDIF0_CLK_PRED. */
    //CLOCK_SetDiv(kCLOCK_Spdif0PreDiv, 1);
    /* Set SPDIF0_CLK_PODF. */
    //CLOCK_SetDiv(kCLOCK_Spdif0Div, 7);
    /* Set Spdif clock source. */
    //CLOCK_SetMux(kCLOCK_SpdifMux, 3);
    /* Set FLEXIO1_CLK_PRED. */
    //CLOCK_SetDiv(kCLOCK_Flexio1PreDiv, 1);
    /* Set FLEXIO1_CLK_PODF. */
    //CLOCK_SetDiv(kCLOCK_Flexio1Div, 7);
    /* Set Flexio1 clock source. */
    //CLOCK_SetMux(kCLOCK_Flexio1Mux, 3);
    /* Set FLEXIO2_CLK_PRED. */
    //CLOCK_SetDiv(kCLOCK_Flexio2PreDiv, 1);
    /* Set FLEXIO2_CLK_PODF. */
    //CLOCK_SetDiv(kCLOCK_Flexio2Div, 7);
    /* Set Flexio2 clock source. */
    //CLOCK_SetMux(kCLOCK_Flexio2Mux, 3);
    /* Set Pll3 sw clock source. */
    //CLOCK_SetMux(kCLOCK_Pll3SwMux, 0);

    /* Set lvds1 clock source. */
    CCM_ANALOG->MISC1 = (CCM_ANALOG->MISC1 & (~CCM_ANALOG_MISC1_LVDS1_CLK_SEL_MASK)) | CCM_ANALOG_MISC1_LVDS1_CLK_SEL(7);	//set to VIDEO_PLL which is disabled
    /* Set clock out1 divider. */
    CCM->CCOSR = (CCM->CCOSR & (~CCM_CCOSR_CLKO1_DIV_MASK)) | CCM_CCOSR_CLKO1_DIV(0);
    /* Set clock out1 source. */
    CCM->CCOSR = (CCM->CCOSR & (~CCM_CCOSR_CLKO1_SEL_MASK)) | CCM_CCOSR_CLKO1_SEL(1);
    /* Set clock out2 divider. */
    CCM->CCOSR = (CCM->CCOSR & (~CCM_CCOSR_CLKO2_DIV_MASK)) | CCM_CCOSR_CLKO2_DIV(0);
    /* Set clock out2 source. */
    CCM->CCOSR = (CCM->CCOSR & (~CCM_CCOSR_CLKO2_SEL_MASK)) | CCM_CCOSR_CLKO2_SEL(18);
    /* Set clock out1 drives clock out1. */
    CCM->CCOSR &= ~CCM_CCOSR_CLK_OUT_SEL_MASK;
    /* Disable clock out1. */
    CCM->CCOSR &= ~CCM_CCOSR_CLKO1_EN_MASK;
    /* Disable clock out2. */
    CCM->CCOSR &= ~CCM_CCOSR_CLKO2_EN_MASK;
    /* Set SystemCoreClock variable. */
    SystemCoreClock = BOARD_BOOTCLOCKRUN_CORE_CLOCK;

    // TODO: Remeber about those disabled clocks

    // Disable CANs peripheral clock
    CLOCK_DisableClock(kCLOCK_Can1);
    CLOCK_DisableClock(kCLOCK_Can1S);
    CLOCK_DisableClock(kCLOCK_Can2);
    CLOCK_DisableClock(kCLOCK_Can2S);

    // Disable UARTs peripheral clock
//    CLOCK_DisableClock(kCLOCK_Lpuart1);
//    CLOCK_DisableClock(kCLOCK_Lpuart2);
//    CLOCK_DisableClock(kCLOCK_Lpuart3);
    CLOCK_DisableClock(kCLOCK_Lpuart4);
    CLOCK_DisableClock(kCLOCK_Lpuart5);
    CLOCK_DisableClock(kCLOCK_Lpuart6);
    CLOCK_DisableClock(kCLOCK_Lpuart7);
    CLOCK_DisableClock(kCLOCK_Lpuart8);

    // Disable SPIs peripheral clock
//    CLOCK_DisableClock(kCLOCK_Lpspi1);
    CLOCK_DisableClock(kCLOCK_Lpspi2);
    CLOCK_DisableClock(kCLOCK_Lpspi3);
    CLOCK_DisableClock(kCLOCK_Lpspi4);

    // Disable I2Cs peripheral clock
//    CLOCK_DisableClock(kCLOCK_Lpi2c1);
    CLOCK_DisableClock(kCLOCK_Lpi2c2);
    CLOCK_DisableClock(kCLOCK_Lpi2c3);
    CLOCK_DisableClock(kCLOCK_Lpi2c4);

    // Disable ADCs peripheral clock
//    CLOCK_DisableClock(kCLOCK_Adc1);
    CLOCK_DisableClock(kCLOCK_Adc2);

    // Disable General Purpose Timers peripheral clock
//    CLOCK_DisableClock(kCLOCK_Gpt1);
//    CLOCK_DisableClock(kCLOCK_Gpt1S);
    CLOCK_DisableClock(kCLOCK_Gpt2);
    CLOCK_DisableClock(kCLOCK_Gpt2S);

    // Disable Keypad peripheral clock
    CLOCK_DisableClock(kCLOCK_Kpp);

    // Disable Analog comparators peripheral clock
    CLOCK_DisableClock(kCLOCK_Acmp1);
    CLOCK_DisableClock(kCLOCK_Acmp2);
    CLOCK_DisableClock(kCLOCK_Acmp3);
    CLOCK_DisableClock(kCLOCK_Acmp4);

    // Disable Analog comparators peripheral clock
//    CLOCK_DisableClock(kCLOCK_Sai1);
//    CLOCK_DisableClock(kCLOCK_Sai2);
//    CLOCK_DisableClock(kCLOCK_Sai3);

    // Disable On-The_fly SW image decrypting engine peripheral clock
    CLOCK_DisableClock(kCLOCK_Bee);

    // Disable clock for the inter-peripheral event-triggering peripheral clocks
    CLOCK_DisableClock(kCLOCK_Xbar1);
    CLOCK_DisableClock(kCLOCK_Xbar2);
    CLOCK_DisableClock(kCLOCK_Xbar3);

    // Disable clock for the GPIO ports
//    CLOCK_DisableClock(kCLOCK_Gpio1);
//    CLOCK_DisableClock(kCLOCK_Gpio2);
//    CLOCK_DisableClock(kCLOCK_Gpio3);
//    CLOCK_DisableClock(kCLOCK_Gpio4);
//    CLOCK_DisableClock(kCLOCK_Gpio5);


    // Disable CSI peripheral clock
    CLOCK_DisableClock(kCLOCK_Csi);

    // Disable Touchscreen controller peripheral clock
    CLOCK_DisableClock(kCLOCK_Tsc);

    // Disable FlexPWM controller peripheral clock
    CLOCK_DisableClock(kCLOCK_Pwm1);
    CLOCK_DisableClock(kCLOCK_Pwm2);
    CLOCK_DisableClock(kCLOCK_Pwm3);
    CLOCK_DisableClock(kCLOCK_Pwm4);

    // Disable LCD display controller peripheral clock
    CLOCK_DisableClock(kCLOCK_Lcd);
    CLOCK_DisableClock(kCLOCK_LcdPixel);

    // Disable PXP peripheral clock
    CLOCK_DisableClock(kCLOCK_Pxp);

    // Disable Medium Audio Quality System peripheral clock
    CLOCK_DisableClock(kCLOCK_Mqs);

    // Disable timers
    CLOCK_DisableClock(kCLOCK_Timer1);
    CLOCK_DisableClock(kCLOCK_Timer2);
    CLOCK_DisableClock(kCLOCK_Timer3);
    CLOCK_DisableClock(kCLOCK_Timer4);

    // Disable True Random Number Generator clock
    CLOCK_DisableClock(kCLOCK_Trng);

    // Disable Quadrature decoder's clocks
    CLOCK_DisableClock(kCLOCK_Enc1);
    CLOCK_DisableClock(kCLOCK_Enc2);
    CLOCK_DisableClock(kCLOCK_Enc3);
    CLOCK_DisableClock(kCLOCK_Enc4);

    // Disable Ethernet clock
    CLOCK_DisableClock(kCLOCK_Enet);

    // Disable And-Or-Inverter modules clocks
    CLOCK_DisableClock(kCLOCK_Aoi1);
    CLOCK_DisableClock(kCLOCK_Aoi1);
} 
#endif