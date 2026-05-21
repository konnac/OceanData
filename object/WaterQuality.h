#ifndef WATER_QUALITY_H
#define WATER_QUALITY_H

#include <stddef.h>

// 单个水质记录实体
typedef struct {
    char DailyStats[20];      // 格式 "YYYY-MM-DD HH:MM:SS"
    float Temp;
    float Salinity;
    float pH;
    float DO;
    float precipitation;
    float Air_temp;
} WaterQualityRecord;

// 数据集实体（动态数组）
typedef struct {
    int count;
    int capacity;
    WaterQualityRecord *records;
} WaterQualityRecords;

// --- 基础操作 ---
void WQ_Init(WaterQualityRecords *dataset, int initialCapacity);
void WQ_Destroy(WaterQualityRecords *dataset);
int  WQ_AddRecord(WaterQualityRecords *dataset, const WaterQualityRecord *record);
const WaterQualityRecord* WQ_GetRecord(const WaterQualityRecords *dataset, int index);
int  WQ_UpdateRecord(WaterQualityRecords *dataset, int index, const WaterQualityRecord *record);
int  WQ_DeleteRecord(WaterQualityRecords *dataset, int index);                 // 单条删除
int  WQ_DeleteRecords(WaterQualityRecords *dataset, int indices[], int count); // 批量删除

// --- 排序与筛选 ---
typedef enum { SORT_ASC, SORT_DESC } SortOrder;
typedef enum { PARAM_TEMP, PARAM_SALINITY, PARAM_PH, PARAM_DO, PARAM_PRECIP, PARAM_AIRTEMP } ParamType;
void WQ_Sort(WaterQualityRecords *dataset, ParamType param, SortOrder order);
int  WQ_FilterByRange(const WaterQualityRecords *dataset, ParamType param, float min, float max, int **out_indices);

// --- 文件操作 ---
int  TxtUtil_LoadFromFile(const char *filename, WaterQualityRecords *records);
int  TxtUtil_SaveToFile(const char *filename, const WaterQualityRecords *records);
int  BinUtil_SaveToFile(const char *filename, const WaterQualityRecords *records);
int  BinUtil_LoadFromFile(const char *filename, WaterQualityRecords *records);
void PerformanceCompare(const WaterQualityRecords *records);

// --- 备份与恢复 ---
int  Backup_Backup(const WaterQualityRecords *records, const char *customName); // 自动生成时间戳文件名
int  Backup_List(char backupList[][256], int maxCount);
int  Backup_Restore(const char *backupFilename, WaterQualityRecords *records);

// --- 辅助函数（参数合理性验证）---
int  IsParamValid(ParamType type, float value);
#endif