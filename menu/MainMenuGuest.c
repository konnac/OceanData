#include "MainMenuGuest.h"
#include <stdio.h>
#include <stdlib.h>

static void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

static void displayGuestMenu() {
    printf("\n========================================\n");
    printf("     海水养殖水质分析系统 v1.0 (访客模式)\n");
    printf("========================================\n");
    printf("   [1] 查看数据概览\n");
    printf("   [2] 查看预警报告\n");
    printf("   [3] 查看分析报告\n");
    printf("   [4] 清屏\n");
    printf("   [0] 退出系统\n");
    printf("========================================\n");
    printf("   请选择操作 (0-4): ");
}

static void viewOverview() {
    printf("\n[访客] 查看数据概览 - 待实现\n");
}

static void viewWarningReport() {
    printf("\n[访客] 查看预警报告 - 待实现\n");
}

static void viewAnalysisReport() {
    printf("\n[访客] 查看分析报告 - 待实现\n");
}

void showGuestMenu() {
    int choice;
    while (1) {
        displayGuestMenu();
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("输入无效，请输入数字！\n");
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