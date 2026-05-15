//数据集结构体,包含实际存储的记录数,当前数组的容量,动态数组对象
#include "WaterQuality.h"

typedef struct WaterQualityRecords
{
    int count;
    int capacity;
    WaterQualityRecord *waterQualityRecord;
};




