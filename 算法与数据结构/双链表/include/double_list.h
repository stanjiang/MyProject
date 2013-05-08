/**
 *  @file   double_list.h
 *  @brief   ˫������
 *  @author   stanjiang
 *  @date   2013-02-16
*/
#ifndef MY_DATASTRUCTURE_DOUBLE_LIST_H_
#define MY_DATASTRUCTURE_DOUBLE_LIST_H_

namespace mydatastructure{

// ˫������ڵ�
struct DoubleListNode
{
    int data;
    struct DoubleListNode* pre;
    struct DoubleListNode* next;
};


// ˫������ĳ�����
class DoubleList
{
public:
    DoubleList();
    ~DoubleList();

    /***
    *  @brief   ������ͷ����һ���ڵ�����
    *  @param   data: �����������
    *  @return  ok:0, error:-1
    ***/
    int PushFront(int data);

    /***
    *  @brief   ������β����һ���ڵ�����
    *  @param   data: �����������
    *  @return  ok:0, error:-1
    ***/
    int PushBack(int data);

    /***
    *  @brief   ����һ���ڵ�����
    *  @param   data: �����ҵ�����
    *  @return  ok:0, error:-1
    ***/
    int Find(int data);

    /***
    *  @brief   ɾ��һ��ָ���ڵ�����
    *  @param   data: ��ɾ��������
    *  @return  ok:0, error:-1
    ***/
    int Remove(int data);

    /***
    *  @brief   ɾ������ͷ����
    *  @param   void
    *  @return  ok:0, error:-1
    ***/
    int PopFront();

    /***
    *  @brief   ɾ����βͷ����
    *  @param   void
    *  @return  ok:0, error:-1
    ***/
    int PopBack();

    /***
    *  @brief   ��ӡ˫������
    *  @param   void
    *  @return  void
    ***/
    void Print();

private:
    /***
    *  @brief   �����յ�˫������
    *  @param   void
    *  @return  void
    ***/
    void CreateEmptyList();


private:
    DoubleListNode* head_; // ��ͷ�ڵ�
};
    


}


#endif


