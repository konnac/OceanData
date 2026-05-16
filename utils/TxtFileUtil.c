//数据存储和读取
#include "TxtFileUtil.h"
#include<stdio.h>

//读取文件
int TxtUtil_LoadFromFile(const char *filename, WaterQualityRecords *records) { 
    extern int WQ_AddRecord(WaterQualityRecords* records, const WaterQualityRecord *newRecord);FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        return -1;//打开失败
    }
    char headerBuffer[1024];
    if (fgets(headerBuffer,sizeof(headerBuffer),fp) == NULL)
    {
        fclose(fp);
        return -1;
    }

    WaterQualityRecord tempRecord;
    while (fscanf(fp, "%f,%f,%f,%f,%f,%f",
         &tempRecord.Air_temp,
          &tempRecord.DO, 
          &tempRecord.Salinity,  
          &tempRecord.Temp, 
          &tempRecord.pH, 
          &tempRecord.precipitation) == 6) { 
        
        if (WQ_AddRecord(records,&tempRecord) != 0)
        {
            fclose(fp);
            return -1;
        }
        
    }

    fclose(fp);
    return 0;
}

//保存文件
int TxtUtil_SaveToFile(const char *filename, const WaterQualityRecords *records) { 
    if(!records || !records->records) return -1;
    FILE *fp = fopen(filename, "w");
    if(!fp) return -1;
    fprintf(fp, "Air_temp,DO,Salinity,Temp,pH,precipitation\n");
    for(int i = 0; i < records->count; i++)
    {
        WaterQualityRecord rec = records->records[i];
        fprintf(fp,"%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
            rec.Air_temp,rec.DO,rec.Salinity,rec.Temp,rec.pH,rec.precipitation);
    }

    fclose(fp);
    return 0;
}
