#include "MainMenuGuest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ExceptionUtil.h"
#include "StatisticUtil.h"
#include "TxtFileUtil.h"
#include "WaterQuality.h"

#define MAX_REPORT_LINES 300

static const char *DATA_FILE_CANDIDATES[] = {
    "data/WaterQuilityRecords.csv",
    "../data/WaterQuilityRecords.csv",
    "../../data/WaterQuilityRecords.csv",
    "WaterQuilityRecords.csv"
};

static const char *PARAM_NAMES[] = {
    "水温", "盐度", "pH", "溶解氧", "降水量", "气温"
};

static void clearScreen(void) {
    system("cls");
}

static void clearInputBuffer(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
    }
}

static int readInt(const char *prompt, int *out) {
    printf("%s", prompt);
    if (scanf("%d", out) != 1) {
        clearInputBuffer();
        printf("输入无效，请输入数字！\n");
        return 0;
    }
    clearInputBuffer();
    return 1;
}

static int canOpenForRead(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        return 0;
    }
    fclose(fp);
    return 1;
}

static int loadDefaultData(void) {
    size_t count = sizeof(DATA_FILE_CANDIDATES) / sizeof(DATA_FILE_CANDIDATES[0]);

    for (size_t i = 0; i < count; i++) {
        if (!canOpenForRead(DATA_FILE_CANDIDATES[i])) {
            continue;
        }

        if (!g_records.records || g_records.capacity <= 0) {
            WQ_Init(&g_records, 1000);
        } else {
            g_records.count = 0;
        }

        if (TxtUtil_LoadFromFile(DATA_FILE_CANDIDATES[i], &g_records) == 0) {
            printf("[提示] 已加载 %d 条记录：%s\n", g_records.count, DATA_FILE_CANDIDATES[i]);
            return 1;
        }
    }

    printf("[提示] 暂未找到默认数据文件。\n");
    return 0;
}

static int ensureDataLoaded(void) {
    if (g_records.count > 0) {
        return 1;
    }
    printf("\n[提示] 当前内存中没有数据，正在尝试加载默认数据文件...\n");
    return loadDefaultData();
}

static void displayGuestMenu(void) {
    printf("\n========================================\n");
    printf("     海水养殖水质分析系统 v1.0 (访客模式)\n");
    printf("========================================\n");
    printf("   [1] 查看数据概览\n");
    printf("   [2] 查看预警报告\n");
    printf("   [3] 查看分析报告\n");
    printf("   [4] 清屏\n");
    printf("   [0] 退出系统\n");
    printf("========================================\n");
}

static void printBasicStatsTable(void) {
    ParamType params[] = {
        PARAM_TEMP, PARAM_SALINITY, PARAM_PH, PARAM_DO, PARAM_PRECIP, PARAM_AIRTEMP
    };

    printf("\n========== 基本统计量 ==========\n");
    printf("参数     |      均值 |    最大值 |    最小值 |      方差 |    标准差\n");
    printf("---------|-----------|-----------|-----------|-----------|-----------\n");
    for (int i = 0; i < 6; i++) {
        float mean, max, min, variance, stddev;
        CalcBasicStats(&g_records, params[i], &mean, &max, &min, &variance, &stddev);
        printf("%-8s | %9.4f | %9.4f | %9.4f | %9.4f | %9.4f\n",
               PARAM_NAMES[i], mean, max, min, variance, stddev);
    }
    printf("================================\n");
}

static void viewOverview(void) {
    DataSummary summary;
    int missingCount = 0;

    if (!ensureDataLoaded()) {
        return;
    }

    summary = check_all_abnormal(&g_records);
    for (int i = 0; i < g_records.count; i++) {
        missingCount += count_missing_params(&g_records.records[i]);
    }

    printf("\n========== 数据概览 ==========\n");
    printf("总记录数: %d\n", g_records.count);
    printf("有效记录数: %d\n", summary.valid);
    printf("含异常记录数: %d\n", summary.abnormal);
    printf("缺失值总数: %d\n", missingCount);
    if (g_records.count > 0) {
        printf("首条时间: %s\n", g_records.records[0].DailyStats);
        printf("末条时间: %s\n", g_records.records[g_records.count - 1].DailyStats);
    }
    printBasicStatsTable();
}

static void displayReportFile(const char *path) {
    FILE *fp = fopen(path, "r");
    char line[1024];
    int lineCount = 0;

    if (!fp) {
        return;
    }

    printf("\n----- %s -----\n", path);
    while (fgets(line, sizeof(line), fp) && lineCount < MAX_REPORT_LINES) {
        printf("%s", line);
        lineCount++;
    }
    if (!feof(fp)) {
        printf("\n[提示] 报告较长，仅显示前 %d 行。\n", MAX_REPORT_LINES);
    }
    printf("\n------------------------------\n");
    fclose(fp);
}

static void displayExistingReports(const char *title, const char *paths[], int pathCount) {
    int shown = 0;

    printf("\n========== %s ==========\n", title);
    for (int i = 0; i < pathCount; i++) {
        if (canOpenForRead(paths[i])) {
            displayReportFile(paths[i]);
            shown++;
        }
    }

    if (!shown) {
        printf("[提示] 暂未找到相关报告，请联系管理员先生成报告。\n");
    }
}

static void viewWarningReport(void) {
    const char *paths[] = {
        "data/warning_dawn.csv",
        "../data/warning_dawn.csv",
        "../../data/warning_dawn.csv",
        "warning_dawn.csv",
        "data/warning_salinity.csv",
        "../data/warning_salinity.csv",
        "../../data/warning_salinity.csv",
        "warning_salinity.csv"
    };
    displayExistingReports("预警报告", paths, (int)(sizeof(paths) / sizeof(paths[0])));
}

static void viewAnalysisReport(void) {
    const char *paths[] = {
        "analysis_report.txt",
        "data/analysis_report.txt",
        "../data/analysis_report.txt",
        "../../data/analysis_report.txt",
        "data_summary.txt",
        "data/data_summary.txt",
        "../data/data_summary.txt",
        "../../data/data_summary.txt",
        "data/stat_report.csv",
        "../data/stat_report.csv",
        "../../data/stat_report.csv",
        "stat_report.csv",
        "data/correlation_matrix.csv",
        "../data/correlation_matrix.csv",
        "../../data/correlation_matrix.csv",
        "correlation_matrix.csv"
    };
    displayExistingReports("分析报告", paths, (int)(sizeof(paths) / sizeof(paths[0])));
}

void showGuestMenu(void) {
    int choice;

    while (1) {
        displayGuestMenu();
        if (!readInt("   请选择操作 (0-4): ", &choice)) {
            continue;
        }

        switch (choice) {
            case 1: viewOverview(); break;
            case 2: viewWarningReport(); break;
            case 3: viewAnalysisReport(); break;
            case 4: clearScreen(); break;
            case 0:
                printf("\n感谢使用系统，再见！\n");
                return;
            default:
                printf("\n无效选项，请重新选择！\n");
                break;
        }
    }
}
