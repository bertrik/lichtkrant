#include <stdint.h>

#define LED_NUM_ROWS    7
#define LED_NUM_COLS    80

typedef struct {
    uint8_t r;
    uint8_t g;
} pixel_t;

typedef void (vsync_fn_t) (int frame_nr);

void led_init(vsync_fn_t * vsync);

void led_write_framebuffer(const void *data);


