/**
 * ui_menu.c - LVGL menu for encoder controller
 *
 * Main screen: circular list of 5 items (Fan / AC / Temperature / Brightness / Volume)
 * Sub screen:  parameter adjustment with slider + value label
 *
 * Navigation:
 *   Rotate encoder → navigate items / adjust value
 *   Press SW       → enter sub-screen / back to main
 */
#include "ui_menu.h"
#include "lv_port.h"
#include "lvgl/lvgl.h"
#include "SDK.h"
#include "main.h"

/* ── Menu definition ─────────────────────────────────────────────────── */

#define MENU_ITEM_COUNT 5

typedef struct {
    const char *name;
    const char *icon;   /* emoji-style symbol (ASCII subset) */
    uint8_t     val;
    uint8_t     min;
    uint8_t     max;
    const char *unit;
} MenuItem;

static MenuItem items[MENU_ITEM_COUNT] = {
    { "Fan",         LV_SYMBOL_REFRESH,  3,  0, 5,  "lv" },
    { "AC Mode",     LV_SYMBOL_LOOP,     8,  0, 12, ""   },
    { "Temperature", LV_SYMBOL_UP,       24, 16, 30, "\xc2\xb0""C" },
    { "Brightness",  LV_SYMBOL_EYE_OPEN, 2,  0, 5,  ""   },
    { "Volume",      LV_SYMBOL_VOLUME_MAX, 6, 0, 10, ""  },
};

/* ── Screens ─────────────────────────────────────────────────────────── */

static lv_obj_t *scr_main  = NULL;
static lv_obj_t *scr_sub   = NULL;
static lv_group_t *group   = NULL;

/* main screen objects */
static lv_obj_t *list      = NULL;

/* sub screen objects */
static lv_obj_t *sub_title   = NULL;
static lv_obj_t *sub_slider  = NULL;
static lv_obj_t *sub_val_lbl = NULL;
static lv_obj_t *sub_back_btn= NULL;

static uint8_t current_item = 0;

/* ── Helpers ─────────────────────────────────────────────────────────── */

static void update_sub_value(void)
{
    MenuItem *m = &items[current_item];
    lv_slider_set_range(sub_slider, m->min, m->max);
    lv_slider_set_value(sub_slider, m->val, LV_ANIM_ON);

    char buf[32];
    lv_snprintf(buf, sizeof(buf), "%d %s", m->val, m->unit);
    lv_label_set_text(sub_val_lbl, buf);
}

/* ── Callbacks ───────────────────────────────────────────────────────── */

static void list_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED) {
        /* find which list button was clicked */
        uint8_t i;
        for (i = 0; i < MENU_ITEM_COUNT; i++) {
            if (lv_obj_get_child(list, i) == obj) {
                current_item = i;
                break;
            }
        }
        /* switch to sub screen */
        lv_label_set_text(sub_title, items[current_item].name);
        update_sub_value();
        lv_group_remove_all_objs(group);
        lv_group_add_obj(group, sub_slider);
        lv_group_add_obj(group, sub_back_btn);
        lv_group_focus_obj(sub_slider);
        lv_scr_load_anim(scr_sub, LV_SCR_LOAD_ANIM_MOVE_LEFT, 200, 0, false);
        LOG_Printf("[UI] Enter: %s\r\n", items[current_item].name);
    }
}

static void slider_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        items[current_item].val = (uint8_t)lv_slider_get_value(sub_slider);
        update_sub_value();

        /* Apply value in real time */
        if (current_item == 3) {  /* Brightness */
            static const uint32_t BrightnessTable[5] = {20, 40, 60, 80, 98};
            uint8_t idx = items[current_item].val;
            if (idx > 4) idx = 4;
            LCD_BrightnessSetting(BrightnessTable[idx]);
        }
        LOG_Printf("[UI] %s = %d\r\n",
                   items[current_item].name, items[current_item].val);
    }
}

static void back_btn_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_group_remove_all_objs(group);
        /* re-add all list buttons */
        uint8_t i;
        for (i = 0; i < MENU_ITEM_COUNT; i++) {
            lv_group_add_obj(group, lv_obj_get_child(list, i));
        }
        lv_group_focus_obj(lv_obj_get_child(list, current_item));
        lv_scr_load_anim(scr_main, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 200, 0, false);
    }
}

/* ── Build UI ────────────────────────────────────────────────────────── */

static void build_main_screen(void)
{
    scr_main = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_main, lv_color_hex(0x1a1a2e), 0);
    lv_obj_set_style_bg_opa(scr_main, LV_OPA_COVER, 0);

    /* Title */
    lv_obj_t *title = lv_label_create(scr_main);
    lv_label_set_text(title, "Encoder Control");
    lv_obj_set_style_text_color(title, lv_color_hex(0xe0e0ff), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);

    /* Divider line */
    lv_obj_t *line = lv_obj_create(scr_main);
    lv_obj_set_size(line, 200, 1);
    lv_obj_align(line, LV_ALIGN_TOP_MID, 0, 28);
    lv_obj_set_style_bg_color(line, lv_color_hex(0x4444aa), 0);
    lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(line, 0, 0);
    lv_obj_clear_flag(line, LV_OBJ_FLAG_SCROLLABLE);

    /* List container */
    list = lv_obj_create(scr_main);
    lv_obj_set_size(list, 220, 175);
    lv_obj_align(list, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_style_pad_all(list, 4, 0);
    lv_obj_set_style_pad_row(list, 4, 0);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_snap_y(list, LV_SCROLL_SNAP_CENTER);

    uint8_t i;
    for (i = 0; i < MENU_ITEM_COUNT; i++) {
        lv_obj_t *btn = lv_btn_create(list);
        lv_obj_set_width(btn, LV_PCT(100));
        lv_obj_set_height(btn, 32);
        lv_obj_set_style_radius(btn, 8, 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x16213e), 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x0f3460), LV_STATE_FOCUSED);
        lv_obj_set_style_border_color(btn, lv_color_hex(0x533483), LV_STATE_FOCUSED);
        lv_obj_set_style_border_width(btn, 2, LV_STATE_FOCUSED);

        /* icon + label row */
        lv_obj_t *row = lv_obj_create(btn);
        lv_obj_set_size(row, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_pad_all(row, 0, 0);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START,
                              LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_CLICKABLE);

        lv_obj_t *icon_lbl = lv_label_create(row);
        lv_label_set_text(icon_lbl, items[i].icon);
        lv_obj_set_style_text_color(icon_lbl, lv_color_hex(0x53c8f5), 0);
        lv_obj_set_width(icon_lbl, 24);

        lv_obj_t *name_lbl = lv_label_create(row);
        lv_label_set_text(name_lbl, items[i].name);
        lv_obj_set_style_text_color(name_lbl, lv_color_hex(0xe0e0ff), 0);

        /* value badge on right */
        lv_obj_t *val_lbl = lv_label_create(btn);
        char buf[16];
        lv_snprintf(buf, sizeof(buf), "%d", items[i].val);
        lv_label_set_text(val_lbl, buf);
        lv_obj_set_style_text_color(val_lbl, lv_color_hex(0xaaaacc), 0);
        lv_obj_align(val_lbl, LV_ALIGN_RIGHT_MID, -4, 0);

        lv_obj_add_event_cb(btn, list_event_cb, LV_EVENT_CLICKED, NULL);
        lv_group_add_obj(group, btn);
    }
}

static void build_sub_screen(void)
{
    scr_sub = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_sub, lv_color_hex(0x1a1a2e), 0);
    lv_obj_set_style_bg_opa(scr_sub, LV_OPA_COVER, 0);

    /* Title */
    sub_title = lv_label_create(scr_sub);
    lv_label_set_text(sub_title, "");
    lv_obj_set_style_text_color(sub_title, lv_color_hex(0x53c8f5), 0);
    lv_obj_align(sub_title, LV_ALIGN_TOP_MID, 0, 12);

    /* Value label (large, center) */
    sub_val_lbl = lv_label_create(scr_sub);
    lv_label_set_text(sub_val_lbl, "0");
    lv_obj_set_style_text_color(sub_val_lbl, lv_color_hex(0xffffff), 0);
    lv_obj_align(sub_val_lbl, LV_ALIGN_CENTER, 0, -20);

    /* Slider */
    sub_slider = lv_slider_create(scr_sub);
    lv_obj_set_size(sub_slider, 180, 12);
    lv_obj_align(sub_slider, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_bg_color(sub_slider, lv_color_hex(0x0f3460), LV_PART_MAIN);
    lv_obj_set_style_bg_color(sub_slider, lv_color_hex(0x533483), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(sub_slider, lv_color_hex(0x53c8f5), LV_PART_KNOB);
    lv_obj_add_event_cb(sub_slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /* Back button */
    sub_back_btn = lv_btn_create(scr_sub);
    lv_obj_set_size(sub_back_btn, 80, 32);
    lv_obj_align(sub_back_btn, LV_ALIGN_BOTTOM_MID, 0, -16);
    lv_obj_set_style_bg_color(sub_back_btn, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_bg_color(sub_back_btn, lv_color_hex(0x533483), LV_STATE_FOCUSED);
    lv_obj_t *back_lbl = lv_label_create(sub_back_btn);
    lv_label_set_text(back_lbl, LV_SYMBOL_LEFT " Back");
    lv_obj_set_style_text_color(back_lbl, lv_color_hex(0xe0e0ff), 0);
    lv_obj_center(back_lbl);
    lv_obj_add_event_cb(sub_back_btn, back_btn_cb, LV_EVENT_CLICKED, NULL);
}

/* ── Public API ──────────────────────────────────────────────────────── */

void ui_menu_init(void)
{
    group = lv_group_create();
    lv_group_set_default(group);
    lv_indev_set_group(lv_port_indev, group);

    build_main_screen();
    build_sub_screen();

    lv_scr_load(scr_main);
    LOG_Printf("[UI] LVGL menu ready\r\n");
}
