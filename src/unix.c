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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#include <eatft.h>

#define CONFIG_EATFT_PORT "/dev/ttyS0"
#define CONFIG_EATFT_BAUD B115200
#define CONFIG_EATFT_TIMEOUT 1000

struct unix_driver {
    struct eatft *tft;
    int fd;
};

static void unix_receive(struct eatft *tft);
static void unix_transmit(struct eatft *tft);
static bool unix_data_ready(struct eatft *tft);


static void unix_receive(struct eatft *tft)
{
    struct unix_driver *priv = tft->driver;
    int n;

    /* read header */
    n = read(priv->fd, tft->ibuf, 2);
    /* read len + checksum */
    read(priv->fd, tft->ibuf + 2, tft->ibuf[1] + 1);
    tft->ilen = tft->ibuf[1] + 3;
}

static void unix_transmit(struct eatft *tft)
{
    struct unix_driver *priv = tft->driver;
    uint8_t ack;
    int n;
    int i;

    do {
        /* printf("Write: %d\n", tft->olen); */
        /* for (i = 0; i < tft->olen + 3; i++) */
        /*     printf("%02x ", (&tft->dc)[i]); */
        /* printf("\n"); */

        write(priv->fd, &tft->dc, tft->olen + 3);
        n = read(priv->fd, &ack, 1);
        /* printf("response len: %d, ack: %02x\n", n, ack); */

    } while (n <= 0 || ack != EATFT_ACK);
}

static bool unix_ready(struct eatft *tft)
{
    /* we use a blocking approach */
    return true;
}

int eatft_unix_create(struct eatft *tft, const char *dev)
{
    int ret = OK;
    struct unix_driver *priv;
    struct termios tio;

    eatft_init(tft);

    priv = calloc(1, sizeof(struct unix_driver));

    if (priv) {
        memset(priv, 0, sizeof(*priv));
    } else {
        fprintf(stderr, "ERROR: failed to allocate unix_driver\n");
        ret = ERROR;
    }

    if (ret == OK
        && (priv->fd = open(dev, O_RDWR | O_NOCTTY)) > 0) {

        tft->driver = priv;
        tft->receive = unix_receive;
        tft->transmit = unix_transmit;
        tft->ready = unix_ready;

        memset(&tio, 0, sizeof(tio));
        tio.c_cflag = CS8 | CREAD | CLOCAL;
        /* tio.c_cc[VMIN] = 100; */
        tio.c_cc[VTIME] = 50;

        if(cfsetispeed(&tio, CONFIG_EATFT_BAUD) < 0
           || cfsetospeed(&tio, CONFIG_EATFT_BAUD) < 0) {
            fprintf(stderr, "ERROR: failed to set BAUD rate\n");
            ret = ERROR;

        }
        if(ret == OK && tcsetattr(priv->fd, TCSANOW, &tio) < 0) {
            fprintf(stderr, "ERROR: failed to setup serial port\n");
            ret = ERROR;
        }

    } else {
        perror(CONFIG_EATFT_PORT);
        ret = ERROR;
    }

    return ret;
}

int eatft_unix_free(struct eatft *tft)
{
    struct unix_driver *priv = tft->driver;

    close(priv->fd);
    free(tft->driver);
}
