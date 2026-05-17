//数据存储和读取
#include "TxtFileUtil.h"
#include<stdio.h>
#include <stdlib.h>

//读取文件
int TxtUtil_LoadFromFile(const char *filename, WaterQualityRecords *records) { 
    int WQ_AddRecord(WaterQualityRecords* records, const WaterQualityRecord *newRecord);FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("无法打开文件 %s\n", filename);
        return -1;//打开失败
    }
    char headerBuffer[1024];
    if (fgets(headerBuffer,sizeof(headerBuffer),fp) == NULL)
    {
        fclose(fp);
        printf("文件为空或读取表头失败\n");
        return -1;
    }

    //下面这个主要是用来扩容的
    //具体逻辑：如果已知数据量很大，则可以在加载前扩大dataset的容量，
    //我前面设置的是100，这里就直接扩容到10000(约500KB)


    if(records->capacity<1000){
        int newCapacity = 10000;//这里直接扩容到10000
        WaterQualityRecord *newRecords =
         (WaterQualityRecord *)realloc(records->records,newCapacity*sizeof(WaterQualityRecord));
        if (newRecords)
        {
            records->records = newRecords;
            records->capacity = newCapacity;
        }
        //如果realloc失败，保持原状，让WQ_AddRecord函数处理后续扩容
    }


    WaterQualityRecord tempRecord;
    int count = 0;
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
        count++;
    }
    //这里是防止加载时间过长让用户以为程序卡死,显示加载进度
    if (count%1000 == 0)
    {
        printf("已加载%d行数据...\n",count);
        fflush(stdout);
    }
    printf("\n加载完成，共%d行数据\n", count);

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
