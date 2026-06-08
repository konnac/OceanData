// Main.c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h> // 用于检查文件是否存在

// 引入项目头文件
#include "../utils/StatisticUtil.h" 
#include "../utils/TxtFileUtil.h"
#include "../object/WaterQuality.h"

/**
 * @brief 检查文件是否存在
 */
int file_exists(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

/**
 * @brief 预处理函数：为原始数据添加时间戳
 * 
 * @param inputFile 原始无时间戳的CSV文件路径
 * @param outputFile 生成的带时间戳的CSV文件路径
 * @param startYear 起始年份
 * @param startMonth 起始月份 (1-12)
 * @param startDay 起始日期 (1-31)
 */
void GenerateTimeStampedCSV(const char* inputFile, const char* outputFile, 
                            int startYear, int startMonth, int startDay) {
    FILE* fin = fopen(inputFile, "r");
    FILE* fout = fopen(outputFile, "w");
    
    if (!fin) {
        printf("错误：无法打开源文件 %s\n", inputFile);
        return;
    }
    if (!fout) {
        printf("错误：无法创建目标文件 %s\n", outputFile);
        fclose(fin);
        return;
    }

    // 1. 写入新表头 (匹配 WaterQualityRecord 解析格式)
    fprintf(fout, "DailyStats,Temp(degC),Salinity(PSU),pH,DO(mg/l),precipitation(mm),Air_temp(degC)\n");

    // 2. 跳过旧表头
    char header[256];
    if (fgets(header, sizeof(header), fin) == NULL) {
        printf("警告：源文件为空或格式错误\n");
        fclose(fin); fclose(fout);
        return;
    }

    // 3. 初始化时间
    struct tm startTime = {0};
    startTime.tm_year = startYear - 1900; // tm_year 是从1900年开始的偏移量
    startTime.tm_mon = startMonth - 1;    // tm_mon 是 0-11
    startTime.tm_mday = startDay;
    startTime.tm_hour = 0;
    startTime.tm_min = 0;
    startTime.tm_sec = 0;
    
    time_t currentTime = mktime(&startTime);
    if (currentTime == -1) {
        printf("错误：时间初始化失败\n");
        fclose(fin); fclose(fout);
        return;
    }

    char line[512];
    int count = 0;
    while (fgets(line, sizeof(line), fin)) {
        // 去除换行符
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0) continue;

        // 格式化当前时间: YYYY-MM-DD HH:MM:SS
        struct tm* ptm = localtime(&currentTime);
        char timeStr[20];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", ptm);

        // 写入：时间 + 逗号 + 原始数据行
 fprintf(fout, "%s,%s\n", timeStr, line);

        // 时间增加5分钟 (300秒)
        currentTime += 300;
        count++;
        
        // 可选：每处理1000条打印一次进度
        if (count % 1000 == 0) {
            printf("已处理 %d 条数据...\n", count);
        }
    }

    fclose(fin);
    fclose(fout);
    printf("预处理完成！共生成 %d 条带时间戳的数据。\n", count);
    printf("保存至: %s\n", outputFile);
}

int main() {
    printf("========================================\n");
    printf("   海洋水质数据分析系统 - 测试启动\n");
    printf("========================================\n");

    // 定义文件路径
    // 注意：这些路径是相对于程序运行目录的。
    // 假设在 E:\OceanData\menu 下运行 exe，则 ../data 指向 E:\OceanData\data
    const char* rawFile = "../data/WaterQuilityRecords.csv";       // 原始文件
    const char* readyFile = "../data/WaterQuality_Ready.csv";      // 处理后文件

    // 1. 数据预处理阶段
    printf("\n[步骤 1] 检查并预处理数据...\n");
    
    // 如果已经存在处理好的文件，可以直接使用，否则生成
    if (!file_exists(readyFile)) {
        printf("未找到预处理文件 %s，正在从 %s 生成...\n", readyFile, rawFile);
        if (file_exists(rawFile)) {
            // 假设数据从 2020年1月1日 开始 (根据CSV内容调整)
            GenerateTimeStampedCSV(rawFile, readyFile, 2020, 1, 1);
        } else {
            printf("错误：原始数据文件 %s 也不存在！请检查路径。\n", rawFile);
            return -1;
        }
    } else {
        printf("发现已存在的预处理文件: %s，跳过生成步骤。\n", readyFile);
    }

    // 2. 初始化数据结构
    printf("\n[步骤 2] 初始化内存并加载数据...\n");
    WaterQualityRecords records;
    WQ_Init(&records, 1000); // 初始容量1000

    // 3. 加载带时间的数据
    int loadResult = TxtUtil_LoadFromFile(readyFile, &records);
    
    if (loadResult != 0 || records.count == 0) {
        printf("错误：数据加载失败或数据为空！\n");
        printf("提示：请检查 %s 文件格式是否正确，以及是否包含表头。\n", readyFile);
        WQ_Destroy(&records);
        return -1;
    }
    printf("成功加载 %d 条记录。\n", records.count);

    // 4. 执行统计分析
    printf("\n[步骤 3] 执行统计分析模块...\n");
    
    // 4.1 基本统计量
    printf("  -> 正在计算基本统计量 (均值/最大/最小/方差)...\n");
    GenerateBasicStatsReport(&records);
    
    // 4.2 凌晨缺氧预警
    printf("  -> 正在分析凌晨缺氧风险...\n");
    DawnHypoxiaWarning(&records);
    
    // 4.3 相关性分析
    printf("  -> 正在计算皮尔逊相关系数矩阵...\n");
    CorrelationAnalysis(&records);

    // 4.4 一键执行所有统计 (如果上面单独调用是为了看细节，这里可以注释掉，或者只调用这个)
    // RunAllStatistics(&records); 

    // 5. 清理资源
    printf("\n[步骤 4] 清理内存...\n");
    WQ_Destroy(&records);

    printf("\n========================================\n");
    printf("   程序执行完毕！\n");
    printf("   请查看以下输出文件获取详细结果：\n");
    printf("   1. data_overview.txt (数据概览)\n");
    printf("   2. ../../data/stat_report.csv (基本统计报告)\n");
    printf("   3. ../../data/warning_dawn.csv (缺氧预警)\n");
    printf("   4. ../../data/correlation_matrix.csv (相关性矩阵)\n");
    printf("========================================\n");

    return 0;
}