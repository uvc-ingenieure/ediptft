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

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>

#include <eatft.h>

#include "eatft_avr.h"
#include "eatft.h"


enum eatft_driver_state {
    EATFT_DRIVER_READY = 0,
    EATFT_DRIVER_START,
    EATFT_DRIVER_SEND,
    EATFT_DRIVER_WAIT,
    EATFT_DRIVER_RECEIVE_ACK,
    EATFT_DRIVER_RECEIVE,
    EATFT_DRIVER_RECEIVE_HEADER,
    EATFT_DRIVER_RECEIVE_PAYLOAD
};

struct eatft_spi {
    struct eatft *tft;
    enum eatft_driver_state state;

    uint8_t *pos;
    int len;
};

volatile struct eatft_spi g_driver;

static void eatft_driver_ss_enable(bool enable);
static void eatft_driver_timer_enable(bool enable);
static void eatft_driver_transmit(struct eatft *tft);
static void eatft_driver_receive(struct eatft *tft);
static bool eatft_driver_ready(struct eatft *tft);

static void eatft_driver_ss_enable(bool enable)
{
    if (enable)
        PORTB &= ~0x01;
    else
        PORTB |= 0x01;
}

static void eatft_driver_timer_enable(bool enable)
{
    if (enable) {
        TIMSK |= (1 << OCIE2);
    } else {
        TIMSK &= ~(1 << OCIE2);
    }
}

static void eatft_driver_transmit(struct eatft *tft)
{
    volatile struct eatft_spi *driver = tft->driver;

    driver->state = EATFT_DRIVER_START;
    eatft_driver_timer_enable(true);
}

static void eatft_driver_receive(struct eatft *tft)
{
    volatile struct eatft_spi *driver = tft->driver;

    driver->state = EATFT_DRIVER_RECEIVE;
    eatft_driver_timer_enable(true);
}

static bool eatft_driver_ready(struct eatft *tft)
{
    volatile struct eatft_spi *driver = tft->driver;
    return driver->state == EATFT_DRIVER_READY;
}

void eatft_driver_init(struct eatft *tft)
{
    DDRE = 0x20;

    PORTB = 0xff;
    DDRB = 0x07;

    SPCR = (1 << CPOL) | (1<<SPE) | (1<<DORD) | (1<<CPHA) | (1<<MSTR) | _BV(SPR0);
    DDRD |= 0x80;
    PORTD |= 0x80;
    _delay_ms(20);
    PORTD &= ~0x80;

    TCNT2 = 0;

    /* CTC Mode, prescaler 64, reset on overflow */
    TCCR2 = (0 << CS22) | (1 << CS21) | (1 << CS20) | (1 << WGM21);

    /* OCR2 = F_CPU / 64 / 5000 - 1 */
    /* We need a timer to conform to the timing specs */
    /* Compare Match mit 7.5kHz, */
    OCR2 = (long) F_CPU / (64L * 10000L) - 1;

    g_driver.tft = tft;
    g_driver.state = EATFT_DRIVER_READY;

    tft->driver = (void*)&g_driver;
    tft->transmit = eatft_driver_transmit;
    tft->receive = eatft_driver_receive;
    tft->ready = eatft_driver_ready;

    eatft_init(tft);
}

ISR(TIMER2_COMP_vect)
{
    volatile struct eatft_spi *driver = &g_driver;

    PORTE ^= 0x20;

    switch (driver->state) {

    case EATFT_DRIVER_START:
        eatft_driver_ss_enable(true);
        driver->pos = &driver->tft->dc;

        /* add DCx, len and bcc to len */
        driver->len = driver->tft->olen + 3;

        driver->state = EATFT_DRIVER_SEND;
        break;

    case EATFT_DRIVER_SEND:
        if (driver->len-- > 0) {
            SPDR = *driver->pos++;
        } else {
            eatft_driver_ss_enable(false);
            driver->state = EATFT_DRIVER_WAIT;
        }

        break;

    case EATFT_DRIVER_WAIT:
        /**
         * undocumented wait state.
         * The display needs ~10us before ACK can be queried.
         */
        eatft_driver_ss_enable(true);

        /* trigger SPI for reading ACK */
        SPDR = 0x00;

        driver->state = EATFT_DRIVER_RECEIVE_ACK;
        break;

    case EATFT_DRIVER_RECEIVE_ACK:
        if (SPDR == EATFT_ACK) {
            driver->state = EATFT_DRIVER_READY;
        } else {
            /* NAK or something, try again */
            eatft_driver_ss_enable(false);

            driver->state = EATFT_DRIVER_START;
        }
        break;

    case EATFT_DRIVER_RECEIVE:
        eatft_driver_ss_enable(true);

        /* DCx and len */
        driver->len = 2;
        driver->pos = driver->tft->ibuf;
        driver->state = EATFT_DRIVER_RECEIVE_HEADER;

        /* trigger SPI */
        SPDR = 0x00;
        break;

    case EATFT_DRIVER_RECEIVE_HEADER:
        *driver->pos++ = SPDR;
        SPDR = 0x00;

        if (--driver->len <= 0) {
            driver->len = driver->tft->ibuf[1] + 1;
            driver->tft->ilen = driver->len + 2;
            driver->state = EATFT_DRIVER_RECEIVE_PAYLOAD;
        }
        break;

    case EATFT_DRIVER_RECEIVE_PAYLOAD:
        if (driver->len-- > 0) {
            *driver->pos++ = SPDR;

            if (driver->len > 0)
                SPDR = 0x00;
        } else {
            driver->state = EATFT_DRIVER_READY;
        }

        break;

    case EATFT_DRIVER_READY:
        eatft_driver_timer_enable(false);
        eatft_driver_ss_enable(false);
        break;

    }

}
