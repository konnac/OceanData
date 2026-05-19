#ifndef USER_H
#define USER_H

// 角色常量
#define ROLE_ADMIN   1
#define ROLE_GUEST   2

typedef struct {
    char username[20];
    char password[20];
    int  role;          // ROLE_ADMIN 或 ROLE_GUEST
} User;

// 全局用户列表（在 User.c 中定义，外部声明）
extern User g_users[];
extern int  g_user_count;

// 根据用户名查找用户，返回指针，未找到返回 NULL
User* find_user(const char *username);

#endif