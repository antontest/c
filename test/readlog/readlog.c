#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//time interval
#define TIME_INTERVAL 5
//time threshold
#define THRESHOLD 1
//size of string time
#define SIZE_OF_STRING_TIME 20
//buffer of string
#define SIZE_OF_BUFFER 512

typedef struct tag_time_data time_data, *p_time_data;

/*************************************************
Struct Name     : tag_time_data
Description     : Save time and times of log
Members         : int iTimes -- Contiuous times
              char szTime[SIZE_OF_STRING_TIME] -- Save time
              p_time_data pNext -- next pointer
*************************************************/
struct tag_time_data {
        int iTimes;
        char szTime[SIZE_OF_STRING_TIME];
        p_time_data pNext;
};

//Extract date and time
int get_time_from_line(char *pchInfo, char szTime[], int *iSec);

//print list
void print_list(p_time_data head);

//destory list
int destroy_list(p_time_data head);

//add data to list
int append_data_to_list(time_data **head, const int iTimes, const char szTime[]);

//Use the speed pointer to find the center of the list
p_time_data get_middle_pointer(p_time_data pHead);

//two ordered lists merged into an ordered list
void merge(time_data **pLeft, p_time_data pRight);

//merge sort
void merge_sort(time_data **pHead);

//Function pointer
int cmp(const void **p1, const void **p2);

int main(int argc, char *agrv[])
{
        if (argc < 2) {
                printf("Error of input parameters.\n");
                return -1;
        }

        //definition
        char buf[SIZE_OF_BUFFER] = {0};
        char szCurTime[SIZE_OF_STRING_TIME] = {0}, szPreTime[SIZE_OF_STRING_TIME] = {0}, szStartTime[SIZE_OF_STRING_TIME] = {0};
        int preSec = 0, curSec = 0, diffSec = 0;
        int iConTimes = 0, iSkipLines = 2;
        p_time_data data = NULL;
        FILE *fp = NULL;
        int count = 0, i = 0;

        //Open file
        fp = fopen(agrv[1], "r");
        if (NULL == fp) {
                printf("Open file failed.\n");
                return -1;
        }

        //Skip firstt two row data
        while (!feof(fp) && iSkipLines-- > 0)
                fgets(buf, SIZE_OF_BUFFER, fp);

        //read text remaining data
        while (!feof(fp)) {
                if (fgets(buf, SIZE_OF_BUFFER, fp) == NULL) continue;

                //Extract date and time
                strcpy(szPreTime, szCurTime);
                if (get_time_from_line(buf, szCurTime, &curSec) == -1)
                        continue;

                //To prevent the time interval is not the same day
                diffSec = curSec - preSec;
                if (diffSec < 0) {
                        diffSec = curSec + curSec * 24 * 60 * 60 - preSec;
                }

                //When the time stamp is in line with the conditions, start counting
                if (diffSec < TIME_INTERVAL + THRESHOLD) {
                        iConTimes++;
                        if (iConTimes == 1) {
                                strcpy(szStartTime, szPreTime);
                        }
                } else {
                        if (iConTimes > 1) {
                                //save continuous time records
                                append_data_to_list(&data, iConTimes, szStartTime);
                                count++;
                        }

                        //clear count
                        iConTimes = 0;
                }

                //save last time of log
                preSec = curSec;

        }

        //close file
        fclose(fp);
        fp = NULL;

        //merge sort
        //merge_sort(&data);

        struct timeval start = {0};
        struct timeval end = {0};
        unsigned long timer = 0;

        gettimeofday(&start, NULL);
        //apply for pointer space
        p_time_data pTmp = data;
        time_data **pArr = (time_data**)malloc(sizeof(p_time_data) * count);
        if (NULL == *pArr) return -1;
        for (i = 0; i < count; i++) {
                pArr[i] = pTmp;
                pTmp = pTmp->pNext;
        }

        //quick sort
        qsort(pArr, count, sizeof(pArr[0]), cmp);

        int preTimes = 0, curTimes = 0;
        for (i = 0; i < count; i++) {
                curTimes = pArr[i]->iTimes;
                if (curTimes != preTimes) {
                        printf("Appear %d times:\n", curTimes);
                        preTimes = curTimes;
                }
                printf("    %s\n", pArr[i]->szTime);
        }

        gettimeofday(&end, NULL);
        timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
        printf("timer is %ld us.\n", timer);
        //results print
        //print_list(data);

        //destroy list
        destroy_list(data);

        //free
        free(pArr);

        return 0;
}

/*************************************************
Function        : get_time_from_line
Description     : Extract write time from a line of data
Input           : char *pchInfo -- Row data
Output          : char szTime[] -- time
                          int *iSec -- second
Return          : int -- Status
Others          : No
*************************************************/
int get_time_from_line(char *pchInfo, char szTime[], int *iSec)
{
        if (NULL == pchInfo) return -1;

        struct tm tmTemp = {0};
        const char *inFormat = " ENodeBApp Info %F %T";
        const char *outFormat = "%F %T";
        strptime(pchInfo, inFormat, &tmTemp);
        strftime(szTime, SIZE_OF_STRING_TIME, outFormat, &tmTemp);
        *iSec = tmTemp.tm_hour * 60 * 60 + tmTemp.tm_min * 60 + tmTemp.tm_sec;

        return 0;
}

/*************************************************
Function        : append_data_to_list
Description     : add data to list
Input           : time_data **head -- head pointer of list
              const int iTimes -- The number of consecutive times
              const char szTime[] -- first show time
Output          : time_data **head -- head pointer of list
Return          : int -- Status
Others          : No
*************************************************/
int append_data_to_list(time_data **pHead, const int iTimes, const char szTime[])
{
        p_time_data pTmp = *pHead, pInsert;
        pInsert = (p_time_data)malloc(sizeof(time_data));
        if (NULL == pInsert) return -1;

        //copy data
        pInsert->iTimes = iTimes;
        strcpy(pInsert->szTime, szTime);
        pInsert->pNext = NULL;

        if (NULL == pTmp) {
                *pHead = pInsert;
        } else {
                while (NULL != pTmp->pNext) {
                        pTmp = pTmp->pNext;
                }

                pTmp->pNext = pInsert;
        }

        return 0;
}


/*************************************************
Function        : print_list
Description     : Print structure content
Input           : p_time_data head -- head pointer of list
Output          : No
Return          : No
Others          : No
*************************************************/
void print_list(p_time_data pHead)
{
        p_time_data  p = pHead;
        int preTimes = 0, curTimes = 0;

        while (NULL != p) {
                curTimes = p->iTimes;
                if (curTimes != preTimes)
                        printf("Appear %d times:\n", curTimes);
                preTimes = curTimes;

                printf("    %s\n", p->szTime);
                p = p->pNext;
        }
}

/*************************************************
Function        : destroy_list
Description     : Release the list when not use it again
Input           : p_time_data head -- head pointer of list
Output          : No
Return          : int -- Status
Others          : No
*************************************************/
int destroy_list(p_time_data head)
{
        p_time_data p;

        while (NULL != head) {
                p = head->pNext;
                free(head);
                head = p;
        }

        return 0;
}

/*************************************************
Function        : get_middle_pointer
Description     : Use the speed pointer to find the center of the list
Input           : p_time_data pHead -- head pointer of list
Output          : No
Return          : p_time_data -- return the center pointer
Others          : NO
*************************************************/
p_time_data get_middle_pointer(p_time_data pHead)
{
        //Definition
        p_time_data pMid = pHead, pTmp = pHead, pMidLeft = NULL;

        while (NULL != pTmp && NULL != pTmp->pNext) {
                pTmp = pTmp->pNext->pNext;
                pMidLeft = pMid;
                pMid = pMid->pNext;
        }
        pMidLeft->pNext = NULL;

        return pMid;
}

/*************************************************
Function        : merge
Description     : two ordered lists pLeft and pRight
                          merged into an ordered list pLeft
Input           : time_data **pLeft -- Left pointer
                        : time_data *pRight -- Right pointer
Output          : time_data *pLeft -- Merged list
Return          : NO
Others          : NO
*************************************************/
void merge(time_data **pLeft, p_time_data pRight)
{
        //Definition
        p_time_data pl = *pLeft, pr = pRight;
        p_time_data pHead = NULL;
        p_time_data pResult = NULL;

        //get head pointer
        if (pl->iTimes >= pr->iTimes) {
                pHead = pResult = pl;
                pl = pl->pNext;
        } else {
                pHead = pResult = pr;
                pr = pr->pNext;
        }

        //sort
        while (pl != NULL && pr != NULL) {
                if (pl->iTimes >= pr->iTimes) {
                        pResult->pNext = pl;
                        pl = pl->pNext;
                } else {
                        pResult->pNext = pr;
                        pr = pr->pNext;
                }

                pResult = pResult->pNext;
        }

        //Link the remaining linked list to the tail
        if (pl == NULL) pResult->pNext = pr;
        else if (pr == NULL) pResult->pNext = pl;

        //Point to the linked list after the merger
        *pLeft = pHead;
}

/*************************************************
Function        : merge_sort
Description     : Merge sort
Input           : time_data **pHead -- head pointer of list
Output          : time_data **pHead -- Merged list
Return          : NO
Others          : NO
*************************************************/
void merge_sort(time_data **pHead)
{
        //no need to continue when the  length is less than or equal to 1.
        if (NULL == *pHead || NULL == (*pHead)->pNext) return;

        //get middle pointer
        p_time_data pl, pr;
        pr = get_middle_pointer(*pHead);
        pl = *pHead;

        //Recursive two points and merge
        merge_sort(&pl);
        merge_sort(&pr);
        merge(&pl, pr);

        //Return result header pointer
        *pHead = pl;

        return;
}

/*************************************************
Function        : cmp
Description     : Function pointer
Input           : const void **p1
              const void **p2
Output          :
Return          : int
Others          : NO
*************************************************/
int cmp(const void **p1, const void **p2)
{
        p_time_data p3;
        p_time_data p4;
        p3 = (p_time_data)(*p1);
        p4 = (p_time_data)(*p2);

        return p4->iTimes - p3->iTimes;
}
