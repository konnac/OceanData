#include "MainMenuAdmin.h"
#include <stdio.h>
#include <stdlib.h>   
#include "WaterQuality.h"
#include "ExceptionUtil.h"

// 前向声明
void showDataPreprocessMenu();

// 清屏函数（跨平台）
static void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// 显示菜单选项
static void displayMenu() {
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
    printf("   请选择操作 (0-9): ");
}

// 各功能对应的处理函数（暂为占位，后续实现）
static void dataOperation() {
    printf("\n[功能] 数据基础操作 - 待实现\n");
    // 调用具体的增删改查模块
}

// 数据预处理模块
static void dataPreprocess() {
    showDataPreprocessMenu();
}

// 检测异常值和缺失值并显示统计信息
static void check_abnormal(WaterQualityRecords *dataset) {
    if (dataset->count == 0) {
        printf("\n[提示] 数据为空，请先导入数据！\n");
        return;
    }
    
    DataSummary summary = check_all_abnormal(dataset);
    
    printf("\n========== 异常值检测统计 ==========\n");
    printf("总记录数: %d\n", summary.total);
    printf("有效记录数: %d\n", summary.valid);
    printf("含异常记录数: %d\n", summary.abnormal);
    printf("单条记录最大异常参数个数: %d\n", summary.max_error_params);
    printf("异常率: %.2f%%\n", summary.total > 0 ? (float)summary.abnormal / summary.total * 100 : 0);
    printf("=====================================\n");
    
    // 统计缺失值
    int missing_count = 0;
    for (int i = 0; i < dataset->count; i++) {
        missing_count += count_missing_params(&dataset->records[i]);
    }
    printf("缺失值总数: %d\n", missing_count);
    printf("=====================================\n");
}

// 处理异常数据（删除异常≥3条记录，填充其余异常）
static void handle_abnormal_data() {
    if (g_records.count == 0) {
        printf("\n[提示] 数据为空，请先导入数据！\n");
        return;
    }
    
    DataSummary summary = process_abnormal_data(&g_records);
    
    printf("\n========== 异常数据处理完成 ==========\n");
    printf("删除异常记录数: %d\n", summary.deleted);
    printf("修复异常值数量: %d\n", summary.fixed);
    printf("处理后有效记录数: %d\n", summary.valid);
    printf("======================================\n");
}

// 处理缺失值（均值逼近法填充）
static void handle_missing_values() {
    if (g_records.count == 0) {
        printf("\n[提示] 数据为空，请先导入数据！\n");
        return;
    }
    
    DataSummary summary = process_missing_values(&g_records);
    
    printf("\n========== 缺失值处理完成 ==========\n");
    printf("填充缺失值数量: %d\n", summary.filled);
    printf("总记录数: %d\n", summary.total);
    printf("====================================\n");
}

// 移动平均滤波（平滑水温、盐度、pH、溶解氧）
static void apply_moving_average() {
    if (g_records.count == 0) {
        printf("\n[提示] 数据为空，请先导入数据！\n");
        return;
    }
    
    int window_size;
    printf("\n请输入移动平均窗口大小（建议为奇数，如3、5、7）: ");
    while (scanf("%d", &window_size) != 1 || window_size < 3 || window_size % 2 == 0) {
        while (getchar() != '\n'); // 清空缓冲区
        printf("输入无效，请输入奇数且≥3: ");
    }
    
    // 计算滤波前的标准差
    float std_before[4];
    std_before[0] = calculate_std(&g_records, 0); // 水温
    std_before[1] = calculate_std(&g_records, 1); // 盐度
    std_before[2] = calculate_std(&g_records, 2); // pH值
    std_before[3] = calculate_std(&g_records, 3); // 溶解氧
    
    // 应用滤波
    moving_average_filter(&g_records, window_size);
    
    // 计算滤波后的标准差
    float std_after[4];
    std_after[0] = calculate_std(&g_records, 0);
    std_after[1] = calculate_std(&g_records, 1);
    std_after[2] = calculate_std(&g_records, 2);
    std_after[3] = calculate_std(&g_records, 3);
    
    printf("\n========== 移动平均滤波完成 ==========\n");
    printf("窗口大小: %d\n", window_size);
    printf("滤波前后标准差对比:\n");
    printf("  参数    | 滤波前 | 滤波后 | 噪声减少率\n");
    printf("----------|--------|--------|----------\n");
    printf("  水温    | %.4f  | %.4f  | %.2f%%\n", 
           std_before[0], std_after[0],
           std_before[0] > 0 ? (1 - std_after[0]/std_before[0]) * 100 : 0);
    printf("  盐度    | %.4f  | %.4f  | %.2f%%\n", 
           std_before[1], std_after[1],
           std_before[1] > 0 ? (1 - std_after[1]/std_before[1]) * 100 : 0);
    printf("  pH值    | %.4f  | %.4f  | %.2f%%\n", 
           std_before[2], std_after[2],
           std_before[2] > 0 ? (1 - std_after[2]/std_before[2]) * 100 : 0);
    printf("  溶解氧  | %.4f  | %.4f  | %.2f%%\n", 
           std_before[3], std_after[3],
           std_before[3] > 0 ? (1 - std_after[3]/std_before[3]) * 100 : 0);
    printf("======================================\n");
}

// 保存数据概览报告到文件
static void save_data_summary() {
    if (g_records.count == 0) {
        printf("\n[提示] 数据为空，请先导入数据！\n");
        return;
    }
    
    DataSummary summary = check_all_abnormal(&g_records);
    write_data_summary("data_summary.txt", &summary);
    
    printf("\n[提示] 数据概览报告已保存到 data_summary.txt\n");
}

// 保存详细分析报告（含滤波前后对比）
static void save_analysis_report() {
    if (g_records.count == 0) {
        printf("\n[提示] 数据为空，请先导入数据！\n");
        return;
    }
    
    DataSummary summary = check_all_abnormal(&g_records);
    
    // 计算标准差（假设数据已进行滤波处理）
    float std_before[4] = {0};
    float std_after[4];
    std_after[0] = calculate_std(&g_records, 0);
    std_after[1] = calculate_std(&g_records, 1);
    std_after[2] = calculate_std(&g_records, 2);
    std_after[3] = calculate_std(&g_records, 3);
    
    int window_sizes[] = {7};
    write_analysis_report("analysis_report.txt", &summary, std_before, std_after, window_sizes, 1);
    
    printf("\n[提示] 详细分析报告已保存到 analysis_report.txt\n");
}

static void statisticalAnalysis() {
    printf("\n[功能] 统计分析 - 待实现\n");
}

static void predictionAnalysis() {
    printf("\n[功能] 预测分析 - 待实现\n");
}

static void viewOverview() {
    printf("\n[功能] 查看数据概览 - 待实现\n");
}

static void viewWarningReport() {
    printf("\n[功能] 查看预警报告 - 待实现\n");
}

static void viewAnalysisReport() {
    printf("\n[功能] 查看分析报告 - 待实现\n");
}

static void backupAndRestore() {
    printf("\n[功能] 数据备份与恢复 - 待实现\n");
}

// 管理员主菜单
void showAdminMenu() {
    int choice;
    while (1) {
        displayMenu();
        if (scanf("%d", &choice) != 1) {
            // 输入错误处理
            while (getchar() != '\n'); // 清空缓冲区
            printf("输入无效，请输入数字！\n");
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
                return;  // 退出菜单，返回 Sign.c 的 main 函数，随后程序结束
            default:
                printf("\n无效选项，请重新选择！\n");
                break;
        }
    }
}

// 数据预处理菜单
void showDataPreprocessMenu(){
    int choice;

    while (1) {
        printf("\n========== 数据预处理 ==========\n");
        printf("  [1] 检测并统计异常值/缺失值\n");
        printf("  [2] 处理异常数据（删除异常≥3条记录，填充其余异常）\n");
        printf("  [3] 处理缺失值（均值逼近法填充）\n");
        printf("  [4] 移动平均滤波（平滑水温、盐度、pH、溶解氧）\n");
        printf("  [5] 保存数据概览报告到文件\n");
        printf("  [6] 保存详细分析报告（含滤波前后对比）\n");
        printf("  [7] 返回上级菜单\n");
        printf("===============================\n");
        printf("请选择: ");

        if (scanf("%d", &choice) != 1) {
            // 输入错误处理
            while (getchar() != '\n'); // 清空缓冲区
            printf("输入无效，请输入数字！\n");
            continue;
        }       

        switch (choice){
            case 1: check_abnormal(&g_records); break;
            case 2: handle_abnormal_data(); break;
            case 3: handle_missing_values(); break;
            case 4: apply_moving_average(); break;
            case 5: save_data_summary(); break;
            case 6: save_analysis_report(); break;
            case 7: return; // 返回上级菜单
            default:
                printf("无效选项，请重新选择！\n");
                break;
        }

    }
    

}