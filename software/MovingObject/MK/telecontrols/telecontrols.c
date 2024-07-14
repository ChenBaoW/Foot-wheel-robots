#include "telecontrols.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

#define RX_DMA_MAXSIZE 32

// 创建 结构为struct_message的myData变量
telecontrols_t telecontrols;

void telecontrols_init(){
	memset(&telecontrols, 0, sizeof(telecontrols));
	telecontrols.huart = &huart2;
	telecontrols.myData.key_status = 0;
	telecontrols.myData.sys_status = MOVE;
	telecontrols.myData.x_data = 127;
	telecontrols.myData.y_data = 127;
	telecontrols.myData.z_data = 127;
	telecontrols.myData.k_data = 127;
	HAL_UART_AbortReceive_IT(telecontrols.huart);
	__HAL_UART_CLEAR_IDLEFLAG(telecontrols.huart);
	__HAL_UART_ENABLE_IT(telecontrols.huart, UART_IT_IDLE);
	HAL_UART_Receive_DMA(telecontrols.huart, telecontrols.rx_buffer, telecontrols.rx_buf_len);
}

void telecontrols_CallBack(){
		HAL_UART_AbortReceive_IT(telecontrols.huart);///< 终止接收
		telecontrols.rx_buf_len = RX_DMA_MAXSIZE - __HAL_DMA_GET_COUNTER(telecontrols.huart->hdmarx);
		if(telecontrols.rx_buf_len == sizeof(struct_message)){
				memcpy((uint8_t *)&telecontrols.myData, telecontrols.rx_buffer, telecontrols.rx_buf_len);
		}
		HAL_UART_Receive_DMA(telecontrols.huart, telecontrols.rx_buffer, RX_DMA_MAXSIZE); 
}







