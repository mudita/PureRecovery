#include <hal/console.h>
#include <fsl_common.h>
#include <fsl_lpuart.h>
#include <SEGGER_RTT.h>


#ifndef CONSOLE_UART_DEVICE
#   define CONSOLE_UART_DEVICE LPUART3
#endif

#ifndef CONSOLE_UART_BAUDRATE
#   define CONSOLE_UART_BAUDRATE 115200
#endif

static lpuart_handle_t lp_uart_handle;

/* Get debug console frequency. */
static uint32_t console_src_freq(void)
{
    uint32_t freq;
    /* To make it simple, we assume default PLL and divider settings, and the only variable
       from application is use PLL3 source or OSC source */
    if (CLOCK_GetMux(kCLOCK_UartMux) == 0) /* PLL3 div6 80M */
    {
        freq = (CLOCK_GetPllFreq(kCLOCK_PllUsb1) / 6U) / (CLOCK_GetDiv(kCLOCK_UartDiv) + 1U);
    }
    else
    {
        freq = CLOCK_GetOscFreq() / (CLOCK_GetDiv(kCLOCK_UartDiv) + 1U);
    }
    return freq;
} 



/** Initialize debug console 
 * @return Error code 
*/
int debug_console_init(void)
{
     /* The user initialization should be placed here */
    lpuart_config_t lpuart_config;

    LPUART_Deinit(CONSOLE_UART_DEVICE);
    LPUART_SoftwareReset(CONSOLE_UART_DEVICE);
    LPUART_GetDefaultConfig(&lpuart_config);
    lpuart_config.baudRate_Bps = CONSOLE_UART_BAUDRATE;
    lpuart_config.enableTx = true;
    lpuart_config.enableRx = false;

    LPUART_TransferCreateHandle(CONSOLE_UART_DEVICE, &lp_uart_handle, NULL, NULL);

    LPUART_Init(CONSOLE_UART_DEVICE, &lpuart_config, console_src_freq());
    LPUART_EnableTx(CONSOLE_UART_DEVICE, true);
    LPUART_EnableRx(CONSOLE_UART_DEVICE, false);
    return 0;
}

/** Log to the debug output channels
 */
int debug_console_write(const char *buffer, size_t len)
{
    SEGGER_RTT_Write(0,buffer,len);
    LPUART_WriteBlocking(CONSOLE_UART_DEVICE, (uint8_t*) buffer, len);
    return len;
}

