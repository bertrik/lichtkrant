#ifndef _FRAMEBUFFER_H_
#define _FRAMEBUFFER_H_

#include <stdint.h>

#define LED_HEIGHT      7
#define LED_WIDTH       80

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} pixel_t;

#endif /* _FRAMEBUFFER_H_ */

