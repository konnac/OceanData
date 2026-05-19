#include "BinFileUtil.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int BinUtil_SaveToFile(const char *filename, const WaterQualityRecords *records) {
    if (!records || !records->records) return -1;
    FILE *fp = fopen(filename, "wb");
    if (!fp) return -1;
    fwrite(&records->count, sizeof(int), 1, fp);
    fwrite(records->records, sizeof(WaterQualityRecord), records->count, fp);
    fclose(fp);
    return 0;
}

int BinUtil_LoadFromFile(const char *filename, WaterQualityRecords *records) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) return -1;
    int count;
    if (fread(&count, sizeof(int), 1, fp) != 1) {
        fclose(fp);
        return -1;
    }
    if (records->capacity < count) {
        WaterQualityRecord *newRec = (WaterQualityRecord*)realloc(records->records, count * sizeof(WaterQualityRecord));
        if (!newRec) {
            fclose(fp);
            return -1;
        }
        records->records = newRec;
        records->capacity = count;
    }
    if (fread(records->records, sizeof(WaterQualityRecord), count, fp) != (size_t)count) {
        fclose(fp);
        return -1;
    }
    records->count = count;
    fclose(fp);
    return 0;
}

void PerformanceCompare(const WaterQualityRecords *records) {
    if (!records || records->count == 0) {
        printf("无数据，无法进行性能对比。\n");
        return;
    }
    clock_t start, end;
    double csv_time, bin_time;

    start = clock();
    TxtUtil_SaveToFile("perf_csv.csv", records);
    end = clock();
    csv_time = (double)(end - start) / CLOCKS_PER_SEC;

    start = clock();
    BinUtil_SaveToFile("perf_bin.bin", records);
    end = clock();
    bin_time = (double)(end - start) / CLOCKS_PER_SEC;

    long csv_size = 0, bin_size = 0;
    FILE *f = fopen("perf_csv.csv", "rb");
    if (f) { fseek(f, 0, SEEK_END); csv_size = ftell(f); fclose(f); }
    f = fopen("perf_bin.bin", "rb");
    if (f) { fseek(f, 0, SEEK_END); bin_size = ftell(f); fclose(f); }

    printf("\n========== 存储性能对比 ==========\n");
    printf("格式       文件大小(字节)   写入时间(秒)   人类可读\n");
    printf("CSV文本    %-15ld %-12.4f 是\n", csv_size, csv_time);
    printf("二进制      %-15ld %-12.4f 否\n", bin_size, bin_time);
    printf("结论：二进制格式读写更快、占用空间更小，但不可直接编辑。\n");
}