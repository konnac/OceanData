//数据修改,删除工具
#ifndef DataOperateUtil_h
#define DataOperateUtil_h
#include "../object/WaterQuality.h"

//接口
//数据修改(record为修改后的数据)
int OperateUtil_UpdateRecord(const char *filename, int index, const WaterQualityRecord *record);

//单条数据删除
int OperateUtil_DeleteSingleRecord(const char *filename, int index);

//多条数据删除(将要删除的数据索引保存在indices中,
//count为indices的元素个数,results为删除结果保存在results中,后续会判断其是否删除成功)
int OperateUtil_DeleteRecords(const char *filename, int *indices, int count,int *results);

#endif