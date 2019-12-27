#include "framebuffer.h"

void draw_init(pixel_t *framebuffer);
void draw_pixel(int x, int y, pixel_t p);
int draw_text(const char *text, int x, pixel_t fg, pixel_t bg);

