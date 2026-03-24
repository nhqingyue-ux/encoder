/**
 * uart_log.c - Debug logger via USART3 (debug builds only)
 *
 * USART3 hardware is initialized by comm.c (COMM_Init).
 * This file only provides LOG_Printf formatting.
 * In release builds (NDEBUG), LOG_Printf is a no-op macro in main.h.
 */
#ifndef NDEBUG

#include "main.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef huart3;  /* owned by comm.c */

int LOG_Printf(const char *fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (len > 0) {
        HAL_UART_Transmit(&huart3, (uint8_t *)buf, (uint16_t)len, 100);
    }
    return len;
}

#endif /* !NDEBUG */
