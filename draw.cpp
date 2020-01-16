#include <string.h>

#include "font.h"
#include "framebuffer.h"
#include "draw.h"

static pixel_t *_framebuffer;
static pixel_t _fg;

void draw_init(pixel_t *framebuffer)
{
    _framebuffer = framebuffer;
}

bool draw_pixel(int x, int y, pixel_t c)
{
    if ((x < 0) || (x >= LED_WIDTH) || (y < 0) || (y >= LED_HEIGHT)) {
        return false;
    }
    _framebuffer[y * LED_WIDTH + x] = c;
    return true;
}

void draw_vline(int x, pixel_t c)
{
    for (int y = 0; y < LED_HEIGHT; y++) {
        draw_pixel(x, y, c);
    }
}

void draw_hline(int y, pixel_t c)
{
    for (int x = 0; x < LED_WIDTH; x++) {
        draw_pixel(x, y, c);
    }
}

static pixel_t shade_fg(int x, int y)
{
    return _fg;
}

void draw_fill_ext(color_fn_t *fn)
{
    for (int y = 0; y < LED_HEIGHT; y++) {
        for (int x = 0; x < LED_WIDTH; x++) {
            draw_pixel(x, y, fn(x, y));
        }
    }
}

void draw_fill(pixel_t c)
{
    _fg = c;
    draw_fill_ext(shade_fg);
}

int draw_glyph(int c, int x, color_fn_t *fn, pixel_t bg)
{
    // ASCII?
    if (c > 127) {
        return x;
    }

    // draw glyph
    unsigned char aa = 0;
    for (int col = 0; col < 5; col++) {
        unsigned char a = font[c][col];

        // skip repeating space
        if ((aa == 0) && (a == 0)) {
            continue;
        }
        aa = a;

        // draw column
        for (int y = 0; y < 7; y++) {
            draw_pixel(x, y, (a & 1) ? fn(x, y) : bg);
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
    _fg = fg;
    return draw_text_ext(text, x, shade_fg, bg);
}

int draw_text_ext(const char *text, int x, color_fn_t *fn, pixel_t bg)
{
    for (size_t i = 0; i < strlen(text); i++) {
        x = draw_glyph(text[i], x, fn, bg);
    }
    return x;
}
