/*
 * @Author: ChenBaoW 87351112+ChenBaoW@users.noreply.github.com
 * @Date: 2024-04-23 19:43:34
 * @LastEditors: ChenBaoW 87351112+ChenBaoW@users.noreply.github.com
 * @LastEditTime: 2024-07-10 23:09:56
 * @FilePath: \sender\src\main.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <esp_now.h>
#include <WiFi.h>
#include <SSD1306Wire.h>

//引脚分布
#define KEY       25  //跳跃功能选择按键引脚（只能在跳跃模式下起作用）
#define STATUS    26  //模式选择按键引脚（选择功能模式）
#define PS1_X     32  //左摇杆X轴引脚
#define PS1_Y     33  //左摇杆y轴引脚
#define PS2_X     34  //右摇杆X轴引脚
#define PS2_Y     35  //右摇杆y轴引脚
#define RESOLUTION 12 /


//模式选择
#define MOVE 		0	//移动
#define RISER 	1	//直立
#define JUMP 		2	//跳跃
#define BALANCE 3	//平衡

// 定义接收端的mac地址,这里的地址请替换成接收端ESP32的MAC地址
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
char str[32] = {0};

// 发送数据的结构示例
// 在C中使用 typedef struct 定义一个结构体类型,名为struct_message
// 必须与接收方的结构相匹配一致
#pragma pack (1)
typedef struct struct_message {
  uint8_t   	key_status; //按键状态：1（按下）; 0（松开）
  uint8_t  	  sys_status; //系统状态：0（MOVE）; 1（RISER）; 2（JUMP）; 3（平衡）
  uint16_t  	x_data;     //左摇杆x轴数据
  uint16_t  	y_data;     //左摇杆y轴数据
  uint16_t  	z_data;     //右摇杆x轴数据
  uint16_t  	k_data;     //右摇杆y轴数据
} struct_message;
#pragma pack ()

// 创建 结构为struct_message的myData变量
struct_message myData;

// 声明对等网络信息实体类变量
esp_now_peer_info_t peerInfo;

//OLED引脚定义
#define SDA   19
#define SCL   18
SSD1306Wire display(0x3c, SDA, SCL);

/*此功能用于将0-4095操纵手柄值映射到0-254。因此127是我们发送的中心值。
 *它还可以调整操纵手柄中的死区。
 *Jostick值的范围为0-4095。但它的中心值并不总是2047。这没什么不同。
 *所以我们需要在中心值上加一些死区。在我们的案例中是1800-2200。在这个死区范围内的任何值都被映射到中心127。
 */
int mapAndAdjustJoystickDeadBandValues(int value, bool reverse)
{
  if (value >= 2200)
  {
    value = map(value, 2200, 4095, 127, 254);
  }
  else if (value <= 1800)
  {
    value = map(value, 1800, 0, 127, 0);  
  }
  else
  {
    value = 127;
  }

  if (reverse)
  {
    value = 254 - value;
  }
  return value;
}

// 当发送信息时，触发的回调函数
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //用于调试查看，可以注释
  Serial.print("\r\nLast Packet Send Status:\t"); 
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  myData.sys_status = MOVE;	//设置机器人为移动状态

  // 配置输入模式
  pinMode(KEY, INPUT);    // 设置引脚为输入模式
  pinMode(STATUS, INPUT); // 设置引脚为输入模式
  pinMode(PS1_X, INPUT);  // 设置引脚为输入模式
  pinMode(PS1_Y, INPUT);  // 设置引脚为输入模式
  pinMode(PS2_X, INPUT);  // 设置引脚为输入模式
  pinMode(PS2_Y, INPUT);  // 设置引脚为输入模式

  display.init();//初始化UI
  display.flipScreenVertically();//垂直翻转屏幕设置
  display.setFont(ArialMT_Plain_16);//设置字体大小
  display.display();//将缓存数据写入到显示器

  // 设置串口波特率
  Serial.begin(9600);

  // 设置设备WIFI模式为WIFI_STA
  WiFi.mode(WIFI_STA);

  // 初始化ESPNOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // 当ESPNOW初始化成功,我们将会注册一个回调函数（callback，CB）
  // 获得数据包的发送情况
  esp_now_register_send_cb(OnDataSent);

  // 配置对等（对等点）网络
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  // 添加对等（对等点）网络       
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}


void loop() {
  myData.key_status = 0; // 读取引脚的电平值
  // 赋值需要发送的变量数据 
  if(digitalRead(KEY) == LOW){
    delay(10);
    while(digitalRead(KEY) == LOW);
    myData.key_status = 1;
  }
  if(digitalRead(STATUS) == LOW){
    delay(10);
    while(digitalRead(STATUS) == LOW);
    switch (myData.sys_status)
    {
      case MOVE:
        myData.sys_status = RISER;
        break;
      case RISER:
        myData.sys_status = JUMP;
        break;
      case JUMP:
        myData.sys_status = BALANCE;
        break;
      case BALANCE:
        myData.sys_status = MOVE;
        break;
      default:
        break;
    }
  }
  myData.x_data = mapAndAdjustJoystickDeadBandValues(analogRead(PS1_X), false);
  myData.y_data = mapAndAdjustJoystickDeadBandValues(analogRead(PS2_Y), false);
  myData.z_data = mapAndAdjustJoystickDeadBandValues(analogRead(PS2_X), false);
  myData.k_data = mapAndAdjustJoystickDeadBandValues(analogRead(PS1_Y), false);

  // 通过ESPNOW发送信息
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  // 根据 result 返回结果判断是否发送成功
  if (result == ESP_OK) {// 当 发送成功 时
    Serial.println("Sent with success");
  }
  else { // 当 发送失败 时
    Serial.println("Error sending the data");
  }
  Serial.printf("x = %d  y = %d key = %d\n", myData.x_data, myData.y_data, myData.key_status);
  Serial.printf("z = %d  k = %d sys = %d\n", myData.z_data, myData.k_data, myData.sys_status);
  
  display.clear();
  sprintf(str, "x = %d  y = %d\n", myData.x_data, myData.y_data);
  display.drawString(0, 0, str);//显示
  sprintf(str, "z = %d  k = %d\n", myData.z_data, myData.k_data);
  display.drawString(0, 20, str);//显示
  switch (myData.sys_status)
  {
    case MOVE:
      sprintf(str, "MOVE");
      break;
    case RISER:
      sprintf(str, "RISER");
      break;
    case JUMP:
      sprintf(str, "JUMP");
      break;
    case BALANCE:
      sprintf(str, "BALANCE");
      break;
    default:
      break;
  }
  display.drawString(0, 40, str);
  display.display();//将缓存数据写入到显示器
  delay(100);
}