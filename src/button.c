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
#include <stdlib.h>
#include <eatft.h>

#define CONFIG_EATFT_BUTTON_HEIGHT 50

struct eatft_widget *eatft_wdt_button_vcreatei(
    struct eatft *tft, uint16_t x, uint16_t y, uint16_t width, uint16_t height,
    eatft_callback_t callback, void *priv, enum eatft_align align,
    const char *fmt, va_list args)
{
    struct eatft_widget *wdt = NULL;
    int i;

    /* search buttons from front to back */
    for (i = 0; i < CONFIG_EATFT_MAX_WIDGETS; i++) {
        if (tft->widgets[i].fun == NULL) {
            wdt = &tft->widgets[i];
            break;
        }
    }

    DEBUG_ASSERT(wdt);

    if (wdt) {
        wdt->fun = callback;
        wdt->priv = priv;
        /* event 0 means disabled, so we start from 1 */
        i++;
        eatft_button_vcreatei(tft, x, y, width, height,
                              i | 0x80, i, align, fmt, args);
        eatft_flush(tft);
    }

    return wdt;
}

struct eatft_widget *eatft_wdt_button_createi(
    struct eatft *tft, uint16_t x, uint16_t y, uint16_t width, uint16_t height,
    eatft_callback_t callback, void *priv, enum eatft_align align,
    const char *fmt, ...)
{
    struct eatft_widget *wdt;
    char fmtbuf[32];
    va_list args;

    strcpy_P(fmtbuf, fmt);
    va_start(args, fmt);
    wdt = eatft_wdt_button_vcreatei(tft, x, y, width, height, callback, priv,
                                    align, fmtbuf, args);
    va_end(args);
    return wdt;
}

struct eatft_widget *eatft_wdt_button_creater(
    struct eatft *tft, const struct eatft_rect *rect, eatft_callback_t callback,
    void *priv, enum eatft_align align, const char *fmt, ...)
{
    struct eatft_rect r;
    struct eatft_widget *wdt;
    char fmtbuf[32];
    va_list args;

    memcpy_P(&r, rect, sizeof(r));
    strcpy_P(fmtbuf, fmt);

    va_start(args, fmt);
    wdt = eatft_wdt_button_vcreatei(tft, r.x, r.y, r.width, r.height,
                                    callback, priv,
                                    align, fmtbuf, args);
    va_end(args);

    return wdt;
}

struct eatft_widget *eatft_wdt_button_createw(
    struct eatft *tft, eatft_callback_t callback, void *priv,
    enum eatft_align align, const char *fmt, ...)
{
    struct eatft_widget *wdt;
    char fmtbuf[32];
    va_list args;

    strcpy_P(fmtbuf, fmt);

    va_start(args, fmt);

    tft->window.y += CONFIG_EATFT_MARGIN_Y;
    wdt = eatft_wdt_button_vcreatei(
        tft,
        tft->window.x, tft->window.y,
        tft->window.width, CONFIG_EATFT_BUTTON_HEIGHT,
        callback, priv,
        align, fmtbuf, args);
    tft->window.y += CONFIG_EATFT_BUTTON_HEIGHT;

    va_end(args);

    return wdt;
}
