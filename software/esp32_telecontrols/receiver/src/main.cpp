/*
 * @Author: ChenBaoW 87351112+ChenBaoW@users.noreply.github.com
 * @Date: 2024-04-23 19:42:19
 * @LastEditors: ChenBaoW 87351112+ChenBaoW@users.noreply.github.com
 * @LastEditTime: 2024-07-10 23:38:02
 * @FilePath: \receiver\src\main.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
//#include <ESP32Servo.h>

#define LEFT_GPIO_PIN   25
#define RIGHT_GPIO_PIN  26


//模式
#define MOVE 		0	//移动
#define RISER 	1	//直立
#define JUMP 		2	//跳跃
#define BALANCE 3	//平衡


// 发送数据的结构示例
// 在C中使用 typedef struct 定义一个结构体类型,名为struct_message
// 必须与接收方的结构相匹配一致
#pragma pack (1)
typedef struct struct_message {
  uint8_t   	key_status; //按键状态
  uint8_t    	sys_status; //系统状态
  uint16_t  	x_data;     //左摇杆x轴数据
  uint16_t  	y_data;     //左摇杆y轴数据
  uint16_t  	z_data;     //右摇杆x轴数据
  uint16_t  	k_data;     //右摇杆y轴数据
} struct_message;
#pragma pack ()

// 创建 结构为struct_message的myData变量
struct_message myData;
uint8_t data;

// 当收到数据时将执行的回调函数
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  // 按字节发送结构体
  byte *ptr = (byte *)&myData;
  // Serial.printf("x = %d  y = %d key = %d\n", myData.x_data, myData.y_data, myData.key_status);
  // Serial.printf("z = %d  k = %d sys = %d\n", myData.z_data, myData.k_data, myData.sys_status);
  // Serial.printf("receive success\n");
  for (size_t i = 0; i < sizeof(myData); i++) {
    Serial.write(*ptr++);
  }
}

void setup() {
  // 设置串口波特率
  Serial.begin(115200);

  // 设置设备WIFI模式为WIFI_STA
  WiFi.mode(WIFI_STA);

  // 初始化ESPNOW
  if (esp_now_init() != ESP_OK) {
    //Serial.println("Error initializing ESP-NOW");
    return;
  }
  // 当ESPNOW初始化成功,我们将会注册一个回调函数（callback，CB）
  // 获得回收的包装信息
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  
}