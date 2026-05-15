#ifndef WATER_QUALITY_H
#define WATER_QUALITY_H

#include <stdlib.h>

//单个记录结构体
typedef struct {
    float Temp;
    float Salinity;
    float pH;
    float DO;
    float precipitation;
    float Air_temp;
} WaterQualityRecord;

//数据集结构体
typedef struct {
    int count;  //实际存储的记录数
    int capacity;   //当前数组容量
    WaterQualityRecord* records;    //动态数组指针
} WaterQualityRecords;

#endif