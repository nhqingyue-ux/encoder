#pragma once
#include "lvgl/lvgl.h"

extern lv_indev_t *lv_port_indev;

void lv_port_init(void);
void lv_port_enc_update(int8_t delta, bool sw);
