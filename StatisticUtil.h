//统计分析,分段统计,相关性分析
#ifndef STATISTICUTIL_H
#define STATISTICUTIL_H

#include "WaterQuality.h"

//基本统计量：计算指定参数的平均值、最大值、最小值、方差、标准差
void CalcBasicStats(const WaterQualityRecords *dataset, ParamType param,
                    float *mean, float *max, float *min, float *variance, float *stddev);

//生成所有参数的基本统计量报告(写入 stst_report.csv)
void GenerateBasicStatsReport(const WaterQualityRecords *records);

//凌晨缺氧预警(每天3:00~5:00的Do均值)
//结果写入 warning_report.csv
void DawnHypoxiaWarning(const WaterQualityRecords *records);

//相关性分析：计算6x6相关系数矩阵，写入stat_report.csv 并打印出最强相关
void CorrelationAnalysis(const WaterQualityRecords *records);

//盐度突变预警
void SalinityShockWarning(const WaterQualityRecords *records);

//一键执行以上所有功能
void RunAllStatistics(const WaterQualityRecords *records);
#endif


