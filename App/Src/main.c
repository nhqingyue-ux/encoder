/**
 * main.c - Application entry point
 *
 * STM32G070 encoder demo with LVGL menu UI.
 * Hardware: 240x240 SPI LCD, rotary encoder (SW on PF1),
 *           W25Qxx external flash (SPI2).
 */
#include "main.h"
#include "SDK.h"
#include "lvgl/lvgl.h"
#include "lv_port.h"
#include "ui_menu.h"
#include "comm.h"

/* ── Forward declarations ──────────────────────────────────────────── */
void SystemClock_Config(void);

/* ── Brightness table (shared with ui_menu.c) ──────────────────────── */
const uint32_t BrightnessTable[5] = {20, 40, 60, 80, 98};

/* ── Main ──────────────────────────────────────────────────────────── */
int main(void)
{
    HAL_Init();
    SystemClock_Config();

    LOG_Init();
    LOG_Printf("\r\n[BOOT] encoder demo start\r\n");

    MX_GPIO_Init();
    MX_SPI1_Init();
    MX_SPI2_Init();
    LOG_Printf("[BOOT] GPIO/SPI init done\r\n");

    LCD_init();
    LOG_Printf("[BOOT] LCD init done\r\n");

    bool flash_ok = Flash_Init();
    LOG_Printf("[BOOT] Flash init %s\r\n", flash_ok ? "OK" : "FAIL");

    Encoder_Init();
    LOG_Printf("[BOOT] Encoder init done\r\n");

    COMM_Init();

    /* Set LCD brightness */
    LCD_BrightnessInit(BrightnessTable[2]);  /* default: level 2 = 60% */
    LOG_Printf("[BOOT] Brightness=60\r\n");

    /* ── LVGL init ─────────────────────────────────────────────────── */
    LOG_Printf("[LVGL] lv_init...\r\n");
    lv_init();
    LOG_Printf("[LVGL] lv_port_init...\r\n");
    lv_port_init();
    LOG_Printf("[LVGL] ui_menu_init...\r\n");
    ui_menu_init();
    LOG_Printf("[MAIN] LVGL ready, entering main loop\r\n");

    /* Encoder raw value for delta calculation */
    static uint8_t enc_raw = 64;
    static uint8_t enc_raw_prev = 64;

    while (1) {
        /* ── Read encoder hardware ─────────────────────────────────── */
        Enocde_Process();
        Encoder2Index(&enc_raw, 127, 0, 0, 1);
        int8_t delta = (int8_t)((int16_t)enc_raw - (int16_t)enc_raw_prev);
        enc_raw_prev = enc_raw;

        bool sw = (HAL_GPIO_ReadPin(SW_Port, SW_Pin) == GPIO_PIN_RESET);
        if (delta != 0 || sw) {
            lv_port_enc_update(delta, sw);
        }

        /* ── LVGL tick + handler ───────────────────────────────────── */
        lv_timer_handler();
    }
}

/* ── System Clock: HSI 16MHz → PLL → 64MHz SYSCLK ─────────────────── */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
    RCC_OscInitStruct.PLL.PLLN = 8;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
        Error_Handler();

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
        Error_Handler();
}

/* ── Error handler ─────────────────────────────────────────────────── */
void Error_Handler(void)
{
    __disable_irq();
    while (1) { }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
}
#endif
