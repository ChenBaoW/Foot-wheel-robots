
#ifndef STM32_USART1_H
#define STM32_USART1_H
/******************************************************************************/
#include "stdio.h"
#include "stm32f10x.h"

/******************************************************************************/
#define USART_REC_LEN 256
/******************************************************************************/
extern unsigned char USART_RX_BUF[USART_REC_LEN];
extern unsigned short USART_RX_STA;
/******************************************************************************/
void uart_init(unsigned long bound);
void uart_send_tx(u8 ch);
void uart_send_tx_s(u8 *pString, u16 p_len);
/******************************************************************************/


#endif
