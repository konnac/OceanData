//用户结构体
#include "User.h"
#include <string.h>  
#include <stddef.h> 

// 预设用户
User g_users[] = {
    {"admin", "123456", ROLE_ADMIN},
    {"guest", "guest",  ROLE_GUEST}
};

// 计算数组元素个数
int g_user_count = sizeof(g_users) / sizeof(User);

// 比较两个字符串, 返回0说明字符串相等
User* find_user(const char *username) {
    for (int i = 0; i < g_user_count; i++) {
        if (strcmp(g_users[i].username, username) == 0) {
            return &g_users[i];
        }
    }
    return NULL;
}