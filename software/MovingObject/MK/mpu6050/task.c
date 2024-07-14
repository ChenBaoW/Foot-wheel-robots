/*
 * @Author: your name
 * @Date: 2021-05-28 17:07:56
 * @LastEditTime: 2021-05-28 21:56:24
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \FiveSchoolLeague_Code\Moudle\task.c
 */
/* Private includes -----------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "stm32f1xx_hal.h"
#include "task.h" 
#include "math.h"
#include "usart.h"

/* Private variables ----------------------------------------------------------*/
/* USER CODE BEGIN PD ----------------------------*/
/** 
*   @brief IMU所用相关变量
*/
uint8_t rx_buffer[RX_MAXSIZE] = {0};
uint8_t rx_dma_buffer[RX_DMA_MAXSIZE] = {0};
uint8_t rx_dma_buffer_shadow[RX_DMA_MAXSIZE] = {0};///< DMA接收影子数组


mpu6500_t g_mpu6500_2;

uint8_t success_receive_imudata_flag = 0;///< 接收正确数据负载的标志位
uint16_t success_receive_imudata_count = 0;///< 接收正确数据负载的次数

uint8_t angle_zero[3] = {0xff,0xaa,0x52};//角度初始化
uint8_t acc_zero[3] = {0xff,0xaa,0x67};  //加速度清零
uint8_t level_place[3] = {0xff,0xaa,0x65};//水平放置
uint8_t vertical_place[3] = {0xff,0xaa,0x66};//竖直放置
uint8_t jy61_data[255];
uint16_t str_len = 0;
uint16_t uartrevcount = 0;

initial_data_angle_state_t initial_data_angle_state; //起始角度的正负

volatile uint8_t rx_count = 0;///< 记录接收的数据帧数（一帧11字节）
volatile data_state_t data_state = SEARCH_FRAMEHEAD;
volatile uint8_t effective_data_count = 0;
volatile uint8_t effective_data[RX_MAXSIZE];
volatile uint8_t crc_data = 0;

volatile struct SAcc 		acc; //加速度
volatile struct SGyro 		gyro;//角速度
volatile struct SAngle 	    angle;//角度


//extern UART_HandleTypeDef huart2;
//extern UART_HandleTypeDef huart1;
//extern DMA_HandleTypeDef hdma_usart1_rx;

extern uint8_t task_first_flag;
extern uint8_t task_second_flag;
extern uint8_t task_third_flag;
extern uint8_t task_free_flag;
extern uint8_t task_five_flag;


//task_state_t task_state;
/* USER CODE BEGIN PD ----------------------------*/

//提前声明
void ture_data_handle2(uint8_t* data,uint8_t count);
void uart_rev_analysis2(uint8_t * data,uint16_t size);
uint8_t uart_effective_data_analysis2(uint8_t data);
/* Private function prototypes ------------------------------------------------*/
/* USER CODE BEGIN PD ----------------------------*/

/** 
*   @brief 串口空闲中断回调函数
*   @name: wxz
*   @param huart    串口
*/ 
void HAL_UART_IDLECallBack(UART_HandleTypeDef * huart)
{
	if(huart == g_mpu6500_2.huart) {
        HAL_UART_AbortReceive_IT(huart);///< 终止接收
        uartrevcount = RX_DMA_MAXSIZE - __HAL_DMA_GET_COUNTER(g_mpu6500_2.huart->hdmarx);
        memcpy(g_mpu6500_2.rx_dma_buffer, g_mpu6500_2.rx_dma_buffer_shadow, uartrevcount);
        memset(g_mpu6500_2.rx_dma_buffer_shadow, 0, uartrevcount);
        HAL_UART_Receive_DMA(g_mpu6500_2.huart, g_mpu6500_2.rx_dma_buffer_shadow, RX_DMA_MAXSIZE);           
        uart_rev_analysis2(g_mpu6500_2.rx_dma_buffer, uartrevcount); 
    }
}

/** 
*   @brief 串口接收完成中断回调函数
*   @name: wxz
*   @param huart    串口
*/ 
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	uint16_t temp = 0;
	
//	if(huart == &huart1)
//	{
//        temp = RX_DMA_MAXSIZE - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);
//		memcpy(rx_dma_buffer,rx_dma_buffer_shadow,RX_DMA_MAXSIZE);
//		memset(rx_dma_buffer_shadow,0,RX_DMA_MAXSIZE);
//		
//		HAL_UART_Receive_DMA(&huart1,rx_dma_buffer_shadow,RX_DMA_MAXSIZE);

//		uart_rev_analysis(rx_dma_buffer,RX_DMA_MAXSIZE);
//	}
//    else if (huart == &huart2) {
//        temp = RX_DMA_MAXSIZE - __HAL_DMA_GET_COUNTER(g_mpu6500_2.huart->hdmarx);
//        memcpy(g_mpu6500_2.rx_dma_buffer, g_mpu6500_2.rx_dma_buffer_shadow, temp);
//        memset(g_mpu6500_2.rx_dma_buffer_shadow, 0, temp);
//   
//        HAL_UART_Receive_DMA(g_mpu6500_2.huart, g_mpu6500_2.rx_dma_buffer_shadow, RX_DMA_MAXSIZE);           
//        uart_rev_analysis2(g_mpu6500_2.rx_dma_buffer, temp); 
//    }
	if (huart == &huart2) {
        temp = RX_DMA_MAXSIZE - __HAL_DMA_GET_COUNTER(g_mpu6500_2.huart->hdmarx);
        memcpy(g_mpu6500_2.rx_dma_buffer, g_mpu6500_2.rx_dma_buffer_shadow, temp);
        memset(g_mpu6500_2.rx_dma_buffer_shadow, 0, temp);
   
        HAL_UART_Receive_DMA(g_mpu6500_2.huart, g_mpu6500_2.rx_dma_buffer_shadow, RX_DMA_MAXSIZE);           
        uart_rev_analysis2(g_mpu6500_2.rx_dma_buffer, temp); 
    }
}

/** 
*   @brief 对接收到串口数据解析
*   @name: wxz
*   @param data    数据
*   @param size    大小
*/ 
void uart_rev_analysis(uint8_t * data,uint16_t size)
{
	 uint8_t i = 0;
	 for(i = 0;i < size;i++)
	 {
        if(uart_effective_data_analysis(data[i]) == 0)///< 接收数据有误
        {
            ///< 水平、角度测量接收中断关闭, 需重新开启
//            if(task_state == LEVELING_SURVEY)
//            {   
//                imu_open_to_receive();
//            }           
        }
      
	 }   
}

/** 
*   @brief 对接收到串口数据解析
*   @name: wxz
*   @param data    数据
*   @param size    大小
*/ 
void uart_rev_analysis2(uint8_t * data,uint16_t size)
{
	 uint8_t i = 0;
	 for(i = 0;i < size;i++)
	 {
        if(uart_effective_data_analysis2(data[i]) == 0)///< 接收数据有误
        {
            ///< 水平、角度测量接收中断关闭, 需重新开启
//            if(task_state == LEVELING_SURVEY)
//            {                   
//                mpu6500__open_to_receive(&g_mpu6500_2);
//            }           
        }
      
	 }   
}

/** 
*   @brief 对接收到的IMU有效负载数据解析
*   @name: wxz
*   @param data    有效负载数据
*   @return 解析失败返回 0
*/ 
uint8_t uart_effective_data_analysis(uint8_t data)
{	
	if(data_state == SEARCH_FRAMEHEAD)
	{
		if(data == 0x55)
		{
			crc_data = 0;
			effective_data_count = 0;
			crc_data += data;
			data_state = SEARCH_FRAMETAIL;
		}
		else
			return 0;
	}
    else if(data_state == SEARCH_FRAMETAIL)
    {
      if(data == 0x55)
      {
        crc_data = 0;
        effective_data_count = 0;
        crc_data += data;
        data_state = SEARCH_FRAMETAIL;
        return 0;
      }
      if(data == 0x51 || data == 0x52 || data == 0x53)
      {
        effective_data[effective_data_count++] = data;
        crc_data += data;
        data_state = SEARCH_DATA;
      }
    else 
		{
			data_state = SEARCH_FRAMEHEAD;
			return 0;
		}
		
	}
    else if(data_state == SEARCH_DATA)
	{
		effective_data[effective_data_count++] = data;
		
		if(effective_data_count == 10)
		{
			if(crc_data == data) // 校验和满足
			{
				ture_data_handle((uint8_t*)effective_data, effective_data_count);
				data_state = SEARCH_FRAMEHEAD;
				return 1;
			}
			else 
				data_state = SEARCH_FRAMEHEAD;
			return 0;
		}
		crc_data += data;
	}
    return 0;
}

uint8_t uart_effective_data_analysis2(uint8_t data)
{	
	if(g_mpu6500_2.data_state == SEARCH_FRAMEHEAD)
	{
		if(data == 0x55)
		{
			g_mpu6500_2.crc_data = 0;
			g_mpu6500_2.effective_data_count = 0;
			g_mpu6500_2.crc_data += data;
			g_mpu6500_2.data_state = SEARCH_FRAMETAIL;
		}
		else
			return 0;
	}
    else if(g_mpu6500_2.data_state == SEARCH_FRAMETAIL)
    {
		if(data == 0x55)
		{
			g_mpu6500_2.crc_data = 0;
			g_mpu6500_2.effective_data_count = 0;
			g_mpu6500_2.crc_data += data;
			g_mpu6500_2.data_state = SEARCH_FRAMETAIL;
			return 0;
		}
		if(data == 0x51 || data == 0x52 || data == 0x53)
		{
			g_mpu6500_2.effective_data[g_mpu6500_2.effective_data_count++] = data;
			g_mpu6500_2.crc_data += data;
			g_mpu6500_2.data_state = SEARCH_DATA;
		}
        else 
		{
			g_mpu6500_2.data_state = SEARCH_FRAMEHEAD;
			return 0;
		}
		
	}
    else if(g_mpu6500_2.data_state == SEARCH_DATA)
	{
		g_mpu6500_2.effective_data[g_mpu6500_2.effective_data_count++] = data;
		
		if(g_mpu6500_2.effective_data_count == 10)
		{
			if(g_mpu6500_2.crc_data == data) // 校验和满足
			{
				ture_data_handle2((uint8_t*)g_mpu6500_2.effective_data, g_mpu6500_2.effective_data_count);                                
				g_mpu6500_2.data_state = SEARCH_FRAMEHEAD;
				return 1;
			}
			else 
				g_mpu6500_2.data_state = SEARCH_FRAMEHEAD;
			return 0;
		}
		g_mpu6500_2.crc_data += data;
	}
    return 0;
}


/** 
*   @brief 接收到的正确 IMU数据处理函数
*   @name: wxz
*   @param data    数据负载
*   @param count   数据负载大小
*/ 
void ture_data_handle(uint8_t* data,uint8_t count)
{				
	short *ps;
    memcpy(rx_buffer, &data[1],count - 1); //effective_data[0]是帧标识 
    switch(data[0])
    {	
        case 0x52:
			// memcpy(gyro.w,&rx_buffer[2],8);
            gyro.w[0] = (short)(rx_buffer [1]<<8| rx_buffer [0])*2000/32768.0; 
            gyro.w[1] = (short)(rx_buffer [3]<<8| rx_buffer [2])*2000/32768.0; 
            gyro.w[2] = (short)(rx_buffer [5]<<8| rx_buffer [4])*2000/32768.0;
			// gyro.w[3] = (short)(rx_buffer [7]<<8| rx_buffer [6])/340.0+36.53;
            break;						
        case 0x53:
			// memcpy(angle.Angle,&rx_buffer[2],8);
            ps = (short*)&rx_buffer[0];
            angle.Angle[0] = (*ps)/32768.0*180.0; 
            
            ps = (short*)&rx_buffer[2];
            angle.Angle[1] = (*ps)/32768.0*180.0; 
        
            ps = (short*)&rx_buffer[4];
            angle.Angle[2] = (*ps)/32768.0*180.0; 
			// angle.Angle[3] = (short)(rx_buffer [7]<<8| rx_buffer [6])/340.0+36.53;
//            g_test3.angle_iscome = 1;
//			g_test4.angle_iscome = 1;
//			g_test5.angle_iscome = 1;
            break;						
        case 0x51:
			// memcpy(acc.a,&rx_buffer[2],8);
			ps = (short*)&rx_buffer[0];
            acc.a[0] = (*ps)*16*9.8/32768.0; 
				
            ps = (short*)&rx_buffer[2];
            acc.a[1] = (*ps)*16*9.8/32768.0; 
				
            ps = (short*)&rx_buffer[4];
            acc.a[2] = (*ps)*16*9.8/32768.0; 
			// acc.a[3] = (short)(rx_buffer[7]<<8|rx_buffer[6])/340.0+36.53;
        break;
        default:
            break;
    }	    	
}

#pragma pack(1)
typedef struct rev_gyro_t{
    uint8_t cmdid;
    short   gy1;
    short   gy2;
    short   gy3;
}rev_gyro_t;
#pragma pack()

#pragma pack(1)
typedef struct rev_angle_t{
    uint8_t cmdid;
    short   a1;
    short   a2;
    short   a3;
}rev_angle_t;
#pragma pack()

#pragma pack(1)
typedef struct rev_acc_t{
    uint8_t cmdid;
    short   acc1;
    short   acc2;
    short   acc3;
}rev_acc_t;
#pragma pack()


void ture_data_handle2(uint8_t* data,uint8_t count)
{				    
    rev_gyro_t *gy;
    rev_angle_t *pangle;
    rev_acc_t * pacc;

    memcpy(rx_buffer, &data[1],count - 1); //effective_data[0]是帧标识 
    switch(data[0])
    {	
        case 0x52:
			// memcpy(gyro.w,&rx_buffer[2],8);
            gy = (rev_gyro_t*)data;
            g_mpu6500_2.gyro.w[0] = gy->gy1*2000.0/32768.0; 
            g_mpu6500_2.gyro.w[1] = gy->gy2*2000.0/32768.0; 
            g_mpu6500_2.gyro.w[2] = gy->gy3*2000.0/32768.0;
			// gyro.w[3] = (short)(rx_buffer [7]<<8| rx_buffer [6])/340.0+36.53;
            break;						
        case 0x53:
			// memcpy(angle.Angle,&rx_buffer[2],8);
            pangle = (rev_angle_t*)data;            
            g_mpu6500_2.angle.Angle[0] = pangle->a1*180.0/32768.0 * (-1);                         
            g_mpu6500_2.angle.Angle[1] = pangle->a2*180.0/32768.0 * (-1);                     
            g_mpu6500_2.angle.Angle[2] = pangle->a3*180.0/32768.0 * (-1); 
			// angle.Angle[3] = (short)(rx_buffer [7]<<8| rx_buffer [6])/340.0+36.53;

            g_mpu6500_2.angle_iscome = 1;
            g_mpu6500_2.angle_come_count++;
			
//						sprintf((char*)g_mpu6500_2.angle_send_buffer, "{%f;%f}", g_mpu6500_2.angle.Angle[0], degree_2_X(g_mpu6500_2.angle.Angle[0]));
//						HAL_UART_Transmit_DMA(&huart3, (uint8_t*)(g_mpu6500_2.angle_send_buffer), strlen((const char*)g_mpu6500_2.angle_send_buffer));
            
            break;						
        case 0x51:
			// memcpy(acc.a,&rx_buffer[2],8);
            pacc = (rev_acc_t*)data;            
            g_mpu6500_2.acc.a[0] = pacc->acc1*16*9.8/32768.0; 
            g_mpu6500_2.acc.a[1] = pacc->acc2*16*9.8/32768.0; 
            g_mpu6500_2.acc.a[2] = pacc->acc3*16*9.8/32768.0;
			// acc.a[3] = (short)(rx_buffer[7]<<8|rx_buffer[6])/340.0+36.53;
						break;
        default:
            break;
    }	    	
}
/** 
*   @brief IMU校准, z轴归零
*   @name: wxz
*/ 
void imu_calibration_to_zero()
{
	uint8_t count = 0;
//  HAL_UART_Transmit(&huart1,(uint8_t *)angle_zero,3,3*1000);
//	HAL_Delay(100);
//	HAL_UART_Transmit(&huart1,(uint8_t *)acc_zero,3,3*1000);
//	HAL_Delay(100);
//  HAL_UART_Transmit(&huart1,(uint8_t *)level_place,3,3*1000);

  HAL_UART_Transmit(g_mpu6500_2.huart, (uint8_t *)angle_zero, 3, 3*1000);
	HAL_Delay(1000); 
	HAL_UART_Transmit(g_mpu6500_2.huart, (uint8_t *)acc_zero, 3, 3*1000);
	HAL_Delay(1000);
  //HAL_UART_Transmit(&huart2,(uint8_t *)level_place,3,3*1000);
	HAL_Delay(100);
	while (1) 
  {
		count++;
		HAL_Delay(100);
		if(fabs(g_mpu6500_2.angle.Angle[0] ) < 0.05 || count > 100)   //判断校准成功
    {
			break;
		}
	}

}

/** 
*   @brief 开启串口中断以及 DMA接收
*   @name: wxz
*/ 
//void imu_open_to_receive(void)
//{
//    HAL_UART_AbortReceive_IT(&huart1);
//    __HAL_UART_CLEAR_IDLEFLAG(&huart1);
//    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
//    HAL_UART_Receive_DMA(&huart1,rx_dma_buffer_shadow,RX_DMA_MAXSIZE);
//}


void mpu6500__open_to_receive(mpu6500_t *mpu6500)
{
    HAL_UART_AbortReceive_IT(mpu6500->huart);
    __HAL_UART_CLEAR_IDLEFLAG(mpu6500->huart);
    __HAL_UART_ENABLE_IT(mpu6500->huart, UART_IT_IDLE);
    HAL_UART_Receive_DMA(mpu6500->huart, mpu6500->rx_dma_buffer_shadow, RX_DMA_MAXSIZE);
}

/** 
*   @brief 进行水平测量
*   @name: wxz
*/ 
state_t level_test(void)   
{
    if(fabs(angle.Angle[1]) < (1.e-5 + 0.5))///< 考虑 x即可
    {
        return level_state;
    }

    return no_level_state;
}



/** 
*   @brief 把角度变成弧度
*   @name: wxz
*   @param degree           角度
*/ 
float degrees_to_rad(float degree)
{
   float rad = 0;
   rad = (float)(degree * PI / 180.0);
   return rad;
}




void mpu6500__init(mpu6500_t *mpu6500, UART_HandleTypeDef *huart)
{
    memset(mpu6500, 0, sizeof(mpu6500_t));

    mpu6500->huart =huart;
}

// /* USER CODE END PD ------------------------------*/
