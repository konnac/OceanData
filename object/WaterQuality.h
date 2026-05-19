// WaterQuality.h
#ifndef WATER_QUALITY_H
#define WATER_QUALITY_H

#include <stddef.h> // 使用 size_t 等标准类型

// 1. 单个水质记录实体
typedef struct {
    float Temp; // 水温
    float Salinity; // 盐度
    float pH; // 酸碱度
    float DO; // 溶解氧
    float precipitation; // 降水量
    float Air_temp; // 空气温度
} WaterQualityRecord;

// 2. 数据集实体（管理动态数组）
typedef struct {
    int count;              // 当前记录数
    int capacity;           // 数组容量
    WaterQualityRecord *records; // 指向动态数组的指针
} WaterQualityRecords;

//全局变量声明
extern WaterQualityRecords g_records;

// 3. 接口声明（数据集操作）
void WQ_Init(WaterQualityRecords *dataset, int initialCapacity);
void WQ_Destroy(WaterQualityRecords *dataset);
int  WQ_AddRecord(WaterQualityRecords *dataset, const WaterQualityRecord *record);
const WaterQualityRecord* WQ_GetRecord(const WaterQualityRecords *dataset, int index);

// 4. 接口声明（文件操作）
//读取文件
int TxtUtil_LoadFromFile(const char *filename, WaterQualityRecords *records);

//保存文件
int TxtUtil_SaveToFile(const char *filename, const WaterQualityRecords *records);

//修改数据
int OperateUtil_UpdateRecord(const char *filename, int index, const WaterQualityRecord *record);

//删除数据
int OperateUtil_DeleteRecord(const char *filename, int index);
#endif