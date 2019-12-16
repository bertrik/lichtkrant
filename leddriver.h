#include <stdint.h>

typedef struct {
    uint8_t r;
    uint8_t g;
} pixel_t;

/** the main led driver task */
void led_task(void *params);

/** clears the entire panel */
void led_clear(void);

/** writes a pixel at a specific position */
void led_setpixel(int x, int y, pixel_t c);

