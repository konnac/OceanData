/**
 * @file ExceptionUtil.h
 * @brief 检测异常值、缺失值操作、数据滤波模块头文件
 */

#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "WaterQuality.h"

/**
 * @brief 数据统计摘要结构体
 */
typedef struct {
    int total;                      /**< 总记录数 */
    int abnormal;                   /**< 含异常记录数 */
    int valid;                      /**< 有效记录数 */
    int deleted;                    /**< 被删除记录数 */
    int filled;                     /**< 填充缺失值数量 */
    int fixed;                      /**< 修复异常值数量 */
    int max_error_params;           /**< 单条记录最大异常参数个数 */
    char abnormal_start_time[20];   /**< 异常数据最早时间（字符串） */
    char abnormal_end_time[20];     /**< 异常数据最晚时间（字符串） */
} DataSummary;

/**
 * @brief 检测单个水质参数值是否异常（超出合理范围或为 NaN/Inf）
 * @param value 待检测的浮点数值
 * @param type 参数类型：0-水温,1-盐度,2-pH,3-溶解氧,4-降水量,5-空气温度
 * @return 1=异常，0=正常
 */
int is_abnormal(float value, int type);

/**
 * @brief 统计单条记录中异常参数的个数
 * @param record 指向水质记录结构体的指针
 * @return 异常参数的数量（0~6）
 */
int count_abnormal_params(const WaterQualityRecord *record);

/**
 * @brief 统计单条记录中缺失参数的个数
 * @param record 指向水质记录结构体的指针
 * @return 缺失参数的数量（0~6）
 */
int count_missing_params(const WaterQualityRecord *record);

/**
 * @brief 计算某个参数列的均值（排除异常值和缺失值）
 * @param dataset 指向数据集结构体的指针
 * @param param_type 参数类型（0~5）
 * @return 有效值的均值；若无有效数据则返回0.0f
 */
float calculate_column_mean(WaterQualityRecords *dataset, int param_type);

/**
 * @brief 使用均值逼近法填充单条记录中的所有异常值
 * @param dataset 指向数据集结构体的指针
 * @param record_index 待填充记录在数组中的索引
 * @return void
 */
void fill_abnormal_with_mean(WaterQualityRecords *dataset, int record_index);

/**
 * @brief 检测数据集中所有异常，并统计概览信息
 * @param dataset 指向数据集结构体的指针
 * @return 包含总记录数、异常记录数、最大异常参数个数等信息的 DataSummary 结构体
 */
DataSummary check_all_abnormal(WaterQualityRecords *dataset);

/**
 * @brief 处理异常数据（核心逻辑）：删除异常参数>=3的记录，填充0<异常<3的记录
 * @param dataset 指向数据集结构体的指针（会被原地修改）
 * @return 处理后的统计摘要（包含删除、修复等计数）
 */
DataSummary process_abnormal_data(WaterQualityRecords *dataset);

/**
 * @brief 输出数据概览到文件
 * @param filename 输出文件名
 * @param summary 指向 DataSummary 结构体的指针
 * @return void
 */
void write_data_summary(const char *filename, const DataSummary *summary);

/**
 * @brief 检测单个水质参数值是否为缺失值（NaN、Inf、-999、-9999）
 * @param value 待检测的浮点数值
 * @return 1=缺失，0=正常
 */
int is_missing(float value);

/**
 * @brief 处理缺失值：使用均值逼近法填充所有缺失值
 * @param dataset 指向数据集结构体的指针（会被原地修改）
 * @return 包含总记录数和填充数量等信息的 DataSummary 结构体
 */
DataSummary process_missing_values(WaterQualityRecords *dataset);

/**
 * @brief 计算某个参数列的标准差（样本标准差，分母 n-1）
 * @param dataset 指向数据集结构体的指针
 * @param param_type 参数类型（0~5）
 * @return 标准差，如果有效数据个数≤1则返回0.0f
 */
float calculate_std(WaterQualityRecords *dataset, int param_type);

/**
 * @brief 移动平均滤波（对四个核心参数：水温、盐度、pH值、溶解氧）
 * @param dataset 指向数据集结构体的指针（会被原地修改）
 * @param window_size 窗口大小，必须为奇数且≥3
 * @return void
 */
void moving_average_filter(WaterQualityRecords *dataset, int window_size);

/**
 * @brief 输出分析讨论报告到文件（包含方法对比、滤波前后标准差对比等）
 * @param filename 输出文件名
 * @param summary 指向 DataSummary 结构体的指针
 * @param std_before 滤波前四个参数的标准差数组（长度4）
 * @param std_after 滤波后四个参数的标准差数组（长度4）
 * @param window_sizes 窗口大小数组
 * @param window_count 窗口大小数组长度
 * @return void
 */
void write_analysis_report(const char *filename, const DataSummary *summary, 
                          float std_before[], float std_after[], int window_sizes[], int window_count);

#endif /* EXCEPTION_H */