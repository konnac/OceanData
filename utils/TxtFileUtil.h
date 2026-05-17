//数据存储和读取
#ifndef TxtFileUtil_H
#define TxtFileUtil_H

#include "../object/WaterQuality.h" 

//接口
// 从 txt 文件读取水质数据到记录集
int TxtUtil_LoadFromFile(const char *filename, WaterQualityRecords *records);

// 将水质记录集保存到 txt 文件
int TxtUtil_SaveToFile(const char *filename, const WaterQualityRecords *records);

#endif
