#ifndef _EATFT_HEADER_
#define _EATFT_HEADER_

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>


#ifdef DEBUG
#define dbg(format, arg...)         \
    do {                            \
    printf(format, ##arg);          \
    fflush(stdout);                 \
    } while (0)

#else
#  define dbg(x...)
#endif


#define CONFIG_EATFT_WIDTH 800
#define CONFIG_EATFT_HEIGHT 480

#define CONFIG_EATFT_MARGIN_X 2
#define CONFIG_EATFT_MARGIN_Y 2
#define CONFIG_EATFT_MAX_WIDGETS 16

#ifdef __AVR__
#  include <avr/pgmspace.h>
#  include <stdio.h>
#  define DEBUG_ASSERT(f) do {                          \
        if (!(f)) {                                     \
            printf("Assertion in %s line %d failed",    \
                   __func__, __LINE__);                 \
            fflush(stdout);                             \
            for (;;)                                    \
                __asm__("nop");                         \
        }                                               \
    } while (0)
#else
#  include <string.h>
#  include <assert.h>
#  define DEBUG_ASSERT assert
#  define memcpy_P memcpy
#  define strcpy_P strcpy
#  define PSTR
#  define PROGMEM
#endif

#ifndef DEBUG
#  undef DEBUG_ASSERT
#  define DEBUG_ASSERT(f)
#endif


#define CONFIG_EATFT_OBUF_SIZE 64
#define CONFIG_EATFT_IBUF_SIZE 32

#define EATFT_ACK 0x06
#define EATFT_NAK 0x15

#define EATFT_DC1 0x11
#define EATFT_DC2 0x12


#define OK     0
#define ERROR -1

#define EATFT_FONT_4X6          1
#define EATFT_FONT_6X8          2
#define EATFT_FONT_7X12         3
#define EATFT_FONT_GENEVA10     4
#define EATFT_FONT_CHICAGO14    5
#define EATFT_FONT_SWISS30      6
#define EATFT_FONT_BIGZIF50     7
#define EATFT_FONT_BIGZIF100    8

#define EATFT_BLACK   1
#define EATFT_BLUE    2
#define EATFT_RED     3
#define EATFT_GREEN   4
#define EATFT_YELLOW  5
#define EATFT_WHITE   8
#define EATFT_GRAY    16

enum eatft_text_pos {
    EATFT_TOP_LEFT = 1,
    EATFT_TOP_CENTER,
    EATFT_TOP_RIGHT,
    EATFT_MID_LEFT,
    EATFT_MID_CENTER,
    EATFT_MID_RIGHT,
    EATFT_BOT_LEFT,
    EATFT_BOT_CENTER,
    EATFT_BOT_RIGHT
};

enum eatft_align {
    EATFT_ALIGN_LEFT = 'L',
    EATFT_ALIGN_CENTER = 'C',
    EATFT_ALIGN_RIGHT = 'R'
};

enum eatft_state {
    EATFT_READY = 0,
    EATFT_TRANSMIT,
    EATFT_RECEIVE,
    EATFT_EVENT,
    EATFT_RESET
};

struct eatft_rect {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
};

struct eatft_point {
    uint16_t x;
    uint16_t y;
};

struct eatft;
struct eatft_widget;

#define EATFT_WIDGET_INVALID -1;

typedef void (*eatft_callback_t)(struct eatft *tft, struct eatft_widget *widget,
                                 bool down);

struct eatft_widget {
    struct eatft_rect rect;
    eatft_callback_t fun;
    void *priv;
};


struct eatft {
    uint8_t dc;
    uint8_t olen;
    uint8_t obuf[CONFIG_EATFT_OBUF_SIZE];
    uint8_t bcc;

    uint8_t ilen;
    uint8_t ibuf [CONFIG_EATFT_IBUF_SIZE];

    enum eatft_state state;
    enum eatft_state next_state;

    void (*transmit)(struct eatft *tft);
    void (*receive)(struct eatft *tft);
    bool (*ready)(struct eatft *tft);
    void (*user_action)(struct eatft *tft);
    void *driver;
    void *user;

    /* widget section */
    struct eatft_widget widgets[CONFIG_EATFT_MAX_WIDGETS];
    struct eatft_rect window;

} __attribute__ ((packed));

void eatft_register_user_action(struct eatft *tft, void (*action)(struct eatft*));

/**
 * NOTE:
 * All struct eatft_rect and point_s based interfaces expect structs in PROGMEM.
 */

void eatft_info(struct eatft *tft);
void eatft_clear(struct eatft *tft);
void eatft_terminal_enable(struct eatft *tft, bool enable);
void eatft_color_set(struct eatft *tft, uint8_t fg, uint8_t bg);
void eatft_touch_enable(struct eatft *tft, uint8_t enable);
void eatft_touch_beep(struct eatft *tft, bool enable);

void eatft_button_setfont(struct eatft *tft, uint8_t font);
void eatft_button_setfontzoom(struct eatft *tft, uint8_t factor);
void eatft_button_setfontcolor(struct eatft *tft, uint8_t norm, uint8_t sel);
void eatft_button_setoffset(struct eatft *tft, uint8_t x, uint8_t y);
void eatft_button_setframecolor(struct eatft *tft, uint8_t n1, uint8_t n2,
                                uint8_t n3, uint8_t s1, uint8_t s2, uint8_t s3);
void eatft_button_setframe(struct eatft *tft, uint8_t n1, uint8_t angle);

void eatft_button_createi(struct eatft *tft, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height,
                          uint8_t downcode, uint8_t upcode,
                          enum eatft_align align, const char *text);
void eatft_button_vcreatei(struct eatft *tft, uint16_t x, uint16_t y,
                           uint16_t width, uint16_t height,
                           uint8_t downcode, uint8_t upcode,
                           enum eatft_align align,
                           const char *fmt, va_list args);

void eatft_switch_vcreatei(struct eatft *tft, uint16_t x, uint16_t y,
                           uint16_t width, uint16_t height,
                           uint8_t downcode, uint8_t upcode,
                           enum eatft_align align,
                           const char *fmt, va_list args);
void eatft_switch_createi(struct eatft *tft, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height,
                          uint8_t downcode, uint8_t upcode,
                          enum eatft_align align, const char *text);
void eatft_switch_set(struct eatft *tft, uint16_t code, bool enable);

void eatft_radio_group(struct eatft *tft, uint8_t group);

void eatft_button_remove(struct eatft *tft, uint8_t code);

void eatft_touch_areai(struct eatft *tft, uint16_t x, uint16_t y,
                       uint16_t width, uint16_t height);
void eatft_touch_arear(struct eatft *tft, const struct eatft_rect *rect);
void eatft_touch_area_removei(struct eatft *tft, uint16_t x, uint16_t y,
                              uint16_t width, uint16_t height);
void eatft_touch_area_remover(struct eatft *tft, const struct eatft_rect *rect);


void eatft_frame_setcolor(struct eatft *tft, uint8_t inner, uint8_t outer,
                          uint8_t fill);
void eatft_frame_drawi(struct eatft *tft, uint16_t x, uint16_t y,
                       uint16_t width, uint16_t height);
void eatft_frame_drawr(struct eatft *tft, const struct eatft_rect *rect);

void eatft_line_setwidth(struct eatft *tft, uint8_t width);
void eatft_line_drawi(struct eatft *tft, uint16_t x1, uint16_t y1,
                      uint16_t x2, uint16_t y2);
void eatft_line_drawp(struct eatft *tft, const struct eatft_point *p1,
                      const struct eatft_point *p2);

void eatft_rect_drawi(struct eatft *tft, uint16_t x, uint16_t y,
                      uint16_t width, uint16_t height);
void eatft_rect_drawr(struct eatft *tft, const struct eatft_rect *rect);
void eatft_rect_cleari(struct eatft *tft, uint16_t x, uint16_t y,
                       uint16_t width, uint16_t height);
void eatft_rect_clearr(struct eatft *tft, const struct eatft_rect *rect);
void eatft_rect_filli(struct eatft *tft, uint16_t x, uint16_t y,
                      uint16_t width, uint16_t height, uint8_t color);
void eatft_rect_fillr(struct eatft *tft, const struct eatft_rect *rect,
                      uint8_t color);

void eatft_setfont(struct eatft *tft, uint8_t font);
void eatft_setfontcolor(struct eatft *tft, uint8_t fore, uint8_t back);

void eatft_text_drawi(struct eatft *tft, uint16_t x, uint16_t y,
                      uint16_t width, uint16_t height, enum eatft_text_pos pos,
                      const char *text);
void eatft_text_drawr(struct eatft *tft, const struct eatft_rect *rect,
                      enum eatft_text_pos pos, const char *fmt, ...);

void eatft_init(struct eatft *tft);
void eatft_process(struct eatft *tft);
bool eatft_chk_matches(struct eatft *tft);

int eatft_appendf(struct eatft *tft, const char *fmt, ...);
void eatft_poll(struct eatft *tft);
void eatft_flush(struct eatft *tft);
void eatft_reset_buffer(struct eatft *tft);

/**
 * Widget API
 */

/**
 * An window, when set, places widgets into that parent window.
 */
void eatft_wdt_window_set(struct eatft *tft, const struct eatft_rect *window);
void eatft_wdt_window_clear(struct eatft *tft);

struct eatft_widget *eatft_wdt_touch_createi(
    struct eatft *tft, uint16_t x, uint16_t y, uint16_t width, uint16_t height,
    eatft_callback_t callback, void *priv);
struct eatft_widget *eatft_wdt_touch_creater(
    struct eatft *tft, const struct eatft_rect *rect, eatft_callback_t callback,
    void *priv);

void eatft_wdt_free(struct eatft *tft, struct eatft_widget *widget);

/* Button */
struct eatft_widget *eatft_wdt_button_vcreatei(
    struct eatft *tft, uint16_t x, uint16_t y, uint16_t width, uint16_t height,
    eatft_callback_t callback, void *priv, enum eatft_align align,
    const char *fmt, va_list args);
struct eatft_widget *eatft_wdt_button_createi(
    struct eatft *tft, uint16_t x, uint16_t y, uint16_t width, uint16_t height,
    eatft_callback_t callback, void *priv, enum eatft_align align,
    const char *fmt, ...);
struct eatft_widget *eatft_wdt_button_creater(
    struct eatft *tft, const struct eatft_rect *rect, eatft_callback_t callback,
    void *priv, enum eatft_align align, const char *fmt, ...);
struct eatft_widget *eatft_wdt_button_createw(
    struct eatft *tft, eatft_callback_t callback, void *priv,
    enum eatft_align align, const char *fmt, ...);

struct eatft_widget *eatft_wdt_switch_vcreatei(
    struct eatft *tft, uint16_t x, uint16_t y, uint16_t width, uint16_t height,
    eatft_callback_t callback, void *priv, enum eatft_align align,
    const char *fmt, va_list args);
struct eatft_widget *eatft_wdt_switch_createi(
    struct eatft *tft, uint16_t x, uint16_t y, uint16_t width, uint16_t height,
    eatft_callback_t callback, void *priv, enum eatft_align align,
    const char *fmt, ...);
struct eatft_widget *eatft_wdt_switch_creater(
    struct eatft *tft, const struct eatft_rect *rect, eatft_callback_t callback,
    void *priv, enum eatft_align align, const char *fmt, ...);
struct eatft_widget *eatft_wdt_switch_createw(
    struct eatft *tft, eatft_callback_t callback, void *priv,
    enum eatft_align align, const char *fmt, ...);

void eatft_wdt_switch_set(struct eatft *tft, struct eatft_widget *widget,
                          bool enable);

#endif  /* _EATFT_HEADER_ */
