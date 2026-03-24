/**
 * ui_menu.c - Encoder input indicator for round 240x240 display
 *
 * Shows directional feedback on circular LCD:
 *   Clockwise rotation  → "R" (Right)
 *   Counter-clockwise   → "L" (Left)
 *   Button press        → "Y" (Yes/Confirm)
 *
 * Each indicator displays briefly then fades back to idle.
 */
#include "ui_menu.h"
#include "lv_port.h"
#include "lvgl/lvgl.h"
#include "SDK.h"
#include "main.h"
#include "comm.h"

/* ── Constants ───────────────────────────────────────────────────────── */
#define INDICATOR_HOLD_MS  400   /* how long indicator stays visible */

/* ── Colors ──────────────────────────────────────────────────────────── */
#define COL_BG          0x0a0a1a
#define COL_IDLE        0x333355
#define COL_RIGHT       0x00cc88   /* green-cyan for R */
#define COL_LEFT        0x5588ff   /* blue for L */
#define COL_YES         0xff8800   /* orange for Y */
#define COL_TEXT_DIM    0x555577
#define COL_RING        0x1a1a3e

/* ── State ───────────────────────────────────────────────────────────── */
typedef enum {
    STATE_IDLE = 0,
    STATE_RIGHT,
    STATE_LEFT,
    STATE_YES,
} IndicatorState;

static IndicatorState cur_state = STATE_IDLE;

/* ── LVGL objects ────────────────────────────────────────────────────── */
static lv_obj_t *scr        = NULL;
static lv_obj_t *ring_arc   = NULL;   /* decorative ring */
static lv_obj_t *big_lbl    = NULL;   /* big letter: R / L / Y / - */
static lv_obj_t *desc_lbl   = NULL;   /* description: Right / Left / Yes / Idle */
static lv_obj_t *hint_lbl   = NULL;   /* bottom hint */
static lv_obj_t *focus_btn  = NULL;   /* invisible encoder target */
static lv_timer_t *fade_tmr = NULL;   /* timer to return to idle */

/* ── Idle text ───────────────────────────────────────────────────────── */
static const char *idle_char = "-";
static const char *idle_desc = "Ready";

/* ── Update display ──────────────────────────────────────────────────── */

static void set_indicator(IndicatorState st)
{
    cur_state = st;
    uint32_t color;
    const char *ch;
    const char *desc;

    switch (st) {
    case STATE_RIGHT:
        color = COL_RIGHT; ch = "R"; desc = "Right";  break;
    case STATE_LEFT:
        color = COL_LEFT;  ch = "L"; desc = "Left";   break;
    case STATE_YES:
        color = COL_YES;   ch = "Y"; desc = "Yes";    break;
    default:
        color = COL_IDLE;  ch = idle_char; desc = idle_desc; break;
    }

    lv_label_set_text(big_lbl, ch);
    lv_obj_set_style_text_color(big_lbl, lv_color_hex(color), 0);

    lv_label_set_text(desc_lbl, desc);
    lv_obj_set_style_text_color(desc_lbl, lv_color_hex(color), 0);

    /* Animate the ring arc */
    if (st != STATE_IDLE) {
        lv_obj_set_style_arc_color(ring_arc, lv_color_hex(color), LV_PART_INDICATOR);
        lv_arc_set_value(ring_arc, 100);
    } else {
        lv_obj_set_style_arc_color(ring_arc, lv_color_hex(COL_RING), LV_PART_INDICATOR);
        lv_arc_set_value(ring_arc, 0);
    }
}

static void fade_timer_cb(lv_timer_t *tmr)
{
    (void)tmr;
    set_indicator(STATE_IDLE);
    /* one-shot: pause until next event */
    lv_timer_pause(fade_tmr);
}

static void trigger_indicator(IndicatorState st)
{
    set_indicator(st);

    /* Reset fade timer */
    if (fade_tmr) {
        lv_timer_reset(fade_tmr);
        lv_timer_resume(fade_tmr);
    }
}

/* ── Encoder event handler ───────────────────────────────────────────── */

static void enc_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_KEY) {
        uint32_t key = lv_event_get_key(e);
        if (key == LV_KEY_RIGHT) {
            trigger_indicator(STATE_RIGHT);
            COMM_SendRight();
            LOG_Printf("[ENC] R\r\n");
        } else if (key == LV_KEY_LEFT) {
            trigger_indicator(STATE_LEFT);
            COMM_SendLeft();
            LOG_Printf("[ENC] L\r\n");
        }
    } else if (code == LV_EVENT_CLICKED || code == LV_EVENT_PRESSED) {
        trigger_indicator(STATE_YES);
        COMM_SendYes();
        LOG_Printf("[ENC] Y\r\n");
    }
}

/* ── Build screen ────────────────────────────────────────────────────── */

static void build_screen(void)
{
    scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(COL_BG), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(scr, 0, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    /* ── Decorative ring (full circle arc) ───────────────── */
    ring_arc = lv_arc_create(scr);
    lv_obj_set_size(ring_arc, 210, 210);
    lv_obj_align(ring_arc, LV_ALIGN_CENTER, 0, 0);
    lv_arc_set_rotation(ring_arc, 270);
    lv_arc_set_bg_angles(ring_arc, 0, 360);
    lv_arc_set_range(ring_arc, 0, 100);
    lv_arc_set_value(ring_arc, 0);
    lv_obj_set_style_arc_width(ring_arc, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_color(ring_arc, lv_color_hex(COL_RING), LV_PART_MAIN);
    lv_obj_set_style_arc_width(ring_arc, 6, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(ring_arc, lv_color_hex(COL_RING), LV_PART_INDICATOR);
    /* Hide knob */
    lv_obj_set_style_pad_all(ring_arc, 0, LV_PART_KNOB);
    lv_obj_set_style_bg_opa(ring_arc, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_clear_flag(ring_arc, LV_OBJ_FLAG_CLICKABLE);

    /* ── Big letter ──────────────────────────────────────── */
    big_lbl = lv_label_create(scr);
    lv_label_set_text(big_lbl, idle_char);
    lv_obj_set_style_text_color(big_lbl, lv_color_hex(COL_IDLE), 0);
    lv_obj_align(big_lbl, LV_ALIGN_CENTER, 0, -15);

    /* ── Description ─────────────────────────────────────── */
    desc_lbl = lv_label_create(scr);
    lv_label_set_text(desc_lbl, idle_desc);
    lv_obj_set_style_text_color(desc_lbl, lv_color_hex(COL_IDLE), 0);
    lv_obj_align(desc_lbl, LV_ALIGN_CENTER, 0, 10);

    /* ── Bottom hint ─────────────────────────────────────── */
    hint_lbl = lv_label_create(scr);
    lv_label_set_text(hint_lbl, "encoder input");
    lv_obj_set_style_text_color(hint_lbl, lv_color_hex(COL_TEXT_DIM), 0);
    lv_obj_align(hint_lbl, LV_ALIGN_CENTER, 0, 50);

    /* ── Invisible focusable button ──────────────────────── */
    focus_btn = lv_btn_create(scr);
    lv_obj_set_size(focus_btn, 1, 1);
    lv_obj_set_style_bg_opa(focus_btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(focus_btn, 0, 0);
    lv_obj_set_style_shadow_width(focus_btn, 0, 0);
    lv_obj_set_style_outline_width(focus_btn, 0, 0);
    lv_obj_set_style_outline_width(focus_btn, 0, LV_STATE_FOCUSED);
    lv_obj_align(focus_btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(focus_btn, enc_event_cb, LV_EVENT_ALL, NULL);
}

/* ── Public API ──────────────────────────────────────────────────────── */

void ui_menu_init(void)
{
    lv_group_t *group_local = lv_group_create();
    lv_group_set_default(group_local);
    lv_indev_set_group(lv_port_indev, group_local);

    build_screen();

    lv_group_add_obj(group_local, focus_btn);
    lv_group_focus_obj(focus_btn);

    /* Fade-back timer (one-shot, starts paused) */
    fade_tmr = lv_timer_create(fade_timer_cb, INDICATOR_HOLD_MS, NULL);
    lv_timer_pause(fade_tmr);

    lv_scr_load(scr);
    LOG_Printf("[UI] Encoder indicator ready\r\n");
}
