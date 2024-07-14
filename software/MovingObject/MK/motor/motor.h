#ifndef		_MOTOR_H
#define		_MOTOR_H

#include "main.h"

#pragma pack (1)
typedef struct Data{
		uint8_t first;
		float speed;
		uint8_t last;
}Data;
#pragma pack ()

typedef struct Motor{
	uint8_t rx_buffer[32];
	uint8_t rx_buf_len;
	volatile Data data;
	
	UART_HandleTypeDef *huart;
	volatile float speed;
}Motor;

extern Motor motor_left;
extern Motor motor_right;
extern float MOTO1,MOTO2;								//电机装载变量

void motor_run(float speed1, float speed2);
void motor_init();
void motor_left_speed_CallBack();
void motor_right_speed_CallBack();
void Limit(float *motoA, float *motoB);
void Load(float moto1,float moto2);

#endif		/* _MOTOR_H */

