#include <Arduino.h>

#include <string.h>

#include "leddriver.h"

#define NUM_ROWS    7
#define NUM_COLS    80

static pixel_t framebuffer[NUM_ROWS][NUM_COLS];
static pixel_t pwmstate[NUM_ROWS][NUM_COLS];
static bool red[NUM_ROWS][NUM_COLS];
static bool grn[NUM_ROWS][NUM_COLS];

static void update_framebuffer(void)
{
    for (int col = 0; col < NUM_COLS; col++) {
        for (int row = 0; row < NUM_ROWS; row++) {
            pixel_t c1 = pwmstate[row][col];
            pixel_t c2 = framebuffer[row][col];
            // determine binary value
            int r = c1.r + c2.r;
            red[row][col] = (r > 255);
            int g = c1.g + c2.g;
            grn[row][col] = (g > 255);
            // write back
            pwmstate[row][col].r = r;
            pwmstate[row][col].g = g;
        }
    }
}

static void display_framebuffer(void)
{
    for (int row = 0; row < NUM_ROWS; row++) {
        // write the shift register
        // ...
        
        // activate the desired row for some time
        // ...
    }
    
    // select the unused row
    // ...
}

void led_task(void *params)
{
    for (;;) {
//        update_framebuffer();
//        display_framebuffer();
    }
}

void led_clear(void)
{
    // clear the frame buffer
    memset(framebuffer, 0, sizeof(framebuffer));

    // initialise the pwm state with random values
    for (int row = 0; row < NUM_ROWS; row++) {
        for (int col = 0; col < NUM_COLS; col++) {
            pwmstate[row][col].r = random(256);
            pwmstate[row][col].g = random(256);
        }
    }
}

void led_setpixel(int x, int y, pixel_t c)
{
    framebuffer[y][x] = c;
}

