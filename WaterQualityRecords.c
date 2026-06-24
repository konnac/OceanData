#include "WaterQuality.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

WaterQualityRecords g_records;

//初始化函数
void WQ_Init(WaterQualityRecords *dataset, int initialCapacity) {
    if (!dataset) return;
    dataset->capacity = initialCapacity > 0 ? initialCapacity : 10;
    dataset->count = 0;
    dataset->records = (WaterQualityRecord*)malloc(dataset->capacity * sizeof(WaterQualityRecord));
    if (!dataset->records) dataset->capacity = 0;
}


//摧毁函数
void WQ_Destroy(WaterQualityRecords *dataset) {
    if (dataset && dataset->records) {
        free(dataset->records);
        dataset->records = NULL;
        dataset->capacity = 0;
        dataset->count = 0;
    }
}

//添加函数
int WQ_AddRecord(WaterQualityRecords *dataset, const WaterQualityRecord *record) {
    if (!dataset || !record) return -1;
    if (dataset->count >= dataset->capacity) {
        int newCap = dataset->capacity * 2;
        WaterQualityRecord *newRec = (WaterQualityRecord*)realloc(dataset->records, newCap * sizeof(WaterQualityRecord));
        if (!newRec) return -1;
        dataset->records = newRec;
        dataset->capacity = newCap;
    }
    dataset->records[dataset->count] = *record;
    dataset->count++;
    return 0;
}

//获取函数
const WaterQualityRecord* WQ_GetRecord(const WaterQualityRecords *dataset, int index) {
    if (!dataset || index < 0 || index >= dataset->count) return NULL;
    return &dataset->records[index];
}

//更新函数
int WQ_UpdateRecord(WaterQualityRecords *dataset, int index, const WaterQualityRecord *record) {
    if (!dataset || index < 0 || index >= dataset->count || !record) return -1;
    dataset->records[index] = *record;
    return 0;
}

//删除函数
int WQ_DeleteRecord(WaterQualityRecords *dataset, int index) {
    if (!dataset || index < 0 || index >= dataset->count) return -1;
    for (int i = index; i < dataset->count - 1; i++)
        dataset->records[i] = dataset->records[i + 1];
    dataset->count--;
    return 0;
}

//批量删除函数
int WQ_DeleteRecords(WaterQualityRecords *dataset, int indices[], int count) {
    if (!dataset || !indices || count <= 0) return -1;
    int *toDelete = (int*)calloc(dataset->count, sizeof(int));
    if (!toDelete) return -1;
    for (int i = 0; i < count; i++) {
        if (indices[i] >= 0 && indices[i] < dataset->count)
            toDelete[indices[i]] = 1;
    }
    int newCount = 0;
    for (int i = 0; i < dataset->count; i++) {
        if (!toDelete[i])
            dataset->records[newCount++] = dataset->records[i];
    }
    dataset->count = newCount;
    free(toDelete);
    return 0;
}

// ---------- 比较函数 ----------
static int cmpTempAsc(const void *a, const void *b) {
    float diff = ((WaterQualityRecord*)a)->Temp - ((WaterQualityRecord*)b)->Temp;
    return (diff > 0) - (diff < 0);
}
static int cmpTempDesc(const void *a, const void *b) { return -cmpTempAsc(a, b); }

static int cmpSalinityAsc(const void *a, const void *b) {
    float diff = ((WaterQualityRecord*)a)->Salinity - ((WaterQualityRecord*)b)->Salinity;
    return (diff > 0) - (diff < 0);
}
static int cmpSalinityDesc(const void *a, const void *b) { return -cmpSalinityAsc(a, b); }

static int cmpPHAsc(const void *a, const void *b) {
    float diff = ((WaterQualityRecord*)a)->pH - ((WaterQualityRecord*)b)->pH;
    return (diff > 0) - (diff < 0);
}
static int cmpPHDesc(const void *a, const void *b) { return -cmpPHAsc(a, b); }

static int cmpDOAsc(const void *a, const void *b) {
    float diff = ((WaterQualityRecord*)a)->DO - ((WaterQualityRecord*)b)->DO;
    return (diff > 0) - (diff < 0);
}
static int cmpDODesc(const void *a, const void *b) { return -cmpDOAsc(a, b); }

static int cmpPrecipAsc(const void *a, const void *b) {
    float diff = ((WaterQualityRecord*)a)->precipitation - ((WaterQualityRecord*)b)->precipitation;
    return (diff > 0) - (diff < 0);
}
static int cmpPrecipDesc(const void *a, const void *b) { return -cmpPrecipAsc(a, b); }

static int cmpAirTempAsc(const void *a, const void *b) {
    float diff = ((WaterQualityRecord*)a)->Air_temp - ((WaterQualityRecord*)b)->Air_temp;
    return (diff > 0) - (diff < 0);
}
static int cmpAirTempDesc(const void *a, const void *b) { return -cmpAirTempAsc(a, b); }

void WQ_Sort(WaterQualityRecords *dataset, ParamType param, SortOrder order) {
    if (!dataset || dataset->count <= 1) return;
    int (*cmp)(const void*, const void*) = NULL;
    switch (param) {
        case PARAM_TEMP:      cmp = (order == SORT_ASC) ? cmpTempAsc : cmpTempDesc; break;
        case PARAM_SALINITY:  cmp = (order == SORT_ASC) ? cmpSalinityAsc : cmpSalinityDesc; break;
        case PARAM_PH:        cmp = (order == SORT_ASC) ? cmpPHAsc : cmpPHDesc; break;
        case PARAM_DO:        cmp = (order == SORT_ASC) ? cmpDOAsc : cmpDODesc; break;
        case PARAM_PRECIP:    cmp = (order == SORT_ASC) ? cmpPrecipAsc : cmpPrecipDesc; break;
        case PARAM_AIRTEMP:   cmp = (order == SORT_ASC) ? cmpAirTempAsc : cmpAirTempDesc; break;
        default: return;
    }
    qsort(dataset->records, dataset->count, sizeof(WaterQualityRecord), cmp);
}

int WQ_FilterByRange(const WaterQualityRecords *dataset, ParamType param, float min, float max, int **out_indices) {
    if (!dataset || !out_indices) return 0;
    int *temp = (int*)malloc(dataset->count * sizeof(int));
    if (!temp) return 0;
    int found = 0;
    for (int i = 0; i < dataset->count; i++) {
        float val = 0;
        switch (param) {
            case PARAM_TEMP:      val = dataset->records[i].Temp; break;
            case PARAM_SALINITY:  val = dataset->records[i].Salinity; break;
            case PARAM_PH:        val = dataset->records[i].pH; break;
            case PARAM_DO:        val = dataset->records[i].DO; break;
            case PARAM_PRECIP:    val = dataset->records[i].precipitation; break;
            case PARAM_AIRTEMP:   val = dataset->records[i].Air_temp; break;
        }
        if (val >= min && val <= max)
            temp[found++] = i;
    }
    if (found > 0) {
        *out_indices = (int*)malloc(found * sizeof(int));
        if (*out_indices)
            memcpy(*out_indices, temp, found * sizeof(int));
        else
            found = 0;
    } else {
        *out_indices = NULL;
    }
    free(temp);
    return found;
}
