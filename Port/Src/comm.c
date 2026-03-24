/**
 * comm.c - USART3 driver + Encoder → Host communication
 *
 * USART3 PB8=TX (AF4), PB9=RX (AF4), 115200 8N1
 * Owns the UART hardware init; also used by LOG (debug builds only).
 * Protocol: R\n, L\n, Y\n
 */
#include "comm.h"
#include "main.h"

UART_HandleTypeDef huart3;

void COMM_Init(void)
{
    /* 1. Enable clocks */
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* 2. Configure PB8 (TX) and PB9 (RX) as AF4 */
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin       = GPIO_PIN_8 | GPIO_PIN_9;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_LOW;
    gpio.Alternate = GPIO_AF4_USART3;
    HAL_GPIO_Init(GPIOB, &gpio);

    /* 3. Init USART3 */
    huart3.Instance          = USART3;
    huart3.Init.BaudRate     = 115200;
    huart3.Init.WordLength   = UART_WORDLENGTH_8B;
    huart3.Init.StopBits     = UART_STOPBITS_1;
    huart3.Init.Parity       = UART_PARITY_NONE;
    huart3.Init.Mode         = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart3);
}

static void send_str(const char *s, uint8_t len)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)s, len, 10);
}

void COMM_SendRight(void) { send_str("R\n", 2); }
void COMM_SendLeft(void)  { send_str("L\n", 2); }
void COMM_SendYes(void)   { send_str("Y\n", 2); }

void COMM_Poll(void)
{
    /* Non-blocking check: is there a byte waiting? */
    if (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_RXNE)) {
        uint8_t rx = (uint8_t)(huart3.Instance->RDR & 0xFF);
        if (rx == '?') {
            send_str("OK\n", 3);
        }
        /* ignore all other incoming bytes */
    }
}
