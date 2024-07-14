#include "servo.h"
#include "motor.h"
#include "tim.h"

#define LEFT_PWM_MAX 1600
#define LEFT_PWM_MIN 1000

#define RIGHT_PWM_MAX 2000
#define RIGHT_PWM_MIN 1400

servo_t left_servo, right_servo;

// 定义PWM输出值
uint16_t left_pwm_value = 0, right_pwm_value = 0;

void servo_init(TIM_HandleTypeDef *left_htim, uint32_t left_Channel,
								TIM_HandleTypeDef *right_htim, uint32_t right_Channel){
	left_servo.htim = left_htim;
	left_servo.Channel = left_Channel;
	right_servo.htim = right_htim;
	right_servo.Channel = right_Channel;
	HAL_TIM_PWM_Start(left_servo.htim, left_servo.Channel);//定时器初始化
	HAL_TIM_PWM_Start(right_servo.htim, right_servo.Channel);//定时器初始化
	HAL_Delay(1000);
}
								
void set_pwm_value(uint16_t left_value, uint16_t right_value){
		// 限制PWM值在最大和最小范围内
    if (left_value > LEFT_PWM_MAX) {
        left_pwm_value = LEFT_PWM_MAX;
    } else if (left_value < LEFT_PWM_MIN) {
        left_pwm_value = LEFT_PWM_MIN;
    } else {
        left_pwm_value = left_value;
    }
		
		if (right_value > RIGHT_PWM_MAX) {
        right_pwm_value = RIGHT_PWM_MAX;
    } else if (right_value < RIGHT_PWM_MIN) {
        right_pwm_value = RIGHT_PWM_MIN;
    } else {
        right_pwm_value = right_value;
    }
		
		__HAL_TIM_SET_COMPARE(left_servo.htim, left_servo.Channel, left_pwm_value);
		__HAL_TIM_SET_COMPARE(right_servo.htim, right_servo.Channel, right_pwm_value);
}	


void servo_jump(){
	__HAL_TIM_SET_COMPARE(left_servo.htim, left_servo.Channel, 1000);
	__HAL_TIM_SET_COMPARE(right_servo.htim, right_servo.Channel, 2000);
	HAL_Delay(50);
	Load(0.0, 0.0);
	__HAL_TIM_SET_COMPARE(left_servo.htim, left_servo.Channel, 1600);
	__HAL_TIM_SET_COMPARE(right_servo.htim, right_servo.Channel, 1400);
	HAL_Delay(50);
}






