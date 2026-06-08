#ifndef TXTFILEUTIL_H
#define TXTFILEUTIL_H

#include "../object/WaterQuality.h"

int TxtUtil_LoadFromFile(const char *filename, WaterQualityRecords *records);
int TxtUtil_SaveToFile(const char *filename, const WaterQualityRecords *records);

#endif
