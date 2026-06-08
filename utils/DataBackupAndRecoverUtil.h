//数据备份与恢复
#ifndef DataBackupAndRecoverUtil_H
#define DataBackupAndRecoverUtil_H

#include "WaterQuality.h"

int Backup_Backup(const WaterQualityRecords *records, const char *customName);
int Backup_List(char backupList[][256], int maxCount);
int Backup_Restore(const char *backupFilename, WaterQualityRecords *records);

#endif