#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct tagTimeData TimeData,*pTimeData;

struct tagTimeData
{
    int nTimes;
    int nId;
    pTimeData pNext;
};

int initList(TimeData **L)
{
    *L = (pTimeData)malloc(sizeof(TimeData));
    if(NULL == *L) return -1;
    (*L)->pNext = NULL;
}

int addDataToList(TimeData **L,int id)
{
    pTimeData pHead = *L,pTmp = *L,pInsert;
    if(NULL == pHead)
    {
        pInsert = (pTimeData)malloc(sizeof(TimeData));
        if(NULL == pInsert) return -1;
        pInsert->nId = id;
        pInsert->pNext = NULL;
        *L = pInsert;
    }
    else
    {
        pInsert = (pTimeData)malloc(sizeof(TimeData));
        if(NULL == pInsert) return -1;
        memset(pInsert,0,sizeof(TimeData));
        pInsert->nId = id;
        pInsert->pNext = NULL;
        
        while(NULL != pHead->pNext)
        {
            pHead = pHead->pNext;
        }
        pHead->pNext = pInsert;
        *L = pTmp;
    }
}

int destroyList(pTimeData L)
{
    pTimeData p;
    while(NULL != L)
    {
        p = L->pNext;
        free(L);
        L = p;
    }

    return 0;
}

int getlength(pTimeData L)
{
    pTimeData pTmp = L;
    int count = 0;
    while(NULL != pTmp)
    {
        count++;
        pTmp = pTmp->pNext;
    }
    
    return count;
}

pTimeData bubbleSort(pTimeData head)
{
    pTimeData p,p1,p2,p3;
    TimeData h, t;
    if(head == NULL) return NULL;
    h.pNext=head;
    p=&h;
    while (p->pNext!=NULL)
    {
        p=p->pNext;
    }
    p=p->pNext=&t;
    while (p!=h.pNext)
    {
        p3=&h;
        p1=p3->pNext;
        p2=p1->pNext;
        while (p2!=p) 
        {
            if ((p1->nId)>(p2->nId))
            {
                p1->pNext=p2->pNext;
                p2->pNext=p1;
                p3->pNext=p2;
				
                p3=p2; 
                p2=p1->pNext;
				
            }
            else
            {
                p3=p1;
                p1=p2;
                p2=p2->pNext;
            }
        }
        p=p1;
    }
    while (p->pNext!=&t)
    {
        p=p->pNext;
    }
    p->pNext=NULL;
    return h.pNext;
}

void printfList(pTimeData L)
{
    pTimeData  p = L;
    while(NULL != p)
    {
        printf("%d ",p->nId);
        p = p->pNext;
    }
    printf("\n");
}

int main()
{
    pTimeData pDataHead = NULL;
    addDataToList(&pDataHead,1);
    addDataToList(&pDataHead,4);
    addDataToList(&pDataHead,3);
    addDataToList(&pDataHead,5);
    printfList(pDataHead);
    bubbleSort(pDataHead);
    
    //bubbleSort(pDataHead);
    printfList(pDataHead);
    destroyList(pDataHead);
    return 0;
                 
}
