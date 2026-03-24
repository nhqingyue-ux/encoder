/**
 * comm.c - Encoder → Host UART communication
 *
 * USART1 PA9=TX (AF1), PA10=RX (AF1), 115200 8N1
 * Sends single ASCII commands: R\n, L\n, Y\n
 */
#include "comm.h"
#include "main.h"

static UART_HandleTypeDef huart1;

void COMM_Init(void)
{
    /* 1. Enable clocks */
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* 2. Configure PA9 (TX) and PA10 (RX) as AF1 (USART1) */
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin       = GPIO_PIN_9 | GPIO_PIN_10;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_LOW;
    gpio.Alternate = GPIO_AF1_USART1;
    HAL_GPIO_Init(GPIOA, &gpio);

    /* 3. Init USART1 */
    huart1.Instance          = USART1;
    huart1.Init.BaudRate     = 115200;
    huart1.Init.WordLength   = UART_WORDLENGTH_8B;
    huart1.Init.StopBits     = UART_STOPBITS_1;
    huart1.Init.Parity       = UART_PARITY_NONE;
    huart1.Init.Mode         = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);

    LOG_Printf("[COMM] USART1 init done (PA9=TX, PA10=RX, 115200)\r\n");
}

static void send_cmd(const char *cmd, uint8_t len)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)cmd, len, 10);
}

void COMM_SendRight(void) { send_cmd("R\n", 2); }
void COMM_SendLeft(void)  { send_cmd("L\n", 2); }
void COMM_SendYes(void)   { send_cmd("Y\n", 2); }
