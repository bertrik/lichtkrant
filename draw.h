#include "framebuffer.h"

void draw_init(pixel_t *framebuffer);

bool draw_pixel(int x, int y, pixel_t p);
void draw_vline(int x, pixel_t c);
void draw_hline(int y, pixel_t c);
int draw_text(const char *text, int x, pixel_t fg, pixel_t bg);

