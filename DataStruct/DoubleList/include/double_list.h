/**
 *  @file   double_list.h
 *  @brief   双向链表
 *  @author   stanjiang
 *  @date   2013-02-16
*/
#ifndef MY_DATASTRUCTURE_DOUBLE_LIST_H_
#define MY_DATASTRUCTURE_DOUBLE_LIST_H_

namespace mydatastructure{

// 双向链表节点
struct DoubleListNode
{
    int data;
    struct DoubleListNode* pre;
    struct DoubleListNode* next;
};


// 双向链表的抽象类
class DoubleList
{
public:
    DoubleList();
    ~DoubleList();

    /***
    *  @brief   在链表头插入一个节点数据
    *  @param   data: 待插入的数据
    *  @return  ok:0, error:-1
    ***/
    int PushFront(int data);

    /***
    *  @brief   在链表尾插入一个节点数据
    *  @param   data: 待插入的数据
    *  @return  ok:0, error:-1
    ***/
    int PushBack(int data);

    /***
    *  @brief   查找一个节点数据
    *  @param   data: 待查找的数据
    *  @return  ok:0, error:-1
    ***/
    int Find(int data);

    /***
    *  @brief   删除一个指定节点数据
    *  @param   data: 待删除的数据
    *  @return  ok:0, error:-1
    ***/
    int Remove(int data);

    /***
    *  @brief   删除链表头数据
    *  @param   void
    *  @return  ok:0, error:-1
    ***/
    int PopFront();

    /***
    *  @brief   删除链尾头数据
    *  @param   void
    *  @return  ok:0, error:-1
    ***/
    int PopBack();

    /***
    *  @brief   打印双向链表
    *  @param   void
    *  @return  void
    ***/
    void Print();

private:
    /***
    *  @brief   创建空的双向链表
    *  @param   void
    *  @return  void
    ***/
    void CreateEmptyList();


private:
    DoubleListNode* head_; // 表头节点
};
    


}


#endif


