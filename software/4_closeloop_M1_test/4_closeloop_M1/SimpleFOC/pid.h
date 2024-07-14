#ifndef PID_H
#define PID_H

extern float pid_vel_P, pid_ang_P;
extern float pid_vel_I, pid_ang_D;

/******************************************************************************/
void PID_init(void);
float PID_velocity(float error);
float PID_angle(float error);
/******************************************************************************/

#endif

