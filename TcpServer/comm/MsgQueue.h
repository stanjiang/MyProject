/**
 *  @file   MsgQueue.h
 *  @brief   基于共享内存的消息循环队列
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
     *  @brief   初始化
     *  @return   void
     ***/
    void Init(int size, ENMShmMode mode);

    /***
     *  @brief   插入一个节点至队首
     *  @param   data: 节点数据指针
     *  @param   size: 节点大小
     *  @return   0: ok , -1: error
     ***/
    int Push(char* data, int size);

    /***
     *  @brief   从队尾提取一个节点数据
     *  @param   data: 节点数据指针
     *  @param   size: 节点大小
     *  @return   0: ok , -1: error
     ***/
    int Pop(char data[MAX_CSPKG_LEN], int& size);


    /***
     *  @brief   设置共享内存首地址
     *  @param   attr_ptr: mq共享内存首地址
     *  @return   0: ok , -1: error
     ***/
    static int InitShmAttr(char* attr_ptr);
 
    /***
     *  @brief   重载new与delete操作,使其对象在共享内存中分配与回收空间
     *  @return   void
     ***/
    static void* operator new(size_t size);
    static void operator delete(void* mem);


private:
    static char* current_shmptr_; // 消息队列共享内存起始地址
    int size_; // 队列大小
    int count_; // 队列消息数量
    int front_; // 队首位置
    int rear_; // 队尾位置    
};



#endif

