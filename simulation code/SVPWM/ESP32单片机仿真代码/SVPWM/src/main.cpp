/*
 * @Author: ChenBaoW 87351112+ChenBaoW@users.noreply.github.com
 * @Date: 2024-05-03 20:53:30
 * @LastEditors: ChenBaoW 87351112+ChenBaoW@users.noreply.github.com
 * @LastEditTime: 2024-05-03 20:58:20
 * @FilePath: \SVPWM\src\main.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <Arduino.h>
#include <math.h>

#define PI 3.14159265359
#define PI_2 1.57079632679
#define PI_3 1.0471975512
#define _SQRT3 1.73205080757
#define voltage_power_supply 12.0

float normalizeAngle(float angle) {
    float a = fmod(angle, 2 * PI);
    return a >= 0 ? a : (a + 2 * PI);
}

void setPwm(float Ua, float Ub, float Uc) {
    Serial.print(Ua);
    Serial.print(",");
    Serial.print(Ub);
    Serial.print(",");
    Serial.println(Uc);
}

void setTorque(float Uq, float angle_el) {
 if (Uq < 0)
    angle_el += PI;
  Uq = abs(Uq);

  angle_el = normalizeAngle(angle_el + PI_2);
  int sector = floor(angle_el / PI_3) + 1;
  // calculate the duty cycles
  float T1 = _SQRT3 * sin(sector * PI_3 - angle_el) * Uq / voltage_power_supply;
  float T2 = _SQRT3 * sin(angle_el - (sector - 1.0) * PI_3) * Uq / voltage_power_supply;
  float T0 = 1 - T1 - T2;

  float Ta, Tb, Tc;
  switch (sector)
  {
  case 1:
    Ta = T1 + T2 + T0 / 2;
    Tb = T2 + T0 / 2;
    Tc = T0 / 2;
    break;
  case 2:
    Ta = T1 + T0 / 2;
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
  default:
    Ta = 0;
    Tb = 0;
    Tc = 0;
  }

  float Ua = Ta * voltage_power_supply;
  float Ub = Tb * voltage_power_supply;
  float Uc = Tc * voltage_power_supply;

  setPwm(Ua, Ub, Uc);
}

void setup() {
    Serial.begin(115200);  // Make sure to match the baud rate with Serial Monitor
}

void loop() {
    float Uq = 1/_SQRT3;  // Test value for voltage
    float angle_el = 0;  // Test value for angle

    // Test values across a full circle
    for (int i = 0; i < 360; i++) {
        angle_el = i * PI / 180;
        setTorque(Uq, angle_el);
        delay(20);  // Delay for visibility in plotter
    }
}
