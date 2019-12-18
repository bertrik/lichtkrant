#include <stdint.h>

#define LED_NUM_ROWS    7
#define LED_NUM_COLS    80

typedef struct {
    uint8_t r;
    uint8_t g;
} pixel_t;

typedef void (frame_fn_t) (void);

void led_init(frame_fn_t * frame_fn);

void led_tick(void);

void led_write_framebuffer(void *data);


