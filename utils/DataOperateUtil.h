//数据修改,删除工具
#ifndef DataOperateUtil_h
#define DataOperateUtil_h
#include "WaterQuality.h"

//接口
//数据修改
int OperateUtil_UpdateRecord(const char *filename, int index, const WaterQualityRecord *record);

//数据删除工具
int OperateUtil_DeleteRecord(const char *filename, int index);
#endif