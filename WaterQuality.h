#ifndef WATER_QUALITY_H
#define WATER_QUALITY_H

#include <stddef.h>

typedef struct {
    char DailyStats[20];  // 日期
    float Temp;  // 水温
    float Salinity;  // 含盐度
    float pH;  // PH值
    float DO;  // 溶解氧
    float precipitation;  // 降水量
    float Air_temp;  // 空气温度
} WaterQualityRecord;

typedef struct {
    int count;  // 数据集大小
    int capacity;  // 数据集容量
    WaterQualityRecord *records;  // 数据集
} WaterQualityRecords;

extern WaterQualityRecords g_records;  // 全局数据集

/**
 * 初始化数据集
 * @param dataset 数据集
 * @param initialCapacity 初始容量
*/
void WQ_Init(WaterQualityRecords *dataset, int initialCapacity);

/**
 * 销毁数据集
 * @param dataset 数据集
*/
void WQ_Destroy(WaterQualityRecords *dataset);

/**
 * 添加数据
 * @param dataset 数据集
 * @param record 数据
 * @return 添加成功返回1，失败返回0
*/
int  WQ_AddRecord(WaterQualityRecords *dataset, const WaterQualityRecord *record);  // 添加数据

/**
 * 获取数据
 * @param dataset 数据集
 * @param index 数据索引
 * @return 数据
*/


const WaterQualityRecord* WQ_GetRecord(const WaterQualityRecords *dataset, int index);  // 获取数据

/**
 * 更新数据
 * @param dataset 数据集
 * @param index 数据索引
 * @param record 数据
*/
int  WQ_UpdateRecord(WaterQualityRecords *dataset, int index, const WaterQualityRecord *record);  // 更新数据

/**
 * 删除数据
 * @param dataset 数据集
 * @param index 数据索引
*/
int  WQ_DeleteRecord(WaterQualityRecords *dataset, int index);  // 删除数据

/**
 * 批量删除数据
 * @param dataset 数据集
 * @param indices 需要删除的数据索引数组
 * @param count 删除数量
 */
int  WQ_DeleteRecords(WaterQualityRecords *dataset, int indices[], int count);  // 批量删除数据

/**
 * 排序和过滤
 * @param dataset 数据集
 * @param param 参数类型
 * @param order 排序顺序
*/
typedef enum { SORT_ASC, SORT_DESC } SortOrder;  // 排序顺序

/**
 * 参数类型
 * @param PARAM_TEMP 水温
 * @param PARAM_SALINITY 含盐度
 * @param PARAM_PH PH值
 * @param PARAM_DO 溶解氧
 * @param PARAM_PRECIP 降水量
 * @param PARAM_AIRTEMP 空气温度
 */
typedef enum { PARAM_TEMP, PARAM_SALINITY, PARAM_PH, PARAM_DO, PARAM_PRECIP, PARAM_AIRTEMP } ParamType;  // 参数类型

/**
 * 排序
 * @param dataset 数据集
 * @param param 参数类型
 * @param order 排序顺序
*/
void WQ_Sort(WaterQualityRecords *dataset, ParamType param, SortOrder order);  // 排序

/**
 * 按范围过滤
 * @param dataset 数据集
 * @param param 筛选参数
 * @param min 最小值
 * @param max 最大值
 * @param out_indices 输出索引数组
 * @return 筛选后的数据数量
*/
int  WQ_FilterByRange(const WaterQualityRecords *dataset, ParamType param, float min, float max, int **out_indices);  //// 按范围过滤

/**
 * 从文件中加载数据
 * @param filename 文件名
 * @param records 数据集
 * @return 成功返回1，失败返回0
*/
int  TxtUtil_LoadFromFile(const char *filename, WaterQualityRecords *records);  // 从文件中加载数据

/**
 * 保存数据到文件中
 * @param filename 文件名
 * @param records 数据集
 * @return 成功返回1，失败返回0
*/
int  TxtUtil_SaveToFile(const char *filename, const WaterQualityRecords *records);  // 保存数据到文件中

/**
 * 备份和恢复
 * @param records 数据集
 * @param customName 自定义备份名称
 * @return 成功返回1，失败返回0
*/
int  Backup_Backup(const WaterQualityRecords *records, const char *customName);  // 备份数据

/**
 * 列出备份
 * @param backupList 备份列表
 * @param maxCount 最大备份数量
 * @return 成功返回备份数量，失败返回0
*/
int  Backup_List(char backupList[][256], int maxCount);  // 列出备份

/**
 * 恢复数据
 * @param backupFilename 备份文件名
 * @param records 数据集
 * @return 恢复成功返回1，失败返回0
*/
int  Backup_Restore(const char *backupFilename, WaterQualityRecords *records);  // 恢复数据

#endif
