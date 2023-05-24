#include <hal/emmc.h>
#include <boot/board.h>
#include <fsl_mmc.h>
#include <fsl_sdmmc_host.h>
#include <errno.h>
#include <stdbool.h>

static mmc_card_t mmc_card;
static sdmmchost_t sdmmchost;
static bool init_ok;

#define BOARD_SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE (32U)
#define BOARD_SDMMC_HOST_CACHE_CONTROL kSDMMCHOST_CacheControlRWBuffer

#define BOARD_SDMMC_MMC_HOST_SUPPORT_HS200_FREQ (180000000U)
#define BOARD_SDMMC_MMC_VCC_SUPPLY  kMMC_VoltageWindows270to360
#define BOARD_SDMMC_MMC_VCCQ_SUPPLY kMMC_VoltageWindows270to360

AT_NONCACHEABLE_SECTION_ALIGN(uint32_t s_sdmmcHostDmaBuffer[BOARD_SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE],
                              SDMMCHOST_DMA_DESCRIPTOR_BUFFER_ALIGN_SIZE);

static void BOARD_MMC_Pin_Config(uint32_t freq)
{
}

/** Enable the clocks in the emmc card 
 * @return error
 */
void emmc_enable(void)
{
    /* Configure USDHC clock source and divider */
    CLOCK_SetDiv(kCLOCK_Usdhc2Div, 2); //bylo 2
    CLOCK_SetMux(kCLOCK_Usdhc2Mux, 0); // CSCMR1  (17) 0 - PLL2_PFD2, 1 - PLL2_PFD0
    CLOCK_EnableClock(kCLOCK_Usdhc2);
}

/** Initialize the EMMC card */
int emmc_init(void) 
{
    //config_emmc();
    /* Configure base eMMC parameters*/
    memset(&mmc_card, 0, sizeof(mmc_card));

    sdmmchost.dmaDesBuffer         = s_sdmmcHostDmaBuffer;
    sdmmchost.dmaDesBufferWordsNum = BOARD_SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE;
    sdmmchost.enableCacheControl   = BOARD_SDMMC_HOST_CACHE_CONTROL;
#if defined SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER && SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER
    s_host.cacheAlignBuffer     = s_sdmmcCacheLineAlignBuffer;
    s_host.cacheAlignBufferSize = BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE * 2U;
#endif

    mmc_card.host                                = &sdmmchost;
    mmc_card.host->hostController.base           = BOARD_MMC_HOST_BASEADDR;
    mmc_card.host->hostController.sourceClock_Hz = BOARD_MMC_HOST_CLK_FREQ;
    mmc_card.usrParam.ioStrength                 = BOARD_MMC_Pin_Config;
    mmc_card.usrParam.maxFreq                    = BOARD_SDMMC_MMC_HOST_SUPPORT_HS200_FREQ;
    mmc_card.hostVoltageWindowVCC  = BOARD_SDMMC_MMC_VCC_SUPPLY;
    mmc_card.hostVoltageWindowVCCQ = BOARD_SDMMC_MMC_VCCQ_SUPPLY;
    /* card detect type */
#if defined DEMO_SDCARD_POWER_CTRL_FUNCTION_EXIST
    g_sd.usrParam.pwr = &s_sdCardPwrCtrl;
#endif
    if (MMC_Init(&mmc_card) != kStatus_Success)
    {
        return -1;
    }
    init_ok = true;
    return 0;
}

/* Retrive the mmc card object structure
*/
struct _mmc_card* emmc_card()
{
    return (init_ok)?(&mmc_card):(NULL);
}
