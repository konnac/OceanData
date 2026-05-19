#include "QueryUtil.h"
#include <stdio.h>
#include <stdlib.h>

void DisplayPage(const WaterQualityRecords *dataset, int page, int rowsPerPage) {
    if (!dataset || dataset->count == 0) {
        printf("无数据\n");
        return;
    }
    int totalPages = (dataset->count + rowsPerPage - 1) / rowsPerPage;
    if (page < 1 || page > totalPages) {
        printf("页码无效！共 %d 页\n", totalPages);
        return;
    }
    int start = (page - 1) * rowsPerPage;
    int end = start + rowsPerPage;
    if (end > dataset->count) end = dataset->count;
    printf("\n===== 第 %d / %d 页 =====\n", page, totalPages);
    for (int i = start; i < end; i++) {
        WaterQualityRecord *r = &dataset->records[i];
        printf("%d: %s | 水温:%.2f 盐度:%.2f pH:%.2f DO:%.2f 降水:%.2f 气温:%.2f\n",
               i+1, r->datetime, r->Temp, r->Salinity, r->pH, r->DO, r->precipitation, r->Air_temp);
    }
}

void FilterAndDisplay(const WaterQualityRecords *dataset) {
    int paramChoice;
    float minVal, maxVal;
    printf("选择参数：1-水温 2-盐度 3-pH 4-DO 5-降水 6-气温: ");
    scanf("%d", &paramChoice);
    printf("输入范围（最小值 最大值）: ");
    scanf("%f %f", &minVal, &maxVal);
    ParamType param = (ParamType)(paramChoice - 1);
    int *indices = NULL;
    int found = WQ_FilterByRange(dataset, param, minVal, maxVal, &indices);
    if (found == 0) {
        printf("没有符合条件的记录。\n");
    } else {
        printf("共找到 %d 条记录：\n", found);
        for (int i = 0; i < found; i++) {
            int idx = indices[i];
            WaterQualityRecord *r = &dataset->records[idx];
            float val;
            switch (param) {
                case PARAM_TEMP: val = r->Temp; break;
                case PARAM_SALINITY: val = r->Salinity; break;
                case PARAM_PH: val = r->pH; break;
                case PARAM_DO: val = r->DO; break;
                case PARAM_PRECIP: val = r->precipitation; break;
                case PARAM_AIRTEMP: val = r->Air_temp; break;
                default: val = 0;
            }
            printf("%d: %s | 值:%.2f\n", idx+1, r->datetime, val);
        }
    }
    free(indices);
}

void SortAndDisplay(WaterQualityRecords *dataset) {
    int paramChoice, orderChoice;
    printf("排序参数：1-水温 2-盐度 3-pH 4-DO 5-降水 6-气温: ");
    scanf("%d", &paramChoice);
    printf("排序方式：1-升序 2-降序: ");
    scanf("%d", &orderChoice);
    ParamType param = (ParamType)(paramChoice - 1);
    SortOrder order = (orderChoice == 1) ? SORT_ASC : SORT_DESC;
    WQ_Sort(dataset, param, order);
    printf("排序完成，当前数据集已按指定顺序排列。\n");
    DisplayPage(dataset, 1, 15);
}