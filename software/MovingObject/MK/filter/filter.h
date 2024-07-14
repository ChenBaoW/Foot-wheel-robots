#ifndef		_FILTER_H
#define		_FILTER_H

// 定义移动平均滤波器的窗口大小
#define WINDOW_SIZE 10

// 定义移动平均滤波器结构体
typedef struct {
    float buffer[WINDOW_SIZE]; // 滤波器的缓冲区
    int index;                 // 当前缓冲区的索引
    int count;                 // 当前缓冲区中的元素数量
} MovingAverageFilter;

extern MovingAverageFilter filter;

// 初始化滤波器
void init_filter(MovingAverageFilter *filter);

// 向滤波器添加数据并计算平均值
float add_and_get_average(MovingAverageFilter *filter, float new_data);

#endif		/* _FILTER_H */

