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
#include <eatft.h>

#include "test.h"

#define LABEL_FONT  3
#define BUTTON_FONT 1
#define BACKGROUND_COLOR EATFT_BLUE
#define FOREGROUND_COLOR EATFT_BLACK

#define LABEL_HEIGHT   120
#define WIDGETS_WIDTH  (500)
#define BUTTONS_HEIGHT (CONFIG_EATFT_HEIGHT - LABEL_HEIGHT)
#define WIDGETS_X      ((CONFIG_EATFT_WIDTH - WIDGETS_WIDTH) / 2)



static const struct eatft_rect label_rect PROGMEM = {
    .x = WIDGETS_X,
    .y = 0,
    .width = WIDGETS_WIDTH,
    .height = LABEL_HEIGHT
};

static const struct eatft_rect buttons_rect PROGMEM = {
    .x = WIDGETS_X,
    .y = LABEL_HEIGHT,
    .width = WIDGETS_WIDTH,
    .height = CONFIG_EATFT_HEIGHT - LABEL_HEIGHT
};

static void render_label(struct eatft *tft, const char *text)
{

    eatft_setfont(tft, LABEL_FONT);
    eatft_setfontcolor(tft,
                       FOREGROUND_COLOR,
                       BACKGROUND_COLOR);

    eatft_text_drawr(tft, &label_rect, EATFT_MID_CENTER, text);
    eatft_flush(tft);
}

static void button_clicked(struct eatft *tft, struct eatft_widget *widget,
                           bool down)
{
    int button = (int)widget->priv;
    char str[32];

    if (!down)
        return;

    sprintf(str, "Pushed %d", button);
    puts(str);
    render_label(tft, str);
}

static void render_buttons(struct eatft *tft)
{
    int i;
    char str[32];

    eatft_button_setframe(tft, 6, 0);
    eatft_button_setframecolor(tft,
                               EATFT_BLACK, EATFT_BLACK, EATFT_WHITE,
                               EATFT_BLACK, EATFT_BLACK, EATFT_GRAY);
    eatft_button_setfontcolor(
        tft,
        FOREGROUND_COLOR,
        FOREGROUND_COLOR);
    eatft_button_setfont(tft, BUTTON_FONT);
    eatft_button_setoffset(tft, 1, 2);

    eatft_flush(tft);

    eatft_wdt_window_set(tft, &buttons_rect);

    for (i = 0; i < 5; i++) {
        sprintf(str, "Button %d", i);

        eatft_wdt_button_createw(
            tft,
            button_clicked,     /* callback */
            (void*)i,           /* priv */
            EATFT_ALIGN_LEFT, str);

    }

    eatft_wdt_window_clear(tft);
}

void test_render(struct eatft *tft)
{
    /* initialize tft */
    eatft_terminal_enable(tft, false);
    eatft_color_set(tft, EATFT_WHITE, EATFT_WHITE);
    eatft_clear(tft);
    eatft_flush(tft);

    eatft_setfontcolor(tft, FOREGROUND_COLOR, BACKGROUND_COLOR);
    eatft_flush(tft);

    render_label(tft, "Push a button!");
    render_buttons(tft);
}
