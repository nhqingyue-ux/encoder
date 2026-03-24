/**
 * ui_menu.c - LVGL menu for round 240x240 encoder controller
 *
 * Design for circular display:
 *   - Main screen: center-focused carousel, 3 visible items
 *     (selected item large in center, neighbors smaller above/below)
 *   - Sub screen:  arc indicator for parameter value
 *   - All elements stay within r=110 safe zone
 *
 * Navigation:
 *   Rotate encoder → cycle through items / adjust value
 *   Press SW       → enter sub-screen / back to main
 */
#include "ui_menu.h"
#include "lv_port.h"
#include "lvgl/lvgl.h"
#include "SDK.h"
#include "main.h"

/* ── Constants ───────────────────────────────────────────────────────── */
#define SCREEN_SIZE     240
#define CENTER          120
#define MENU_ITEM_COUNT 5

/* ── Menu definition ─────────────────────────────────────────────────── */
typedef struct {
    const char *name;
    const char *icon;
    uint8_t     val;
    uint8_t     min;
    uint8_t     max;
    const char *unit;
} MenuItem;

static MenuItem items[MENU_ITEM_COUNT] = {
    { "Fan",         LV_SYMBOL_REFRESH,    3,  0,  5, "lv" },
    { "AC Mode",     LV_SYMBOL_LOOP,       8,  0, 12, ""   },
    { "Temp",        LV_SYMBOL_UP,        24, 16, 30, "\xc2\xb0""C" },
    { "Bright",      LV_SYMBOL_EYE_OPEN,   2,  0,  5, ""   },
    { "Volume",      LV_SYMBOL_VOLUME_MAX,  6,  0, 10, ""  },
};

/* ── State ───────────────────────────────────────────────────────────── */
static lv_obj_t  *scr_main = NULL;
static lv_obj_t  *scr_sub  = NULL;
static lv_group_t *group   = NULL;
static uint8_t    sel      = 0;   /* selected item index */

/* Main screen objects */
static lv_obj_t *main_icon_lbl  = NULL;   /* center icon */
static lv_obj_t *main_name_lbl  = NULL;   /* center name */
static lv_obj_t *main_val_lbl   = NULL;   /* center value */
static lv_obj_t *main_prev_lbl  = NULL;   /* previous item (top) */
static lv_obj_t *main_next_lbl  = NULL;   /* next item (bottom) */
static lv_obj_t *main_dot[MENU_ITEM_COUNT]; /* dot indicators */
static lv_obj_t *main_focus_btn = NULL;   /* invisible focusable btn */

/* Sub screen objects */
static lv_obj_t *sub_title_lbl  = NULL;
static lv_obj_t *sub_val_lbl    = NULL;
static lv_obj_t *sub_arc        = NULL;
static lv_obj_t *sub_focus_btn  = NULL;   /* invisible focusable btn */

/* ── Colors ──────────────────────────────────────────────────────────── */
#define COL_BG       0x0a0a1a
#define COL_TEXT     0xe0e0ff
#define COL_DIM      0x555577
#define COL_ACCENT   0x53c8f5
#define COL_ARC_BG   0x1a1a3e
#define COL_ARC_IND  0x533483
#define COL_DOT_ACT  0x53c8f5
#define COL_DOT_DIM  0x333355

/* ── Helpers ─────────────────────────────────────────────────────────── */

static void refresh_main_screen(void)
{
    MenuItem *m = &items[sel];

    /* Center item */
    lv_label_set_text(main_icon_lbl, m->icon);
    lv_label_set_text(main_name_lbl, m->name);

    char buf[16];
    lv_snprintf(buf, sizeof(buf), "%d %s", m->val, m->unit);
    lv_label_set_text(main_val_lbl, buf);

    /* Previous item (above) */
    uint8_t prev = (sel == 0) ? MENU_ITEM_COUNT - 1 : sel - 1;
    lv_snprintf(buf, sizeof(buf), "%s %s", items[prev].icon, items[prev].name);
    lv_label_set_text(main_prev_lbl, buf);

    /* Next item (below) */
    uint8_t next = (sel + 1) % MENU_ITEM_COUNT;
    lv_snprintf(buf, sizeof(buf), "%s %s", items[next].icon, items[next].name);
    lv_label_set_text(main_next_lbl, buf);

    /* Dot indicators */
    uint8_t i;
    for (i = 0; i < MENU_ITEM_COUNT; i++) {
        lv_obj_set_style_bg_color(main_dot[i],
            lv_color_hex(i == sel ? COL_DOT_ACT : COL_DOT_DIM), 0);
    }
}

/* ── Callbacks ───────────────────────────────────────────────────────── */

static void main_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_KEY) {
        uint32_t key = lv_event_get_key(e);
        if (key == LV_KEY_RIGHT) {
            sel = (sel + 1) % MENU_ITEM_COUNT;
            refresh_main_screen();
        } else if (key == LV_KEY_LEFT) {
            sel = (sel == 0) ? MENU_ITEM_COUNT - 1 : sel - 1;
            refresh_main_screen();
        }
    } else if (code == LV_EVENT_CLICKED || code == LV_EVENT_PRESSED) {
        /* Enter sub screen */
        MenuItem *m = &items[sel];
        lv_label_set_text(sub_title_lbl, m->name);
        lv_arc_set_range(sub_arc, m->min, m->max);
        lv_arc_set_value(sub_arc, m->val);

        char buf[16];
        lv_snprintf(buf, sizeof(buf), "%d %s", m->val, m->unit);
        lv_label_set_text(sub_val_lbl, buf);

        lv_group_remove_all_objs(group);
        lv_group_add_obj(group, sub_arc);
        lv_group_focus_obj(sub_arc);
        lv_scr_load(scr_sub);
        LOG_Printf("[UI] Enter: %s\r\n", m->name);
    }
}

static void sub_arc_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        int16_t v = lv_arc_get_value(sub_arc);
        items[sel].val = (uint8_t)v;

        char buf[16];
        lv_snprintf(buf, sizeof(buf), "%d %s", v, items[sel].unit);
        lv_label_set_text(sub_val_lbl, buf);

        /* Real-time brightness control */
        if (sel == 3) {
            extern const uint32_t BrightnessTable[5];
            uint8_t idx = (uint8_t)v;
            if (idx > 4) idx = 4;
            LCD_BrightnessSetting(BrightnessTable[idx]);
        }
        LOG_Printf("[UI] %s = %d\r\n", items[sel].name, v);
    } else if (code == LV_EVENT_CLICKED || code == LV_EVENT_PRESSED) {
        /* Back to main */
        lv_group_remove_all_objs(group);
        lv_group_add_obj(group, main_focus_btn);
        lv_group_focus_obj(main_focus_btn);
        refresh_main_screen();
        lv_scr_load(scr_main);
    }
}

/* ── Build Main Screen ───────────────────────────────────────────────── */

static void build_main_screen(void)
{
    scr_main = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_main, lv_color_hex(COL_BG), 0);
    lv_obj_set_style_bg_opa(scr_main, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(scr_main, 0, 0);
    lv_obj_clear_flag(scr_main, LV_OBJ_FLAG_SCROLLABLE);

    /* ── Previous item (top, dimmed) ─────────────────────── */
    main_prev_lbl = lv_label_create(scr_main);
    lv_obj_set_style_text_color(main_prev_lbl, lv_color_hex(COL_DIM), 0);
    lv_obj_align(main_prev_lbl, LV_ALIGN_TOP_MID, 0, 45);

    /* ── Center icon (large) ─────────────────────────────── */
    main_icon_lbl = lv_label_create(scr_main);
    lv_obj_set_style_text_color(main_icon_lbl, lv_color_hex(COL_ACCENT), 0);
    lv_obj_align(main_icon_lbl, LV_ALIGN_CENTER, 0, -30);

    /* ── Center name ─────────────────────────────────────── */
    main_name_lbl = lv_label_create(scr_main);
    lv_obj_set_style_text_color(main_name_lbl, lv_color_hex(COL_TEXT), 0);
    lv_obj_align(main_name_lbl, LV_ALIGN_CENTER, 0, -10);

    /* ── Center value ────────────────────────────────────── */
    main_val_lbl = lv_label_create(scr_main);
    lv_obj_set_style_text_color(main_val_lbl, lv_color_hex(COL_ACCENT), 0);
    lv_obj_align(main_val_lbl, LV_ALIGN_CENTER, 0, 10);

    /* ── Next item (bottom, dimmed) ──────────────────────── */
    main_next_lbl = lv_label_create(scr_main);
    lv_obj_set_style_text_color(main_next_lbl, lv_color_hex(COL_DIM), 0);
    lv_obj_align(main_next_lbl, LV_ALIGN_BOTTOM_MID, 0, -45);

    /* ── Dot indicators (horizontal, near bottom) ────────── */
    int16_t dot_start_x = -(MENU_ITEM_COUNT * 12 - 4) / 2;
    uint8_t i;
    for (i = 0; i < MENU_ITEM_COUNT; i++) {
        main_dot[i] = lv_obj_create(scr_main);
        lv_obj_set_size(main_dot[i], 8, 8);
        lv_obj_set_style_radius(main_dot[i], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(main_dot[i], 0, 0);
        lv_obj_set_style_bg_opa(main_dot[i], LV_OPA_COVER, 0);
        lv_obj_align(main_dot[i], LV_ALIGN_CENTER, dot_start_x + i * 12, 35);
        lv_obj_clear_flag(main_dot[i], LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    }

    /* ── Invisible focusable button for encoder input ────── */
    main_focus_btn = lv_btn_create(scr_main);
    lv_obj_set_size(main_focus_btn, 1, 1);
    lv_obj_set_style_bg_opa(main_focus_btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(main_focus_btn, 0, 0);
    lv_obj_set_style_shadow_width(main_focus_btn, 0, 0);
    lv_obj_set_style_outline_width(main_focus_btn, 0, 0);
    lv_obj_set_style_outline_width(main_focus_btn, 0, LV_STATE_FOCUSED);
    lv_obj_align(main_focus_btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(main_focus_btn, main_event_cb, LV_EVENT_ALL, NULL);

    refresh_main_screen();
}

/* ── Build Sub Screen ────────────────────────────────────────────────── */

static void build_sub_screen(void)
{
    scr_sub = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_sub, lv_color_hex(COL_BG), 0);
    lv_obj_set_style_bg_opa(scr_sub, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(scr_sub, 0, 0);
    lv_obj_clear_flag(scr_sub, LV_OBJ_FLAG_SCROLLABLE);

    /* ── Title (top center) ──────────────────────────────── */
    sub_title_lbl = lv_label_create(scr_sub);
    lv_label_set_text(sub_title_lbl, "");
    lv_obj_set_style_text_color(sub_title_lbl, lv_color_hex(COL_ACCENT), 0);
    lv_obj_align(sub_title_lbl, LV_ALIGN_CENTER, 0, -50);

    /* ── Arc indicator ───────────────────────────────────── */
    sub_arc = lv_arc_create(scr_sub);
    lv_obj_set_size(sub_arc, 180, 180);
    lv_obj_align(sub_arc, LV_ALIGN_CENTER, 0, 0);
    lv_arc_set_rotation(sub_arc, 135);
    lv_arc_set_bg_angles(sub_arc, 0, 270);
    lv_arc_set_range(sub_arc, 0, 100);
    lv_obj_set_style_arc_width(sub_arc, 12, LV_PART_MAIN);
    lv_obj_set_style_arc_color(sub_arc, lv_color_hex(COL_ARC_BG), LV_PART_MAIN);
    lv_obj_set_style_arc_width(sub_arc, 12, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(sub_arc, lv_color_hex(COL_ACCENT), LV_PART_INDICATOR);
    /* Hide the knob */
    lv_obj_set_style_pad_all(sub_arc, 0, LV_PART_KNOB);
    lv_obj_set_style_bg_opa(sub_arc, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_add_event_cb(sub_arc, sub_arc_event_cb, LV_EVENT_ALL, NULL);

    /* ── Value label (large, center of arc) ──────────────── */
    sub_val_lbl = lv_label_create(scr_sub);
    lv_label_set_text(sub_val_lbl, "0");
    lv_obj_set_style_text_color(sub_val_lbl, lv_color_hex(COL_TEXT), 0);
    lv_obj_align(sub_val_lbl, LV_ALIGN_CENTER, 0, 10);

    /* ── Hint text ───────────────────────────────────────── */
    lv_obj_t *hint = lv_label_create(scr_sub);
    lv_label_set_text(hint, "press to back");
    lv_obj_set_style_text_color(hint, lv_color_hex(COL_DIM), 0);
    lv_obj_align(hint, LV_ALIGN_CENTER, 0, 55);
}

/* ── Public API ──────────────────────────────────────────────────────── */

void ui_menu_init(void)
{
    group = lv_group_create();
    lv_group_set_default(group);
    lv_indev_set_group(lv_port_indev, group);

    build_main_screen();
    build_sub_screen();

    lv_group_add_obj(group, main_focus_btn);
    lv_group_focus_obj(main_focus_btn);

    lv_scr_load(scr_main);
    LOG_Printf("[UI] LVGL menu ready\r\n");
}
