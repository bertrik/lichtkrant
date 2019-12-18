#include <string.h>

#include <Arduino.h>

#include "leddriver.h"

#define PIN_MUX_0   D1
#define PIN_MUX_1   D2
#define PIN_MUX_2   D3

#define PIN_SHIFT   D5
#define PIN_DATA_1  D6
#define PIN_DATA_2  D7
#define PIN_LATCH   D8

static const vsync_fn_t *vsync_fn;
static pixel_t framebuffer[LED_NUM_ROWS][LED_NUM_COLS];
static pixel_t pwmstate[LED_NUM_ROWS][LED_NUM_COLS];
static int row = 0;

void led_tick(void)
{
    // latch the columns of the previous row
    digitalWrite(PIN_LATCH, 1);

    // set the row multiplexer of the previous row
    digitalWrite(PIN_MUX_0, row & 1);
    digitalWrite(PIN_MUX_1, row & 2);
    digitalWrite(PIN_MUX_2, row & 4);

    // deactivate latch
    digitalWrite(PIN_LATCH, 0);

    // next row
    row++;

    // write the column shift registers
    pixel_t *pwmrow = pwmstate[row];
    pixel_t *fb_row = framebuffer[row];
    for (int col = 0; col < LED_NUM_COLS; col++) {
        // dither
        pixel_t c1 = pwmrow[col];
        pixel_t c2 = fb_row[col];
        int r = c1.r + c2.r;
        int g = c1.g + c2.g;

        digitalWrite(PIN_SHIFT, 0);
        digitalWrite(PIN_DATA_1, r & 256);
        digitalWrite(PIN_DATA_2, g & 256);

        // write back
        pwmrow[col].r = r;
        pwmrow[col].g = g;

        // shift
        digitalWrite(PIN_SHIFT, 1);
    }

    // signal vsync
    if (row == 7) {
        row = 0;
        vsync_fn();
    }
}

void led_write_framebuffer(void *data)
{
    memcpy(framebuffer, data, sizeof(framebuffer));
}

void led_init(const vsync_fn_t * vsync)
{
    // copy
    vsync_fn = vsync;

    pinMode(PIN_LATCH, OUTPUT);
    pinMode(PIN_SHIFT, OUTPUT);
    pinMode(PIN_DATA_1, OUTPUT);
    pinMode(PIN_DATA_2, OUTPUT);
    pinMode(PIN_MUX_0, OUTPUT);
    pinMode(PIN_MUX_1, OUTPUT);
    pinMode(PIN_MUX_2, OUTPUT);

    // clear the frame buffer
    memset(framebuffer, 0, sizeof(framebuffer));

    // initialise the pwm state with random values
    for (int row = 0; row < LED_NUM_ROWS; row++) {
        for (int col = 0; col < LED_NUM_COLS; col++) {
            pwmstate[row][col].r = random(256);
            pwmstate[row][col].g = random(256);
        }
    }

    row = 0;
}


