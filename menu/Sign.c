//用户登录

#include <stdio.h>
#include <string.h>
#include "SignUtil.h" 
#include "User.h"
#include "MainMenuAdmin.h"   
#include "MainMenuGuest.h" 
#include "WaterQuality.h"
#include "TxtFileUtil.h"

int main(){
    int role;
    char username[20], password[20];
    int retry = 1;

    printf("========== 欢迎使用系统 ==========\n");

    while (retry){
        printf("\n [登陆菜单] \n");
         printf("请输入用户名: ");
        if (scanf("%19s", username) != 1) {
            printf("输入错误，程序退出。\n");
            return 1;
        }
        printf("请输入密码: ");
        if (scanf("%19s", password) != 1) {
            printf("输入错误，程序退出。\n");
            return 1;
        }

        // 调用工具类验证
        if (verifyUser(username, password, &role)) {
            //初始化数据文件
            WQ_Init(&g_records, 100);
            TxtUtil_LoadFromFile("WaterQuilityRecords.csv", &g_records);
            // 登录成功
            printf("\n登录成功！");
            if (role == ROLE_ADMIN) {
                printf(" 欢迎管理员 %s！\n", username);
                showAdminMenu();
            } else if (role == ROLE_GUEST) {
                printf(" 欢迎访客 %s！\n", username);
                showGuestMenu();
            } else {
                printf("\n登录失败，请重试。\n");
            }
            break;  // 登录成功，退出循环
        } else {
            // 登录失败
            printf("\n登录失败，是否重试？(1=重试, 0=退出): ");
            int choice;
            if (scanf("%d", &choice) != 1) choice = 0;
            if (choice != 1) {
                printf("程序退出。\n");
                retry = 0;
            }
        }
    }
    
    return 0;
}