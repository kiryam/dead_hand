#ifndef DISPLAY_HOMEPAGE_H
#define DISPLAY_HOMEPAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f2xx_conf.h"
#include "ssd1306.h"
void render_homepage();
void controller_homepage(int btn);


#ifdef __cplusplus
}
#endif

#endif
