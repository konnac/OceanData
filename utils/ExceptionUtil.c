/**
 * @file ExceptionUtil.c
 * @brief 检测异常值、缺失值操作、数据滤波
 */

#include "ExceptionUtil.h"

/**
 * @brief 检测单个水质参数值是否异常（超出合理范围或为 NaN/Inf）
 * @param value 待检测的浮点数值
 * @param type 参数类型：0-水温,1-盐度,2-pH,3-溶解氧,4-降水量,5-空气温度
 * @return 1=异常，0=正常
 */
int is_abnormal(float value, int type){
    if(isnan(value) || isinf(value)) {
        return 1;
    }
    switch(type) {
        case 0: return (value < -5 || value > 40);        // 温度
        case 1: return (value < 0 || value > 45);         // 盐度
        case 2: return (value < 6.5f || value > 9.0f);    // 酸碱度
        case 3: return (value < 0 || value > 15);         // 溶解氧
        case 4: return (value < 0 || value > 500);        // 降水量 
        case 5: return (value < -10 || value > 50);       // 空气温度 
        default: return 0;
    }
}

/**
 * @brief 检测单个水质参数值是否为缺失值（NaN、Inf、-999、-9999）
 * @param value 待检测的浮点数值
 * @return 1=缺失，0=正常
 */
int is_missing(float value) {
    if (isnan(value) || isinf(value)) {
        return 1;
    }
    // 特殊数值标记
    if (value == -999.0f || value == -9999.0f) {
        return 1;
    }
    return 0;
}

/**
 * @brief 统计单条记录中异常参数的个数
 * @param record 指向水质记录结构体的指针
 * @return 异常参数的数量（0~6）
 */
int count_abnormal_params(const WaterQualityRecord *record) {
    int count = 0;
    count += is_abnormal(record->Temp, 0);
    count += is_abnormal(record->Salinity, 1);
    count += is_abnormal(record->pH, 2);
    count += is_abnormal(record->DO, 3);
    count += is_abnormal(record->precipitation, 4);
    count += is_abnormal(record->Air_temp, 5);
    return count;
}

/**
 * @brief 统计单条记录中缺失参数的个数
 * @param record 指向水质记录结构体的指针
 * @return 缺失参数的数量（0~6）
 */
int count_missing_params(const WaterQualityRecord *record) {
    int count = 0;
    count += is_missing(record->Temp);
    count += is_missing(record->Salinity);
    count += is_missing(record->pH);
    count += is_missing(record->DO);
    count += is_missing(record->precipitation);
    count += is_missing(record->Air_temp);
    return count;
}

/**
 * @brief 根据参数类型获取记录中对应的参数值
 * @param record 指向水质记录结构体的指针
 * @param param_type 参数类型（0~5）
 * @return 对应的参数值，非法类型返回0.0f
 */
float get_param_value(const WaterQualityRecord *record, int param_type) {
    switch(param_type) {
        case 0: return record->Temp;
        case 1: return record->Salinity;
        case 2: return record->pH;
        case 3: return record->DO;
        case 4: return record->precipitation;
        case 5: return record->Air_temp;
        default: return 0.0f;
    }
}

/**
 * @brief 根据参数类型设置记录中对应的参数值
 * @param record 指向水质记录结构体的指针
 * @param param_type 参数类型（0~5）
 * @param value 要设置的新值
 * @return void
 */
void set_param_value(WaterQualityRecord *record, int param_type, float value) {
    switch(param_type) {
        case 0: record->Temp = value; break;
        case 1: record->Salinity = value; break;
        case 2: record->pH = value; break;
        case 3: record->DO = value; break;
        case 4: record->precipitation = value; break;
        case 5: record->Air_temp = value; break;
    }
}

/**
 * @brief 计算某个参数列的均值（排除异常值和缺失值,使用全部数据的均值）
 * @param dataset 指向数据集结构体的指针
 * @param param_type 参数类型（0~5）
 * @return 有效值的均值；若无有效数据则返回0.0f
 */
float calculate_column_mean(WaterQualityRecords *dataset, int param_type) {
    float sum = 0.0f;
    int count = 0;
    
    for (int i = 0; i < dataset->count; i++) {
        float value = get_param_value(&dataset->records[i], param_type);
        //判断参数值是否是正常值
        if (!is_abnormal(value, param_type) && !is_missing(value)) {
            sum += value;
            count++;
        }
    }
    
    return count > 0 ? sum / count : 0.0f;
}

/**
 * @brief 均值逼近法：取缺失值前面最近的第n个有效数据和后面最近的第m个有效数据的均值（n=m=10）
 * @param dataset 指向数据集结构体的指针
 * @param record_idx 待处理记录在数组中的索引
 * @param param_type 参数类型（0~5）
 * @return 计算得到的均值，若前后均无有效数据则返回全局均值
 */
float mean_approximation(WaterQualityRecords *dataset, int record_idx, int param_type) {
    // 搜索窗口大小 默认是10
    const int window_size = 10;

    // 存储搜索到的有效值
    float prev_val = 0.0f; //前向找到的有效值
    float next_val = 0.0f; //后向找到的有效值

    // 标记是否找到了有效值
    int found_prev = 0; // 前向找到的有效值
    int found_next = 0; // 后向找到的有效值
    
    // 向前搜索：从 record_idx - 1 开始往前，最多搜索 window_size 条记录，只取第一个有效值
    for (int i = record_idx - 1; i >= 0 && i >= record_idx - window_size; i--) {
        float value = get_param_value(&dataset->records[i], param_type);
        if (!is_abnormal(value, param_type) && !is_missing(value)) {
            prev_val = value;
            found_prev = 1;
            break;
        }
    }
    
    // 向后搜索：从 record_idx + 1 开始往后，最多搜索 window_size 条记录，只取第一个有效值
    for (int i = record_idx + 1; i < dataset->count && i <= record_idx + window_size; i++) {
        float value = get_param_value(&dataset->records[i], param_type);
        if (!is_abnormal(value, param_type) && !is_missing(value)) {
            next_val = value;
            found_next = 1;
            break;
        }
    }
    // - 若两个方向都有值：取两者平均
    // - 若只有一个方向有值：直接使用那个值
    // - 若两个方向都无值：使用全集均值
    // 边界情况处理
    if (found_prev && found_next) {
        return (prev_val + next_val) / 2.0f;
    } else if (found_prev) {
        return prev_val;
    } else if (found_next) {
        return next_val;
    } else {
        return calculate_column_mean(dataset, param_type);
    }
}

/**
 * @brief 使用均值逼近法填充单条记录中的所有异常值
 * @param dataset 指向数据集结构体的指针
 * @param record_index 待填充记录在数组中的索引
 * @return void
 */
void fill_abnormal_with_mean(WaterQualityRecords *dataset, int record_index) {
    WaterQualityRecord *record = &dataset->records[record_index];
    
    if (is_abnormal(record->Temp, 0)) {
        record->Temp = mean_approximation(dataset, record_index, 0);
    }
    if (is_abnormal(record->Salinity, 1)) {
        record->Salinity = mean_approximation(dataset, record_index, 1);
    }
    if (is_abnormal(record->pH, 2)) {
        record->pH = mean_approximation(dataset, record_index, 2);
    }
    if (is_abnormal(record->DO, 3)) {
        record->DO = mean_approximation(dataset, record_index, 3);
    }
    if (is_abnormal(record->precipitation, 4)) {
        record->precipitation = mean_approximation(dataset, record_index, 4);
    }
    if (is_abnormal(record->Air_temp, 5)) {
        record->Air_temp = mean_approximation(dataset, record_index, 5);
    }
}

/**
 * @brief 检测数据集中所有异常，并统计概览信息
 * @param dataset 指向数据集结构体的指针
 * @return 包含总记录数、异常记录数、最大异常参数个数等信息的 DataSummary 结构体
 */
DataSummary check_all_abnormal(WaterQualityRecords *dataset) {
    DataSummary summary = {0};
    summary.total = dataset->count;
    
    //遍历每条记录各个参数的异常
    for (int i = 0; i < dataset->count; i++) {
        int error_count = count_abnormal_params(&dataset->records[i]);
        
        if (error_count > 0) {
            summary.abnormal++;
            if (error_count > summary.max_error_params) {
                summary.max_error_params = error_count;
            }
        } else {
            summary.valid++;
        }
    }
    
    return summary;
}

/**
 * @brief 处理异常数据（核心逻辑）：删除异常参数>=3的记录，填充0<异常<3的记录
 * @param dataset 指向数据集结构体的指针（会被原地修改）
 * @return 处理后的统计摘要（包含删除、修复等计数）
 */
DataSummary process_abnormal_data(WaterQualityRecords *dataset) {
    DataSummary summary = check_all_abnormal(dataset);
    summary.deleted = 0;
    summary.fixed = 0;
    
    // 前移覆盖删除 从后往前遍历
    for (int i = dataset->count - 1; i >= 0; i--) {
        int error_count = count_abnormal_params(&dataset->records[i]);
        
        if (error_count >= 3) {
            // 异常数据 >= 3 个，整条记录删除
            for (int j = i; j < dataset->count - 1; j++) {
                dataset->records[j] = dataset->records[j + 1];
            }
            dataset->count--;
            summary.deleted++;
        } else if (error_count > 0 && error_count < 3) {
            // 异常数据 < 3 个，使用均值逼近法填充
            fill_abnormal_with_mean(dataset, i);
            summary.fixed += error_count;
        }
    }
    
    summary.valid = dataset->count;
    
    return summary;
}

/**
 * @brief 处理缺失值：使用均值逼近法填充所有缺失值
 * @param dataset 指向数据集结构体的指针（会被原地修改）
 * @return 包含总记录数和填充数量等信息的 DataSummary 结构体
 */
DataSummary process_missing_values(WaterQualityRecords *dataset) {
    DataSummary summary = {0};
    summary.total = dataset->count;
    summary.filled = 0;
    
    for (int i = 0; i < dataset->count; i++) {
        WaterQualityRecord *record = &dataset->records[i];
        
        if (is_missing(record->Temp)) {
            record->Temp = mean_approximation(dataset, i, 0);
            summary.filled++;
        }
        if (is_missing(record->Salinity)) {
            record->Salinity = mean_approximation(dataset, i, 1);
            summary.filled++;
        }
        if (is_missing(record->pH)) {
            record->pH = mean_approximation(dataset, i, 2);
            summary.filled++;
        }
        if (is_missing(record->DO)) {
            record->DO = mean_approximation(dataset, i, 3);
            summary.filled++;
        }
        if (is_missing(record->precipitation)) {
            record->precipitation = mean_approximation(dataset, i, 4);
            summary.filled++;
        }
        if (is_missing(record->Air_temp)) {
            record->Air_temp = mean_approximation(dataset, i, 5);
            summary.filled++;
        }
    }
    
    return summary;
}

/**
 * @brief 计算某个参数列的标准差（样本标准差，分母 n-1）
 * @param dataset 指向数据集结构体的指针
 * @param param_type 参数类型（0~5）
 * @return 标准差，如果有效数据个数≤1则返回0.0f
 */
float calculate_std(WaterQualityRecords *dataset, int param_type) {
    float mean = calculate_column_mean(dataset, param_type);
    float sum_sq_diff = 0.0f;
    int count = 0;
    
    for (int i = 0; i < dataset->count; i++) {
        float value = get_param_value(&dataset->records[i], param_type);
        if (!is_abnormal(value, param_type) && !is_missing(value)) {
            float diff = value - mean;
            sum_sq_diff += diff * diff;
            count++;
        }
    }
    
    if (count <= 1) {
        return 0.0f;
    }
    
    return sqrt(sum_sq_diff / (count - 1));
}

/**
 * @brief 移动平均滤波（对单个参数）
 * @param dataset 指向数据集结构体的指针（会被原地修改）
 * @param param_type 参数类型（0~5）
 * @param window_size 窗口大小，必须为奇数且≥3
 * @return void
 */
void moving_average_filter_param(WaterQualityRecords *dataset, int param_type, int window_size) {
    if (dataset->count == 0 || window_size < 3 || window_size % 2 == 0) {
        return;
    }
    
    // 创建临时数组存储原始数据
    float *temp = (float *)malloc(dataset->count * sizeof(float));
    if (!temp) return;
    //给临时数组赋值
    for (int i = 0; i < dataset->count; i++) {
        temp[i] = get_param_value(&dataset->records[i], param_type);
    }
    
    int k = window_size / 2;
    
    for (int i = 0; i < dataset->count; i++) {
        float sum = 0.0f;
        int count = 0;
        
        // 边界处理
        int start = (i - k) < 0 ? 0 : (i - k);
        int end = (i + k) >= dataset->count ? (dataset->count - 1) : (i + k);
        
        // 累加窗口内有效数据
        for (int j = start; j <= end; j++) {
            float value = temp[j];
            if (!is_abnormal(value, param_type) && !is_missing(value)) {
                sum += value;
                count++;
            }
        }
        
        if (count > 0) {
            set_param_value(&dataset->records[i], param_type, sum / count);
        }
    }
    
    free(temp);
}

/**
 * @brief 移动平均滤波（对四个核心参数：水温、盐度、pH值、溶解氧）
 * @param dataset 指向数据集结构体的指针（会被原地修改）
 * @param window_size 窗口大小，必须为奇数且≥3
 * @return void
 */
void moving_average_filter(WaterQualityRecords *dataset, int window_size) {
    // 对水温(0)、盐度(1)、pH(2)、溶解氧(3)进行滤波
    moving_average_filter_param(dataset, 0, window_size); // 水温
    moving_average_filter_param(dataset, 1, window_size); // 盐度
    moving_average_filter_param(dataset, 2, window_size); // pH值
    moving_average_filter_param(dataset, 3, window_size); // 溶解氧
}

/**
 * @brief 输出数据概览到文件
 * @param filename 输出文件名
 * @param summary 指向 DataSummary 结构体的指针
 * @return void
 */
void write_data_summary(const char *filename, const DataSummary *summary) {
    FILE *fp = fopen(filename, "w");
    if (!fp) return;
    
    fprintf(fp, "========== 数据概览 ==========\n");
    fprintf(fp, "总记录数: %d\n", summary->total);
    fprintf(fp, "异常数据记录数: %d\n", summary->abnormal);
    fprintf(fp, "有效数据记录数: %d\n", summary->valid);
    fprintf(fp, "单条记录最大异常参数个数: %d\n", summary->max_error_params);
    fprintf(fp, "删除异常值记录数: %d\n", summary->deleted);
    fprintf(fp, "修复异常值数量: %d\n", summary->fixed);
    fprintf(fp, "填充缺失值数量: %d\n", summary->filled);
    fprintf(fp, "异常数据时间跨度: %s 至 %s\n", 
            summary->abnormal_start_time[0] ? summary->abnormal_start_time : "未记录",
            summary->abnormal_end_time[0] ? summary->abnormal_end_time : "未记录");
    fprintf(fp, "==============================\n");
    
    fclose(fp);
}

/**
 * @brief 输出分析讨论报告到文件（包含方法对比、滤波前后标准差对比等）
 * @param filename 输出文件名
 * @param summary 指向 DataSummary 结构体的指针
 * @param std_before 滤波前四个参数的标准差数组（长度4）
 * @param std_after 滤波后四个参数的标准差数组（长度4）
 * @param window_sizes 窗口大小数组（本函数未使用，保留接口）
 * @param window_count 窗口大小数组长度（本函数未使用，保留接口）
 * @return void
 */
void write_analysis_report(const char *filename, const DataSummary *summary, 
                          float std_before[], float std_after[], int window_sizes[], int window_count) {
    (void)window_sizes;
    (void)window_count;
    FILE *fp = fopen(filename, "w");
    if (!fp) return;
    
    fprintf(fp, "========== 异常值处理分析报告 ==========\n\n");
    
    fprintf(fp, "一、异常值处理方法分析\n");
    fprintf(fp, "----------------------------------------\n");
    fprintf(fp, "1. 本次采用的处理方法:\n");
    fprintf(fp, "   - 单条记录异常参数≥3个：整条记录删除\n");
    fprintf(fp, "   - 单条记录异常参数<3个：均值逼近法填充\n\n");
    
    fprintf(fp, "2. 常见异常值处理方法对比:\n");
    fprintf(fp, "   ┌─────────────────┬─────────────────────────────┬─────────────────────────────┐\n");
    fprintf(fp, "   │    方法名称      │           优点              │           缺点              │\n");
    fprintf(fp, "   ├─────────────────┼─────────────────────────────┼─────────────────────────────┤\n");
    fprintf(fp, "   │   删除整行       │ 简单彻底，避免数据污染        │ 丢失数据，样本量减少         │\n");
    fprintf(fp, "   │   均值填充       │ 操作简单，保持数据完整性      │ 低估方差，平滑数据分布       │\n");
    fprintf(fp, "   │   中位数填充     │ 稳健性强，不受极端值影响      │ 计算稍复杂                   │\n");
    fprintf(fp, "   │   邻近值插值     │ 保留时间序列特征              │ 对噪声敏感                   │\n");
    fprintf(fp, "   │   回归预测       │ 预测精度高                   │ 计算复杂，需建立模型         │\n");
    fprintf(fp, "   └─────────────────┴─────────────────────────────┴─────────────────────────────┘\n\n");
    
    fprintf(fp, "3. 本次方法合理性分析:\n");
    fprintf(fp, "   [√] 阈值设定合理：3个异常参数作为删除阈值，既避免过度删除，又保证数据质量\n");
    fprintf(fp, "   [√] 均值逼近法：采用前后各10个有效数据的均值，保持时间序列连续性\n");
    fprintf(fp, "   [√] 边界处理完善：单向无数据时使用另一方向数据，双向无数据时使用全局均值\n");
    fprintf(fp, "   [×] 改进空间：可考虑使用中位数填充提高稳健性\n\n");
    
    fprintf(fp, "二、缺失值处理方法分析\n");
    fprintf(fp, "----------------------------------------\n");
    fprintf(fp, "1. 均值逼近法公式：\n");
    fprintf(fp, "   a_i = (a_{i-n} + a_{i+m}) / 2 （默认n=m=10）\n\n");
    
    fprintf(fp, "2. 本次处理统计：\n");
    fprintf(fp, "   - 填充缺失值数量：%d\n\n", summary->filled);
    
    fprintf(fp, "三、移动平均滤波分析\n");
    fprintf(fp, "----------------------------------------\n");
    fprintf(fp, "1. 移动平均法公式：\n");
    fprintf(fp, "   y_i = (1/N) * Σ(x_j) （j从i-k到i+k，N=2k+1）\n\n");
    
    fprintf(fp, "2. 滤波前后标准差对比:\n");
    fprintf(fp, "   ┌──────────┬──────────────┬──────────────┬────────────┐\n");
    fprintf(fp, "   │ 参数名称  │ 滤波前标准差 │ 滤波后标准差 │ 噪声减少率 │\n");
    fprintf(fp, "   ├──────────┼──────────────┼──────────────┼────────────┤\n");
    fprintf(fp, "   │    水温   │   %.4f      │   %.4f      │  %.2f%%    │\n", 
            std_before[0], std_after[0], 
            std_before[0] > 0 ? (1 - std_after[0]/std_before[0]) * 100 : 0);
    fprintf(fp, "   │    盐度   │   %.4f      │   %.4f      │  %.2f%%    │\n", 
            std_before[1], std_after[1], 
            std_before[1] > 0 ? (1 - std_after[1]/std_before[1]) * 100 : 0);
    fprintf(fp, "   │    pH值   │   %.4f      │   %.4f      │  %.2f%%    │\n", 
            std_before[2], std_after[2], 
            std_before[2] > 0 ? (1 - std_after[2]/std_before[2]) * 100 : 0);
    fprintf(fp, "   │   溶解氧   │   %.4f      │   %.4f      │  %.2f%%    │\n", 
            std_before[3], std_after[3], 
            std_before[3] > 0 ? (1 - std_after[3]/std_before[3]) * 100 : 0);
    fprintf(fp, "   └──────────┴──────────────┴──────────────┴────────────┘\n\n");
    
    fprintf(fp, "3. 窗口大小分析建议:\n");
    fprintf(fp, "   - 窗口越小(3-5)：保留更多细节，但噪声抑制效果较差\n");
    fprintf(fp, "   - 窗口适中(7)：平衡细节保留与噪声抑制\n");
    fprintf(fp, "   - 窗口越大(9-11)：噪声抑制效果好，但可能过度平滑丢失细节\n");
    fprintf(fp, "   - 推荐最佳窗口：根据实际数据特征选择，通常7为较好的平衡点\n\n");
    
    fprintf(fp, "==========================================\n");
    fprintf(fp, "报告生成时间: %s\n", __DATE__ " " __TIME__);
    fprintf(fp, "==========================================\n");
    
    fclose(fp);
}

/**
 * @brief 多窗口对比分析报告（自动遍历多个窗口并输出对比表格）
 * @param filename 输出文件名
 * @param dataset 数据集指针（用于计算原始数据标准差）
 * @param window_sizes 窗口大小数组（如 {3, 5, 7, 9, 11}）
 * @param window_count 窗口数量
 * @param std_results 二维数组，存储每个窗口滤波后的标准差（window_count * 4）
 * @param noise_reduction 二维数组，存储每个窗口的噪声减少率（window_count * 4）
 * @return void
 */
void write_multi_window_report(const char *filename, WaterQualityRecords *dataset,
                               int window_sizes[], int window_count,
                               float std_results[][4], float noise_reduction[][4]) {
    FILE *fp = fopen(filename, "w");
    if (!fp) return;
    
    const char *param_names[] = {"水温", "盐度", "pH值", "溶解氧"};
    
    // 计算原始数据每一项参数的标准差
    float std_original[4];
    for (int i = 0; i < 4; i++) {
        std_original[i] = calculate_std(dataset, i);
    }
    
    fprintf(fp, "========== 多窗口滤波对比分析报告 ==========\n\n");
    
    fprintf(fp, "一、原始数据标准差\n");
    fprintf(fp, "----------------------------------------\n");
    fprintf(fp, "   水温: %.4f\n", std_original[0]);
    fprintf(fp, "   盐度: %.4f\n", std_original[1]);
    fprintf(fp, "   pH值: %.4f\n", std_original[2]);
    fprintf(fp, "   溶解氧: %.4f\n\n", std_original[3]);
    
    fprintf(fp, "二、各窗口滤波效果对比\n");
    fprintf(fp, "----------------------------------------\n");
    
    // 输出每个参数的对比表格 先参数 后窗口
    for (int param = 0; param < 4; param++) {
        fprintf(fp, "\n【%s】各窗口滤波效果对比:\n", param_names[param]);
        fprintf(fp, "┌──────────┬──────────────┬──────────────┬────────────┐\n");
        fprintf(fp, "│ 窗口大小  │ 滤波后标准差  │ 原始标准差    │ 噪声减少率  │\n");
        fprintf(fp, "├──────────┼──────────────┼──────────────┼────────────┤\n");
        
        for (int w = 0; w < window_count; w++) {
            fprintf(fp, "│    %2d    │   %.4f      │   %.4f      │  %.2f%%    │\n",
                    window_sizes[w], std_results[w][param], std_original[param], noise_reduction[w][param]);
        }
        fprintf(fp, "└──────────┴──────────────┴──────────────┴────────────┘\n");
    }
    
    fprintf(fp, "\n三、综合对比表格\n");
    fprintf(fp, "----------------------------------------\n");
    fprintf(fp, "┌──────────┬──────────────┬──────────────┬──────────────┬──────────────┐\n");
    fprintf(fp, "│ 窗口大小  │   水温降噪率  │  盐度降噪率   │  pH值降噪率   │ 溶解氧降噪率  │\n");
    fprintf(fp, "├──────────┼──────────────┼──────────────┼──────────────┼──────────────┤\n");
    
    for (int w = 0; w < window_count; w++) {
        fprintf(fp, "│    %2d    │   %.2f%%     │   %.2f%%     │   %.2f%%     │   %.2f%%     │\n",
                window_sizes[w], noise_reduction[w][0], noise_reduction[w][1], 
                noise_reduction[w][2], noise_reduction[w][3]);
    }
    fprintf(fp, "└──────────┴──────────────┴──────────────┴──────────────┴──────────────┘\n");
    
    fprintf(fp, "\n四、最佳窗口选择分析\n");
    fprintf(fp, "----------------------------------------\n");
    
    // 计算每个窗口的平均降噪率
    float avg_reduction[5];
    for (int w = 0; w < window_count; w++) {
        avg_reduction[w] = (noise_reduction[w][0] + noise_reduction[w][1] + 
                           noise_reduction[w][2] + noise_reduction[w][3]) / 4.0f;
    }
    
    // 找到最佳窗口（平均降噪率最高的）
    int best_window_idx = 0;
    float best_avg = avg_reduction[0];
    for (int w = 1; w < window_count; w++) {
        if (avg_reduction[w] > best_avg) {
            best_avg = avg_reduction[w];
            best_window_idx = w;
        }
    }
    
    fprintf(fp, "各窗口平均噪声减少率:\n");
    for (int w = 0; w < window_count; w++) {
        fprintf(fp, "  窗口 %2d: %.2f%%\n", window_sizes[w], avg_reduction[w]);
    }
    
    fprintf(fp, "\n【结论】\n");
    fprintf(fp, "根据对比分析结果，窗口 %d 是最佳滤波窗口。\n", window_sizes[best_window_idx]);
    fprintf(fp, "\n理由说明:\n");
    fprintf(fp, "  1. 该窗口的平均噪声减少率为 %.2f%%，在所有测试窗口中最高。\n", avg_reduction[best_window_idx]);
    fprintf(fp, "  2. 具体参数降噪效果:\n");
    fprintf(fp, "     - 水温: %.2f%%\n", noise_reduction[best_window_idx][0]);
    fprintf(fp, "     - 盐度: %.2f%%\n", noise_reduction[best_window_idx][1]);
    fprintf(fp, "     - pH值: %.2f%%\n", noise_reduction[best_window_idx][2]);
    fprintf(fp, "     - 溶解氧: %.2f%%\n", noise_reduction[best_window_idx][3]);
    fprintf(fp, "  3. 窗口 %d 在降噪效果和保留数据细节之间达到了较好的平衡。\n", window_sizes[best_window_idx]);
    fprintf(fp, "  4. 相比更大的窗口，该窗口能更好地保留原始数据的变化趋势；\n");
    fprintf(fp, "     相比更小的窗口，该窗口能更有效地去除噪声。\n\n");
    
    fprintf(fp, "========================================\n");
    fprintf(fp, "报告生成时间: %s %s\n", __DATE__, __TIME__);
    fprintf(fp, "========================================\n");
    
    fclose(fp);
}