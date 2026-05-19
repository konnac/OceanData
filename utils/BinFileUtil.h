#ifndef BINFILEUTIL_H
#define BINFILEUTIL_H

#include "WaterQuality.h"

int BinUtil_SaveToFile(const char *filename, const WaterQualityRecords *records);
int BinUtil_LoadFromFile(const char *filename, WaterQualityRecords *records);
void PerformanceCompare(const WaterQualityRecords *records);

#endif