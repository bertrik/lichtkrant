#include <string.h>

#include <Arduino.h>

#include "leddriver.h"

#define PIN_MUX_0   D3      // J1.11
#define PIN_MUX_1   D2      // J1.13
#define PIN_MUX_2   D1      // J1.15

#define PIN_SHIFT   D5      // J1.1
#define PIN_LATCH   D6      // J1.3
#define PIN_DATA_G  D7      // J1.5
#define PIN_DATA_R  D8      // J1.7

static const vsync_fn_t *vsync_fn;
static pixel_t framebuffer[LED_NUM_ROWS][LED_NUM_COLS];
static pixel_t pwmstate[LED_NUM_ROWS][LED_NUM_COLS];
static int row = 0;

void led_tick(void)
{
    // set the row multiplexer
    digitalWrite(PIN_MUX_0, row & 1);
    digitalWrite(PIN_MUX_1, row & 2);
    digitalWrite(PIN_MUX_2, row & 4);

    // prepare to latch the columns
    digitalWrite(PIN_LATCH, 0);

    // write column data
    if (row < 8) {
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
            digitalWrite(PIN_DATA_G, g < 256);
            digitalWrite(PIN_DATA_R, r < 256);

            // write back
            pwmrow[col].r = r;
            pwmrow[col].g = g;

            // shift
            digitalWrite(PIN_SHIFT, 1);
        }
    } else {
        row = 0;
        vsync_fn();
    }
    row++;

    digitalWrite(PIN_LATCH, 1);
}

void led_write_framebuffer(const void *data)
{
    memcpy(framebuffer, data, sizeof(framebuffer));
}

void led_init(const vsync_fn_t * vsync)
{
    // copy
    vsync_fn = vsync;

    pinMode(PIN_LATCH, OUTPUT);
    pinMode(PIN_SHIFT, OUTPUT);
    pinMode(PIN_DATA_R, OUTPUT);
    pinMode(PIN_DATA_G, OUTPUT);
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


