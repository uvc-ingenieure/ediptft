/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013 UVC Ingenieure http://uvc-ingenieure.de/
 * Author: Max Holtzberg <mholtzberg@uvc-ingenieure.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <eatft.h>

void eatft_terminal_enable(struct eatft *tft, bool enable)
{
    eatft_appendf(tft, PSTR("T%c"), enable ? 'E' : 'A');
}

void eatft_touch_enable(struct eatft *tft, uint8_t enable)
{
    eatft_appendf(tft, PSTR("AA%c"), !!enable);
}

void eatft_touch_beep(struct eatft *tft, bool enable)
{
    eatft_appendf(tft, PSTR("AS%c"), enable);
}

void eatft_info(struct eatft *tft)
{
    eatft_appendf(tft, PSTR("T%c"), 'I');
}

void eatft_button_setfont(struct eatft *tft, uint8_t font)
{
    eatft_appendf(tft, PSTR("AF%c"), font);
}

void eatft_button_setfontzoom(struct eatft *tft, uint8_t factor)
{
    eatft_appendf(tft, PSTR("AZ%c%c"), factor, factor);
}

void eatft_button_setfontcolor(struct eatft *tft, uint8_t norm, uint8_t sel)
{
    eatft_appendf(tft, PSTR("FA%c%c"), norm, sel);
}

void eatft_button_setoffset(struct eatft *tft, uint8_t x, uint8_t y)
{
    eatft_appendf(tft, PSTR("AO%c%c"), x, y);
}

void eatft_button_setframecolor(struct eatft *tft,
                                uint8_t n1, uint8_t n2, uint8_t n3,
                                uint8_t s1, uint8_t s2, uint8_t s3)
{
    eatft_appendf(tft, PSTR("FE%c%c%c%c%c%c"),
                n1, n2, n3,
                s1, s2, s3);
}

void eatft_button_createi(struct eatft *tft, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height,
                          uint8_t downcode, uint8_t upcode,
                          enum eatft_align align, const char *text)
{
    eatft_appendf(tft, PSTR("AT%D%D%D%D%c%c%c%s"),
                  x + CONFIG_EATFT_MARGIN_X,
                  y + CONFIG_EATFT_MARGIN_Y,
                  x + width - CONFIG_EATFT_MARGIN_X * 2,
                  y + height - CONFIG_EATFT_MARGIN_Y * 2,
                  downcode, upcode,
                  align,
                  text);
}

void eatft_button_vcreatei(struct eatft *tft, uint16_t x, uint16_t y,
                           uint16_t width, uint16_t height,
                           uint8_t downcode, uint8_t upcode,
                           enum eatft_align align, const char *fmt, va_list args)
{
    char text[32];
    vsprintf(text, fmt, args);
    eatft_button_createi(tft, x, y, width, height, downcode, upcode, align, text);
}

void eatft_button_setframe(struct eatft *tft, uint8_t n1, uint8_t angle)
{
    eatft_appendf(tft, PSTR("AE%c%c"), n1, angle);
}

void eatft_button_remove(struct eatft *tft, uint8_t code)
{
    eatft_appendf(tft, PSTR("AL%c\x01"), code);
}

void eatft_switch_createi(struct eatft *tft, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height,
                          uint8_t downcode, uint8_t upcode,
                          enum eatft_align align, const char *text)
{
    eatft_appendf(tft, PSTR("AK%D%D%D%D%c%c%c%s"),
                  x + CONFIG_EATFT_MARGIN_X,
                  y + CONFIG_EATFT_MARGIN_Y,
                  x + width - CONFIG_EATFT_MARGIN_X * 2,
                  y + height - CONFIG_EATFT_MARGIN_Y * 2,
                  downcode, upcode,
                  align,
                  text);
}

void eatft_switch_vcreatei(struct eatft *tft, uint16_t x, uint16_t y,
                           uint16_t width, uint16_t height,
                           uint8_t downcode, uint8_t upcode,
                           enum eatft_align align,
                           const char *fmt, va_list args)
{
    char text[32];
    vsprintf(text, fmt, args);
    eatft_switch_createi(tft, x, y, width, height, downcode, upcode,
                         align, text);
}

void eatft_switch_set(struct eatft *tft, uint16_t code, bool enable)
{
    eatft_appendf(tft, PSTR("AP%c%c"), code, enable);
}

void eatft_radio_group(struct eatft *tft, uint8_t group)
{
    eatft_appendf(tft, PSTR("AR%c"), group);
}

void eatft_touch_areai(struct eatft *tft, uint16_t x, uint16_t y,
                       uint16_t width, uint16_t height)
{
    eatft_appendf(tft, PSTR("AH%D%D%D%D"),
                  x, y,
                  x + width, y + height);
}

void eatft_touch_arear(struct eatft *tft, const struct eatft_rect *rect)
{
    struct eatft_rect _rect;
    memcpy_P(&_rect, rect, sizeof(_rect));
    eatft_touch_areai(tft, _rect.x, _rect.y, _rect.width, _rect.height);
}

void eatft_touch_area_removei(struct eatft *tft, uint16_t x, uint16_t y,
                              uint16_t width, uint16_t height)
{
    eatft_appendf(tft, PSTR("AV%D%D\x01"),
                  x + width / 2,
                  y + height / 2);

}

void eatft_touch_area_remover(struct eatft *tft, const struct eatft_rect *rect)
{
    struct eatft_rect _rect;
    memcpy_P(&_rect, rect, sizeof(_rect));
    eatft_touch_area_removei(tft, _rect.x, _rect.y, _rect.width, _rect.height);
}

void eatft_frame_setcolor(struct eatft *tft, uint8_t inner, uint8_t outer,
                          uint8_t fill)
{
    eatft_appendf(tft, PSTR("FR%c%c%c"), outer, inner, fill);
}

void eatft_frame_drawi(struct eatft *tft, uint16_t x, uint16_t y,
                       uint16_t width, uint16_t height)
{
    eatft_appendf(tft, PSTR("RR%D%D%D%D"), x, y, x + width, y + height);
}

void eatft_frame_drawr(struct eatft *tft, const struct eatft_rect *rect)
{
    struct eatft_rect _rect;
    memcpy_P(&_rect, rect, sizeof(_rect));
    eatft_frame_drawi(tft, _rect.x, _rect.y, _rect.width, _rect.height);
}

void eatft_rect_drawi(struct eatft *tft, uint16_t x, uint16_t y,
                      uint16_t width, uint16_t height)
{
    eatft_appendf(tft, PSTR("GR%D%D%D%D"), x, y, x + width, y + height);
}

void eatft_rect_drawr(struct eatft *tft, const struct eatft_rect *rect)
{
    struct eatft_rect _rect;
    memcpy_P(&_rect, rect, sizeof(_rect));
    eatft_rect_drawi(tft, _rect.x, _rect.y, _rect.width, _rect.height);
}

void eatft_rect_cleari(struct eatft *tft, uint16_t x, uint16_t y,
                       uint16_t width, uint16_t height)
{
    eatft_appendf(tft, PSTR("RL%D%D%D%D"), x, y, x + width, y + height);
}

void eatft_rect_clearr(struct eatft *tft, const struct eatft_rect *rect)
{
    struct eatft_rect _rect;
    memcpy_P(&_rect, rect, sizeof(_rect));
    eatft_rect_cleari(tft, _rect.x, _rect.y, _rect.width, _rect.height);
}

void eatft_rect_filli(struct eatft *tft, uint16_t x, uint16_t y,
                      uint16_t width, uint16_t height,
                      uint8_t color)
{
    eatft_appendf(tft, PSTR("RF%D%D%D%D%c"),
                  x, y,
                  x + width, y + height,
                  color);
}

void eatft_rect_fillr(struct eatft *tft, const struct eatft_rect *rect,
                      uint8_t color)
{
    struct eatft_rect _rect;
    memcpy_P(&_rect, rect, sizeof(_rect));
    eatft_rect_filli(tft, _rect.x, _rect.y, _rect.width, _rect.height, color);
}


void eatft_line_setwidth(struct eatft *tft, uint8_t width)
{
    eatft_appendf(tft, PSTR("GZ%c%c"), width, width);
}

void eatft_line_drawp(struct eatft *tft, const struct eatft_point *p1,
                      const struct eatft_point *p2)
{
    struct eatft_point _p1;
    struct eatft_point _p2;
    memcpy_P(&_p1, p1, sizeof(_p1));
    memcpy_P(&_p2, p2, sizeof(_p2));
    eatft_line_drawi(tft, _p1.x, _p1.y, _p2.x, _p2.y);
}

void eatft_setfontcolor(struct eatft *tft, uint8_t fore, uint8_t back)
{
    eatft_appendf(tft, PSTR("FZ%c%c"), fore, back);
}

void eatft_setfont(struct eatft *tft, uint8_t font)
{
    eatft_appendf(tft, PSTR("ZF%c"), font);
}

void eatft_text_draw(struct eatft *tft, uint16_t x, uint16_t y, char align,
                     const char *text)
{
    eatft_appendf(tft, PSTR("Z%c%D%D%s"), align, x, y, text);
}

void eatft_text_drawi(struct eatft *tft, uint16_t x, uint16_t y,
                      uint16_t width, uint16_t height, enum eatft_text_pos pos,
                      const char *text)
{
    eatft_appendf(tft, PSTR("ZB%D%D%D%D%c%s"),
                  x + CONFIG_EATFT_MARGIN_X,
                  y + CONFIG_EATFT_MARGIN_Y,
                  x + width - CONFIG_EATFT_MARGIN_X * 2,
                  y + height - CONFIG_EATFT_MARGIN_Y * 2,
                  pos, text);
}

/* NOTE: >=V1.2 only */
void eatft_text_drawr(struct eatft *tft, const struct eatft_rect *rect,
                      enum eatft_text_pos pos, const char *fmt, ...)
{
    struct eatft_rect _rect;
    va_list args;

    char fmtbuf[32];
    char text[32];

    strcpy_P(fmtbuf, fmt);

    va_start(args, fmt);
    vsprintf(text, fmtbuf, args);
    va_end(args);

    memcpy_P(&_rect, rect, sizeof(_rect));
    eatft_text_drawi(tft, _rect.x, _rect.y,
                     _rect.width, _rect.height,
                     pos, text);
}


void eatft_clear(struct eatft *tft)
{
    eatft_appendf(tft, PSTR("DL"));
}

void eatft_color_set(struct eatft *tft, uint8_t fg, uint8_t bg)
{
    eatft_appendf(tft, PSTR("FD%c%c"), fg, bg);
}

void eatft_line_drawi(struct eatft *tft, uint16_t x1, uint16_t y1,
                      uint16_t x2, uint16_t y2)
{
    eatft_appendf(tft, PSTR("GD%D%D%D%D"), x1, y1, x2, y2);
}

void eatft_register_user_action(struct eatft *tft, void (*action)(struct eatft*))
{
    tft->user_action = action;
}

void eatft_init(struct eatft *tft)
{
    memset(tft->widgets, 0, sizeof(tft->widgets));
    eatft_reset_buffer(tft);
    eatft_wdt_window_clear(tft);
}
