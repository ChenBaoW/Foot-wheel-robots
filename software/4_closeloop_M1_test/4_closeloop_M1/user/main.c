
#include "stm32f10x.h"
#include <stdlib.h>
#include "MyProject.h"


/************************************************
SimpleMotor开发板
闭环控制  演示
1、力矩模式
2、速度模式
3、位置模式
=================================================
本程序仅供学习，引用代码请标明出处
使用教程：https://blog.csdn.net/loop222/article/details/120471390
         《SimpleFOC移植STM32(四) —— 闭环控制》
创建日期：20230208
作    者：loop222 @郑州
************************************************/
/******************************************************************************/
#define LED_blink    GPIOC->ODR^=(1<<13)
/******************************************************************************/
float target;


#pragma pack (1)
typedef struct Data{
		uint8_t first;
		float speed;
		uint8_t last;
}Data;
#pragma pack ()

Data speed_data = {0xaa, 0.0, 0xbb};
Data target_data = {0xcc, 0.0, 0xdd};
/******************************************************************************/
void commander_run(void);
/******************************************************************************/
void GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE); //使能GPIOA,GPIOB,GPIOC,AFIO;
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;         //PC13是LED
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;  //速度
    GPIO_Init(GPIOC, &GPIO_InitStructure);             //对选中管脚初始化
    GPIO_ResetBits(GPIOC, GPIO_Pin_13);                //上电点亮LED

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;          //PB9是motor1的使能
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_9);                 //低电平解除,Motor_init()中使能
}

//void onMotor(char* cmd){ command.motor(&motor, cmd); }    //定义函数
//command.add('M',onMotor,"my motor");    //setup中加入这个函数
//motor.monitor();    //loop中加入这个函数，使用simpleFOC Studio上位机设置的时候，这句一定要打开。会影响程序执行速度，慎用

/******************************************************************************/
int main(void)
{
    unsigned int count_i = 0;

    GPIO_Config();
    uart_init(115200);
#if M1_AS5600
    I2C_Init_();               //AS5600
//    printf("AS5600\r\n");
#elif M1_TLE5012B
    SPI2_Init_();              //TLE5012B
    printf("TLE5012B\r\n");
#endif
    TIM2_PWM_Init();
    TIM4_1ms_Init();           //interrupt per 1ms

    delay_ms(1000);            //Wait for the system to stabilize
    MagneticSensor_Init();     //AS5600 or TLE5012B

    voltage_power_supply = 12; //V，电源电压
    pole_pairs = 7;            //电机极对数，按照实际设置，虽然可以上电检测但有失败的概率
    voltage_limit = 5;         //V，最大值需小于12/1.732=6.9
    velocity_limit = 20;       //rad/s angleOpenloop() and PID_angle() use it
    voltage_sensor_align = 3; //V  重要参数，航模电机大功率0.5-1，云台电机小功率2-3
    torque_controller = Type_voltage; //当前只有电压模式
    controller = Type_torque; //Type_angle; //Type_torque;    //Type_velocity
    target = 0;

    Motor_init();
    Motor_initFOC();

    systick_CountMode();   //不能再调用delay_us()和delay_ms()函数
		M1_Enable;
    while(1)
    {
        if(time1_cntr >= 200) //0.2s
        {
            time1_cntr = 0;
            LED_blink;
        }
				if(time1_cntr % 10 == 0){
					speed_data.speed = shaftVelocity();
					uart_send_tx_s((uint8_t *)&speed_data, sizeof(speed_data));
				}
				voltage.q = target;
				loopFOC();
        commander_run();
    }
}
/******************************************************************************/


void commander_run(void)
{
    if((USART_RX_STA & 0x8000) != 0)
    {
        switch(USART_RX_BUF[0])
        {
        case 'H':
            printf("Hello World!\r\n");
            break;
        case 'T':   //T6.28
            target = atof((const char *)(USART_RX_BUF + 1));
						target_data.speed = target;
						uart_send_tx_s((uint8_t *)&target_data, sizeof(target_data));
//            printf("RX=%.4f\r\n", target);
            break;
        case 'D':   //D
            M1_Disable;
            printf("OK!\r\n");
            break;
        case 'E':   //E
            M1_Enable;
            printf("OK!\r\n");
            break;
				case 'S':   //T6.28
            speed_data.speed = shaftVelocity();
						uart_send_tx_s((uint8_t *)&speed_data, sizeof(speed_data));
            break;
        }
        USART_RX_STA = 0;
    }
}
/******************************************************************************/



