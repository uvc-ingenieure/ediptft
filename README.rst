**********************
EA eDIPTFT GUI Library
**********************

Library for convenient handling of the Electronic Assembly eDIPTFT display
modules.
The library was originally written for an AVR target but should be easily
protable to other architectures.

========
Features
========

* Simple and easy to use Widget API
* No dynamic memory allocation
* No dependencies
* Driver for UNIX and AVR for testing on PC before deployment to microcontroller
* MIT License

.. image:: https://raw.github.com/uvc-ingenieure/ediptft/master/images/test.jpg

==========
Interfaces
==========

Included is a simple UNIX driver for writing programs on PC and a code snippet
for integrating the library with an AVR.

===
API
===

All API calls are buffered and sent to the display with `eatft_flush(...)`.
The API is devided into a lower layer part and a more abstract widget like API.
Calls which need to set a string are implemented with format string to allow easy
formatting.

**NOTE:** The library was originally written for an AVR project with very
limited memory constraints. That's why some of the parameters in the API
are expected to be located in PROGMEM. These are for example all the
`struct eatft_rect` and format strings. If you are experiencing strange crashes
it most properly has to do with PROGMEM.

For PC builds the PROGMEM handling is def'ed out.

Lower Layer
===========

The lower layer API is the base for the higher level widget API. It can be used
to implement further widgets or for applications where fine grained control is
needed.

Mostly all primitives creatable with two signatures:

.. code-block:: c

    /* Create by integers for x, y, width and height */

    void eatft_button_createi(
        struct eatft *tft,
	uint16_t x,
	uint16_t y,
    	uint16_t width,
	uint16_t height,
        uint8_t downcode,
	uint8_t upcode,
        enum eatft_align align,
	const char *text);

    /* Same as above but with va_list for formatting text */

    void eatft_button_vcreatei(
        struct eatft *tft,
	uint16_t x,
	uint16_t y,
        uint16_t width,
	uint16_t height,
        uint8_t downcode,
	uint8_t upcode,
        enum eatft_align align,
        const char *fmt, va_list args);


Widget API
==========


Widgets are encapsulated in instances of `struct eatft_widget`. The library
handles the different types of widgets like buttons, touch areas and switches
and registeres callbacks.
This makes it easy to handle the memory management in more complex designs.

.. code-block:: c

    struct eatft_widget *button;

    void button_clicked(struct eatft *tft, struct eatft_widget *widget, bool down)
    {
        const char *priv = (const char*)widget->priv;
        printf("Button clicked: %s\n", priv);
    }

    void create_button(struct eatft *tft)
    {
        button = eatft_wdt_button_createi(
            tft,
	    200,		/* x */
	    150,                /* y */
	    200,                /* width */
	    100,                /* height */
            button_clicked,     /* callback */
            "Private Data",     /* private data */
            EATFT_ALIGN_LEFT,   /* text alignment */
	    "button %d", 3);    /* formatted text */

	eatft_flush(tft);
    }

    void delete_button(struct eatft *tft)
    {
        eatft_wdt_free(tft, button);
    }


All widgets are creatable with the two signatures from the lower plus further
two:

.. code-block:: c

    /* Create button from rect */

    struct eatft_widget *eatft_wdt_button_creater(
        struct eatft *tft,
	const struct eatft_rect *rect,
	eatft_callback_t callback,
    	void *priv,
	enum eatft_align align,
	const char *fmt, ...);

    /* Create and add button to window */

    struct eatft_widget *eatft_wdt_button_createw(
        struct eatft *tft,
	eatft_callback_t callback,
	void *priv,
    	enum eatft_align align,
	const char *fmt, ...);


Creating multiple widgets of similar kind
-----------------------------------------

For creating lists or menus the window API has been implemented:

The window is a rect which can be set using `eatft_wdt_window_set(...)`.
Following calls to `eatft_wdt_create*` will be added into the window
with automatically set offsets and spacings.
When finished with adding widgets to the window `eatft_wdt_window_clear`
must be called. This technique is used in the ./test/test.c application
included in this repository.

=====
Build
=====

For the UNIX build depends on CMake. The example uses /dev/ttyS0 hardcoded
as default you have to set it to your needs.

.. code-block:: bash

    git clone http://github.com/uvc-ingenieure/ediptft.git
    cd eatft && mkdir build && cmake .. && make
    ./test

For using the lib on microcontrollers there is no makefile supplied,
because it's most likely that you will integrate the code into your
own build system anyway.
