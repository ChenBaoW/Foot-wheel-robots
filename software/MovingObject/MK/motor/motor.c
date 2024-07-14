#include "motor.h"
#include "usart.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

#define RX_DMA_MAXSIZE 32

#define motor_left_uart (huart1)
#define motor_right_uart (huart2)

Motor motor_left;
Motor motor_right;

double MOTOR_SPEED_MAX = 5000;
double MOTOR_SPEED_MIN = -5000;

float MOTO1, MOTO2;								//电机装载变量
uint8_t motor_left_status = 0, motor_right_status = 0;

char buff[20];

void motor_run(float speed1, float speed2)
{
		motor_left_status = 0;
		motor_right_status = 0;
		while(motor_left_status == 0 || motor_right_status == 0){
			sprintf(buff, "T%f\r\n", speed1);
			HAL_UART_Transmit(motor_left.huart, (uint8_t *)buff, strlen(buff), 10);
			sprintf(buff, "T%f\r\n", speed2);
			HAL_UART_Transmit(motor_right.huart, (uint8_t *)buff, strlen(buff), 10);
		}
}

/*电机初始化函数*/
void motor_init()
{
		memset(&motor_left, 0, sizeof(motor_left));
		memset(&motor_right, 0, sizeof(motor_right));

		motor_left.huart = &huart1;
    motor_right.huart = &huart3;
	
		HAL_UART_AbortReceive_IT(motor_left.huart);
    __HAL_UART_CLEAR_IDLEFLAG(motor_left.huart);
    __HAL_UART_ENABLE_IT(motor_left.huart, UART_IT_IDLE);
    HAL_UART_Receive_DMA(motor_left.huart, motor_left.rx_buffer, motor_left.rx_buf_len);
	
		HAL_UART_AbortReceive_IT(motor_right.huart);
    __HAL_UART_CLEAR_IDLEFLAG(motor_right.huart);
    __HAL_UART_ENABLE_IT(motor_right.huart, UART_IT_IDLE);
    HAL_UART_Receive_DMA(motor_right.huart, motor_right.rx_buffer, motor_right.rx_buf_len);
	
		HAL_Delay(2000);
	
    motor_run(0, 0);
}


void motor_left_speed_CallBack(){
		HAL_UART_AbortReceive_IT(motor_left.huart);///< 终止接收
		motor_left.rx_buf_len = RX_DMA_MAXSIZE - __HAL_DMA_GET_COUNTER(motor_left.huart->hdmarx);
		if(motor_left.rx_buf_len == sizeof(Data)){
				memcpy((uint8_t *)&motor_left.data, motor_left.rx_buffer, motor_left.rx_buf_len);
				if(motor_left.data.first == 0xaa && motor_left.data.last == 0xbb){
					motor_left.speed = -motor_left.data.speed;
				}else if(motor_left.data.first == 0xcc && motor_left.data.last == 0xdd){
					motor_left_status = 1;
				}
		}
		HAL_UART_Receive_DMA(motor_left.huart, motor_left.rx_buffer, RX_DMA_MAXSIZE);           
}

void motor_right_speed_CallBack(){
		HAL_UART_AbortReceive_IT(motor_right.huart);///< 终止接收
		motor_right.rx_buf_len = RX_DMA_MAXSIZE - __HAL_DMA_GET_COUNTER(motor_right.huart->hdmarx);
		if(motor_right.rx_buf_len == sizeof(Data)){
				memcpy((uint8_t *)&motor_right.data, motor_right.rx_buffer, motor_right.rx_buf_len);
				if(motor_right.data.first == 0xaa && motor_right.data.last == 0xbb){
					motor_right.speed = motor_right.data.speed;
				}
				else if(motor_right.data.first == 0xcc && motor_right.data.last == 0xdd){
 					motor_right_status = 1;
				}
		}
		HAL_UART_Receive_DMA(motor_right.huart, motor_right.rx_buffer, RX_DMA_MAXSIZE);  
}


/**********************
速度读取函数
入口参数：串口
**********************/


Data data = {0, 0.0, 0};

char sprrd_str[]= "S\r\n";


/*限幅函数*/
void Limit(float *motoA, float *motoB)
{
    if(*motoA > MOTOR_SPEED_MAX)	*motoA = MOTOR_SPEED_MAX;
    if(*motoA < MOTOR_SPEED_MIN)	*motoA = MOTOR_SPEED_MIN;

    if(*motoB > MOTOR_SPEED_MAX)	*motoB = MOTOR_SPEED_MAX;
    if(*motoB < MOTOR_SPEED_MIN)	*motoB = MOTOR_SPEED_MIN;
}


/*赋值函数*/
/*入口参数：PID运算完成后的最终速度值*/
void Load(float moto1, float moto2) //
{
    motor_run(moto1, moto2);
}


