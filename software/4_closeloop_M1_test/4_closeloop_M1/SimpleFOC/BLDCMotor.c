
#include "MyProject.h"


/************************************************
main中调用的接口函数都在当前文件中
=================================================
本程序仅供学习，引用代码请标明出处
使用教程：https://blog.csdn.net/loop222/article/details/120471390
创建日期：20210925
作    者：loop222 @郑州
************************************************/
/******************************************************************************/
extern float target;
/******************************************************************************/
long sensor_direction;
float voltage_power_supply;
float voltage_limit;
float voltage_sensor_align;
int  pole_pairs;
unsigned long open_loop_timestamp;
float velocity_limit;
/******************************************************************************/
int alignSensor(void);
float velocityOpenloop(float target_velocity);
float angleOpenloop(float target_angle);
/******************************************************************************/
void Motor_init(void)
{
//    printf("MOT: Init\r\n");

    //	new_voltage_limit = current_limit * phase_resistance;
    //	voltage_limit = new_voltage_limit < voltage_limit ? new_voltage_limit : voltage_limit;
    if(voltage_sensor_align > voltage_limit) voltage_sensor_align = voltage_limit;

    sensor_direction = UNKNOWN;

    M1_Enable;
//    printf("MOT: Enable driver.\r\n");
}
/******************************************************************************/
void Motor_initFOC(void)
{
    alignSensor();    //检测零点偏移量和极对数

    //added the shaft_angle update
    angle_prev = getAngle(); //getVelocity(),make sure velocity=0 after power on
    delay_ms(5);
    shaft_velocity = shaftVelocity();  //必须调用一次，进入主循环后速度为0
    delay_ms(5);
    shaft_angle = shaftAngle();// shaft angle
    if(controller == Type_angle)target = shaft_angle; //角度模式，以当前的角度为目标角度，进入主循环后电机静止

    delay_ms(200);
}
/******************************************************************************/
int alignSensor(void)
{
    long i;
    float angle;
    float mid_angle, end_angle;
    float moved;

		sensor_direction = CW;

    setPhaseVoltage(voltage_sensor_align, 0,  _3PI_2);  //计算零点偏移角度
    delay_ms(700);
    zero_electric_angle = _normalizeAngle(_electricalAngle(sensor_direction * getAngle(), pole_pairs));
    delay_ms(20);

//    setPhaseVoltage(0, 0, 0);
    delay_ms(200);

    return 1;
}
/******************************************************************************/
void loopFOC(void)
{
    if( controller == Type_angle_openloop || controller == Type_velocity_openloop ) return;

    shaft_angle = shaftAngle();// shaft angle
    electrical_angle = electricalAngle();// electrical angle - need shaftAngle to be called first

    // set the phase voltage - FOC heart function :)
    setPhaseVoltage(voltage.q, voltage.d, electrical_angle);
}
/******************************************************************************/
void move(float new_target)
{
    shaft_velocity = shaftVelocity();

    switch(controller)
    {
    case Type_torque:
        if(torque_controller == Type_voltage)voltage.q = new_target; // if voltage torque control
        else
            current_sp = new_target; // if current/foc_current torque control
        break;
    case Type_angle:
        // angle set point
        shaft_angle_sp = new_target;
        // calculate velocity set point
        shaft_velocity_sp = PID_angle( shaft_angle_sp - shaft_angle );
        // calculate the torque command
        current_sp = PID_velocity(shaft_velocity_sp - shaft_velocity); // if voltage torque control
        // if torque controlled through voltage
        if(torque_controller == Type_voltage)
        {
            voltage.q = current_sp;
            voltage.d = 0;
        }
        break;
    case Type_velocity:
        // velocity set point
        shaft_velocity_sp = new_target;
        // calculate the torque command
        current_sp = PID_velocity(shaft_velocity_sp - shaft_velocity); // if current/foc_current torque control
        // if torque controlled through voltage control
        if(torque_controller == Type_voltage)
        {
            voltage.q = current_sp;  // use voltage if phase-resistance not provided
            voltage.d = 0;
        }
        break;
    case Type_velocity_openloop:
        // velocity control in open loop
        shaft_velocity_sp = new_target;
        voltage.q = velocityOpenloop(shaft_velocity_sp); // returns the voltage that is set to the motor
        voltage.d = 0;
        break;
    case Type_angle_openloop:
        // angle control in open loop
        shaft_angle_sp = new_target;
        voltage.q = angleOpenloop(shaft_angle_sp); // returns the voltage that is set to the motor
        voltage.d = 0;
        break;
    }
}
/******************************************************************************/
void setPhaseVoltage(float Uq, float Ud, float angle_el)
{
    float Uout;
    uint32_t sector;
    float T0, T1, T2;
    float Ta, Tb, Tc;

    if(Ud) // only if Ud and Uq set
    {
        // _sqrt is an approx of sqrt (3-4% error)
        Uout = _sqrt(Ud * Ud + Uq * Uq) / voltage_power_supply;
        // angle normalisation in between 0 and 2pi
        // only necessary if using _sin and _cos - approximation functions
        angle_el = _normalizeAngle(angle_el + atan2(Uq, Ud));
    }
    else
    {
        // only Uq available - no need for atan2 and sqrt
        Uout = Uq / voltage_power_supply;
        // angle normalisation in between 0 and 2pi
        // only necessary if using _sin and _cos - approximation functions
        angle_el = _normalizeAngle(angle_el + _PI_2);
    }
    if(Uout > 0.577)Uout = 0.577;
    if(Uout < -0.577)Uout = -0.577;

    sector = (angle_el / _PI_3) + 1;
    T1 = _SQRT3 * _sin(sector * _PI_3 - angle_el) * Uout;
    T2 = _SQRT3 * _sin(angle_el - (sector - 1.0) * _PI_3) * Uout;
    T0 = 1 - T1 - T2;

    // calculate the duty cycles(times)
    switch(sector)
    {
    case 1:
        Ta = T1 + T2 + T0 / 2;
        Tb = T2 + T0 / 2;
        Tc = T0 / 2;
        break;
    case 2:
        Ta = T1 +  T0 / 2;
        Tb = T1 + T2 + T0 / 2;
        Tc = T0 / 2;
        break;
    case 3:
        Ta = T0 / 2;
        Tb = T1 + T2 + T0 / 2;
        Tc = T2 + T0 / 2;
        break;
    case 4:
        Ta = T0 / 2;
        Tb = T1 + T0 / 2;
        Tc = T1 + T2 + T0 / 2;
        break;
    case 5:
        Ta = T2 + T0 / 2;
        Tb = T0 / 2;
        Tc = T1 + T2 + T0 / 2;
        break;
    case 6:
        Ta = T1 + T2 + T0 / 2;
        Tb = T0 / 2;
        Tc = T1 + T0 / 2;
        break;
    default:  // possible error state
        Ta = 0;
        Tb = 0;
        Tc = 0;
    }

    TIM_SetCompare1(TIM2, Ta * PWM_Period);
    TIM_SetCompare2(TIM2, Tb * PWM_Period);
    TIM_SetCompare3(TIM2, Tc * PWM_Period);
}
/******************************************************************************/
float velocityOpenloop(float target_velocity)
{
    unsigned long now_us;
    float Ts, Uq;

    now_us = SysTick->VAL; //_micros();
    if(now_us < open_loop_timestamp)Ts = (float)(open_loop_timestamp - now_us) / 9 * 1e-6;
    else
        Ts = (float)(0xFFFFFF - now_us + open_loop_timestamp) / 9 * 1e-6;
    open_loop_timestamp = now_us; //save timestamp for next call
    // quick fix for strange cases (micros overflow)
    if(Ts == 0 || Ts > 0.5) Ts = 1e-3;

    // calculate the necessary angle to achieve target velocity
    shaft_angle = _normalizeAngle(shaft_angle + target_velocity * Ts);

    Uq = voltage_limit;
    // set the maximal allowed voltage (voltage_limit) with the necessary angle
    setPhaseVoltage(Uq,  0, _electricalAngle(shaft_angle, pole_pairs));

    return Uq;
}
/******************************************************************************/
float angleOpenloop(float target_angle)
{
    unsigned long now_us;
    float Ts, Uq;

    now_us = SysTick->VAL; //_micros();
    if(now_us < open_loop_timestamp)Ts = (float)(open_loop_timestamp - now_us) / 9 * 1e-6;
    else
        Ts = (float)(0xFFFFFF - now_us + open_loop_timestamp) / 9 * 1e-6;
    open_loop_timestamp = now_us;  //save timestamp for next call
    // quick fix for strange cases (micros overflow)
    if(Ts == 0 || Ts > 0.5) Ts = 1e-3;

    // calculate the necessary angle to move from current position towards target angle
    // with maximal velocity (velocity_limit)
    if(fabs( target_angle - shaft_angle ) > velocity_limit * Ts)
    {
        shaft_angle += _sign(target_angle - shaft_angle) * velocity_limit * Ts;
        //shaft_velocity = velocity_limit;
    }
    else
    {
        shaft_angle = target_angle;
        //shaft_velocity = 0;
    }

    Uq = voltage_limit;
    // set the maximal allowed voltage (voltage_limit) with the necessary angle
    setPhaseVoltage(Uq,  0, _electricalAngle(shaft_angle, pole_pairs));

    return Uq;
}
/******************************************************************************/


