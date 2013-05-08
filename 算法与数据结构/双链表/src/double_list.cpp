#include "double_list.h"
#include <cstring>
#include <cstdio>
using namespace mydatastructure;

DoubleList::DoubleList()
{
    CreateEmptyList();
}

DoubleList::~DoubleList()
{
}

void DoubleList::CreateEmptyList()
{
    head_ = new DoubleListNode;
    memset(head_, 0, sizeof(DoubleListNode));
}

int DoubleList::PushFront(int data)
{
    if(NULL == head_)
    {
        printf("double list head is null!\n");
        return -1;
    }

    DoubleListNode* node = new DoubleListNode;
    node->data = data;
    node->next = head_->next;
    node->pre = head_;
    if(node->next != NULL)
    {
        node->next->pre = node;
    }
    
    head_->next = node;
    // 头节点的前指针指向末尾最后一个数据节点
    if(NULL == head_->pre)
    {
        head_->pre = node;
    }
    printf("push front data ok, data=%d\n", data);

    return 0;
}

int DoubleList::PushBack(int data)
{
     

    return 0;
}


int DoubleList::Find(int data)
{
    DoubleListNode* node = head_->next;
    while(node != NULL)
    {        
        if(node->data == data)
        {
            printf("find data ok, data=%d\n", data);
            return 0;
        }
        node = node->next;
    }
    printf("not found data: %d\n", data);
    return -1;
}

int DoubleList::Remove(int data)
{
    return 0;
}

int DoubleList::PopFront()
{
     

    return 0;
}

int DoubleList::PopBack()
{
     

    return 0;
}


void DoubleList::Print()
{
    printf("the double linklist forward content:\n");
    DoubleListNode* node = head_->next;
    while(node != NULL)
    {
        printf("%d ", node->data);
        node = node->next;
    }

    printf("\nthe double linklist backward content:\n");
    node = head_->pre;
    while(node != head_)
    {
        printf("%d ", node->data);
        node = node->pre;
    }

}
