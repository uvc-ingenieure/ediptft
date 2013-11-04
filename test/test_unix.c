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

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <eatft.h>

#include "test.h"

struct eatft g_tft;



static void signal_handler(int signal)
{
    eatft_unix_free(&g_tft);
    exit(1);
}

int main(int argc, char *argv[])
{
    int ret = OK;
    struct sigaction sig;
    const char str[] = "Hallo Terminal\r\n";
    int c;

    sig.sa_handler = signal_handler;
    sigemptyset(&sig.sa_mask);
    sig.sa_flags = 0;
    sigaction(SIGINT, &sig, NULL);

    ret = eatft_unix_create(&g_tft, "/dev/ttyS0");

    /* initialize tft */

    eatft_terminal_enable(&g_tft, false);
    eatft_color_set(&g_tft, EATFT_WHITE, EATFT_WHITE);
    eatft_clear(&g_tft);
    eatft_flush(&g_tft);

    test_render(&g_tft);

    for (;;) {
        usleep(100);
        eatft_process(&g_tft);
    }

    unix_free(&g_tft);

    return 0;
}
