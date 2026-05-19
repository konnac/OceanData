#include "DataBackupAndRecoverUtil.h"
#include "TxtFileUtil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>

int Backup_Backup(const WaterQualityRecords *records, const char *customName) {
    char backupName[256];
    if (customName && customName[0] != '\0') {
        strcpy(backupName, customName);
    } else {
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        sprintf(backupName, "backup_%04d%02d%02d_%02d%02d%02d.csv",
                tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
                tm->tm_hour, tm->tm_min, tm->tm_sec);
    }
    int ret = TxtUtil_SaveToFile(backupName, records);
    if (ret == 0) {
        printf("备份成功，文件已保存至 %s\n", backupName);
    } else {
        printf("备份失败！\n");
    }
    return ret;
}

int Backup_List(char backupList[][256], int maxCount) {
    DIR *dir = opendir(".");
    if (!dir) return 0;
    struct dirent *entry;
    int count = 0;
    while ((entry = readdir(dir)) != NULL && count < maxCount) {
        if (strstr(entry->d_name, "backup_") != NULL && strstr(entry->d_name, ".csv") != NULL) {
            strcpy(backupList[count], entry->d_name);
            count++;
        }
    }
    closedir(dir);
    return count;
}

int Backup_Restore(const char *backupFilename, WaterQualityRecords *records) {
    // 清空现有数据
    records->count = 0;
    int ret = TxtUtil_LoadFromFile(backupFilename, records);
    if (ret == 0) {
        printf("恢复成功！当前记录数: %d\n", records->count);
    } else {
        printf("恢复失败，文件无效或格式错误。\n");
    }
    return ret;
}