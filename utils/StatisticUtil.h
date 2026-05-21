//统计分析,分段统计,相关性分析
#ifndef __STATISTICUTIL_H__
#define __STATISTICUTIL_H__

#include "WaterQuality.h"


void CalcBasicStats(const WaterQualityRecords *records, ParamType param,
                    float *mean, float *max, float *min, float *variance, float *stddev);

void GenerateBasicStatsReport(const WaterQualityRecords *records);

void DawnHypoxiaWarning(const WaterQualityRecords *records);

void SalinityShockWarning(const WaterQualityRecords *records);

double pearsonCount(const double *x, const double *y, int n);

void CorrelationAnalysis(const WaterQualityRecords *records);

void RunAllStatistics(const WaterQualityRecords *records);

#endif