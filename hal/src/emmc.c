#include <hal/emmc.h>
#include <boot/board.h>
#include "MIMXRT1051.h"
#include <errno.h>
#include <stdbool.h>

#include <drivers/fsl_iomuxc.h>
#include <hal/delay.h>

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
    mmc_card.enablePreDefinedBlockCount = false; // BIWIN eMMC doesn't work stable when this flag is true
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
        init_ok = true;
        NVIC_SetPriority(USDHC2_IRQn, USDHC_IRQ_PRIORITY);
        return kStatus_Success;
    }
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

