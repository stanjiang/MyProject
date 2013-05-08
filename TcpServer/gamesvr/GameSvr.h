/**
 *  @file   GameSvr.h
 *  @brief   ��Ϸ�߼�������
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
     *  @brief   ����tcp���ӹ�����ʵ��,���ڴ��й��ڹ����ڴ���
     *  @param   ��
     *  @return   ʵ��ָ��
     ***/
    static CGameSvr*  CreateInstance(void);

    /***
     *  @brief   ����new��delete����,ʹ������ڹ����ڴ��з�������տռ�
     *  @return   void
     ***/
    static void* operator new(size_t size);
    static void operator delete(void* mem);

    /***
     *  @brief   ��ʼ������
     *  @param   void
     *  @return   0: ok , -1: error
     ***/
    int Init(void);

    /***
     *  @brief   ����Ϣ���н�������
     *  @param   void
     *  @return   0: ok , -1: error
     ***/
    int RecvData(void);

    /***
     *  @brief   ��ʱ������
     *  @param   void
     *  @return   void
     ***/
    void TickHandle(void);    

    /***
     *  @brief   �ַ���Ϣ
     *  @param   void
     *  @return   void
     ***/
    void Dispatch(int fd, int uin);

private:

    /***
     *  @brief   ������Ϣ����
     *  @param   type : ��Ϣ��������
     *  @return   0: ok , -1: error
     ***/
    int CreateMQ(MQType type);

    /***
     *  @brief   �����˺���Ӧ��Ϣ 
     *  @param   void
     *  @return   0: ok , -1: error
     ***/
    int AccountLoginRes(int fd, unsigned int uin);

    /***
     *  @brief   ������Ϣ ��tcpsvr
     *  @param   void
     *  @return   0: ok , -1: error
     ***/
    int SendMsgToClient(int fd, const char* msg, size_t size);

private:
    static char* current_shmptr_; // attath�����ڴ��ַ

    CMsgQueue* mq_cs_;
    CMsgQueue* mq_sc_;

    CSPkgHead cspkg_head_[MAX_SOCKET_NUM]; // �������client��csͨ��ͷ

    char recv_buff_[MAX_CSPKG_LEN];

    int send_pkg_count_;
    int recv_pkg_count_;
    time_t laststat_time_;
};


#endif


