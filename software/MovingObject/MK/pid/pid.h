#ifndef		_PID_H
#define		_PID_H

void pid_handle(void);

int Vertical(float Med,float Angle,float gyro_Y);
int Velocity(float target, float encoder_left,float encoder_right);
int Turn(int gyro_Z);
float calculate_angle(float barycenter);

#endif		/* _PID_H */

