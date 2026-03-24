/**
 * comm.c - Encoder → Host UART communication
 *
 * Shares USART3 (PB8=TX, PB9=RX, 115200) with debug logger.
 * Protocol messages: R\n, L\n, Y\n
 * Debug logs start with '[' — host should ignore non-protocol bytes.
 */
#include "comm.h"
#include "main.h"

/* USART3 handle is owned by uart_log.c */
extern UART_HandleTypeDef huart3;

void COMM_Init(void)
{
    /* Nothing to do — USART3 already initialized by LOG_Init() */
    LOG_Printf("[COMM] Using USART3 (shared with LOG)\r\n");
}

static void send_cmd(const char *cmd, uint8_t len)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)cmd, len, 10);
}

void COMM_SendRight(void) { send_cmd("R\n", 2); }
void COMM_SendLeft(void)  { send_cmd("L\n", 2); }
void COMM_SendYes(void)   { send_cmd("Y\n", 2); }
