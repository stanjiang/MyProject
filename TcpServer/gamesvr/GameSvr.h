/**
 *  @file   GameSvr.h
 *  @brief   游戏逻辑服务器
 *  @author   stanjiang
 *  @date   2012-05-01
*/
#ifndef _ZONE_GAMESVR_H_
#define _ZONE_GAMESVR_H_

#include "TcpComm.h"
#include "MsgQueue.h"


class CGameSvr
{
public:
    CGameSvr(){}
    ~CGameSvr(){}

    /***
     *  @brief   创建tcp连接管理器实例,即内存托管在共享内存中
     *  @param   无
     *  @return   实例指针
     ***/
    static CGameSvr*  CreateInstance(void);

    /***
     *  @brief   重载new与delete操作,使其对象在共享内存中分配与回收空间
     *  @return   void
     ***/
    static void* operator new(size_t size);
    static void operator delete(void* mem);

    /***
     *  @brief   初始化数据
     *  @param   void
     *  @return   0: ok , -1: error
     ***/
    int Init(void);

    /***
     *  @brief   从消息队列接收数据
     *  @param   void
     *  @return   0: ok , -1: error
     ***/
    int RecvData(void);

    /***
     *  @brief   定时器处理
     *  @param   void
     *  @return   void
     ***/
    void TickHandle(void);    

    /***
     *  @brief   分发消息
     *  @param   void
     *  @return   void
     ***/
    void Dispatch(int fd, int uin);

private:

    /***
     *  @brief   创建消息队列
     *  @param   type : 消息队列类型
     *  @return   0: ok , -1: error
     ***/
    int CreateMQ(MQType type);

    /***
     *  @brief   发送账号响应消息 
     *  @param   void
     *  @return   0: ok , -1: error
     ***/
    int AccountLoginRes(int fd, unsigned int uin);

    /***
     *  @brief   发送消息 至tcpsvr
     *  @param   void
     *  @return   0: ok , -1: error
     ***/
    int SendMsgToClient(int fd, const char* msg, size_t size);

private:
    static char* current_shmptr_; // attath共享内存地址

    CMsgQueue* mq_cs_;
    CMsgQueue* mq_sc_;

    CSPkgHead cspkg_head_[MAX_SOCKET_NUM]; // 存放所有client的cs通信头

    char recv_buff_[MAX_CSPKG_LEN];

    int send_pkg_count_;
    int recv_pkg_count_;
    time_t laststat_time_;
};


#endif


