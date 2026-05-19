#include "TxtFileUtil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 解析一行 CSV（格式：datetime,temp,salinity,ph,do,precipitation,air_temp）
static int parse_csv_line(const char *line, WaterQualityRecord *record) {
    if (!line || !record) return -1;
    memset(record, 0, sizeof(WaterQualityRecord));

    // 复制行并处理 NaN / nan (保持原有逻辑，防止后续解析出错)
    char tempLine[1024];
    strncpy(tempLine, line, sizeof(tempLine) - 1);
    tempLine[sizeof(tempLine) - 1] = '\0';

    char *p = tempLine;
    while (*p) {
        if (strncmp(p, "NaN", 3) == 0 || strncmp(p, "nan", 3) == 0) {
            *p = 'N'; *(p+1) = 'A'; *(p+2) = 'N';
            p += 3;
        } else {
            p++;
        }
    }

    float vals[6];
    // 修改 sscanf 格式：只读取6个浮点数，不再读取开头的字符串
    int matched = sscanf(tempLine, "%f,%f,%f,%f,%f,%f",
                         &vals[0], &vals[1], &vals[2],
                         &vals[3], &vals[4], &vals[5]);
    
    // 如果成功匹配到6个浮点数，则解析成功
    if (matched != 6) return -1;

    // 由于CSV中没有时间，这里给一个默认空字符串或者你可以生成一个默认时间
    // 如果后续逻辑依赖 datetime 不为空，建议填入 "1970-01-01 00:00:00"
    strcpy(record->datetime, ""); 

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

    fprintf(fp, "datetime,Temp(degC),Salinity(PSU),pH,DO(mg/l),precipitation(mm),Air_temp(degC)\n");
    for (int i = 0; i < records->count; i++) {
        WaterQualityRecord rec = records->records[i];
        fprintf(fp, "%s,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n",
                rec.datetime, rec.Temp, rec.Salinity, rec.pH, rec.DO,
                rec.precipitation, rec.Air_temp);
    }
    fclose(fp);
    return 0;
}