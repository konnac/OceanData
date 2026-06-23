/**
 * @file PredictUtil.c
 * @brief 单因素线性回归预测模型工具库
 * 
 * 本模块实现了基于最小二乘法的线性回归预测模型，用于探究海水养殖水质参数间的线性关联规律。
 * 主要功能包括：
 * - 线性回归拟合（计算斜率和截距）
 * - 决定系数(R2)评估
 * - 留出法评估（基于均方根误差RMSE）
 * - 多因子影响对比分析（气温、水温、pH值、盐度与溶解氧的关系）
 * 
 * @author Auto Generated
 * @date 2026-05-19
 */

#include "PredictUtil.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

/**
 * @brief 检查double值是否有效（非NaN且非无穷大）
 * 
 * @param value 待检查的double值
 * @return int 有效返回1，无效返回0
 */
static int isValidDouble(double value) {
    return !isnan(value) && !isinf(value);
}

/**
 * @brief 基于最小二乘法进行线性回归拟合
 * slope = (Σ(x_i·y_i) - n·x̄·ȳ) / (Σ(x_i²) - n·x̄²)
 * 计算自变量x和因变量y之间的线性回归模型，得到回归方程 y = slope * x + intercept
 * 同时计算决定系数R2评估拟合效果。
 * 
 * @param x 自变量数组
 * @param y 因变量数组
 * @param n 数据点数量
 * @return LinearModel 包含斜率、截距和R2值的线性模型
 */
LinearModel linearRegression(double* x, double* y, int n) {
    LinearModel model;
    model.slope = 0.0;
    model.intercept = 0.0;
    model.r_squared = 0.0;

    if (n < 2) {
        return model;
    }

    double sum_x = 0.0, sum_y = 0.0;
    double sum_xy = 0.0, sum_x2 = 0.0;
    
    int valid_count = 0;
    for (int i = 0; i < n; i++) {
        if (isValidDouble(x[i]) && isValidDouble(y[i])) {
            sum_x += x[i];
            sum_y += y[i];
            sum_xy += x[i] * y[i];
            sum_x2 += x[i] * x[i];
            valid_count++;
        }
    }

    if (valid_count < 2) {
        return model;
    }

    double mean_x = sum_x / valid_count; // x平均值
    double mean_y = sum_y / valid_count; // y平均值

    double numerator = sum_xy - sum_x * mean_y; // 分子
    double denominator = sum_x2 - sum_x * mean_x; // 分母

    // 绝对值小于阈值,认为无效
    if (fabs(denominator) < 1e-10) {
        return model;
    }

    model.slope = numerator / denominator; //计算斜率
    model.intercept = mean_y - model.slope * mean_x; //计算截距

    // 分子是总平方和 和 分母是残差平方和
    double ss_tot = 0.0, ss_res = 0.0;
    for (int i = 0; i < n; i++) {
        if (isValidDouble(x[i]) && isValidDouble(y[i])) {
            //计算出 y^
            double pred = model.slope * x[i] + model.intercept;
            ss_res += (y[i] - pred) * (y[i] - pred); //分子
            ss_tot += (y[i] - mean_y) * (y[i] - mean_y); //分母
        }
    }

    // 计算决定系数R2
    if (ss_tot > 1e-10) {
        model.r_squared = 1.0 - ss_res / ss_tot;
    }

    return model;
}

/**
 * @brief 使用线性回归模型进行预测
 * 
 * 根据已拟合的线性模型，输入自变量值预测因变量值。
 * 
 * @param model 已训练的线性回归模型
 * @param x 自变量输入值
 * @return double 预测的因变量值
 */
double predict(LinearModel model, double x) {
    return model.slope * x + model.intercept;
}

/**
 * @brief 计算决定系数R2
 * 
 * R2值越接近1，说明模型对历史数据的拟合效果越好，预测可靠性越高。
 * 
 * 计算公式: R2 = 1 - Σ(y_i - ŷ_i)² / Σ(y_i - ȳ)²
 * 其中: y_i为真实值, ŷ_i为预测值, ȳ为真实值的均值
 * 
 * @param y_true 真实值数组
 * @param y_pred 预测值数组
 * @param n 数据点数量
 * @return double 决定系数R2值
 */
double calculateRSquared(double* y_true, double* y_pred, int n) {
    if (n < 2) return 0.0;

    double mean_y = 0.0;
    int valid_count = 0;
    for (int i = 0; i < n; i++) {
        if (isValidDouble(y_true[i]) && isValidDouble(y_pred[i])) {
            mean_y += y_true[i];
            valid_count++;
        }
    }

    if (valid_count < 2) return 0.0;
    mean_y /= valid_count;

    double ss_tot = 0.0, ss_res = 0.0;
    for (int i = 0; i < n; i++) {
        if (isValidDouble(y_true[i]) && isValidDouble(y_pred[i])) {
            ss_res += (y_true[i] - y_pred[i]) * (y_true[i] - y_pred[i]);
            ss_tot += (y_true[i] - mean_y) * (y_true[i] - mean_y);
        }
    }

    if (ss_tot < 1e-10) return 0.0;
    return 1.0 - ss_res / ss_tot;
}

/**
 * @brief 计算均方根误差RMSE
 * 
 * RMSE用于衡量预测值与真实值之间的平均差距，值越小说明预测精度越高。
 * 
 * 计算公式: RMSE = sqrt(Σ(y_i^真实 - y_i^预测)² / N)
 * 其中: N为测试集数据总量
 * 
 * @param y_true 真实值数组
 * @param y_pred 预测值数组
 * @param n 数据点数量
 * @return double 均方根误差RMSE值
 */
double calculateRMSE(double* y_true, double* y_pred, int n) {
    if (n < 1) return 0.0;

    double sum_squared_error = 0.0;
    int valid_count = 0;
    for (int i = 0; i < n; i++) {
        if (isValidDouble(y_true[i]) && isValidDouble(y_pred[i])) {
            double error = y_true[i] - y_pred[i];
            sum_squared_error += error * error;
            valid_count++;
        }
    }

    if (valid_count < 1) return 0.0;
    return sqrt(sum_squared_error / valid_count);
}

/**
 * @brief 使用留出法训练模型并评估
 * 
 * 将数据集按时间顺序拆分为训练集（前80%）和测试集（后20%），
 * 使用训练集训练模型，使用测试集评估模型的泛化能力。
 * 
 * @param x 自变量数组
 * @param y 因变量数组
 * @param n 数据点总数
 * @param rmse_out 输出参数，用于返回测试集的RMSE值
 * @return LinearModel 训练得到的线性回归模型
 */
LinearModel trainModelWithHoldout(double* x, double* y, int n, double* rmse_out) {
    *rmse_out = 0.0;
    
    if (n < 5) {
        LinearModel model = {0.0, 0.0, 0.0};
        return model;
    }

    int train_size = (int)(n * 0.8);
    int test_size = n - train_size;

    double* train_x = (double*)malloc(train_size * sizeof(double));
    double* train_y = (double*)malloc(train_size * sizeof(double));
    double* test_x = (double*)malloc(test_size * sizeof(double));
    double* test_y = (double*)malloc(test_size * sizeof(double));
    double* test_pred = (double*)malloc(test_size * sizeof(double));

    int train_idx = 0, test_idx = 0;
    for (int i = 0; i < n; i++) {
        if (isValidDouble(x[i]) && isValidDouble(y[i])) {
            if (i < train_size) {
                train_x[train_idx] = x[i];
                train_y[train_idx] = y[i];
                train_idx++;
            } else {
                test_x[test_idx] = x[i];
                test_y[test_idx] = y[i];
                test_idx++;
            }
        }
    }

    LinearModel model = linearRegression(train_x, train_y, train_idx);

    for (int i = 0; i < test_idx; i++) {
        test_pred[i] = predict(model, test_x[i]);
    }

    *rmse_out = calculateRMSE(test_y, test_pred, test_idx);

    free(train_x);
    free(train_y);
    free(test_x);
    free(test_y);
    free(test_pred);

    return model;
}

/**
 * @brief 分析指定因子与溶解氧(DO)的线性关系
 * 
 * 内部辅助函数，根据因子类型提取对应数据，执行线性回归分析。
 * 
 * @param records 水质数据集
 * @param factor_type 因子类型: 0-气温, 1-水温, 2-pH值, 3-盐度
 * @return LinearModel 线性回归模型
 */
static LinearModel analyzeFactorDO(const WaterQualityRecords* records, int factor_type) {
    if (!records || records->count < 2) {
        LinearModel model = {0.0, 0.0, 0.0};
        return model;
    }

    double* x = (double*)malloc(records->count * sizeof(double));
    double* y = (double*)malloc(records->count * sizeof(double));
    int valid_count = 0;

    for (int i = 0; i < records->count; i++) {
        // 获取对应记录
        const WaterQualityRecord* record = WQ_GetRecord(records, i);
        if (!record) continue;

        double factor_value;
        // 根据因子类型获取对应数据
        switch (factor_type) {
            case 0: factor_value = (double)record->Air_temp; break;
            case 1: factor_value = (double)record->Temp; break;
            case 2: factor_value = (double)record->pH; break;
            case 3: factor_value = (double)record->Salinity; break;
            default: factor_value = 0.0;
        }

        //加入到x y数组中
        if (isValidDouble(factor_value) && isValidDouble((double)record->DO)) {
            x[valid_count] = factor_value;
            y[valid_count] = (double)record->DO;
            valid_count++;
        }
    }

    // 获取线性回归模型
    LinearModel model = linearRegression(x, y, valid_count);
    free(x);
    free(y);
    return model;
}

/**
 * @brief 分析气温(Air_temp)与溶解氧(DO)的线性关系
 * 
 * @param records 水质数据集
 * @return LinearModel 气温-DO线性回归模型
 */
LinearModel analyzeAirTempDO(const WaterQualityRecords* records) {
    return analyzeFactorDO(records, 0);
}

/**
 * @brief 分析水温(Temp)与溶解氧(DO)的线性关系
 * 
 * @param records 水质数据集
 * @return LinearModel 水温-DO线性回归模型
 */
LinearModel analyzeTempDO(const WaterQualityRecords* records) {
    return analyzeFactorDO(records, 1);
}

/**
 * @brief 分析pH值与溶解氧(DO)的线性关系
 * 
 * @param records 水质数据集
 * @return LinearModel pH-DO线性回归模型
 */
LinearModel analyzePhDO(const WaterQualityRecords* records) {
    return analyzeFactorDO(records, 2);
}

/**
 * @brief 分析盐度(Salinity)与溶解氧(DO)的线性关系
 * 
 * @param records 水质数据集
 * @return LinearModel 盐度-DO线性回归模型
 */
LinearModel analyzeSalinityDO(const WaterQualityRecords* records) {
    return analyzeFactorDO(records, 3);
}

/**
 * @brief 对比分析各环境因子对溶解氧(DO)的影响程度
 * 
 * 分别建立气温、水温、pH值、盐度与DO的单因素线性回归模型，
 * 计算各自的R2值并进行排序，量化判断哪个环境因子对溶解氧的影响程度最大。
 * 
 * @param records 水质数据集
 */
void compareFactorsImpact(const WaterQualityRecords* records) {
    typedef struct {
        const char* name;
        const char* var_name;
        LinearModel model;
    } FactorResult;

    // 初始化
    FactorResult factors[] = {
        {"气温(Air_temp)", "Air_temp", analyzeAirTempDO(records)},
        {"水温(Temp)", "Temp", analyzeTempDO(records)},
        {"pH值", "pH", analyzePhDO(records)},
        {"盐度(Salinity)", "Salinity", analyzeSalinityDO(records)}
    };
    const int factor_count = sizeof(factors) / sizeof(factors[0]);

    printf("=== 多因子影响对比分析结果 ===\n");
    for (int i = 0; i < factor_count; i++) {
        printf("%d. %s与溶解氧(DO):\n", i + 1, factors[i].name);
        printf("   回归方程: DO = %.4f * %s + %.4f\n", 
               factors[i].model.slope, factors[i].var_name, factors[i].model.intercept);
        printf("   R2值: %.4f\n\n", factors[i].model.r_squared);
    }

    // 冒泡排序 降序
    for (int i = 0; i < factor_count - 1; i++) {
        for (int j = 0; j < factor_count - 1 - i; j++) {
            if (factors[j].model.r_squared < factors[j + 1].model.r_squared) {
                FactorResult temp = factors[j];
                factors[j] = factors[j + 1];
                factors[j + 1] = temp;
            }
        }
    }

    printf("=== 影响程度对比结论 ===\n");
    printf("各因子对溶解氧(DO)的影响程度排序(R2值从高到低):\n");
    for (int i = 0; i < factor_count; i++) {
        printf("%d. %s: %.4f\n", i + 1, factors[i].name, factors[i].model.r_squared);
    }

    printf("\n结论: %s对溶解氧(DO)的影响程度最大，R2值为%.4f\n", 
           factors[0].name, factors[0].model.r_squared);
}
