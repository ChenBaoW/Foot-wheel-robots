#include "pid.h"
#include "motor.h"
#include "mpu6050.h"
#include "telecontrols.h"
#include "servo.h"
#include "filter.h"
#include <math.h>

#define MID (-13.0)
#define C_MAX (MID + 4)
#define C_MIN (MID - 4)

float barycenter = MID;
float gain = 0.5;

float Med_Angle = MID; //机械中值--能使得小车真正平衡住的角度。
float
Vertical_Kp = 450,	//直立环KP、KD
Vertical_Kd = 1.5;

float
Velocity_Kp = 0.6,	//速度环KP、KI
Velocity_Ki = 0.0030;
float
Turn_Kp = 2.0;

int Vertical_out, Velocity_out, Turn_out; //直立环&速度环&转向环 的输出变量

int angle_status = 0;
float target_speed = 0.0;

int Vertical(float Med, float Angle, float gyro_Y); //函数声明
int Velocity(float target, float encoder_left, float encoder_right);
int Turn(int gyro_Z);

void pid_handle(void)
{
		int PWM_out;
		if(telecontrols.myData.sys_status == BALANCE){
				barycenter = calculate_angle(barycenter);
		}else{
				barycenter = MID;
		}
		Med_Angle = (barycenter);
		target_speed = (telecontrols.myData.y_data - 127) / 10;
    //2.将数据压入闭环控制中，计算出控制输出量。
		Velocity_out = Velocity(target_speed, motor_left.speed, motor_right.speed);	//速度环	
    Vertical_out = Vertical(Velocity_out + Med_Angle, mpu_pose_msg.pitch, mpu_raw_msg.mpu_gyro[1]);				//直立环											//转向环
		Turn_out = Turn(mpu_raw_msg.mpu_gyro[2]);
	
    PWM_out = Vertical_out; //最终输出
    //3.把控制输出量加载到电机上，完成最终的的控制。
    MOTO1 = PWM_out-Turn_out; //左电机
    MOTO2 = -(PWM_out+Turn_out); //右电机
    Limit(&MOTO1, &MOTO2); //PWM限幅
    Load(MOTO1 / 1000, MOTO2 / 1000); //加载到电机上。
}


/*********************
直立环PD控制器：Kp*Ek+Kd*Ek_D

入口：期望角度、真实角度、真实角速度
出口：直立环输出
*********************/
int Vertical(float Med, float Angle, float gyro_Y)
{
		int PWM_out;
    PWM_out = Vertical_Kp * (Angle - Med) + Vertical_Kd * (gyro_Y - 0); //【1】

    return PWM_out;
}



/*********************
速度环PI：Kp*Ek+Ki*Ek_S
*********************/
int Velocity(float target, float encoder_left, float encoder_right)
{
    static int PWM_out, Encoder_Err, Encoder_S, EnC_Err_Lowout, EnC_Err_Lowout_last; //【2】
    float a = 0.7; //【3】

    //1.计算速度偏差
    Encoder_Err = (encoder_left + encoder_right) - target; //舍去误差
    //2.对速度偏差进行低通滤波
    //low_out=(1-a)*Ek+a*low_out_last;
    EnC_Err_Lowout = (1 - a) * Encoder_Err + a * EnC_Err_Lowout_last; //使得波形更加平滑，滤除高频干扰，防止速度突变。
    EnC_Err_Lowout_last = EnC_Err_Lowout; //防止速度过大的影响直立环的正常工作。
    //3.对速度偏差积分，积分出位移
    Encoder_S += EnC_Err_Lowout; //【4】
    //4.积分限幅
    Encoder_S = Encoder_S > 5000 ? 5000 : (Encoder_S < (-5000) ? (-5000) : Encoder_S);
    //5.速度环控制输出计算
    PWM_out = Velocity_Kp * EnC_Err_Lowout + Velocity_Ki * Encoder_S; //【5】
    return PWM_out;
}



/*********************
转向环：系数*Z轴角速度
*********************/
int Turn(int gyro_Z)
{
    int PWM_out;
		int Turn_Amplitude;
		if(telecontrols.myData.x_data > 150){
			Turn_Amplitude = 1500;
		}else if(telecontrols.myData.x_data < 110){
			Turn_Amplitude = -1500;
		}else{
			Turn_Amplitude = 0;
		}

    PWM_out = Turn_Kp * gyro_Z;
		PWM_out += Turn_Amplitude;
    return PWM_out;
}

//计算重心角度
float temp = 0.0;
float calculate_angle(float barycenter){
	static int i = 0;
	if(fabs(mpu_pose_msg.pitch - barycenter) < 0.5){
		angle_status = 1;
	}else{
		angle_status = 0;
		i = 0;
	}
	if(angle_status == 1){
		if(i == 10){
			if(fabs(motor_right.speed) > 1){
				temp = gain * motor_right.speed;
				temp = temp > 0.5 ? 0.5 : (temp < (-0.5) ? (-0.5) : temp);
				barycenter += temp;
			}
			i = 0;
		}else{
			++i;
		}
	}
	if(barycenter > C_MAX){
		barycenter = C_MAX;
	}
	if(barycenter < C_MIN){
		barycenter = C_MIN;
	}
	return barycenter;
}

