/**
 *  @file   MsgQueue.h
 *  @brief   ���ڹ����ڴ����Ϣѭ������
 *  @author   stanjiang
 *  @date   2012-04-30
*/
#ifndef _SHM_MSG_QUEUE_H_
#define _SHM_MSG_QUEUE_H_

#include "TcpComm.h" 

class CMsgQueue
{
public:
    CMsgQueue(){}
    ~CMsgQueue(){}

    /***
     *  @brief   ��ʼ��
     *  @return   void
     ***/
    void Init(int size, ENMShmMode mode);

    /***
     *  @brief   ����һ���ڵ�������
     *  @param   data: �ڵ�����ָ��
     *  @param   size: �ڵ��С
     *  @return   0: ok , -1: error
     ***/
    int Push(char* data, int size);

    /***
     *  @brief   �Ӷ�β��ȡһ���ڵ�����
     *  @param   data: �ڵ�����ָ��
     *  @param   size: �ڵ��С
     *  @return   0: ok , -1: error
     ***/
    int Pop(char data[MAX_CSPKG_LEN], int& size);


    /***
     *  @brief   ���ù����ڴ��׵�ַ
     *  @param   attr_ptr: mq�����ڴ��׵�ַ
     *  @return   0: ok , -1: error
     ***/
    static int InitShmAttr(char* attr_ptr);
 
    /***
     *  @brief   ����new��delete����,ʹ������ڹ����ڴ��з�������տռ�
     *  @return   void
     ***/
    static void* operator new(size_t size);
    static void operator delete(void* mem);


private:
    static char* current_shmptr_; // ��Ϣ���й����ڴ���ʼ��ַ
    int size_; // ���д�С
    int count_; // ������Ϣ����
    int front_; // ����λ��
    int rear_; // ��βλ��    
};



#endif

