//统计分析,分段统计,相关性分析
#include "StatisticUtil.h"

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

// /辅助函数：提取时间字符串中的小时
static int getHourFromDatetime(const char *datetime){
    int year, month, day, hour, min, sec;
    if (sscanf(datetime,"%d-%d-%d %d:%d:%d",&year,&month,&day,&hour,&min,&sec) == 6)
        return hour;
    return -1;
}

// /辅助函数：提取日期字符串
static void getDateFromDatetime(const char *datetime, char *dateBuf){
    strncpy(dateBuf,datetime,10);
    dateBuf[10] = '\0';
}

//计算指定参数的基本统计量
void CalcBasicStats(const WaterQualityRecords *records, ParamType param,
                    float *mean, float *max, float *min, float *variance, float *stddev) {
    if (!records || records->count == 0) {
        if (mean) *mean = 0.0f;
        if (max) *max = 0.0f;
        if (min) *min = 0.0f;
        if (variance) *variance = 0.0f;
        if (stddev) *stddev = 0.0f;
        return;
    }

    double sum = 0.0;
    double sumSq = 0.0;
    double maxVal = -1e9;
    double minVal = 1e9;
    int count = records->count;

    for (int i = 0; i < count; i++) {
        double val = 0.0;
        switch (param) {
            case PARAM_TEMP:      val = records->records[i].Temp; break;
            case PARAM_SALINITY:  val = records->records[i].Salinity; break;
            case PARAM_PH:        val = records->records[i].pH; break;
            case PARAM_DO:        val = records->records[i].DO; break;
            case PARAM_PRECIP:    val = records->records[i].precipitation; break;
            case PARAM_AIRTEMP:   val = records->records[i].Air_temp; break;
            default: continue;
        }
        sum += val;
        sumSq += val * val;
        if (val > maxVal) maxVal = val;
        if (val < minVal) minVal = val;
    }

    // 计算平均值
    double meanVal = sum / count;
    
    // 计算方差 (总体方差公式: E(X^2) - (E(X))^2 )
    double varianceVal = (sumSq / count) - (meanVal * meanVal);
    
    // 防止浮点误差导致负数
    if (varianceVal < 0.0) varianceVal = 0.0;

    double stddevVal = sqrt(varianceVal);

    // 将结果写回指针指向的内存
    if (mean) *mean = (float)meanVal;
    if (max) *max = (float)maxVal;
    if (min) *min = (float)minVal;
    if (variance) *variance = (float)varianceVal;
    if (stddev) *stddev = (float)stddevVal;
}

//生成所有参数的基本统计量报告
void GenerateBasicStatsReport(const WaterQualityRecords *records) {
    if (!records || records->count == 0) {
        printf("无数据，无法生成统计报告。\n");
        return;
    }
    FILE *fp = fopen("../../data/stat_report.csv", "w");
    if (!fp) {
        printf("无法创建 ../../data/stat_report.csv\n");
        return;
    }
    fprintf(fp, "========== 水质参数基本统计量 ==========\n");
    fprintf(fp, "记录总数: %d\n\n", records->count);

    const char *names[] = {"水温(℃)", "盐度(PSU)", "pH", "溶解氧(mg/l)", "降水量(mm)", "气温(℃)"};
    ParamType params[] = {PARAM_TEMP, PARAM_SALINITY, PARAM_PH, PARAM_DO, PARAM_PRECIP, PARAM_AIRTEMP};

    for (int i = 0; i < 6; i++) {
        float mean, max, min,variance, stddev;
        CalcBasicStats(records, params[i], &mean, &max, &min,&variance, &stddev);
        fprintf(fp, "%s:\n", names[i]);
        fprintf(fp, "  均值: %.4f\n", mean);
        fprintf(fp, "  最大值: %.4f\n", max);
        fprintf(fp, "  最小值: %.4f\n", min);
        fprintf(fp, "  方差: %.4f\n", variance);
        fprintf(fp, "  标准差: %.4f\n\n", stddev);
    }
    fclose(fp);
    printf("基本统计量已写入 stat_report.txt\n");
}

void DawnHypoxiaWarning(const WaterQualityRecords *records){
    if (!records || records->count == 0)
        return;
    FILE *fp = fopen("../../data/warning_dawn.csv", "w");
    if (!fp) return;
    fprintf(fp, "========== 日间水下缺氧提醒 ==========\n");
    fprintf(fp,"日期,凌晨DO均值(mg/L),预警等级,处理建议\n");

    char currentDate[11] = "";
    double sumDO = 0.0;
    int countDO = 0;

    for (int i = 0; i < records->count; i++) {
        char date[11];
        getDateFromDatetime(records->records[i].DailyStats, date);
        int hour = getHourFromDatetime(records->records[i].DailyStats);
        if (hour >= 3 && hour <= 5) {
            // 如果日期变了，且之前有数据，先输出前一天的统计
            if (currentDate[0] != '\0' && strcmp(date, currentDate) != 0) {
                double avgDO = sumDO / countDO;
                if (avgDO < 3.0) 
                    fprintf(fp, "%s,%.4f,严重缺氧警告,需立即投放颗粒氧并减少投喂！\n", currentDate, avgDO);
                else if (avgDO < 4.0) 
                    fprintf(fp, "%s,%.4f,亚缺氧预警,建议开启底部增氧机！\n", currentDate, avgDO);
                
                // 重置累加器
                sumDO = 0.0;
                countDO = 0;
            }
            
            // 累加当前数据
            sumDO += records->records[i].DO;
            countDO++;
            // 更新当前日期
            strcpy(currentDate, date);
        } 
        // 非凌晨时段不做处理

    }
    //最后一天
    if (countDO > 0)
    {
        double avgDO = sumDO / countDO;
        if (avgDO <3.0) fprintf(fp, "%s,%.4f,严重缺氧警告,需立即投放颗粒氧并减少投喂！\n", currentDate, avgDO);
        else if (avgDO < 4.0) fprintf(fp, "%s,%.4f,亚缺氧预警,建议开启底部增氧机！\n", currentDate, avgDO);
    }
    fclose(fp);
    printf("凌晨水下缺氧提醒已写入 warning_dawn.csv\n");
}

//盐度突变预警
void SalinityShockWarning(const WaterQualityRecords *records){
    if (!records || records->count <2 )    return;
    FILE *fp = fopen("../../data/warning_salinity.csv", "w");
    if (!fp) return;

    fprintf(fp, "========== 盐度突变提醒 ==========\n");
    fprintf(fp, "时间,突变类型,盐度(PSU),处理建议\n");

    //每小时变化(12点)
    for (int i = 12; i < records->count; i++)
    {
        double delta = fabs(records->records[i].Salinity - records->records[i-12].Salinity);
        if (delta > 2.0) fprintf(fp, "%s,小时突变,%.4f,关闭进水口，泼洒高稳VC！\n", records->records[i].DailyStats, delta);
    }
    
    //24小时累计降幅(288点)
    for (int i = 288; i < records->count; i++)
    {
        double drop = records->records[i-288].Salinity - records->records[i].Salinity;
        if (drop > 5.0)
        {
            fprintf(fp, "%s,24小时累计降幅,%.4f,关闭进水口，泼洒高稳VC！\n", records->records[i].DailyStats, drop);   
        }
    }
    fclose(fp);
    printf("盐度突变提醒已写入 warning_salinity.csv\n");
}

//皮尔逊相关系数
static double pearsonCount(const double *x, const double *y, int n){
    double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0,sum_x2 = 0.0, sum_y2 = 0.0;
    for (int i = 0; i < n; i++)
    {
        sum_x += x[i];
        sum_y += y[i];
        sum_xy += x[i] * y[i];
        sum_x2 += x[i] * x[i];
        sum_y2 += y[i] * y[i];
    }
    double denom = sqrt((sum_x2 - sum_x * sum_x / n) * (sum_y2 - sum_y * sum_y / n));
    if (denom == 0.0) return 0.0;
    return (sum_xy - sum_x * sum_y / n) / denom;
}

//相关性分析(保存为CSV矩阵)
void CorrelationAnalysis(const WaterQualityRecords *records)
{
    if (!records || records->count == 0)
    {
        printf("数据为空，无法进行相关性分析！\n");
        return;
    }
    int n = records->count;
    double *temps = (double *)malloc(n * sizeof(double));
    double *salinities = (double *)malloc(n * sizeof(double));
    double *phs = (double *)malloc(n * sizeof(double));
    double *dos = (double *)malloc(n * sizeof(double));
    double *precips = (double *)malloc(n * sizeof(double));
    double *airTemps = (double *)malloc(n * sizeof(double));
    if (!temps || !salinities || !phs || !dos || precips || !airTemps)
    {
        printf("内存分配失败！\n");
        goto cleanup;
    }
    
    for (int i = 0; i < n; i++)
    {
        temps[i] = records->records[i].Temp;
        salinities[i] = records->records[i].Salinity;
        phs[i] = records->records[i].pH;
        dos[i] = records->records[i].DO;
        precips[i] = records->records[i].precipitation;
        airTemps[i] = records->records[i].Air_temp;
    }
    double *param[6] = {temps, salinities, phs, dos, precips, airTemps};
    const char *name[6] = {"温度", "盐度", "pH", "溶解氧", "降水量", "空气温度"};
    FILE *fp = fopen("../../data/correlation_matrix.csv", "w");
    if (!fp)
    {
        printf("无法创建correlation_matrix.csv\n");
        goto cleanup;
    }

    //写入表头
    fprintf(fp, "参数");
    for (int j = 0; j < 6; j++) fprintf(fp, ",%s", name[j]);
    fprintf(fp, "\n");
    
    for (int i = 0; i < 6; i++)
    {
        fprintf(fp, "%s", name[i]);
        for (int j = 0; j < 6; j++)
        {
            double r = pearsonCount(param[i], param[j], n);
            fprintf(fp, ",%.4f", r);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    cleanup:
    free(temps);
    free(salinities);
    free(phs);
    free(dos);
    free(precips);
    free(airTemps);
    printf("相关性矩阵已保存为 correlation_matrix.csv\n");
}
    
//一键执行所以统计分析
void RunAllStatistics(const WaterQualityRecords *records)
{
    GenerateBasicStatsReport(records);
    DawnHypoxiaWarning(records);
    SalinityShockWarning(records);
    CorrelationAnalysis(records);
    printf("所有统计分析已完成！结果均已保存至对应的CSV文件!\n");
}





