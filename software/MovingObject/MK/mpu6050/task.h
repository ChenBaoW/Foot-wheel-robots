/*
 * @Author: your name
 * @Date: 2021-05-28 17:07:45
 * @LastEditTime: 2021-05-28 21:57:32
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \FiveSchoolLeague_Code\Moudle\task.h
 */
#ifndef TASK_H_
#define TASK_H_

/* Private includes -----------------------------------------------------------*/
#include "usart.h"
#include "stdint.h"
#include "math.h"

/* Private define -------------------------------------------------------------*/
/* This class of macros represents BEGIN ---------*/
/** 
*   @brief A brief description of the macro
*/ 
#define USE_AREA_VERSION    0
#define USE_SIGN_VERSION    1
#define VERSION             USE_SIGN_VERSION


#define PI (4.0*atan(1.0))

#define RX_MAXSIZE  11
#define RX_DMA_MAXSIZE 99

/* This class of macros represents END -----------*/


/* Private typedef ------------------------------------------------------------*/
/* This kind of structural representation BEGIN --*/
/** 
*   @brief A brief description of the macro
*/ 
typedef enum data_state_t
{
	SEARCH_FRAMEHEAD,///< 正在寻找帧头
	SEARCH_FRAMETAIL,///< 已寻找到帧头, 正在寻找帧尾
	SEARCH_DATA,
}data_state_t;

typedef enum state_t
{
	no_level_state = 0,///< 不水平状态
	level_state///< 水平状态
}state_t;

typedef enum initial_data_angle_state_t
{
	positive_state = 0, //正数
	negative_state,     //负数
}initial_data_angle_state_t;

// ///< 当前任务状态
// typedef enum task_state_t
// {
// 	LEVELING_SURVEY = 0,///< 水平测量任务
// 	TILT_SURVEY,///< 倾斜测量
// 	SCROLL_SURVEY,///< 滚动测量
// 	HORIZONTAL_ROTATION_SURVEY,///< 水平转动测量
// 	SLOPE_HEIGHT_SURVEY,///< 斜板高度测量
// }task_state_t;

struct SAcc
{
	float a[4];
};

struct SGyro
{
	float w[4];
};

struct SAngle
{
	float Angle[4];
};

//float sin_ca[1]=
//{
//0.08715574274765817,0.09584575252022398,0.10452846326765346,0.11320321376790671,0.12186934340514748,0.13052619222005157,0.13917310096006544,0.1478094111296106,0.15643446504023087,0.16504760586067765,0.17364817766693033,0.18223552549214747,0.1908089953765448,0.19936793441719716,0.20791169081775931,0.21643961393810288,0.22495105434386498,0.2334453638559054,0.24192189559966773,0.25038000405444144,0.25881904510252074
//};

extern volatile struct SAcc 		acc; //加速度
extern volatile struct SGyro 	gyro;//角速度  
extern volatile struct SAngle 	angle;//角度

extern uint8_t angle_zero[3];//角度初始化
extern uint8_t acc_zero[3];  //加速度清零
extern uint8_t level_place[3];//水平放置
extern uint8_t vertical_place[3];//竖直放置

/* This kind of structural representation END ----*/


/* Private variables ----------------------------------------------------------*/
/* USER CODE BEGIN PD ----------------------------*/
typedef struct test_3_t {
  float acc_angle;//累积的角度
  float distance;//计算的距离

  float preangle;//前一个角度值
  float nowangle;//当前角度值
    
  float deltaangle;

  volatile uint8_t angle_iscome;//角度已经接收

  volatile uint16_t calccount;//已经计算了多少次

}test_3_t;

extern test_3_t g_test3;

typedef struct test_4_t {
	
  float acc_angle;//累积的角度
  float rotation_number;//计算的距离

  float preangle;//前一个角度值
  float nowangle;//当前角度值
    
  float deltaangle;

  volatile uint8_t angle_iscome;//角度已经接收

  volatile uint16_t calccount;//已经计算了多少次

}test_4_t;

extern test_4_t g_test4;

typedef struct test_5_t {
  float acc_angle;//累积的角度
  float distance;//计算的距离
  float enddistance;//计算的距离

  float preangle;//前一个角度值
  float nowangle;//当前角度值

  volatile uint8_t angle_iscome;//角度已经接收

  volatile uint16_t calccount;//已经计算了多少次

  float beginangle;//开始角度

  float beginacceleration;//开始加速度
  float deltaacc;

}test_5_t;

extern test_5_t g_test5;


typedef struct mpu6500_t 
{
    uint8_t rx_buffer[RX_MAXSIZE];
    uint8_t rx_dma_buffer[RX_DMA_MAXSIZE];
    uint8_t rx_dma_buffer_shadow[RX_DMA_MAXSIZE];///< DMA接收影子数组
    
    UART_HandleTypeDef *huart;

    volatile uint8_t rx_count;///< 记录接收的数据帧数（一帧11字节）
    volatile data_state_t data_state;
    volatile uint8_t effective_data_count;
    volatile uint8_t effective_data[RX_MAXSIZE];
    volatile uint8_t crc_data;

    volatile struct SAcc 	acc; //加速度
    volatile struct SGyro 	gyro;//角速度  
    volatile struct SAngle 	angle;//角度
    volatile uint8_t angle_iscome;
    volatile uint32_t angle_come_count;

    volatile char   angle_send_buffer[128];//发送角度字符串
}mpu6500_t;

extern mpu6500_t g_mpu6500_2;
void mpu6500__init(mpu6500_t *mpu6500, UART_HandleTypeDef *huart);
void mpu6500__open_to_receive(mpu6500_t *mpu6500);
/* USER CODE BEGIN PD ----------------------------*/


/* Private function prototypes ------------------------------------------------*/
/* USER CODE BEGIN PD ----------------------------*/
void HAL_UART_IDLECallBack(UART_HandleTypeDef *huart);///< 串口空闲中断回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);///< 串口接收完成中断回调函数
void uart_rev_analysis(uint8_t * data, uint16_t size);///< 对接收到串口数据解析
uint8_t uart_effective_data_analysis(uint8_t data);///< 对接收到的IMU有效负载数据解析
void ture_data_handle(uint8_t * effective_data, uint8_t effective_data_count);///< 接收到的正确 IMU数据处理函数

void imu_calibration_to_zero();///< IMU校准, z轴归零
void imu_open_to_receive(void);///< 开启串口中断以及 DMA接收

initial_data_angle_state_t state_return (float num);///< 判断当前数据正负

state_t level_test(void);///< 1.水平测量
float tilt_measuring(void);///< 2.倾斜测量
float roll_distance_measure(void);///< 3.滚动测量
float rotate_cylinder(void);///< 4.滚筒水平转动圈数 z轴    

float measure_slope_degree(void);///< 测量斜坡角度
float measure_slope_distance(void);///< 测量斜坡长度
float degrees_to_rad(float degree);///< 把角度变成弧度
float calculate_slope_height(float angle_init, uint16_t distance_finish);///< 5.计算斜坡高度

/* USER CODE END PD ------------------------------*/
#endif // TASK_H_
