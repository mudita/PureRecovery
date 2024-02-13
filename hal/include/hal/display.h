/*
 * @file display.h
 * @author Lukasz Skrzypczak (lukasz.skrzypczak@mudita.com)
 * @date Oct 8, 2018
 * @brief Insert brief information about this file purpose.
 * @copyright Copyright (C) 2018 mudita.com.
 * @details More detailed information related to this code.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <hal/ED028TC1.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
  * @brief  LCD drawing Line alignment mode definitions
  */
typedef enum eink_align_mode {
    EINK_CENTER_MODE = 0x01, /*!< Center mode */
    EINK_RIGHT_MODE = 0x02,  /*!< Right mode  */
    EINK_LEFT_MODE = 0x03    /*!< Left mode   */
} eink_align_mode_t;

/** Initialize eink display
 */
void eink_init(void);

/** Display char at selected position
 * @param xpos X axis position
 * @param ypos Y axis position
 * @param ascii ascii char to display
 */
void eink_display_char(uint16_t xpos, uint16_t ypos, uint8_t ascii);

/** Display string beginning from position
 * @param xpos X starting position
 * @param ypos Y starting position
 * @param text Text for display
 * @param mode Alignment mode @see eink_align_mode
 */
void eink_display_string_at(uint16_t xpos, uint16_t ypos, const char *text, eink_align_mode_t mode);

/** Display refresh text at selected position
 * @param frame - pointer to struct describing image frame, i.e. x, y, width, height
 */
EinkStatus_e eink_refresh_text(const EinkFrame_t *frame, EinkRefreshMode_e mode);

/** Display to the eink like log console
 * @param text Text for display
 * @param flush whether an immediate refresh is required
 */
void eink_log(const char *text, bool flush);

/** Printf value using the eink and doesn't refresh
 * Use standard printf tool
 */
void eink_log_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

/** Flush log into the display
 */
void eink_log_refresh(void);

/** Clear eink log console
 */
void eink_clear_log(void);

/** Write rectangle to the display
 * @param frame - pointer to struct describing image frame, i.e. x, y, width, height
 * @param black - color of the rectangle to write
 */
void eink_write_rectangle(const EinkFrame_t *frame, bool black);

/** Write raw bitmap to the display
 * @param pixels - pixel data of the image in display's format
 * @param size - size of the image data
 * @return EinkOk if successful
 */
EinkStatus_e eink_display_image(const uint8_t *pixels, size_t size);

#ifdef __cplusplus
}
#endif