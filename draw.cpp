#include "framebuffer.h"

static pixel_t *_framebuffer;

void draw_init(pixel_t *framebuffer)
{
    _framebuffer = framebuffer;
}

void draw_pixel(int x, int y, pixel_t p)
{
    if ((x < 0) || (x >= LED_NUM_COLS) || (y < 0) || (y >= LED_NUM_ROWS)) {
        return;
    }
    int i = y * LED_NUM_COLS + x;
    _framebuffer[i] = p;
}

