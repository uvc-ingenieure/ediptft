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

#include <stdio.h>
#include <string.h>

#include "private.h"

static bool eatft_is_in_rect(const struct eatft_rect *rect, int x, int y)
{
    if (rect == NULL)
        return false;

    return (x >= rect->x)
        && (x <= (rect->x + rect->width))
        && (y >= rect->y)
        && (y <= (rect->y + rect->height));
}

static void eatft_touch_dispatch(struct eatft *tft)
{
    struct eatft_widget *wdt = NULL;
    bool down = false;
    uint16_t x, y;
    int i;

    /* for any odd reason, there are sometimes short packages */
    if (tft->ibuf[1] != 8)
        return;

    switch (tft->ibuf[5]) {
    case 0:
        down = false;
        break;
    case 1:
        down = true;
        break;
    case 2:
        /* down repead */
        return;
    }

    x = tft->ibuf[6] | tft->ibuf[7] << 8;
    y = tft->ibuf[8] | tft->ibuf[9] << 8;

    wdt = NULL;
    /* search areas from back to front */
    for (i = CONFIG_EATFT_MAX_WIDGETS - 1; i >= 0; i--) {
        if (eatft_is_in_rect(&tft->widgets[i].rect, x, y)) {
            wdt = &tft->widgets[i];
            break;
        }
    }

    if (wdt) {
        wdt->fun(tft, wdt, down);
    } else {
        dbg("WARNING: unregistered touch event\n");
    }
}

static void eatft_button_dispatch(struct eatft *tft)
{
    struct eatft_widget *wdt = NULL;
    bool down;
    uint8_t btn = tft->ibuf[5];

    down = btn & 0x80;
    btn = (btn & 0x7f) - 1;

    if (btn < CONFIG_EATFT_MAX_WIDGETS
        && (wdt = &tft->widgets[btn])->fun != NULL) {
        wdt->fun(tft, wdt, down);
    } else {
        dbg("WARNING: unregistered touch event\n");
    }
}

void eatft_dispatch_event(struct eatft *tft)
{
    switch (tft->ibuf[3]) {
    case 'A':
        /* touch button event */
        eatft_button_dispatch(tft);
        break;
    case 'H':
        /* free touch area pressed */
        eatft_touch_dispatch(tft);
        break;

    }

    /* propagate that a user action has happened */
    /* maybe for screensavers */
    if (tft->user_action)
        tft->user_action(tft);
}

struct eatft_widget *eatft_wdt_touch_createi(
    struct eatft *tft, uint16_t x, uint16_t y, uint16_t width, uint16_t height,
    eatft_callback_t callback, void *priv)
{
    struct eatft_widget *wdt = NULL;
    int i;

    for (i = CONFIG_EATFT_MAX_WIDGETS - 1; i >= 0; i--)
        if (tft->widgets[i].fun == NULL) {
            wdt = &tft->widgets[i];
            break;
        }

    DEBUG_ASSERT(wdt);

    if (wdt) {
        /* create touch area */
        eatft_touch_areai(tft, x, y, width, height);
        eatft_flush(tft);

        /* register callback */
        wdt->fun = callback;
        wdt->priv = priv;

        wdt->rect.x = x;
        wdt->rect.y = y;
        wdt->rect.width = width;
        wdt->rect.height = height;
    }

    return wdt;
}

struct eatft_widget *eatft_wdt_touch_creater(
    struct eatft *tft, const struct eatft_rect *rect, eatft_callback_t callback,
    void *priv)
{
    struct eatft_rect r;
    memcpy_P(&r, rect, sizeof(r));
    return eatft_wdt_touch_createi(tft, r.x, r.y, r.width, r.height, callback, priv);
}

void eatft_wdt_free(struct eatft *tft, struct eatft_widget *widget)
{
    struct eatft_rect *r;
    if (widget != NULL) {
        if (widget->rect.width == 0 && widget->rect.height == 0) {
            eatft_button_remove(tft, widget - tft->widgets + 1);
        } else {
            r = &widget->rect;
            eatft_touch_area_removei(tft, r->x, r->y, r->width, r->height);
        }

        /* mark slot as free */
        memset(widget, 0, sizeof(*widget));

        /* flush after freeing to prevent further callback calls */
        eatft_flush(tft);
    }
}

void eatft_wdt_window_set(struct eatft *tft, const struct eatft_rect *window)
{
    memcpy_P(&tft->window, window, sizeof(*window));
}

void eatft_wdt_window_clear(struct eatft *tft)
{
    memset(&tft->window, 0, sizeof(tft->window));
}
