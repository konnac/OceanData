//用户登录与权限管理工具
#include "SignUtil.h"
#include "User.h"      // 使用 User 结构体和 find_user
#include <string.h>
#include <stdio.h>

int verifyUser(const char *username, const char *password, int *role) {
    User *user = find_user(username);
    if (user == NULL) {
        printf("用户名不存在！\n");
        return 0;
    }
    if (strcmp(user->password, password) == 0) {
        *role = user->role;
        return 1;
    } else {
        printf("密码错误！\n");
        return 0;
    }
}