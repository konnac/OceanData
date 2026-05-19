//用户登录与权限管理工具
#ifndef SIGNUTIL_H
#define SIGNUTIL_H
#include "User.h"

/**
 * 登陆函数
 * @param username 用户名
 * @param password 密码  
 * @param role 角色
 * @return 1 成功 0 失败
 */
int verifyUser(const char *username, const char *password, int *role);

#endif