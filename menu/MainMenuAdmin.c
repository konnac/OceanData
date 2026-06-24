/**
 * @file MainMenuAdmin.c
 * @brief 海水养殖水质分析系统 - 管理员主菜单实现
 *
 * 提供水质数据管理、预处理、统计分析、预测分析、报告生成
 * 以及数据备份与恢复等功能模块的菜单交互界面。
 *
 * @author OceanData Team
 * @version 1.0
 */

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

/** @brief 路径字符串最大长度 */
#define MAX_PATH_LEN 260
/** @brief 报告文件最大显示行数 */
#define MAX_REPORT_LINES 300

/** @brief 当前加载的数据文件路径 */
static char g_current_data_file[MAX_PATH_LEN] = "";
/** @brief 滤波前各参数标准差（水温、盐度、pH、溶解氧） */
static float g_last_std_before[4] = {0};
/** @brief 最佳滤波的各参数标准差（水温、盐度、pH、溶解氧） */
static float g_last_std_after[4] = {0};
/** @brief 最佳窗口大小 */
static int g_last_window_size = 0;

/**
 * @brief 清空上次的滤波分析结果
 */
static void clear_last_filter_analysis(void) {
    memset(g_last_std_before, 0, sizeof(g_last_std_before));
    memset(g_last_std_after, 0, sizeof(g_last_std_after));
    g_last_window_size = 0;
}

/**
 * @brief 保存本次滤波分析前后的标准差及窗口大小
 *
 * 将当前滤波分析的结果（滤波前标准差、滤波后标准差、使用的窗口大小）
 * 存储到静态全局变量中
 *
 * @param std_before  滤波前各参数标准差数组，长度为4，顺序为：
 *                     [水温, 盐度, pH, 溶解氧]
 * @param std_after   滤波后各参数标准差数组，长度与顺序同上
 * @param window_size 本次移动平均滤波使用的窗口大小
 */
static void store_last_filter_analysis(const float std_before[4],
                                       const float std_after[4],
                                       int window_size) {
    memcpy(g_last_std_before, std_before, sizeof(g_last_std_before));
    memcpy(g_last_std_after, std_after, sizeof(g_last_std_after));
    g_last_window_size = window_size;
}

/**
 * @brief 候选数据文件路径列表
 *
 * 按优先级尝试查找可用的数据文件路径
 */
static const char *DATA_FILE_CANDIDATES[] = {
    "data/WaterQuilityRecords.csv",
    "../data/WaterQuilityRecords.csv",
    "../../data/WaterQuilityRecords.csv",
    "WaterQuilityRecords.csv"
};

/**
 * @brief 参数名称数组
 *
 * 依次对应：水温、盐度、pH、溶解氧、降水量、气温
 */
static const char *PARAM_NAMES[] = {
    "水温", "盐度", "pH", "溶解氧", "降水量", "气温"
};

void showDataPreprocessMenu(void);

// 函数声明
static void dataOperation(void);
static void dataPreprocess(void);
static void statisticalAnalysis(void);
static void predictionAnalysis(void);
static void viewOverview(void);
static void viewWarningReport(void);
static void viewAnalysisReport(void);
static void backupAndRestore(void);
static void apply_moving_average(void);

/**
 * @brief 清屏函数
 *
 * 根据操作系统类型调用相应的清屏命令
 */
static void clearScreen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/**
 * @brief 清除输入缓冲区
 *
 * 读取并丢弃输入缓冲区中的所有字符，直到换行符或EOF
 */
static void clearInputBuffer(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
    }
}

/**
 * @brief 读取整数输入
 *
 * @param[in] prompt 提示信息
 * @param[out] out 输出参数，存储读取的整数
 * @return 读取成功返回1，失败返回0
 */
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

/**
 * @brief 读取浮点数输入
 *
 * @param[in] prompt 提示信息
 * @param[out] out 输出参数，存储读取的浮点数
 * @return 读取成功返回1，失败返回0
 */
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

/**
 * @brief 读取字符串单词（不含空格）
 *
 * @param[in] prompt 提示信息
 * @param[out] out 输出缓冲区，存储读取的字符串
 * @param[in] size 输出缓冲区大小
 * @return 读取成功返回1，失败返回0
 */
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

/**
 * @brief 确认操作提示
 *
 * @param[in] prompt 确认提示信息
 * @return 用户输入y/Y返回1，否则返回0
 */
static int confirmAction(const char *prompt) {
    char answer[16];
    if (!readWord(prompt, answer, sizeof(answer))) {
        return 0;
    }
    return answer[0] == 'y' || answer[0] == 'Y';
}






/**
 * @brief 检查文件是否可读
 *
 * @param[in] path 文件路径
 * @return 可读返回1，否则返回0
 */
static int canOpenForRead(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        return 0;
    }
    fclose(fp);
    return 1;
}

/**
 * @brief 查找并解析已存在的数据文件
 *
 * @param[out] out 输出缓冲区，存储找到的文件路径
 * @param[in] outSize 输出缓冲区大小
 * @return 找到返回1，未找到返回0
 */
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

/**
 * @brief 获取可写的数据文件路径
 *
 * @return 返回当前数据文件路径字符串
 */
static const char *getWritableDataFile(void) {
    static char resolved[MAX_PATH_LEN];

    //缓存则直接返回
    if (g_current_data_file[0] != '\0') {
        return g_current_data_file;
    }
    //查找并解析已存在的数据文件
    if (resolveExistingDataFile(resolved, sizeof(resolved))) {
        strncpy(g_current_data_file, resolved, sizeof(g_current_data_file) - 1);
        g_current_data_file[sizeof(g_current_data_file) - 1] = '\0';
        return g_current_data_file;
    }
    return DATA_FILE_CANDIDATES[0];
}

/**
 * @brief 加载默认数据文件
 *
 * @return 加载成功返回1，失败返回0
 */
static int loadDefaultData(void) {
    char path[MAX_PATH_LEN];
    if (!resolveExistingDataFile(path, sizeof(path))) {
        printf("\n[错误] 未找到默认数据文件 WaterQuilityRecords.csv。\n");
        printf("请确认 data 目录存在，或把数据文件放在程序运行目录。\n");
        return 0;
    }

    //若records存在则初始化
    if (!g_records.records || g_records.capacity <= 0) {
        WQ_Init(&g_records, 1000);
    } else {
        g_records.count = 0;
    }

    if (TxtUtil_LoadFromFile(path, &g_records) != 0) {
        printf("[错误] 数据加载失败：%s\n", path);
        return 0;
    }


    //给当前加载的数据文件赋值
    strncpy(g_current_data_file, path, sizeof(g_current_data_file) - 1);
    g_current_data_file[sizeof(g_current_data_file) - 1] = '\0';
    clear_last_filter_analysis();
    printf("[提示] 已加载 %d 条记录：%s\n", g_records.count, g_current_data_file);
    return 1;
}

/**
 * @brief 确保数据已加载
 *
 * @return 数据已加载或加载成功返回1，否则返回0
 */
static int ensureDataLoaded(void) {
    if (g_records.count > 0) {
        return 1;
    }
    printf("\n[提示] 当前内存中没有数据，正在尝试加载默认数据文件...\n");
    return loadDefaultData();
}

/**
 * @brief 保存当前数据到文件
 *
 * @return 保存成功返回1，失败返回0
 */
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

/**
 * @brief 打印单条水质记录
 *
 * @param[in] record 水质记录指针
 * @param[in] index 记录序号（从0开始）
 */
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

/**
 * @brief 打印记录表头
 */
static void printRecordHeader(void) {
    printf(" 序号 | 时间                |     水温 |     盐度 |     pH |   溶解氧 |   降水量 |     气温\n");
    printf("------|---------------------|----------|----------|--------|----------|----------|----------\n");
}

/**
 * @brief 输入单条水质记录
 *
 * @param[out] record 输出参数，存储输入的记录
 * @return 输入成功返回1，取消或失败返回0
 */
static int inputRecord(WaterQualityRecord *record) {
    char timeText[sizeof(record->DailyStats)];
    if (!record) {
        return 0;
    }
    //初始化结构体,将所有字段设为零值
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

/**
 * @brief 显示主菜单
 */
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
    printf("   [7] 查看分析报告\n");
    printf("   [8] 数据备份与恢复\n");
    printf("   [9] 清屏\n");
    printf("   [0] 退出系统\n");
    printf("========================================\n");
}

/**
 * @brief 显示管理员主菜单
 *
 * 管理系统主入口，提供各功能模块的菜单导航
 */
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

/**
 * @brief 分页浏览数据
 */
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
    //向上取整, 以防去掉余数
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

/**
 * @brief 新增单条水质记录
 */
static void addRecord(void) {
    WaterQualityRecord record;
    if (!ensureDataLoaded()) {
        return;
    }
    // 给记录赋值
    if (!inputRecord(&record)) {
        printf("[提示] 新增操作已取消。\n");
        return;
    }
    if (WQ_AddRecord(&g_records, &record) == 0) {
        clear_last_filter_analysis();
        printf("[提示] 新增成功，当前记录数：%d\n", g_records.count);
        saveCurrentData();
    } else {
        printf("[错误] 新增失败，可能是内存不足。\n");
    }
}

/**
 * @brief 修改指定记录
 */
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
        clear_last_filter_analysis();
        printf("[提示] 修改成功。\n");
        saveCurrentData();
    } else {
        printf("[错误] 修改失败。\n");
    }
}

/**
 * @brief 删除单条记录
 */
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
        clear_last_filter_analysis();
        printf("[提示] 删除成功，当前记录数：%d\n", g_records.count);
        saveCurrentData();
    } else {
        printf("[错误] 删除失败。\n");
    }
}

/**
 * @brief 批量删除记录
 */
static void deleteBatchRecords(void) {
    int count;
    // 要删除的记录
    int *indices;

    if (!ensureDataLoaded()) {
        return;
    }
    if (!readInt("请输入要删除的记录数量: ", &count) || count <= 0) {
        printf("[错误] 删除数量无效。\n");
        return;
    }

    //根据 count 分配足够存放 count 个 int 的内存空间，指针赋给 indices
    indices = (int *)malloc((size_t)count * sizeof(int));
    if (!indices) {
        printf("[错误] 内存分配失败。\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        char prompt[64];
        //用snprintf拼接好字符串存入内存
        snprintf(prompt, sizeof(prompt), "请输入第 %d 条待删除记录序号: ", i + 1);
        //prompt传给下一行的 readInt 函数打印
        if (!readInt(prompt, &indices[i])) {
            free(indices);
            return;
        }
        //转为实际的数组下标
        indices[i]--;
        //校验是否不合法
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
        clear_last_filter_analysis();
        printf("[提示] 批量删除完成，当前记录数：%d\n", g_records.count);
        saveCurrentData();
    } else {
        printf("[错误] 批量删除失败。\n");
    }
    free(indices);
}

/**
 * @brief 条件筛选数据
 */
static void filterData(void) {
    if (!ensureDataLoaded()) {
        return;
    }
    FilterAndDisplay(&g_records);
    clearInputBuffer();
}

/**
 * @brief 排序数据
 */
static void sortData(void) {
    if (!ensureDataLoaded()) {
        return;
    }
    SortAndDisplay(&g_records);
    clear_last_filter_analysis();
    clearInputBuffer();
    if (confirmAction("是否保存排序后的数据顺序？(y/N): ")) {
        saveCurrentData();
    }
}

/**
 * @brief 数据基础操作子菜单
 *
 * 提供数据加载、浏览、增删改查等基础操作
 */
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

/**
 * @brief 数据预处理入口
 */
static void dataPreprocess(void) {
    showDataPreprocessMenu();
}

/**
 * @brief 检测并统计异常值和缺失值
 *
 * @param[in] dataset 数据集指针
 */
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

/**
 * @brief 处理异常数据
 *
 * 删除异常参数≥3的记录，填充其余异常值
 */
static void handle_abnormal_data(void) {
    DataSummary summary;

    if (!ensureDataLoaded()) {
        return;
    }

    summary = process_abnormal_data(&g_records);
    clear_last_filter_analysis();

    printf("\n========== 异常数据处理完成 ==========\n");
    printf("删除异常记录数: %d\n", summary.deleted);
    printf("修复异常值数量: %d\n", summary.fixed);
    printf("处理后有效记录数: %d\n", summary.valid);
    printf("======================================\n");
    if (confirmAction("是否保存处理后的数据？(y/N): ")) {
        saveCurrentData();
    }
}

/**
 * @brief 处理缺失值
 *
 * 使用均值逼近法填充缺失值
 */
static void handle_missing_values(void) {
    DataSummary summary;

    if (!ensureDataLoaded()) {
        return;
    }

    summary = process_missing_values(&g_records);
    clear_last_filter_analysis();

    printf("\n========== 缺失值处理完成 ==========\n");
    printf("填充缺失值数量: %d\n", summary.filled);
    printf("总记录数: %d\n", summary.total);
    printf("====================================\n");
    if (confirmAction("是否保存处理后的数据？(y/N): ")) {
        saveCurrentData();
    }
}

/**
 * @brief 应用移动平均滤波（多窗口自动遍历）
 *
 * 对水温、盐度、pH、溶解氧四项参数进行移动平均平滑
 * 自动遍历窗口 3、5、7、9、11 并生成对比分析报告
 */
static void apply_moving_average(void) {
    if (!ensureDataLoaded()) {
        return;
    }

    // 定义要测试的窗口大小
    int window_sizes[] = {3, 5, 7, 9, 11};
    int window_count = sizeof(window_sizes) / sizeof(window_sizes[0]);
    
    // 存储每个窗口滤波后的标准差和噪声减少率
    float std_results[5][4] = {0};
    float noise_reduction[5][4] = {0};
    
    // 计算原始数据标准差
    float std_original[4];
    for (int i = 0; i < 4; i++) {
        std_original[i] = calculate_std(&g_records, i);
    }
    
    printf("\n========== 开始多窗口滤波分析 ==========\n");
    printf("原始数据标准差:\n");
    printf("  水温: %.4f, 盐度: %.4f, pH值: %.4f, 溶解氧: %.4f\n\n",
           std_original[0], std_original[1], std_original[2], std_original[3]);
    
    // 为每个窗口创建数据副本并进行滤波
    for (int w = 0; w < window_count; w++) {
        int window_size = window_sizes[w];
        
        printf("正在处理窗口 %d...\n", window_size);
        
        // 创建数据副本
        WaterQualityRecords dataset_copy;
        WQ_Init(&dataset_copy, g_records.capacity);
         
        // 复制数据
        for (int i = 0; i < g_records.count; i++) {
            WQ_AddRecord(&dataset_copy, &g_records.records[i]);
        }
        
        // 对副本进行滤波
        moving_average_filter(&dataset_copy, window_size);
        
        // 计算滤波后的标准差
        for (int param = 0; param < 4; param++) {
            //该窗口各种参数标准差
            std_results[w][param] = calculate_std(&dataset_copy, param);
            
            // 计算噪声减少率
            if (std_original[param] > 0) {
                noise_reduction[w][param] = (1 - std_results[w][param] / std_original[param]) * 100;
            } else {
                noise_reduction[w][param] = 0;
            }
        }
        
        printf("  窗口 %d 完成: 水温降噪 %.2f%%, 盐度降噪 %.2f%%, pH值降噪 %.2f%%, 溶解氧降噪 %.2f%%\n",
               window_size, noise_reduction[w][0], noise_reduction[w][1], 
               noise_reduction[w][2], noise_reduction[w][3]);
        
        // 保存滤波后的数据到文件
        char filename[256];
        snprintf(filename, sizeof(filename), "data/filtered_window_%d.csv", window_size);
        TxtUtil_SaveToFile(filename, &dataset_copy);
        printf("  滤波结果已保存到: %s\n", filename);
        
        // 释放副本内存
        free(dataset_copy.records);
    }
    
    printf("\n========== 滤波分析完成 ==========\n");
    
    // 生成多窗口对比分析报告 
    write_multi_window_report("data/multi_window_analysis_report.txt", &g_records,
                              window_sizes, window_count, std_results, noise_reduction);
    printf("详细分析报告已保存到: data/multi_window_analysis_report.txt\n");
    
    // 输出综合对比表格
    printf("\n综合对比表格:\n");
    printf("┌──────────┬──────────────┬──────────────┬──────────────┬──────────────┐\n");
    printf("│ 窗口大小 │   水温降噪率 │  盐度降噪率  │  pH值降噪率  │ 溶解氧降噪率 │\n");
    printf("├──────────┼──────────────┼──────────────┼──────────────┼──────────────┤\n");
    
    for (int w = 0; w < window_count; w++) {
        printf("│    %2d    │   %.2f%%     │   %.2f%%     │   %.2f%%     │   %.2f%%     │\n",
               window_sizes[w], noise_reduction[w][0], noise_reduction[w][1], 
               noise_reduction[w][2], noise_reduction[w][3]);
    }
    printf("└──────────┴──────────────┴──────────────┴──────────────┴──────────────┘\n");
    
    // 计算并输出最佳窗口
    float avg_reduction[5];
    for (int w = 0; w < window_count; w++) {
        avg_reduction[w] = (noise_reduction[w][0] + noise_reduction[w][1] + 
                           noise_reduction[w][2] + noise_reduction[w][3]) / 4.0f;
    }
    
    int best_window_idx = 0;
    float best_avg = avg_reduction[0];
    for (int w = 1; w < window_count; w++) {
        if (avg_reduction[w] > best_avg) {
            best_avg = avg_reduction[w];
            best_window_idx = w;
        }
    }
    // 保存最佳窗口的参数
    store_last_filter_analysis(std_original,
                               std_results[best_window_idx],
                               window_sizes[best_window_idx]);
    
    printf("\n【结论】\n");
    printf("最佳滤波窗口: %d\n", window_sizes[best_window_idx]);
    printf("平均噪声减少率: %.2f%%\n", avg_reduction[best_window_idx]);
    printf("该窗口在降噪效果和保留数据细节之间达到了较好的平衡。\n");
    printf("======================================\n");
    
    // 询问是否应用最佳窗口的滤波结果到当前数据
    if (confirmAction("\n是否应用最佳窗口的滤波结果到当前数据？(y/N): ")) {
        // 重新使用最佳窗口对当前数据进行滤波
        moving_average_filter(&g_records, window_sizes[best_window_idx]);
        printf("已应用窗口 %d 的滤波结果到当前数据。\n", window_sizes[best_window_idx]);

        if (confirmAction("是否保存滤波后的数据？(y/N): ")) {
            char new_file[MAX_PATH_LEN];
            snprintf(new_file, sizeof(new_file), "data/WaterQuilityRecords_filtered_w%d.csv", window_sizes[best_window_idx]);
            if (TxtUtil_SaveToFile(new_file, &g_records) == 0) {
                printf("[提示] 数据已保存到 %s\n", new_file);
            } else {
                printf("[错误] 数据保存失败：%s\n", new_file);
            }
        }
    }
}

/**
 * @brief 保存数据概览报告
 */
static void save_data_summary(void) {
    DataSummary summary;

    if (!ensureDataLoaded()) {
        return;
    }

    summary = check_all_abnormal(&g_records);
    write_data_summary("data_summary.txt", &summary);

    printf("\n[提示] 数据概览报告已保存到 data_summary.txt\n");
}

/**
 * @brief 保存详细分析报告
 *
 * 包含异常值统计和滤波前后对比数据
 */
static void save_analysis_report(void) {
    DataSummary summary;
    int windowSizes[1];

    if (!ensureDataLoaded()) {
        return;
    }
    if (g_last_window_size <= 0) {
        printf("\n[提示] 请先执行 [4] 移动平均滤波，再保存详细分析报告。\n");
        return;
    }

    summary = check_all_abnormal(&g_records);
    windowSizes[0] = g_last_window_size;
    write_analysis_report("analysis_report.txt",
                          &summary,
                          g_last_std_before,
                          g_last_std_after,
                          windowSizes,
                          1);

    printf("\n[提示] 详细分析报告已保存到 analysis_report.txt\n");
}

/**
 * @brief 打印基本统计量表格
 */
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

/**
 * @brief 统计分析子菜单
 *
 * 提供基本统计量、预警分析、相关性分析等功能
 */
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
        printf("  [4] 一键执行全部统计分析\n");
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

/**
 * @brief 根据选择获取线性回归模型
 *
 * @param[in] choice 因子选择（1-4）
 * @param[out] factorName 输出参数，存储因子名称
 * @return 对应的线性回归模型
 */
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

/**
 * @brief 打印回归分析结果
 *
 * @param[in] factorChoice 因子选择
 */
static void printModelResult(int factorChoice) {
    const char *factorName;
    LinearModel model = modelByChoice(factorChoice, &factorName);

    printf("\n========== 线性回归结果 ==========\n");
    printf("因子: %s -> 溶解氧(DO)\n", factorName);
    printf("回归方程: DO = %.4f * %s + %.4f\n", model.slope, factorName, model.intercept);
    printf("R2值: %.4f\n", model.r_squared);
    printf("==================================\n");
}

/**
 * @brief 根据因子值预测溶解氧
 */
static void predictDOValue(void) {
    int factorChoice; // 因子编号
    float xValue; // 自变量x值
    const char *factorName; // 因子名称
    LinearModel model; // 线性回归模型
    double result; // 预测结果

    printf("\n预测因子：1-气温 2-水温 3-pH 4-盐度\n");
    if (!readInt("请选择预测因子: ", &factorChoice) || factorChoice < 1 || factorChoice > 4) {
        printf("[错误] 预测因子无效。\n");
        return;
    }
    if (!readFloat("请输入因子取值: ", &xValue)) {
        return;
    }

    //获取线性回归模型
    model = modelByChoice(factorChoice, &factorName);
    //预测
    result = predict(model, xValue);
    printf("[结果] 当%s为 %.4f 时，预测溶解氧 DO = %.4f mg/L\n",
           factorName, xValue, result);
}

/**
 * @brief 使用留出法评估模型
 */
static void evaluateWithHoldout(void) {
    int factorChoice;
    const char* factorNames[] = {"气温", "水温", "pH", "盐度"};
    const char* varNames[] = {"Air_temp", "Temp", "pH", "Salinity"};

    printf("\n选择评估因子：1-气温 2-水温 3-pH 4-盐度\n");
    if (!readInt("请选择: ", &factorChoice) || factorChoice < 1 || factorChoice > 4) {
        printf("[错误] 无效选择\n");
        return;
    }

    double rmse;
    LinearModel model = evaluateFactorDOWithHoldout(&g_records, factorChoice - 1, &rmse);
    const char* name = factorNames[factorChoice - 1];
    const char* varName = varNames[factorChoice - 1];

    int train_count = (int)(g_records.count * 0.8);
    int test_count = g_records.count - train_count;

    printf("\n========== 留出法评估结果 ==========\n");
    printf("因子: %s -> 溶解氧(DO)\n", name);
    printf("训练集样本: %d, 测试集样本: %d\n", train_count, test_count);
    printf("回归方程: DO = %.4f * %s + %.4f\n", model.slope, varName, model.intercept);
    printf("R2值: %.4f\n", model.r_squared);
    printf("测试集RMSE: %.4f\n", rmse);
    printf("====================================\n");
}

/**
 * @brief 预测分析子菜单
 *
 * 提供单因子回归分析和多因子对比分析
 */
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
        printf("  [7] 留出法模型评估\n");
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
            case 7:
                evaluateWithHoldout();
                break;
            case 0:
                return;
            default:
                printf("无效选项，请重新选择！\n");
                break;
        }
    }
}

/**
 * @brief 查看数据概览
 */
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

/**
 * @brief 显示报告文件内容
 *
 * @param[in] path 报告文件路径
 */
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

/**
 * @brief 查找并显示已有报告
 *
 * @param[in] title 报告类型标题
 * @param[in] paths 候选路径数组
 * @param[in] pathCount 路径数量
 * @return 找到并显示的报告数量
 */
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

/**
 * @brief 查看预警报告
 */
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

/**
 * @brief 查看分析报告
 */
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

/**
 * @brief 列出可用备份文件
 */
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

/**
 * @brief 从备份恢复数据
 */
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
        clear_last_filter_analysis();
        if (confirmAction("是否将恢复后的数据保存为当前主数据文件？(y/N): ")) {
            saveCurrentData();
        }
    }
}

/**
 * @brief 数据备份与恢复子菜单
 */
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
                    readWord("请输入备份文件名(建议以.csv结尾): ", customName, sizeof(customName))) {
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



/**
 * @brief 显示数据预处理子菜单
 *
 * 提供异常值检测、缺失值处理、移动平均滤波等功能
 */
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
