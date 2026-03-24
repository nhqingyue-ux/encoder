/**
 * lv_conf.h - LVGL v8 config for STM32G070 (128KB Flash, 36KB RAM)
 */

#if 1  /* required sentinel */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   COLOR SETTINGS
 *====================*/
#define LV_COLOR_DEPTH     16      /* RGB565 */
#define LV_COLOR_16_SWAP   1       /* SPI LCD byte swap */
#define LV_COLOR_SCREEN_TRANSP 0

/*====================
   MEMORY SETTINGS
 *====================*/
#define LV_MEM_CUSTOM      0
#define LV_MEM_SIZE        (12 * 1024U)  /* 12KB internal heap */
#define LV_MEM_ADR         0
#define LV_MEM_AUTO_DEFRAG 1
#define LV_MEM_BUF_MAX_NUM 16

/*====================
   HAL SETTINGS
 *====================*/
#define LV_DISP_DEF_REFR_PERIOD   20   /* ms */
#define LV_INDEV_DEF_READ_PERIOD  30   /* ms */
#define LV_TICK_CUSTOM            1
#define LV_TICK_CUSTOM_INCLUDE    "stm32g0xx_hal.h"
#define LV_TICK_CUSTOM_SYS_TIME_EXPR HAL_GetTick()

/*====================
   DISPLAY BUFFER
 *====================*/
/* 240 x 10 lines x 2 bytes = 4800 bytes, fits in RAM */
#define LV_HOR_RES_MAX   240
#define LV_VER_RES_MAX   240

/*====================
   LOGGING
 *====================*/
#define LV_USE_LOG       0

/*====================
   ASSERTS
 *====================*/
#define LV_USE_ASSERT_NULL          0
#define LV_USE_ASSERT_MALLOC        0
#define LV_USE_ASSERT_STYLE         0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ           0

/*====================
   COMPILER
 *====================*/
#define LV_ATTRIBUTE_FAST_MEM
#define LV_ATTRIBUTE_MEM_ALIGN
#define LV_EXPORT_CONST_INT(int_value) struct _silence_gcc_warning
#define LV_USE_LARGE_COORD 0

/*====================
   FONTS (minimal - use unscii_8 ~1.5KB vs montserrat_14 ~13KB)
 *====================*/
#define LV_FONT_MONTSERRAT_14  0
#define LV_FONT_MONTSERRAT_16  0
#define LV_FONT_MONTSERRAT_20  0
#define LV_FONT_MONTSERRAT_24  0
#define LV_FONT_MONTSERRAT_48  0
#define LV_FONT_DEFAULT        &lv_font_unscii_8

/* Disable all other built-in fonts */
#define LV_FONT_MONTSERRAT_8   0
#define LV_FONT_MONTSERRAT_10  0
#define LV_FONT_MONTSERRAT_12  0
#define LV_FONT_MONTSERRAT_18  0
#define LV_FONT_MONTSERRAT_22  0
#define LV_FONT_MONTSERRAT_26  0
#define LV_FONT_MONTSERRAT_28  0
#define LV_FONT_MONTSERRAT_30  0
#define LV_FONT_MONTSERRAT_32  0
#define LV_FONT_MONTSERRAT_34  0
#define LV_FONT_MONTSERRAT_36  0
#define LV_FONT_MONTSERRAT_38  0
#define LV_FONT_MONTSERRAT_40  0
#define LV_FONT_MONTSERRAT_42  0
#define LV_FONT_MONTSERRAT_44  0
#define LV_FONT_MONTSERRAT_46  0
#define LV_FONT_UNSCII_8       1
#define LV_FONT_UNSCII_16      0
#define LV_FONT_SIMSUN_16_CJK  0
#define LV_FONT_DEJAVU_16_PERSIAN_HEBREW 0

/*====================
   WIDGETS (only what we need)
 *====================*/
#define LV_USE_ARC          0
#define LV_USE_BAR          1
#define LV_USE_BTN          1
#define LV_USE_BTNMATRIX    0
#define LV_USE_CANVAS       0
#define LV_USE_CHECKBOX     0
#define LV_USE_DROPDOWN     0
#define LV_USE_IMG          1
#define LV_USE_LABEL        1
#define LV_USE_LINE         0
#define LV_USE_ROLLER       0
#define LV_USE_SLIDER       1
#define LV_USE_SWITCH       0
#define LV_USE_TEXTAREA     0
#define LV_USE_TABLE        0
#define LV_USE_CHART        0
#define LV_USE_COLORWHEEL   0
#define LV_USE_IMGBTN       0
#define LV_USE_KEYBOARD     0
#define LV_USE_LED          0
#define LV_USE_LIST         1
#define LV_USE_MENU         0
#define LV_USE_METER        0
#define LV_USE_MSGBOX       0
#define LV_USE_OBJMASK      0
#define LV_USE_SPINBOX      0
#define LV_USE_SPINNER      0
#define LV_USE_TABVIEW      0
#define LV_USE_TILEVIEW     0
#define LV_USE_WIN          0

/*====================
   THEMES
 *====================*/
#define LV_USE_THEME_DEFAULT  0
#define LV_THEME_DEFAULT_DARK 0
#define LV_USE_THEME_BASIC    1
#define LV_USE_THEME_MONO     0

/*====================
   LAYOUTS
 *====================*/
#define LV_USE_FLEX    1
#define LV_USE_GRID    0

/*====================
   EXTRA (disable all)
 *====================*/
#define LV_USE_SNAPSHOT      0
#define LV_USE_MONKEY        0
#define LV_USE_GRIDNAV       0
#define LV_USE_FRAGMENT      0
#define LV_USE_IMGFONT       0
#define LV_USE_GPU_STM32_DMA2D 0
#define LV_USE_GPU_NXP_PXP   0
#define LV_USE_GPU_NXP_VG_LITE 0
#define LV_USE_GPU_SDL       0
#define LV_USE_FFMPEG        0
#define LV_USE_FSDRV         0
#define LV_USE_FS_STDIO      0
#define LV_USE_FS_POSIX      0
#define LV_USE_FS_WIN32      0
#define LV_USE_FS_FATFS      0
#define LV_USE_PNG           0
#define LV_USE_BMP           0
#define LV_USE_SJPG          0
#define LV_USE_GIF           0
#define LV_USE_QRCODE        0
#define LV_USE_RLOTTIE       0
#define LV_USE_PERF_MONITOR  0
#define LV_USE_MEM_MONITOR   0
#define LV_USE_REFR_DEBUG    0

#define LV_BUILD_EXAMPLES    0
#define LV_USE_DEMO_WIDGETS        0
#define LV_USE_DEMO_KEYPAD_AND_ENCODER 0
#define LV_USE_DEMO_BENCHMARK  0
#define LV_USE_DEMO_STRESS     0
#define LV_USE_DEMO_MUSIC      0

#endif /* LV_CONF_H */
#endif /* end sentinel */
