/*
 * @file display.c
 * @author Lukasz Skrzypczak (lukasz.skrzypczak@mudita.com)
 * @date Oct 8, 2018
 * @brief Insert brief information about this file purpose.
 * @copyright Copyright (C) 2018 mudita.com.
 * @details More detailed information related to this code.
 */

#include <hal/display.h>
#include "fonts.h"
#include <boot/board.h>
#include <stdio.h>
#include <stdarg.h>

#define EINK_LINE_LEN (BOARD_EINK_DISPLAY_RES_X / 17 - 2)
#define EINK_MAX_LINES (BOARD_EINK_DISPLAY_RES_Y / 24)
#define IMAGE_BUFFER_SIZE ((BOARD_EINK_DISPLAY_RES_X * BOARD_EINK_DISPLAY_RES_Y) / PIXELS_PER_BYTE)
#define SCREEN_BUFFER_SIZE (IMAGE_BUFFER_COMMAND_SIZE + IMAGE_BUFFER_SIZE)

static uint8_t eink_bmp_buf[SCREEN_BUFFER_SIZE];
static char eink_text_buf[EINK_MAX_LINES][EINK_LINE_LEN + 1];
static const EinkFrame_t full_frame = {
        .x = 0,
        .y = 0,
        .w = BOARD_EINK_DISPLAY_RES_Y,
        .h = BOARD_EINK_DISPLAY_RES_X
};

/**
 * @brief  Draws a character on LCD.
 * @param  Xpos: Line where to display the character shape
 * @param  Ypos: Start column address
 * @param  c: Pointer to the character data
 */
static void draw_char(uint16_t xpos, uint16_t ypos, const uint8_t *c) {
    uint32_t i = 0, j = 0;
    uint16_t height, width;
    uint8_t offset;
    uint8_t *pchar;
    uint32_t line;
    const eink_font_t *fnt = eink_get_font();
    height = fnt->height;
    width = fnt->width;

    offset = 8 * ((width + 7) / 8) - width;

    for (i = 0; i < height; i++) {
        pchar = ((uint8_t *) c + (width + 7) / 8 * i);

        switch (((width + 7) / 8)) {

            case 1:
                line = pchar[0];
                break;

            case 2:
                line = (pchar[0] << 8) | pchar[1];
                break;

            case 3:
            default:
                line = (pchar[0] << 16) | (pchar[1] << 8) | pchar[2];
                break;
        }

        for (j = 0; j < width; j++) {
            size_t _byte = ((BOARD_EINK_DISPLAY_RES_X - xpos - j) * BOARD_EINK_DISPLAY_RES_Y +
                            (BOARD_EINK_DISPLAY_RES_Y - ypos)) / 8;
            size_t _bit = 7 - (((BOARD_EINK_DISPLAY_RES_X - xpos - j) * BOARD_EINK_DISPLAY_RES_Y +
                                (BOARD_EINK_DISPLAY_RES_Y - ypos)) % 8);
            if (line & (1 << (width - j + offset - 1))) {
                eink_bmp_buf[2 + _byte] &= ~(1 << _bit);
            } else {
                eink_bmp_buf[2 + _byte] |= (1 << _bit);
            }
        }
        ypos++;
    }
}

static void put_pixel(uint16_t x, uint16_t y, bool color) {
    const size_t byte = ((BOARD_EINK_DISPLAY_RES_X - x) * BOARD_EINK_DISPLAY_RES_Y + (BOARD_EINK_DISPLAY_RES_Y - y)) / 8;
    const size_t bit = 7 - (((BOARD_EINK_DISPLAY_RES_X - x) * BOARD_EINK_DISPLAY_RES_Y + (BOARD_EINK_DISPLAY_RES_Y - y)) % 8);
    if (color) {
        eink_bmp_buf[2 + byte] &= ~(1 << bit);
    }
    else {
        eink_bmp_buf[2 + byte] |= (1 << bit);
    }
}

/**
 * @brief  Displays one character in currently active layer.
 * @param  Xpos: Start column address
 * @param  Ypos: Line where to display the character shape.
 * @param  Ascii: Character ascii code
 *           This parameter must be a number between Min_Data = 0x20 and Max_Data = 0x7E
 */
void eink_display_char(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii) {
    const eink_font_t *fnt = eink_get_font();
    draw_char(Xpos, Ypos, &fnt->table[(Ascii - ' ') * fnt->height * ((fnt->width + 7) / 8)]);
}

/**
 * @brief  Displays characters in currently active layer.
 * @param  Xpos: X position (in pixel)
 * @param  Ypos: Y position (in pixel)
 * @param  Text: Pointer to string to display on LCD
 * @param  Mode: Display mode
 *          This parameter can be one of the following values:
 *            @arg  CENTER_MODE
 *            @arg  RIGHT_MODE
 *            @arg  LEFT_MODE
 */
void eink_display_string_at(uint16_t xpos, uint16_t ypos, const char *text, eink_align_mode_t mode) {
    uint16_t refcolumn = 1, i = 0;
    uint32_t size = 0, xsize = 0;
    const char *ptr = text;

    /* Get the text size */
    while (*ptr++) {
        size++;
    }

    const eink_font_t *fnt = eink_get_font();
    /* Characters number per line */
    xsize = (BOARD_EINK_DISPLAY_RES_X / fnt->width);

    switch (mode) {
        case EINK_CENTER_MODE: {
            refcolumn = xpos + ((xsize - size) * fnt->width) / 2;
            break;
        }
        case EINK_LEFT_MODE: {
            refcolumn = xpos;
            break;
        }
        case EINK_RIGHT_MODE: {
            refcolumn = -xpos + ((xsize - size) * fnt->width);
            break;
        }
        default: {
            refcolumn = xpos;
            break;
        }
    }

    /* Check that the Start column is located in the screen */
    if ((refcolumn < 1) || (refcolumn >= 0x8000)) {
        refcolumn = 1;
    }

    /* Send the string character by character on LCD */
    while ((*text != 0) & (((BOARD_EINK_DISPLAY_RES_X - (i * fnt->width)) & 0xFFFF) >= fnt->width)) {
        /* Display one character on LCD */
        eink_display_char(refcolumn, ypos, (uint8_t) *text);
        /* Decrement the column position by 16 */
        refcolumn += fnt->width;

        /* Point on the next character */
        text++;
        i++;
    }
}

EinkStatus_e eink_refresh_text(const EinkFrame_t *frame, EinkRefreshMode_e mode) {
    if (frame == NULL) {
        return EinkInitErr;
    }

    eink_bmp_buf[0] = EinkDataStartTransmission1;
    eink_bmp_buf[1] = 0;
    return EinkDisplayImage(frame, eink_bmp_buf, mode);
}

void eink_log(const char *text, bool flush) {
    if ((text == NULL) || (text[0] == 0)) {
        return;
    }

    /* Scroll text up */
    for (int i = (EINK_MAX_LINES - 1); i > 0; --i) {
        if (eink_text_buf[i - 1][0] == 0) {
            continue;
        }
        memset(eink_text_buf[i], ' ', EINK_LINE_LEN - 1);
        const size_t len = strlen(eink_text_buf[i - 1]);
        memcpy(eink_text_buf[i], eink_text_buf[i - 1], (len > EINK_LINE_LEN) ? EINK_LINE_LEN : len);
        eink_text_buf[i][EINK_LINE_LEN] = 0;
    }

    memset(eink_text_buf[0], ' ', EINK_LINE_LEN);
    strncpy(eink_text_buf[0], text, EINK_LINE_LEN);
    eink_text_buf[0][EINK_LINE_LEN] = '\0';
    if (flush) {
        eink_log_refresh();
    }
}

void eink_log_refresh() {
    const eink_font_t *fnt = eink_get_font();
    int i = 1;
    while ((eink_text_buf[i - 1][0] != 0) && (i < EINK_MAX_LINES)) {
        eink_display_string_at(fnt->width, BOARD_EINK_DISPLAY_RES_Y - (i * fnt->height), eink_text_buf[i - 1],
                               EINK_LEFT_MODE);
        i++;
    }

    eink_refresh_text(&full_frame, REFRESH_DEEP);
}

void eink_clear_log(void) {
    memset(eink_text_buf, 0x00, sizeof(eink_text_buf));
}

void eink_init(void) {
    int i;
    //fill white screen table
    eink_bmp_buf[0] = EinkDataStartTransmission1;
    eink_bmp_buf[1] = 0x00;
    for (i = 0; i < (480 * 600 / 8); i++) {
        eink_bmp_buf[2 + i] = 0xFF;
    }
    EinkInitialize();
}

void eink_log_printf(const char *fmt, ...) {
    char buf[EINK_LINE_LEN + 1];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    eink_log(buf, false);
    va_end(args);
}

void eink_write_rectangle(const EinkFrame_t *frame, bool color) {
    if (frame == NULL) {
        return;
    }

    for (uint32_t dy = 0; dy < frame->h; dy++) {
        for (uint32_t dx = 0; dx < frame->w; dx++) {
            put_pixel(BOARD_EINK_DISPLAY_RES_X - (frame->y + dy), frame->x + dx, color);
        }
    }
}

EinkStatus_e eink_display_image(const uint8_t *pixels, size_t size) {
    if ((pixels == NULL) || (size > sizeof(eink_bmp_buf))) {
        return EinkInitErr;
    }

    memcpy(eink_bmp_buf, pixels, size);
    return eink_refresh_text(&full_frame, REFRESH_DEEP);
}
