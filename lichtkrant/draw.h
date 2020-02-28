#include "framebuffer.h"

typedef pixel_t (color_fn_t)(int x, int y);

void draw_init(pixel_t *framebuffer);

bool draw_pixel(int x, int y, pixel_t p);

void draw_vline(int x, pixel_t c);
void draw_hline(int y, pixel_t c);

void draw_fill(pixel_t c);
void draw_fill_ext(color_fn_t *color_fn);

int draw_text(const char *text, int x, pixel_t fg, pixel_t bg);
int draw_text_ext(const char *text, int x, color_fn_t *fn, pixel_t bg);


