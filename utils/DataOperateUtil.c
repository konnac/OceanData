//数据修改,删除工具
#include "DataOperateUtil.h"
#include "TxtFileUtil.h"
#include <stdio.h>

int OperateUtil_UpdateRecord(const char *filename, int index, const WaterQualityRecord *newRecord) { 
    if(!filename||!newRecord) return -1;
    WaterQualityRecords dataset;
    WQ_Init(&dataset, 100);
    if(TxtUtil_LoadFromFile(filename, &dataset) != 0){
        printf("出错了：好像读取不到文件('o_o), 要不去 %s 看看?\n",filename);
        WQ_Destroy(&dataset);
        return -1;
    }

    if(index < 0 || index >= dataset.count) {
        printf("出错了：索引 %d 超出范围(o_o'), 当前共有%d条数据 \n",index, dataset.count);
        WQ_Destroy(&dataset);
        return -1;
    }

    dataset.records[index] = *newRecord;
    if(TxtUtil_SaveToFile(filename, &dataset) != 0) {
        printf("出错了：无法保存文件 %s (O_O') \n",filename);
        WQ_Destroy(&dataset);
        return -1;
    }

    printf("成功修改第 %d 条数据(对应第 %d 行) \(OvO)/ \n", index, index + 2);
    WQ_Destroy(&dataset);
    return 0;
}

int OperateUtil_DeleteRecord(const char *filename, int index) { 
    if(!filename) return -1;
    WaterQualityRecords dataset;
    



}
