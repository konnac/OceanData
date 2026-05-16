#include "WaterQuality.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void WQ_Init(WaterQualityRecords *dataset, int initialCapacity) {
    if(!dataset) return;
    dataset->capacity = initialCapacity > 0 ? initialCapacity : 10;
    dataset->count = 0;
    dataset->records = (WaterQualityRecord*)malloc(initialCapacity * sizeof(WaterQualityRecord));
    if (!dataset -> records)
    {
        dataset->capacity = 0;
    }
}

//析构函数
void WQ_Destroy(WaterQualityRecords *dataset) {
    if(dataset && dataset->records) {
        free(dataset->records);
        dataset->records = NULL;
        dataset->capacity = 0;
        dataset->count = 0;
    }
}

//添加记录
int WQ_AddRecord(WaterQualityRecords *dataset, const WaterQualityRecord *record) {
    if(!dataset || !record) return -1;
    if(dataset->count >= dataset->capacity) {
        int newCapacity = dataset->capacity * 2;
        WaterQualityRecord *newRecords = 
        (WaterQualityRecord*)realloc(dataset->records, newCapacity * sizeof(WaterQualityRecord));
        if(!newRecords) return -1;
        dataset->records = newRecords;
        dataset->capacity = newCapacity;
    }
    dataset->records[dataset->count] = *record;
    dataset->count++;
    return 0;
}

//获取记录
const WaterQualityRecord* WQ_GetRecord(const WaterQualityRecords *dataset, int index) {
    if(!dataset || index < 0 || index >= dataset->count) return NULL;
    return &dataset->records[index];
}

//修改记录