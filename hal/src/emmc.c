#include <hal/emmc.h>
#include <boot/board.h>
#include "MIMXRT1051.h"
#include <errno.h>
#include <stdbool.h>

#include <drivers/fsl_iomuxc.h>
#include <hal/delay.h>

//#define DEBUG_USDHC
#ifdef DEBUG_USDHC
#include <drivers/fsl_src.h>

#include "log.h"
#include "eink.h"

static void reportUSDHCRegisterSerial(void);
static void reportUSDHCRegistersEink(status_t error);
#endif

#define BOARD_SDMMC_MMC_HOST_SUPPORT_HS200_FREQ (180000000U)
#define DMA_BUFFER_WORD_SIZE                    (1024U)
#define USDHC_IRQ_PRIORITY                      (6U)

AT_NONCACHEABLE_SECTION_ALIGN(uint32_t s_sdmmc_hostDmaBuffer[DMA_BUFFER_WORD_SIZE], SDMMCHOST_DMA_DESCRIPTOR_BUFFER_ALIGN_SIZE);

static mmc_card_t mmc_card;
static sdmmchost_t mmc_host;
static bool init_ok;

static void emmc_pin_config(uint32_t freq);

void emmc_enable(void)
{
    CLOCK_SetDiv(kCLOCK_Usdhc2Div, 3); 
    CLOCK_SetMux(kCLOCK_Usdhc2Mux, 0); // 0 - PLL2_PFD2, 1 - PLL2_PFD0
    CLOCK_EnableClock(kCLOCK_Usdhc2);
}

status_t emmc_init(void) 
{
    memset(&mmc_card, 0, sizeof(mmc_card));
    memset(&mmc_host, 0, sizeof(mmc_host));
    mmc_card.host = &mmc_host;

    mmc_host.dmaDesBuffer         = s_sdmmc_hostDmaBuffer;
    mmc_host.dmaDesBufferWordsNum = DMA_BUFFER_WORD_SIZE;
    mmc_host.enableCacheControl   = kSDMMCHOST_CacheControlRWBuffer;
#if defined SDmmc_host_ENABLE_CACHE_LINE_ALIGN_TRANSFER && SDmmc_host_ENABLE_CACHE_LINE_ALIGN_TRANSFER
    mmc_host.cacheAlignBuffer     = s_sdmmcCacheLineAlignBuffer;
    mmc_host.cacheAlignBufferSize = BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE * 2U;
#endif

    mmc_card.busWidth = kMMC_DataBusWidth8bit;
    mmc_card.busTiming = kMMC_HighSpeed200Timing;
    mmc_card.enablePreDefinedBlockCount = true;
    mmc_card.host->hostController.base = BOARD_MMC_HOST_BASEADDR;
    mmc_card.host->hostController.sourceClock_Hz = BOARD_MMC_HOST_CLK_FREQ;
    mmc_card.usrParam.ioStrength                 = emmc_pin_config;
    mmc_card.usrParam.maxFreq                    = BOARD_SDMMC_MMC_HOST_SUPPORT_HS200_FREQ;
    mmc_card.hostVoltageWindowVCCQ = kMMC_VoltageWindow120;
    mmc_card.hostVoltageWindowVCC = kMMC_VoltageWindow170to195;
    /* card detect type */
#if defined DEMO_SDCARD_POWER_CTRL_FUNCTION_EXIST
    g_sd.usrParam.pwr = &s_sdCardPwrCtrl;
#endif

    const status_t status = MMC_Init(&mmc_card);
    if (status == kStatus_Success) {
#ifdef DEBUG_USDHC
        reportUSDHCRegisterSerial();
#endif
        init_ok = true;
        NVIC_SetPriority(USDHC2_IRQn, USDHC_IRQ_PRIORITY);
        return kStatus_Success;
    }

#ifdef DEBUG_USDHC
    reportUSDHCRegisterSerial();
    reportUSDHCRegistersEink(status);
#endif

    return status;    
}

static void emmc_pin_config(uint32_t freq)
{
    uint32_t speed = 0U, strength = 0U;

    if (freq <= 50000000)
    {
        speed    = 0U;
        strength = 7U;
    }
    else if (freq <= 100000000)
    {
        speed    = 2U;
        strength = 7U;
    }
    else
    {
        speed    = 3U;
        strength = 7U;
    }

    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_05_USDHC2_CMD,
                        IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |
                            IOMUXC_SW_PAD_CTL_PAD_DSE(strength));
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_04_USDHC2_CLK,
                        IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(0) |
                            IOMUXC_SW_PAD_CTL_PAD_DSE(strength));
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_03_USDHC2_DATA0,
                        IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |
                            IOMUXC_SW_PAD_CTL_PAD_DSE(strength));
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_02_USDHC2_DATA1,
                        IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |
                            IOMUXC_SW_PAD_CTL_PAD_DSE(strength));
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_01_USDHC2_DATA2,
                        IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |
                            IOMUXC_SW_PAD_CTL_PAD_DSE(strength));
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_00_USDHC2_DATA3,
                        IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |
                            IOMUXC_SW_PAD_CTL_PAD_DSE(strength));
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_08_USDHC2_DATA4,
                        IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |
                            IOMUXC_SW_PAD_CTL_PAD_DSE(strength));
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_09_USDHC2_DATA5,
                        IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |
                            IOMUXC_SW_PAD_CTL_PAD_DSE(strength));
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_10_USDHC2_DATA6,
                        IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |
                            IOMUXC_SW_PAD_CTL_PAD_DSE(strength));
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_11_USDHC2_DATA7,
                        IOMUXC_SW_PAD_CTL_PAD_SPEED(speed) | IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK |
                            IOMUXC_SW_PAD_CTL_PAD_HYS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(1) |
                            IOMUXC_SW_PAD_CTL_PAD_DSE(strength));
}

struct _mmc_card* emmc_card(void)
{
    return init_ok ? &mmc_card : NULL;
}

// Overwrite a weak function for getting msec
uint32_t OSA_TimeGetMsec(void)
{
    return get_jiffiess();
}

#ifdef DEBUG_USDHC
static void reportUSDHCRegisterSerial(void)
{
    const uint32_t clock_syspfd2 = CLOCK_GetFreq(kCLOCK_SysPllPfd2Clk);
    const uint32_t clock_usdhc = CLOCK_GetDiv(kCLOCK_Usdhc2Div);
    LOG(LOG_INFO, "CLOCK_SysPllPfd:%ld CLOCK_Usdhc2Div:%ld CLOCK_eMMC:%ld", clock_syspfd2, clock_usdhc, (clock_syspfd2 / (clock_usdhc + 1U)));
    LOG(LOG_INFO, "BOOTCFG1:0x%X BOOTCFG2:0x%X", SRC_GetBootModeWord1(SRC), SRC_GetBootModeWord2(SRC));
    LOG(LOG_INFO, "Boot:0x%X IntStat:0x%X ", USDHC2->MMC_BOOT, USDHC2->INT_STATUS);
    LOG(LOG_INFO, "IstaE:0x%X IsigE:0x%X ", USDHC2->INT_STATUS_EN, USDHC2->INT_SIGNAL_EN);
    LOG(LOG_INFO, "C12Err:0x%X AdmaErr:0x%X ", USDHC2->AUTOCMD12_ERR_STATUS, USDHC2->ADMA_ERR_STATUS);
    LOG(LOG_INFO, "TunSt:0x%X Tctrl:0x%X ", USDHC2->CLK_TUNE_CTRL_STATUS, USDHC2->TUNING_CTRL);
    LOG(LOG_INFO, "1:0x%X 2:0x%X ", USDHC2->DS_ADDR, USDHC2->BLK_ATT);
    LOG(LOG_INFO, "3:0x%X 4:0x%X ", USDHC2->CMD_ARG, USDHC2->CMD_XFR_TYP);
    LOG(LOG_INFO, "5:0x%X 6:0x%X ", USDHC2->CMD_RSP0, USDHC2->CMD_RSP1);
    LOG(LOG_INFO, "7:0x%X 8:0x%X ", USDHC2->CMD_RSP2, USDHC2->CMD_RSP3);
    LOG(LOG_INFO, "9:0x%X 10:0x%X ", USDHC2->DATA_BUFF_ACC_PORT, USDHC2->PRES_STATE);
    LOG(LOG_INFO, "11:0x%X 12:0x%X ", USDHC2->PROT_CTRL, USDHC2->SYS_CTRL);
    LOG(LOG_INFO, "17:0x%X 18:0x%X ", USDHC2->HOST_CTRL_CAP, USDHC2->WTMK_LVL);
    LOG(LOG_INFO, "19:0x%X 20:0x%X ", USDHC2->MIX_CTRL, USDHC2->FORCE_EVENT);
    LOG(LOG_INFO, "22:0x%X 23:0x%X ", USDHC2->ADMA_SYS_ADDR, USDHC2->DLL_CTRL);
    LOG(LOG_INFO, "24:0x%X 26:0x%X 28:0x%X", USDHC2->DLL_STATUS, USDHC2->VEND_SPEC, USDHC2->VEND_SPEC2);
}

static void reportUSDHCRegistersEink(status_t error) 
{
    char buff_[200] = {};
    eink_clear_log();
    snprintf(buff_, sizeof(buff_), "Error:%ld", error);
    eink_log_write(buff_, false, ALIGNMENT_CENTER, REFRESH_NONE);
    snprintf(buff_, sizeof(buff_), "BOOTCFG1:0x%X BOOTCFG2:0x%X ", SRC_GetBootModeWord1(SRC), SRC_GetBootModeWord2(SRC));
    eink_log_write(buff_, true, ALIGNMENT_CENTER, REFRESH_NONE);
    snprintf(buff_, sizeof(buff_), "Boot:0x%X IntStat:0x%X ", USDHC2->MMC_BOOT, USDHC2->INT_STATUS);
    eink_log_write(buff_, true, ALIGNMENT_CENTER, REFRESH_NONE);
    snprintf(buff_, sizeof(buff_), "IstaE:0x%X IsigE:0x%X ", USDHC2->INT_STATUS_EN, USDHC2->INT_SIGNAL_EN);
    eink_log_write(buff_, true, ALIGNMENT_CENTER, REFRESH_NONE);
    snprintf(buff_, sizeof(buff_), "C12Err:0x%X AdmaErr:0x%X ", USDHC2->AUTOCMD12_ERR_STATUS, USDHC2->ADMA_ERR_STATUS);
    eink_log_write(buff_, true, ALIGNMENT_CENTER, REFRESH_NONE);
    snprintf(buff_, sizeof(buff_), "TunSt:0x%X Tctrl:0x%X ", USDHC2->CLK_TUNE_CTRL_STATUS, USDHC2->TUNING_CTRL);
    eink_log_write(buff_, true, ALIGNMENT_CENTER, REFRESH_NONE);
    snprintf(buff_, sizeof(buff_), "1:0x%X 2:0x%X ", USDHC2->DS_ADDR, USDHC2->BLK_ATT);
    eink_log_write(buff_, true, ALIGNMENT_CENTER, REFRESH_NONE);
    snprintf(buff_, sizeof(buff_), "3:0x%X 4:0x%X ", USDHC2->CMD_ARG, USDHC2->CMD_XFR_TYP);
    eink_log_write(buff_, true, ALIGNMENT_CENTER, REFRESH_NONE);
    snprintf(buff_, sizeof(buff_), "9:0x%X 10:0x%X ", USDHC2->DATA_BUFF_ACC_PORT, USDHC2->PRES_STATE);
    eink_log_write(buff_, true, ALIGNMENT_CENTER, REFRESH_NONE);
    snprintf(buff_, sizeof(buff_), "11:0x%X 12:0x%X ", USDHC2->PROT_CTRL, USDHC2->SYS_CTRL);
    eink_log_write(buff_, true, ALIGNMENT_CENTER, REFRESH_NONE);
    snprintf(buff_, sizeof(buff_), "17:0x%X 18:0x%X ", USDHC2->HOST_CTRL_CAP, USDHC2->WTMK_LVL);
    eink_log_write(buff_, true, ALIGNMENT_CENTER, REFRESH_NONE);
    snprintf(buff_, sizeof(buff_), "19:0x%X 20:0x%X ", USDHC2->MIX_CTRL, USDHC2->FORCE_EVENT);
    eink_log_write(buff_, true, ALIGNMENT_CENTER, REFRESH_NONE);
    snprintf(buff_, sizeof(buff_), "22:0x%X 23:0x%X ", USDHC2->ADMA_SYS_ADDR, USDHC2->DLL_CTRL);
    eink_log_write(buff_, true, ALIGNMENT_CENTER, REFRESH_NONE);
    snprintf(buff_, sizeof(buff_), "24:0x%X 26:0x%X 28:0x%X", USDHC2->DLL_STATUS, USDHC2->VEND_SPEC, USDHC2->VEND_SPEC2);
    eink_log_write(buff_, true, ALIGNMENT_CENTER, REFRESH_DEEP);
}
#endif

