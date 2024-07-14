#ifndef		_TELECONTROLS_H
#define		_TELECONTROLS_H
#include "main.h"


#define MOVE 		0	//移动
#define RISER 	1	//直立
#define JUMP 		2	//跳跃
#define BALANCE 3	//平衡
//typedef enum SYS_STATUS{
//	MOVE,	//移动
//	RISER,	//直立
//	JUMP,	//跳跃
//	BALANCE,	//平衡
//}SYS_STATUS;

// 发送数据的结构示例
// 在C中使用 typedef struct 定义一个结构体类型,名为struct_message
// 必须与接收方的结构相匹配一致
#pragma pack (1)
typedef struct struct_message {
  uint8_t   	key_status;
  uint8_t 		sys_status;
  uint16_t  	x_data;
  uint16_t  	y_data;
  uint16_t  	z_data;
  uint16_t  	k_data;
} struct_message;
#pragma pack ()

typedef struct{
	uint8_t rx_buffer[32];
	uint8_t rx_buf_len;
	
	struct_message myData;
	
	UART_HandleTypeDef *huart;
}telecontrols_t;

extern telecontrols_t telecontrols;

void telecontrols_init();
void telecontrols_CallBack();

#endif		/* _TELECONTROLS_H */

