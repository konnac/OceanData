/**
 * @file PredictUtil.h
 * @brief 单因素线性回归预测模型工具库接口定义
 * 
 * 本模块提供了基于最小二乘法的线性回归预测模型接口，用于探究海水养殖水质参数间的线性关联规律。
 * 复用 object/WaterQuality.h 中的数据结构，避免代码冗余。
 * 
 * @author Auto Generated
 * @date 2026-05-19
 */

#ifndef PREDICT_UTIL_H
#define PREDICT_UTIL_H

#include <stdlib.h>
#include "../object/WaterQuality.h"  // 复用水质数据结构

/**
 * @brief 线性回归模型结构体
 * 
 * 存储线性回归拟合得到的参数，包括斜率slope、截距intercept和决定系数r_squared。
 */
typedef struct {
    double slope;          /**< 直线斜率（回归系数） */
    double intercept;      /**< 直线截距 */
    double r_squared;      /**< 决定系数R2，评估拟合效果 */
} LinearModel;

/**
 * @brief 基于最小二乘法进行线性回归拟合
 * 
 * @param x 自变量数组
 * @param y 因变量数组
 * @param n 数据点数量
 * @return LinearModel 包含斜率、截距和R2值的线性模型
 */
LinearModel linearRegression(double* x, double* y, int n);

/**
 * @brief 使用线性回归模型进行预测
 * 
 * @param model 已训练的线性回归模型
 * @param x 自变量输入值
 * @return double 预测的因变量值
 */
double predict(LinearModel model, double x);

/**
 * @brief 计算决定系数R2
 * 
 * @param y_true 真实值数组
 * @param y_pred 预测值数组
 * @param n 数据点数量
 * @return double 决定系数R2值
 */
double calculateRMSE(double* y_true, double* y_pred, int n);

/**
 * @brief 使用留出法训练模型并评估
 * 
 * @param x 自变量数组
 * @param y 因变量数组
 * @param n 数据点总数
 * @param rmse_out 输出参数，用于返回测试集的RMSE值
 * @return LinearModel 训练得到的线性回归模型
 */
LinearModel trainModelWithHoldout(double* x, double* y, int n, double* rmse_out);

/**
 * @brief 分析气温(Air_temp)与溶解氧(DO)的线性关系
 * 
 * @param records 水质数据集
 * @return LinearModel 气温-DO线性回归模型
 */
LinearModel analyzeAirTempDO(const WaterQualityRecords* records);

/**
 * @brief 分析水温(Temp)与溶解氧(DO)的线性关系
 * 
 * @param records 水质数据集
 * @return LinearModel 水温-DO线性回归模型
 */
LinearModel analyzeTempDO(const WaterQualityRecords* records);

/**
 * @brief 分析pH值与溶解氧(DO)的线性关系
 * 
 * @param records 水质数据集
 * @return LinearModel pH-DO线性回归模型
 */
LinearModel analyzePhDO(const WaterQualityRecords* records);

/**
 * @brief 分析盐度(Salinity)与溶解氧(DO)的线性关系
 * 
 * @param records 水质数据集
 * @return LinearModel 盐度-DO线性回归模型
 */
LinearModel analyzeSalinityDO(const WaterQualityRecords* records);

/**
 * @brief 使用留出法评估指定因子与溶解氧的预测模型
 *
 * @param records 水质数据集
 * @param factor_type 因子类型: 0-气温, 1-水温, 2-pH值, 3-盐度
 * @param rmse_out 输出参数，用于返回测试集的RMSE值
 * @return LinearModel 训练得到的线性回归模型
 */
LinearModel evaluateFactorDOWithHoldout(const WaterQualityRecords* records, 
                                       int factor_type, double* rmse_out);

/**
 * @brief 对比分析各环境因子对溶解氧(DO)的影响程度
 *
 * @param records 水质数据集
 */
void compareFactorsImpact(const WaterQualityRecords* records);

#endif
