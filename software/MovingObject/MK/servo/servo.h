#ifndef		_SERVO_H
#define		_SERVO_H

#include "tim.h"

typedef struct {
		TIM_HandleTypeDef *htim;
		uint32_t Channel;
}servo_t;

extern servo_t left_servo, right_servo;

void servo_init(TIM_HandleTypeDef *left_htim, uint32_t left_Channel,
								TIM_HandleTypeDef *right_htim, uint32_t right_Channel);

void set_pwm_value(uint16_t left_value, uint16_t right_value);
void servo_jump();

#endif		/* _SERVO_H */

