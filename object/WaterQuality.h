#ifndef WATER_QUALITY_H
#define WATER_QUALITY_H

#include <stddef.h>

typedef struct {
    char DailyStats[20];
    float Temp;
    float Salinity;
    float pH;
    float DO;
    float precipitation;
    float Air_temp;
} WaterQualityRecord;

typedef struct {
    int count;
    int capacity;
    WaterQualityRecord *records;
} WaterQualityRecords;

extern WaterQualityRecords g_records;

void WQ_Init(WaterQualityRecords *dataset, int initialCapacity);
void WQ_Destroy(WaterQualityRecords *dataset);
int  WQ_AddRecord(WaterQualityRecords *dataset, const WaterQualityRecord *record);
const WaterQualityRecord* WQ_GetRecord(const WaterQualityRecords *dataset, int index);
int  WQ_UpdateRecord(WaterQualityRecords *dataset, int index, const WaterQualityRecord *record);
int  WQ_DeleteRecord(WaterQualityRecords *dataset, int index);
int  WQ_DeleteRecords(WaterQualityRecords *dataset, int indices[], int count);

typedef enum { SORT_ASC, SORT_DESC } SortOrder;
typedef enum { PARAM_TEMP, PARAM_SALINITY, PARAM_PH, PARAM_DO, PARAM_PRECIP, PARAM_AIRTEMP } ParamType;
void WQ_Sort(WaterQualityRecords *dataset, ParamType param, SortOrder order);
int  WQ_FilterByRange(const WaterQualityRecords *dataset, ParamType param, float min, float max, int **out_indices);

int  TxtUtil_LoadFromFile(const char *filename, WaterQualityRecords *records);
int  TxtUtil_SaveToFile(const char *filename, const WaterQualityRecords *records);
int  BinUtil_SaveToFile(const char *filename, const WaterQualityRecords *records);
int  BinUtil_LoadFromFile(const char *filename, WaterQualityRecords *records);
void PerformanceCompare(const WaterQualityRecords *records);

int  Backup_Backup(const WaterQualityRecords *records, const char *customName);
int  Backup_List(char backupList[][256], int maxCount);
int  Backup_Restore(const char *backupFilename, WaterQualityRecords *records);

int  IsParamValid(ParamType type, float value);

#endif
