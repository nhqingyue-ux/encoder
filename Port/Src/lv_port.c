/**
 * lv_port.c - LVGL display + encoder input driver for STM32G070
 *
 * Display: 240x240 SPI LCD via SDK's LCD_ShowImg / LCD_set_frame
 * Input:   Rotary encoder via SDK's Enocde_Process + Encoder2Index,
 *          SW button on PF1
 */
#include "lv_port.h"
#include "lvgl/lvgl.h"
#include "SDK.h"
#include "main.h"

/* ── Display ─────────────────────────────────────────────────────────── */

/* Line buffer: 240 px wide, 10 lines tall = 4800 bytes */
#define BUF_LINES 10
static lv_color_t disp_buf1[LV_HOR_RES_MAX * BUF_LINES];
static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t      disp_drv;

static void disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p)
{
    uint16_t w = area->x2 - area->x1 + 1;
    uint16_t h = area->y2 - area->y1 + 1;

    struct LCD_frame frame = {
        .start = { area->x1, area->y1 },
        .end   = { area->x2, area->y2 }
    };
    LCD_set_frame(frame);
    LCD_ShowImg(area->x1, area->y1, w, h, (uint8_t *)color_p);

    lv_disp_flush_ready(drv);
}

/* ── Encoder Input ───────────────────────────────────────────────────── */

static lv_indev_drv_t  enc_drv;
static int8_t  enc_diff  = 0;   /* accumulated delta from ISR/poll */
static bool    sw_pressed = false;

/* Called from Enocde_Process() context or tick */
void lv_port_enc_update(int8_t delta, bool sw)
{
    enc_diff  += delta;
    sw_pressed = sw;
}

static void enc_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    (void)drv;
    data->enc_diff = enc_diff;
    enc_diff = 0;

    if (sw_pressed) {
        data->state   = LV_INDEV_STATE_PRESSED;
        sw_pressed    = false;
    } else {
        data->state   = LV_INDEV_STATE_RELEASED;
    }
}

/* ── Init ────────────────────────────────────────────────────────────── */

lv_indev_t *lv_port_indev = NULL;

void lv_port_init(void)
{
    /* Display */
    lv_disp_draw_buf_init(&draw_buf, disp_buf1, NULL,
                          LV_HOR_RES_MAX * BUF_LINES);

    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res    = 240;
    disp_drv.ver_res    = 240;
    disp_drv.flush_cb   = disp_flush;
    disp_drv.draw_buf   = &draw_buf;
    disp_drv.full_refresh = 0;
    lv_disp_drv_register(&disp_drv);

    /* Encoder input device */
    lv_indev_drv_init(&enc_drv);
    enc_drv.type    = LV_INDEV_TYPE_ENCODER;
    enc_drv.read_cb = enc_read;
    lv_port_indev = lv_indev_drv_register(&enc_drv);
}
