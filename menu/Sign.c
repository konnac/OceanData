//用户登录

#include <stdio.h>
#include <string.h>
#include "SignUtil.h" 
#include "User.h"
#include "MainMenuAdmin.h"   
#include "MainMenuGuest.h" 
#include "WaterQuality.h"
#include "TxtFileUtil.h"

//存储水质记录的文件路径
static const char *DATA_FILE_CANDIDATES[] = {
    "data/WaterQuilityRecords.csv",
    "../data/WaterQuilityRecords.csv",
    "../../data/WaterQuilityRecords.csv",
    "WaterQuilityRecords.csv"
};

//检查文件是否可以读取
static int canOpenForRead(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        return 0;
    }
    fclose(fp);
    return 1;
}

//加载初始数据
static void loadInitialData(void) {
    //计算数组元素个数
    size_t count = sizeof(DATA_FILE_CANDIDATES) / sizeof(DATA_FILE_CANDIDATES[0]);

    //初始化全局数据结构，预分配1000条记录的内存空间
    WQ_Init(&g_records, 1000);
    //遍历4个候选路径，找到第一个可用的数据文件并加载
    for (size_t i = 0; i < count; i++) {
        //打不开,继续下一个
        if (!canOpenForRead(DATA_FILE_CANDIDATES[i])) {
            continue;
        }
        //如果文件可以读取，尝试加载数据
        if (TxtUtil_LoadFromFile(DATA_FILE_CANDIDATES[i], &g_records) == 0) {
            printf("[提示] 已加载 %d 条水质记录：%s\n", g_records.count, DATA_FILE_CANDIDATES[i]);
            return;
        }
        //清空之前可能部分加载的数据
        g_records.count = 0;
    }

    printf("[警告] 未能加载默认水质数据文件，进入菜单后可在“数据基础操作”中重新加载。\n");
}

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
            loadInitialData();
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

    WQ_Destroy(&g_records);
    
    return 0;
}
