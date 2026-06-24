#include "TxtFileUtil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_csv_line(const char *line, WaterQualityRecord *record) {
    if (!line || !record) return -1;
    memset(record, 0, sizeof(WaterQualityRecord));

    char tempLine[1024];
    strncpy(tempLine, line, sizeof(tempLine) - 1);
    tempLine[sizeof(tempLine) - 1] = '\0';

    char *firstComma = strchr(tempLine, ',');
    if (!firstComma) return -1;

    int dtLen = (int)(firstComma - tempLine);
    if (dtLen >= (int)sizeof(record->DailyStats)) {
        dtLen = (int)sizeof(record->DailyStats) - 1;
    }
    strncpy(record->DailyStats, tempLine, dtLen);
    record->DailyStats[dtLen] = '\0';

    char *dataPart = firstComma + 1;
    char *p = dataPart;
    while (*p) {
        if (strncmp(p, "NaN", 3) == 0 || strncmp(p, "nan", 3) == 0) {
            *p = '0';
            *(p + 1) = '.';
            *(p + 2) = '0';
            p += 3;
        } else {
            p++;
        }
    }

    float vals[6];
    int matched = sscanf(dataPart, "%f,%f,%f,%f,%f,%f",
                         &vals[0], &vals[1], &vals[2],
                         &vals[3], &vals[4], &vals[5]);
    if (matched != 6) {
        printf("Failed to parse line: %s\n", line);
        return -1;
    }

    record->Temp = vals[0];
    record->Salinity = vals[1];
    record->pH = vals[2];
    record->DO = vals[3];
    record->precipitation = vals[4];
    record->Air_temp = vals[5];
    return 0;
}

int TxtUtil_LoadFromFile(const char *filename, WaterQualityRecords *records) {
    if (!filename || !records) return -1;

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Unable to open file %s\n", filename);
        return -1;
    }

    char line[2048];
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return -1;
    }

    if (records->capacity < 1000) {
        int newCap = 1000;
        WaterQualityRecord *newRec =
            (WaterQualityRecord*)realloc(records->records, newCap * sizeof(WaterQualityRecord));
        if (newRec) {
            records->records = newRec;
            records->capacity = newCap;
        }
    }

    int totalLines = 0;
    int validCount = 0;
    while (fgets(line, sizeof(line), fp)) {
        totalLines++;
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) == 0) continue;

        WaterQualityRecord rec;
        if (parse_csv_line(line, &rec) == 0) {
            if (WQ_AddRecord(records, &rec) == 0) {
                validCount++;
            }
        } else {
            printf("Warning: failed to parse line: %s\n", line);
        }
    }
    fclose(fp);

    FILE *overview = fopen("data_overview.txt", "w");
    if (overview) {
        fprintf(overview, "Total raw records: %d\n", totalLines);
        fprintf(overview, "Valid loaded records: %d\n", validCount);
        fprintf(overview, "Note: missing and abnormal values are not preprocessed here.\n");
        fclose(overview);
    }

    printf("Loaded %d valid records.\n", validCount);
    return 0;
}

int TxtUtil_SaveToFile(const char *filename, const WaterQualityRecords *records) {
    if (!filename || !records || !records->records) return -1;

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
