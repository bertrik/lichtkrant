#include <stdint.h>

typedef void (vsync_fn_t) (int frame_nr);

void led_init(vsync_fn_t * vsync);
void led_write_framebuffer(const void *data);

void led_enable(void);
void led_disable(void);

