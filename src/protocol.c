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

#include "private.h"

/* sets defaults for sending commands without reponse */
void eatft_reset_buffer(struct eatft *tft)
{
    tft->dc = EATFT_DC1;
    tft->olen = 0;
    tft->bcc = 0;
    tft->state = EATFT_READY;
    tft->next_state = EATFT_RESET;
}

bool eatft_chk_matches(struct eatft *tft)
{
    int i = 0;
    uint8_t chk = 0;

    for (i = 0; i < tft->ibuf[1] + 2; i++) {
        chk += tft->ibuf[i];
    }
    return chk == tft->ibuf[i];
}

void eatft_poll(struct eatft *tft)
{
    tft->obuf[0] = 'S';
    tft->bcc += 'S';
    tft->olen = 1;

    /* overwrite defaults */
    tft->next_state = EATFT_RECEIVE;
    tft->dc = EATFT_DC2;

    eatft_flush(tft);
}

void eatft_flush(struct eatft *tft)
{
    while (tft->state != EATFT_READY) {
        eatft_process(tft);
    }

    DEBUG_ASSERT((tft->olen - 3) <= CONFIG_EATFT_OBUF_SIZE);

    if (tft->olen > 0) {

        tft->bcc += tft->dc;
        tft->bcc += tft->olen;

        /* append checksum */
        tft->obuf[tft->olen] = tft->bcc;

        /* schedule transmit */
        tft->state = EATFT_TRANSMIT;
    }
}

/**
 * Sends commands to display.
 * Uses a printf like syntax for formatting.
 * NOTE: All strings are expected to lie in PROGMEM.
 * Integers are placed as binary values as expected by the EA TFT display.
 * Supported format specifiers:
 * %c 8 bit binary
 * %D 16 bit binary
 * %s char*
 */
int eatft_appendf(struct eatft *tft, const char *fmt, ...)
{
    const char *p;
    va_list args;
    uint16_t i;
    char *s;
    uint8_t *rp;
    char fmtstr[32];

    while (tft->state != EATFT_READY) {
        eatft_process(tft);
    }

    rp = tft->obuf + tft->olen;

    /* prepend escape */
    *rp = 0x1b;
    tft->bcc += *rp++;

    strcpy_P(fmtstr, fmt);
    va_start(args, fmt);

    for(p = fmtstr; *p != '\0'; p++) {
        if(*p != '%') {
            *rp++ = *p;
            tft->bcc += *p;
            continue;
        }

        switch(*++p) {
        case 'c':
            *rp = (char)va_arg(args, int);
            tft->bcc += *rp++;
            break;

        case 'D':
            i = (uint16_t)va_arg(args, int);
            *rp = i;
            tft->bcc += *rp++;
            *rp = i >> 8;
            tft->bcc += *rp++;
            break;

        case 's':
            s = va_arg(args, char*);
            do {
                tft->bcc += *s;
                *rp++ = *s;
            } while (*s++ != '\0');
            break;

        case '%':
            *rp = '%';
            tft->bcc += *rp++;
            break;
        default:
            dbg("WARNING: unknown format character\n");
        }
    }
    va_end(args);

    tft->olen = rp - tft->obuf;

    return tft->olen;
}

void eatft_process(struct eatft *tft)
{
    if (tft->ready(tft)) {
        switch (tft->state) {
        case EATFT_READY:
            eatft_poll(tft);
            break;

        case EATFT_TRANSMIT:
            tft->transmit(tft);
            tft->state = tft->next_state;
            break;

        case EATFT_RECEIVE:
            tft->receive(tft);
            tft->state = EATFT_EVENT;
            break;

        case EATFT_EVENT:
            tft->state = EATFT_RESET;

            if (tft->ibuf[1] > 0
                && eatft_chk_matches(tft)) {
                eatft_dispatch_event(tft);
            }
            break;

        case EATFT_RESET:
            eatft_reset_buffer(tft);
            break;
        }
    }

}
