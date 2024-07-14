#ifndef		_TELECONTROLS_H
#define		_TELECONTROLS_H
#include "main.h"


#define MOVE 		0	//�ƶ�
#define RISER 	1	//ֱ��
#define JUMP 		2	//��Ծ
#define BALANCE 3	//ƽ��
//typedef enum SYS_STATUS{
//	MOVE,	//�ƶ�
//	RISER,	//ֱ��
//	JUMP,	//��Ծ
//	BALANCE,	//ƽ��
//}SYS_STATUS;

// �������ݵĽṹʾ��
// ��C��ʹ�� typedef struct ����һ���ṹ������,��Ϊstruct_message
// ��������շ��Ľṹ��ƥ��һ��
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

