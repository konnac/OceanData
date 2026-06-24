#include "MainMenuAdmin.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DataBackupAndRecoverUtil.h"
#include "ExceptionUtil.h"
#include "PredictUtil.h"
#include "QueryUtil.h"
#include "StatisticUtil.h"
#include "TxtFileUtil.h"
#include "WaterQuality.h"

#define MAX_PATH_LEN 260
#define MAX_REPORT_LINES 300

static char g_current_data_file[MAX_PATH_LEN] = "";
static float g_last_std_before[4] = {0};
static float g_last_std_after[4] = {0};
static int g_last_window_size = 0;

static const char *DATA_FILE_CANDIDATES[] = {
    "data/WaterQuilityRecords.csv",
    "../data/WaterQuilityRecords.csv",
    "../../data/WaterQuilityRecords.csv",
    "WaterQuilityRecords.csv"
};

static const char *PARAM_NAMES[] = {
    "水温", "盐度", "pH", "溶解氧", "降水量", "气温"
};

void showDataPreprocessMenu(void);

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

static int readFloat(const char *prompt, float *out) {
    printf("%s", prompt);
    if (scanf("%f", out) != 1) {
        clearInputBuffer();
        printf("输入无效，请输入数字！\n");
        return 0;
    }
    clearInputBuffer();
    return 1;
}

static int readWord(const char *prompt, char *out, size_t size) {
    char line[512];
    size_t len;

    if (!out || size == 0) {
        return 0;
    }
    printf("%s", prompt);
    if (!fgets(line, sizeof(line), stdin)) {
        printf("输入无效！\n");
        return 0;
    }
    line[strcspn(line, "\r\n")] = '\0';
    len = strlen(line);
    if (len == sizeof(line) - 1) {
        clearInputBuffer();
    }
    if (line[0] == '\0') {
        printf("输入不能为空！\n");
        return 0;
    }
    strncpy(out, line, size - 1);
    out[size - 1] = '\0';
    return 1;
}

static int confirmAction(const char *prompt) {
    char answer[16];
    if (!readWord(prompt, answer, sizeof(answer))) {
        return 0;
    }
    return answer[0] == 'y' || answer[0] == 'Y';
}

static int canOpenForRead(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        return 0;
    }
    fclose(fp);
    return 1;
}

static int resolveExistingDataFile(char *out, size_t outSize) {
    size_t count = sizeof(DATA_FILE_CANDIDATES) / sizeof(DATA_FILE_CANDIDATES[0]);
    for (size_t i = 0; i < count; i++) {
        if (canOpenForRead(DATA_FILE_CANDIDATES[i])) {
            strncpy(out, DATA_FILE_CANDIDATES[i], outSize - 1);
            out[outSize - 1] = '\0';
            return 1;
        }
    }
    return 0;
}

static const char *getWritableDataFile(void) {
    static char resolved[MAX_PATH_LEN];

    if (g_current_data_file[0] != '\0') {
        return g_current_data_file;
    }
    if (resolveExistingDataFile(resolved, sizeof(resolved))) {
        strncpy(g_current_data_file, resolved, sizeof(g_current_data_file) - 1);
        g_current_data_file[sizeof(g_current_data_file) - 1] = '\0';
        return g_current_data_file;
    }
    return DATA_FILE_CANDIDATES[0];
}

static int loadDefaultData(void) {
    char path[MAX_PATH_LEN];
    if (!resolveExistingDataFile(path, sizeof(path))) {
        printf("\n[错误] 未找到默认数据文件 WaterQuilityRecords.csv。\n");
        printf("请确认 data 目录存在，或把数据文件放在程序运行目录。\n");
        return 0;
    }

    if (!g_records.records || g_records.capacity <= 0) {
        WQ_Init(&g_records, 1000);
    } else {
        g_records.count = 0;
    }

    if (TxtUtil_LoadFromFile(path, &g_records) != 0) {
        printf("[错误] 数据加载失败：%s\n", path);
        return 0;
    }

    strncpy(g_current_data_file, path, sizeof(g_current_data_file) - 1);
    g_current_data_file[sizeof(g_current_data_file) - 1] = '\0';
    printf("[提示] 已加载 %d 条记录：%s\n", g_records.count, g_current_data_file);
    return 1;
}

static int ensureDataLoaded(void) {
    if (g_records.count > 0) {
        return 1;
    }
    printf("\n[提示] 当前内存中没有数据，正在尝试加载默认数据文件...\n");
    return loadDefaultData();
}

static int saveCurrentData(void) {
    const char *path;
    if (g_records.count == 0) {
        printf("\n[提示] 当前没有可保存的数据。\n");
        return 0;
    }

    path = getWritableDataFile();
    if (TxtUtil_SaveToFile(path, &g_records) == 0) {
        printf("[提示] 数据已保存到 %s\n", path);
        return 1;
    }

    printf("[错误] 数据保存失败：%s\n", path);
    return 0;
}

static void printRecord(const WaterQualityRecord *record, int index) {
    if (!record) {
        return;
    }
    printf("%5d | %-19s | %8.2f | %8.2f | %6.2f | %8.2f | %8.2f | %8.2f\n",
           index + 1,
           record->DailyStats,
           record->Temp,
           record->Salinity,
           record->pH,
           record->DO,
           record->precipitation,
           record->Air_temp);
}

static void printRecordHeader(void) {
    printf(" 序号 | 时间                |     水温 |     盐度 |     pH |   溶解氧 |   降水量 |     气温\n");
    printf("------|---------------------|----------|----------|--------|----------|----------|----------\n");
}

static int inputRecord(WaterQualityRecord *record) {
    char timeText[sizeof(record->DailyStats)];
    if (!record) {
        return 0;
    }
    memset(record, 0, sizeof(*record));

    printf("时间格式建议：YYYY-MM-DD_HH:MM:SS，下划线会自动替换为空格。\n");
    if (!readWord("请输入采样时间: ", timeText, sizeof(timeText))) return 0;
    for (size_t i = 0; timeText[i] != '\0'; i++) {
        if (timeText[i] == '_') {
            timeText[i] = ' ';
        }
    }
    strncpy(record->DailyStats, timeText, sizeof(record->DailyStats) - 1);

    if (!readFloat("请输入水温(℃): ", &record->Temp)) return 0;
    if (!readFloat("请输入盐度(PSU): ", &record->Salinity)) return 0;
    if (!readFloat("请输入pH值: ", &record->pH)) return 0;
    if (!readFloat("请输入溶解氧(mg/L): ", &record->DO)) return 0;
    if (!readFloat("请输入降水量(mm): ", &record->precipitation)) return 0;
    if (!readFloat("请输入气温(℃): ", &record->Air_temp)) return 0;

    return 1;
}

static void displayMenu(void) {
    printf("\n========================================\n");
    printf("     海水养殖水质分析系统 v1.0\n");
    printf("========================================\n");
    printf("   [1] 数据基础操作\n");
    printf("   [2] 数据预处理\n");
    printf("   [3] 统计分析\n");
    printf("   [4] 预测分析\n");
    printf("   [5] 查看数据概览\n");
    printf("   [6] 查看预警报告\n");
    printf("   [7] 查看相关性分析报告\n");
    printf("   [8] 数据备份与恢复\n");
    printf("   [9] 清屏\n");
    printf("   [0] 退出系统\n");
    printf("========================================\n");
}

static void browseData(void) {
    int rowsPerPage = 15;
    int page;
    int totalPages;

    if (!ensureDataLoaded()) {
        return;
    }

    readInt("请输入每页显示行数(默认15，输入0使用默认): ", &rowsPerPage);
    if (rowsPerPage <= 0) {
        rowsPerPage = 15;
    }
    totalPages = (g_records.count + rowsPerPage - 1) / rowsPerPage;

    while (1) {
        printf("\n当前共 %d 条记录，%d 页。\n", g_records.count, totalPages);
        if (!readInt("请输入页码(0返回): ", &page)) {
            continue;
        }
        if (page == 0) {
            return;
        }
        DisplayPage(&g_records, page, rowsPerPage);
    }
}

static void addRecord(void) {
    WaterQualityRecord record;
    if (!ensureDataLoaded()) {
        return;
    }
    if (!inputRecord(&record)) {
        printf("[提示] 新增操作已取消。\n");
        return;
    }
    if (WQ_AddRecord(&g_records, &record) == 0) {
        printf("[提示] 新增成功，当前记录数：%d\n", g_records.count);
        saveCurrentData();
    } else {
        printf("[错误] 新增失败，可能是内存不足。\n");
    }
}

static void updateRecord(void) {
    int index;
    WaterQualityRecord record;

    if (!ensureDataLoaded()) {
        return;
    }
    if (!readInt("请输入要修改的记录序号(从1开始): ", &index)) {
        return;
    }
    index--;
    if (index < 0 || index >= g_records.count) {
        printf("[错误] 记录序号超出范围。\n");
        return;
    }

    printf("\n当前记录：\n");
    printRecordHeader();
    printRecord(&g_records.records[index], index);

    if (!inputRecord(&record)) {
        printf("[提示] 修改操作已取消。\n");
        return;
    }
    if (WQ_UpdateRecord(&g_records, index, &record) == 0) {
        printf("[提示] 修改成功。\n");
        saveCurrentData();
    } else {
        printf("[错误] 修改失败。\n");
    }
}

static void deleteSingleRecord(void) {
    int index;

    if (!ensureDataLoaded()) {
        return;
    }
    if (!readInt("请输入要删除的记录序号(从1开始): ", &index)) {
        return;
    }
    index--;
    if (index < 0 || index >= g_records.count) {
        printf("[错误] 记录序号超出范围。\n");
        return;
    }

    printf("\n即将删除：\n");
    printRecordHeader();
    printRecord(&g_records.records[index], index);
    if (!confirmAction("确认删除？(y/N): ")) {
        printf("[提示] 删除已取消。\n");
        return;
    }

    if (WQ_DeleteRecord(&g_records, index) == 0) {
        printf("[提示] 删除成功，当前记录数：%d\n", g_records.count);
        saveCurrentData();
    } else {
        printf("[错误] 删除失败。\n");
    }
}

static void deleteBatchRecords(void) {
    int count;
    int *indices;

    if (!ensureDataLoaded()) {
        return;
    }
    if (!readInt("请输入要删除的记录数量: ", &count) || count <= 0) {
        printf("[错误] 删除数量无效。\n");
        return;
    }

    indices = (int *)malloc((size_t)count * sizeof(int));
    if (!indices) {
        printf("[错误] 内存分配失败。\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        char prompt[64];
        snprintf(prompt, sizeof(prompt), "请输入第 %d 条待删除记录序号: ", i + 1);
        if (!readInt(prompt, &indices[i])) {
            free(indices);
            return;
        }
        indices[i]--;
        if (indices[i] < 0 || indices[i] >= g_records.count) {
            printf("[错误] 序号 %d 超出范围，批量删除已取消。\n", indices[i] + 1);
            free(indices);
            return;
        }
    }

    printf("\n即将删除以下记录：\n");
    printRecordHeader();
    for (int i = 0; i < count; i++) {
        printRecord(&g_records.records[indices[i]], indices[i]);
    }

    if (!confirmAction("确认批量删除？(y/N): ")) {
        printf("[提示] 删除已取消。\n");
        free(indices);
        return;
    }

    if (WQ_DeleteRecords(&g_records, indices, count) == 0) {
        printf("[提示] 批量删除完成，当前记录数：%d\n", g_records.count);
        saveCurrentData();
    } else {
        printf("[错误] 批量删除失败。\n");
    }
    free(indices);
}

static void filterData(void) {
    if (!ensureDataLoaded()) {
        return;
    }
    FilterAndDisplay(&g_records);
    clearInputBuffer();
}

static void sortData(void) {
    if (!ensureDataLoaded()) {
        return;
    }
    SortAndDisplay(&g_records);
    clearInputBuffer();
    if (confirmAction("是否保存排序后的数据顺序？(y/N): ")) {
        saveCurrentData();
    }
}

static void dataOperation(void) {
    int choice;

    while (1) {
        printf("\n========== 数据基础操作 ==========\n");
        printf("  [1] 重新加载数据文件\n");
        printf("  [2] 分页浏览数据\n");
        printf("  [3] 新增记录\n");
        printf("  [4] 修改记录\n");
        printf("  [5] 删除单条记录\n");
        printf("  [6] 批量删除记录\n");
        printf("  [7] 条件筛选\n");
        printf("  [8] 排序并显示\n");
        printf("  [9] 保存当前数据\n");
        printf("  [0] 返回上级菜单\n");
        printf("===============================\n");

        if (!readInt("请选择: ", &choice)) {
            continue;
        }

        switch (choice) {
            case 1: loadDefaultData(); break;
            case 2: browseData(); break;
            case 3: addRecord(); break;
            case 4: updateRecord(); break;
            case 5: deleteSingleRecord(); break;
            case 6: deleteBatchRecords(); break;
            case 7: filterData(); break;
            case 8: sortData(); break;
            case 9: saveCurrentData(); break;
            case 0: return;
            default: printf("无效选项，请重新选择！\n"); break;
        }
    }
}

static void dataPreprocess(void) {
    showDataPreprocessMenu();
}

static void check_abnormal(WaterQualityRecords *dataset) {
    int missingCount = 0;
    DataSummary summary;

    if (!dataset || dataset->count == 0) {
        printf("\n[提示] 数据为空，请先导入数据！\n");
        return;
    }

    summary = check_all_abnormal(dataset);

    printf("\n========== 异常值检测统计 ==========\n");
    printf("总记录数: %d\n", summary.total);
    printf("有效记录数: %d\n", summary.valid);
    printf("含异常记录数: %d\n", summary.abnormal);
    printf("单条记录最大异常参数个数: %d\n", summary.max_error_params);
    printf("异常率: %.2f%%\n", summary.total > 0 ? (float)summary.abnormal / summary.total * 100 : 0);

    for (int i = 0; i < dataset->count; i++) {
        missingCount += count_missing_params(&dataset->records[i]);
    }
    printf("缺失值总数: %d\n", missingCount);
    printf("=====================================\n");
}

static void handle_abnormal_data(void) {
    DataSummary summary;

    if (!ensureDataLoaded()) {
        return;
    }

    summary = process_abnormal_data(&g_records);

    printf("\n========== 异常数据处理完成 ==========\n");
    printf("删除异常记录数: %d\n", summary.deleted);
    printf("修复异常值数量: %d\n", summary.fixed);
    printf("处理后有效记录数: %d\n", summary.valid);
    printf("======================================\n");
    if (confirmAction("是否保存处理后的数据？(y/N): ")) {
        saveCurrentData();
    }
}

static void handle_missing_values(void) {
    DataSummary summary;

    if (!ensureDataLoaded()) {
        return;
    }

    summary = process_missing_values(&g_records);

    printf("\n========== 缺失值处理完成 ==========\n");
    printf("填充缺失值数量: %d\n", summary.filled);
    printf("总记录数: %d\n", summary.total);
    printf("====================================\n");
    if (confirmAction("是否保存处理后的数据？(y/N): ")) {
        saveCurrentData();
    }
}

static void apply_moving_average(void) {
    int windowSize;

    if (!ensureDataLoaded()) {
        return;
    }

    while (!readInt("请输入移动平均窗口大小（奇数且>=3，如3、5、7）: ", &windowSize) ||
           windowSize < 3 ||
           windowSize % 2 == 0) {
        printf("输入无效，请输入奇数且>=3。\n");
    }

    g_last_std_before[0] = calculate_std(&g_records, 0);
    g_last_std_before[1] = calculate_std(&g_records, 1);
    g_last_std_before[2] = calculate_std(&g_records, 2);
    g_last_std_before[3] = calculate_std(&g_records, 3);

    moving_average_filter(&g_records, windowSize);

    g_last_std_after[0] = calculate_std(&g_records, 0);
    g_last_std_after[1] = calculate_std(&g_records, 1);
    g_last_std_after[2] = calculate_std(&g_records, 2);
    g_last_std_after[3] = calculate_std(&g_records, 3);
    g_last_window_size = windowSize;

    printf("\n========== 移动平均滤波完成 ==========\n");
    printf("窗口大小: %d\n", windowSize);
    printf("  参数    | 滤波前 | 滤波后 | 噪声减少率\n");
    printf("----------|--------|--------|----------\n");
    for (int i = 0; i < 4; i++) {
        float rate = g_last_std_before[i] > 0
                         ? (1 - g_last_std_after[i] / g_last_std_before[i]) * 100
                         : 0;
        printf("  %-6s | %.4f | %.4f | %.2f%%\n",
               PARAM_NAMES[i], g_last_std_before[i], g_last_std_after[i], rate);
    }
    printf("======================================\n");
    if (confirmAction("是否保存滤波后的数据？(y/N): ")) {
        saveCurrentData();
    }
}

static void save_data_summary(void) {
    DataSummary summary;

    if (!ensureDataLoaded()) {
        return;
    }

    summary = check_all_abnormal(&g_records);
    write_data_summary("data_summary.txt", &summary);

    printf("\n[提示] 数据概览报告已保存到 data_summary.txt\n");
}

static void save_analysis_report(void) {
    DataSummary summary;
    int windowSizes[1];
    float emptyBefore[4] = {0};

    if (!ensureDataLoaded()) {
        return;
    }

    summary = check_all_abnormal(&g_records);
    windowSizes[0] = g_last_window_size > 0 ? g_last_window_size : 7;

    if (g_last_window_size > 0) {
        write_analysis_report("analysis_report.txt",
                              &summary,
                              g_last_std_before,
                              g_last_std_after,
                              windowSizes,
                              1);
    } else {
        float currentStd[4];
        currentStd[0] = calculate_std(&g_records, 0);
        currentStd[1] = calculate_std(&g_records, 1);
        currentStd[2] = calculate_std(&g_records, 2);
        currentStd[3] = calculate_std(&g_records, 3);
        write_analysis_report("analysis_report.txt",
                              &summary,
                              emptyBefore,
                              currentStd,
                              windowSizes,
                              1);
    }

    printf("\n[提示] 详细分析报告已保存到 analysis_report.txt\n");
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

static void statisticalAnalysis(void) {
    int choice;

    if (!ensureDataLoaded()) {
        return;
    }

    while (1) {
        printf("\n========== 统计分析 ==========\n");
        printf("  [1] 查看并生成基本统计量报告\n");
        printf("  [2] 生成凌晨缺氧预警报告\n");
        printf("  [3] 生成相关性矩阵报告\n");
        printf("  [4] 生成盐度突变预警报告\n");
        printf("  [5] 一键执行全部统计分析\n");
        printf("  [0] 返回上级菜单\n");
        printf("=============================\n");

        if (!readInt("请选择: ", &choice)) {
            continue;
        }

        switch (choice) {
            case 1:
                printBasicStatsTable();
                GenerateBasicStatsReport(&g_records);
                break;
            case 2:
                DawnHypoxiaWarning(&g_records);
                break;
            case 3:
                CorrelationAnalysis(&g_records);
                break;
            case 4:
                SalinityShockWarning(&g_records);
                break;
            case 5:
                RunAllStatistics(&g_records);
                break;
            case 0:
                return;
            default:
                printf("无效选项，请重新选择！\n");
                break;
        }
    }
}

static LinearModel modelByChoice(int choice, const char **factorName) {
    switch (choice) {
        case 1:
            *factorName = "气温";
            return analyzeAirTempDO(&g_records);
        case 2:
            *factorName = "水温";
            return analyzeTempDO(&g_records);
        case 3:
            *factorName = "pH";
            return analyzePhDO(&g_records);
        case 4:
            *factorName = "盐度";
            return analyzeSalinityDO(&g_records);
        default:
            *factorName = "未知因子";
            return (LinearModel){0.0, 0.0, 0.0};
    }
}

static void printModelResult(int factorChoice) {
    const char *factorName;
    LinearModel model = modelByChoice(factorChoice, &factorName);

    printf("\n========== 线性回归结果 ==========\n");
    printf("因子: %s -> 溶解氧(DO)\n", factorName);
    printf("回归方程: DO = %.4f * %s + %.4f\n", model.slope, factorName, model.intercept);
    printf("R2值: %.4f\n", model.r_squared);
    printf("==================================\n");
}

static void predictDOValue(void) {
    int factorChoice;
    float xValue;
    const char *factorName;
    LinearModel model;
    double result;

    printf("\n预测因子：1-气温 2-水温 3-pH 4-盐度\n");
    if (!readInt("请选择预测因子: ", &factorChoice) || factorChoice < 1 || factorChoice > 4) {
        printf("[错误] 预测因子无效。\n");
        return;
    }
    if (!readFloat("请输入因子取值: ", &xValue)) {
        return;
    }

    model = modelByChoice(factorChoice, &factorName);
    result = predict(model, xValue);
    printf("[结果] 当%s为 %.4f 时，预测溶解氧 DO = %.4f mg/L\n",
           factorName, xValue, result);
}

static void predictionAnalysis(void) {
    int choice;

    if (!ensureDataLoaded()) {
        return;
    }

    while (1) {
        printf("\n========== 预测分析 ==========\n");
        printf("  [1] 气温 -> 溶解氧回归分析\n");
        printf("  [2] 水温 -> 溶解氧回归分析\n");
        printf("  [3] pH -> 溶解氧回归分析\n");
        printf("  [4] 盐度 -> 溶解氧回归分析\n");
        printf("  [5] 多因子影响程度对比\n");
        printf("  [6] 输入因子预测溶解氧\n");
        printf("  [0] 返回上级菜单\n");
        printf("=============================\n");

        if (!readInt("请选择: ", &choice)) {
            continue;
        }

        switch (choice) {
            case 1:
            case 2:
            case 3:
            case 4:
                printModelResult(choice);
                break;
            case 5:
                compareFactorsImpact(&g_records);
                break;
            case 6:
                predictDOValue();
                break;
            case 0:
                return;
            default:
                printf("无效选项，请重新选择！\n");
                break;
        }
    }
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
    printf("当前数据文件: %s\n", getWritableDataFile());
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

static int displayExistingReports(const char *title, const char *paths[], int pathCount) {
    int shown = 0;

    printf("\n========== %s ==========\n", title);
    for (int i = 0; i < pathCount; i++) {
        if (canOpenForRead(paths[i])) {
            displayReportFile(paths[i]);
            shown++;
        }
    }

    if (!shown) {
        printf("[提示] 暂未找到相关报告，请先在菜单中生成报告。\n");
    }
    return shown;
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

static void listBackups(void) {
    char backupList[100][256];
    int count = Backup_List(backupList, 100);

    if (count <= 0) {
        printf("[提示] 当前目录下没有找到备份文件。\n");
        return;
    }

    printf("\n========== 备份列表 ==========\n");
    for (int i = 0; i < count; i++) {
        printf("  [%d] %s\n", i + 1, backupList[i]);
    }
    printf("=============================\n");
}

static void restoreBackup(void) {
    char backupList[100][256];
    int count = Backup_List(backupList, 100);
    int choice;

    if (count <= 0) {
        printf("[提示] 当前目录下没有可恢复的备份文件。\n");
        return;
    }

    listBackups();
    if (!readInt("请选择要恢复的备份序号: ", &choice) || choice < 1 || choice > count) {
        printf("[错误] 备份序号无效。\n");
        return;
    }
    if (!confirmAction("恢复会覆盖当前内存数据，确认继续？(y/N): ")) {
        printf("[提示] 恢复已取消。\n");
        return;
    }

    if (!g_records.records || g_records.capacity <= 0) {
        WQ_Init(&g_records, 1000);
    }
    if (Backup_Restore(backupList[choice - 1], &g_records) == 0) {
        if (confirmAction("是否将恢复后的数据保存为当前主数据文件？(y/N): ")) {
            saveCurrentData();
        }
    }
}

static void backupAndRestore(void) {
    int choice;
    char customName[256];

    while (1) {
        printf("\n========== 数据备份与恢复 ==========\n");
        printf("  [1] 创建自动命名备份\n");
        printf("  [2] 创建自定义名称备份\n");
        printf("  [3] 查看备份列表\n");
        printf("  [4] 从备份恢复\n");
        printf("  [0] 返回上级菜单\n");
        printf("==================================\n");

        if (!readInt("请选择: ", &choice)) {
            continue;
        }

        switch (choice) {
            case 1:
                if (ensureDataLoaded()) {
                    Backup_Backup(&g_records, NULL);
                }
                break;
            case 2:
                if (ensureDataLoaded() &&
                    readWord("请输入备份文件名(建议以backup_开头，以.csv结尾): ", customName, sizeof(customName))) {
                    Backup_Backup(&g_records, customName);
                }
                break;
            case 3:
                listBackups();
                break;
            case 4:
                restoreBackup();
                break;
            case 0:
                return;
            default:
                printf("无效选项，请重新选择！\n");
                break;
        }
    }
}

void showAdminMenu(void) {
    int choice;

    while (1) {
        displayMenu();
        if (!readInt("   请选择操作 (0-9): ", &choice)) {
            continue;
        }

        switch (choice) {
            case 1: dataOperation(); break;
            case 2: dataPreprocess(); break;
            case 3: statisticalAnalysis(); break;
            case 4: predictionAnalysis(); break;
            case 5: viewOverview(); break;
            case 6: viewWarningReport(); break;
            case 7: viewAnalysisReport(); break;
            case 8: backupAndRestore(); break;
            case 9: clearScreen(); break;
            case 0:
                printf("\n感谢使用系统，再见！\n");
                return;
            default:
                printf("\n无效选项，请重新选择！\n");
                break;
        }
    }
}

void showDataPreprocessMenu(void) {
    int choice;

    while (1) {
        printf("\n========== 数据预处理 ==========\n");
        printf("  [1] 检测并统计异常值/缺失值\n");
        printf("  [2] 处理异常数据（删除异常参数>=3的记录，填充其余异常）\n");
        printf("  [3] 处理缺失值（均值逼近法填充）\n");
        printf("  [4] 移动平均滤波（平滑水温、盐度、pH、溶解氧）\n");
        printf("  [5] 保存数据概览报告到文件\n");
        printf("  [6] 保存详细分析报告（含滤波前后对比）\n");
        printf("  [0] 返回上级菜单\n");
        printf("===============================\n");

        if (!readInt("请选择: ", &choice)) {
            continue;
        }

        switch (choice) {
            case 1:
                if (ensureDataLoaded()) {
                    check_abnormal(&g_records);
                }
                break;
            case 2:
                handle_abnormal_data();
                break;
            case 3:
                handle_missing_values();
                break;
            case 4:
                apply_moving_average();
                break;
            case 5:
                save_data_summary();
                break;
            case 6:
                save_analysis_report();
                break;
            case 0:
                return;
            default:
                printf("无效选项，请重新选择！\n");
                break;
        }
    }
}
