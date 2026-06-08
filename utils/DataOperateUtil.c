//数据修改,删除工具
#include "DataOperateUtil.h"
#include "TxtFileUtil.h"
#include <stdio.h>
#include <stdlib.h>

int OperateUtil_UpdateRecord(const char *filename, int index, const WaterQualityRecord *newRecord) { 
    if(!filename||!newRecord) return -1;
    WaterQualityRecords dataset;
    WQ_Init(&dataset, 1000);
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

//单次删除
int OperateUtil_DeleteSingleRecord(const char *filename,int index) { 
    int indices[] = {index};
    int result[] = {0};
    return OperateUtil_DeleteRecords(filename, indices, 1, result);
}

//批量删除
int OperateUtil_DeleteRecords(const char *filename, int indices[], int count, int *results) { 
    if(!filename||!indices||!results) return -1;
    WaterQualityRecords dataset;

    WQ_Init(&dataset, 100);

    if(TxtUtil_LoadFromFile(filename, &dataset) != 0){
        printf("错误：无法读取文件 %s\n", filename);
        WQ_Destroy(&dataset);
        //将所有结果标记为失败
        for (int i = 0; i < count; i++)
        {
            results[i] = -1;
        }
        WQ_Destroy(&dataset);
        return -1;
    }

    //提醒用户确认 
    printf("即将从文件[%s]删除 %d 条数据,请检查下列数据是否为你想删除的数据:\n",filename, count);
    for (int i = 0; i < count; i++)
    {
        //这里用户看到的序号应该是index + 2,(一行表头，一行起始数据)
        printf("-索引 %d(文件第 %d 行)\n", indices[i], indices[i] + 2);
    }
    printf("请输入y/Y以确认删除,输入其他任意键取消:\n");
    char confirm[10];
    if (scanf("%9s",confirm) == 1 && (confirm[0] == 'y' || confirm[0] == 'Y'))
    {
        printf("正在删除...\n");
    }else{
        printf("删除操作已取消(OwO)\n");
        WQ_Destroy(&dataset);
        //将所有结果标记为失败
        for (int i = 0; i < count; i++)
        {
            results[i] = -2;//表示用户取消了删除操作
        }
        return -2;
    }

    //这里的删除逻辑是：创建一个新的临时数据集，
    //将数据按顺序添加到临时数据集，最后将临时数据集保存到文件中
    //原因：删除操作会改变原始数据集的索引，所以不能使用原始数据集进行删除
    //问题：这样的操作如果是在两万多个数据中操作，那么效率会很低
    //而我的数据集的容量设置的是100，后续可能会有问题。(QwQ)
    WaterQualityRecords newDataset;
    WQ_Init(&newDataset, dataset.count);//初始化相同容量
    for (int i = 0; i < count; i++)
    {
        results[i] = 0;
    }

    //为了处理重复索引或者无效索引，先创建一个"删除标记数组"
    //下面这个变量markedForDeletion表示原数据集的第i条数据需要被删除。
    int *markedForDeletion = (int *)calloc((size_t)dataset.count, sizeof(int));
    if (!markedForDeletion)
    {
        printf("错误：无法分配内存(O_O)\n");
        WQ_Destroy(&dataset);
        WQ_Destroy(&newDataset);
        return -1;
    }
    
    //标记需要删除的索引，并检查有效性
    for (int i = 0; i < count; i++)
    {
        int idx = indices[i];
        if (idx < 0 ||idx >= dataset.count)
        {
            results[i] = -1;//索引越界，标记失败
        }
        else if (markedForDeletion[idx])
        {
            results[i] = -3;//重复索引，标记失败
        }
        else
        {
            markedForDeletion[idx] = 1;//标记成功
        }
    }
    //构建新的函数集：只copy未被标记删除的记录
    for (int i = 0; i < dataset.count; i++)
    {
        if (markedForDeletion[i] == 0)
        {
            WQ_AddRecord(&newDataset, &dataset.records[i]);
        }
    }
    free(markedForDeletion);//释放标记数组

    //保存数据集到文件
    if (TxtUtil_SaveToFile(filename, &newDataset) != 0)
    {
        printf("错误：无法保存文件 %s\n", filename);
        WQ_Destroy(&dataset);
        WQ_Destroy(&newDataset);
        return -1;
    }
    else
    {
        printf("成功删除 %d 条数据\n", count);
        WQ_Destroy(&dataset);
        WQ_Destroy(&newDataset);
    }
    
    printf("删除完成！\n");

    int hasFailed = 0;
    for (int i = 0; i < count; i++)
    {
        if (results[i] != 0)
        {
            hasFailed = 1;
            printf("警告：删除第 %d 条数据失败(%d)\n", i + 1, results[i]);
        }
    }
    if (!hasFailed)
    {
        printf("所有数据删除成功！\n");
    }
    
    WQ_Destroy(&dataset);
    WQ_Destroy(&newDataset);

    return hasFailed? -1 : 0;
}



