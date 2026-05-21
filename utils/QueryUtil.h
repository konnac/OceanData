#ifndef QUERY_UTIL_H
#define QUERY_UTIL_H

#include "WaterQuality.h"

// 分页显示
void DisplayPage(const WaterQualityRecords *dataset, int page, int rowsPerPage);
// 条件筛选交互
void FilterAndDisplay(const WaterQualityRecords *dataset);
// 排序交互
void SortAndDisplay(WaterQualityRecords *dataset);

#endif