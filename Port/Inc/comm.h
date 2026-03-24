#ifndef COMM_H
#define COMM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * comm.h - Encoder → Host UART communication
 *
 * USART1 PA9=TX PA10=RX, 115200 8N1
 * Protocol: single ASCII char + '\n'
 */

void COMM_Init(void);
void COMM_SendRight(void);   /* sends "R\n" */
void COMM_SendLeft(void);    /* sends "L\n" */
void COMM_SendYes(void);     /* sends "Y\n" */

#ifdef __cplusplus
}
#endif

#endif /* COMM_H */
