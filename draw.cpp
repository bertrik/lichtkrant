#include <string.h>

#include "glcdfont.h"
#include "framebuffer.h"
#include "draw.h"

static pixel_t *_framebuffer;

void draw_init(pixel_t *framebuffer)
{
    _framebuffer = framebuffer;
}

bool draw_pixel(int x, int y, pixel_t c)
{
    if ((x < 0) || (x >= LED_NUM_COLS) || (y < 0) || (y >= LED_NUM_ROWS)) {
        return false;
    }
    int i = y * LED_NUM_COLS + x;
    _framebuffer[i] = c;
    return true;
}

void draw_vline(int x, pixel_t c)
{
    for (int y = 0; y < LED_NUM_ROWS; y++) {
        draw_pixel(x, y, c);
    }
}

void draw_hline(int y, pixel_t c)
{
    for (int x = 0; x < LED_NUM_COLS; x++) {
        draw_pixel(x, y, c);
    }
}

int draw_glyph(char c, int x, pixel_t fg, pixel_t bg)
{
    // ASCII?
    if (c > 127) {
        return x;
    }

    // draw glyph
    int aa = 0;
    for (int col = 0; col < 5; col++) {
        uint8_t a = font[c * 5 + col];

        // skip repeating space
        if ((aa == 0) && (a == 0)) {
            continue;
        }
        aa = a;

        // draw column
        for (int y = 0; y < 7; y++) {
            draw_pixel(x, y, (a & 1) ? fg : bg);
            a >>= 1;
        }
        x++;
    }

    // draw space until next character
    if (aa != 0) {
        draw_vline(x, bg);
        x++;
    }

    return x;
}

int draw_text(const char *text, int x, pixel_t fg, pixel_t bg)
{
    for (size_t i = 0; i < strlen(text); i++) {
        x = draw_glyph(text[i], x, fg, bg);
    }
    return x;
}

