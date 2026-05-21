<<<<<<< Updated upstream
//数据存储和读取
=======
#include "TxtFileUtil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 解析一行 CSV（格式：datetime,temp,salinity,ph,do,precipitation,air_temp)
static int parse_csv_line(const char *line, WaterQualityRecord *record) {
    if (!line || !record) return -1;
    memset(record, 0, sizeof(WaterQualityRecord));

    char tempLine[1024];
    strncpy(tempLine, line, sizeof(tempLine) - 1);
    tempLine[sizeof(tempLine) - 1] = '\0';

    // 1. 提取 datetime
    // 找到第一个逗号的位置
    char* firstComma = strchr(tempLine, ',');
    if (!firstComma) return -1;
    
    // 复制日期时间字符串
    int dtLen = firstComma - tempLine;
    if (dtLen >= 20) dtLen = 19; // 格式为YYYY-mm-dd HH:mm:ss,大概占20个字符, 这里是防止溢出
    strncpy(record->DailyStats, tempLine, dtLen); 
    record->DailyStats[dtLen] = '\0';

    // 2. 解析剩余的浮点数
    // 指针移动到第一个逗号之后
    char* dataPart = firstComma + 1;
    
    // 处理 NaN
    char* p = dataPart;
    while (*p) {
        if (strncmp(p, "NaN", 3) == 0 || strncmp(p, "nan", 3) == 0) {
            // 简单的替换策略，或者可以在 sscanf 前统一处理
            // 这里为了简单，sscanf 遇到 NaN 可能会失败，建议先替换为 0 或 -1 或者保留 NaN 让 sscanf 处理
             *p = '0'; *(p+1) = '.'; *(p+2) = '0'; // 简单替换为 0.0，避免解析失败
             p += 3;
        } else {
            p++;
        }
    }

    float vals[6];
    // 从 dataPart 开始解析 6 个浮点数
    int matched = sscanf(dataPart, "%f,%f,%f,%f,%f,%f",
                         &vals[0], &vals[1], &vals[2],
                         &vals[3], &vals[4], &vals[5]);
    
    if (matched != 6) {
        printf("解析失败: %s\n", line);
        return -1;
    }

    record->Temp        = vals[0];
    record->Salinity    = vals[1];
    record->pH          = vals[2];
    record->DO          = vals[3];
    record->precipitation = vals[4];
    record->Air_temp    = vals[5];
    
    return 0;
}

int TxtUtil_LoadFromFile(const char *filename, WaterQualityRecords *records) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("无法打开文件 %s\n", filename);
        return -1;
    }

    char line[2048];
    // 跳过表头
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return -1;
    }

    // 预扩容至 10000 条（提高效率）
    if (records->capacity < 1000) {
        int newCap = 10000;
        WaterQualityRecord *newRec = (WaterQualityRecord*)realloc(records->records, newCap * sizeof(WaterQualityRecord));
        if (newRec) {
            records->records = newRec;
            records->capacity = newCap;
        }
    }

    int totalLines = 0;
    int validCount = 0;
    while (fgets(line, sizeof(line), fp)) {
        totalLines++;
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0) continue;

        WaterQualityRecord rec;
        if (parse_csv_line(line, &rec) == 0) {
            WQ_AddRecord(records, &rec);
            validCount++;
        } else {
            printf("警告：解析行失败：%s\n", line);
        }
    }
    fclose(fp);

    // 生成数据概览文件
    FILE *overview = fopen("data_overview.txt", "w");
    if (overview) {
        fprintf(overview, "总记录数（原始）: %d\n", totalLines);
        fprintf(overview, "有效记录数（加载后）: %d\n", validCount);
        fprintf(overview, "注：缺失值和异常值未处理，请运行预处理模块。\n");
        fclose(overview);
    }
    printf("加载完成，有效记录数：%d\n", validCount);
    return 0;
}

int TxtUtil_SaveToFile(const char *filename, const WaterQualityRecords *records) {
    if (!records || !records->records) return -1;
    FILE *fp = fopen(filename, "w");
    if (!fp) return -1;

    fprintf(fp, "DailyStats,Temp(degC),Salinity(PSU),pH,DO(mg/l),precipitation(mm),Air_temp(degC)\n");
    for (int i = 0; i < records->count; i++) {
        WaterQualityRecord rec = records->records[i];
        fprintf(fp, "%s,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n",
                rec.DailyStats, rec.Temp, rec.Salinity, rec.pH, rec.DO,
                rec.precipitation, rec.Air_temp);
    }
    fclose(fp);
    return 0;
}
>>>>>>> Stashed changes
