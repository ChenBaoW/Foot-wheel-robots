#include "filter.h"

// 创建并初始化滤波器
MovingAverageFilter filter;

// 初始化滤波器
void init_filter(MovingAverageFilter *filter) {
    for (int i = 0; i < WINDOW_SIZE; i++) {
        filter->buffer[i] = 0.0;
    }
    filter->index = 0;
    filter->count = 0;
}

// 向滤波器添加数据并计算平均值
float add_and_get_average(MovingAverageFilter *filter, float new_data) {
    // 将新数据添加到缓冲区
    filter->buffer[filter->index] = new_data;
    
    // 更新索引
    filter->index = (filter->index + 1) % WINDOW_SIZE;
    
    // 如果缓冲区未满，增加计数
    if (filter->count < WINDOW_SIZE) {
        filter->count++;
    }
    
    // 计算平均值
    float sum = 0.0;
    for (int i = 0; i < filter->count; i++) {
        sum += filter->buffer[i];
    }
    return sum / filter->count;
}

