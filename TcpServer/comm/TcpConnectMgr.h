/**
 *  @file   TcpConnectMgr.h
 *  @brief   tcp���ӹ�����
 *  @author   stanjiang
 *  @date   2012-03-29
*/
#ifndef _ZONE_TCPCONNECT_MGR_H_
#define _ZONE_TCPCONNECT_MGR_H_

#include "TcpComm.h"

class CMsgQueue;

class CTcpConnectMgr
{
public:
    CTcpConnectMgr(){}
    ~CTcpConnectMgr(){close(epoll_fd_); close(listen_fd_);}
    
    /***
     *  @brief   ����tcp���ӹ�����ʵ��,���ڴ��й��ڹ����ڴ���
     *  @param   ��
     *  @return   ʵ��ָ��
     ***/
    static CTcpConnectMgr*  CreateInstance(void);

    /***
     *  @brief   ���������������ڴ��С
     *  @param   ��
     *  @return   �ڴ��С
     ***/
    static int CountSize(void);

    /***
     *  @brief   ����new��delete����,ʹ������ڹ����ڴ��з�������տռ�
     *  @return   void
     ***/
    static void* operator new(size_t size);
    static void operator delete(void* mem);

    /***
     *  @brief   �������б�־
     *  @param   flag: ͨ�ŷ��������б�־
     *  @return   void
     ***/
    void SetRunFlag(int flag){run_flag_ = flag;}

    /***
     *  @brief   ��ʼ��tcp���ӹ�����
     *  @param   ��
     *  @return   0: ok , -1: error
     ***/
    int Init(void);

    /***
     *  @brief   ����tcp���ӹ�����,������tcp���Ӵ�����ѭ��
     *  @param   ��
     *  @return   void
     ***/
    void Run(void);

    /***
     *  @brief  ����Tcp���ӵķ������ݽӿ�
     *  @param   fd: ����socket
     *  @param   databuf: ����������buf
     *  @param   len: ���������ݳ���
     *  @return   0: ok , -1: error
     ***/
    static int TcpSendData(int fd, const char* databuf, int len);

    /***
     *  @brief  ����Tcp���ӵĽ������ݽӿ�
     *  @param   fd: ����socket
     *  @param   databuf: ����������buf
     *  @param   len: ���յ����ݵ�ʵ�ʳ���
     *  @return   0: ok , -1: error
     ***/
    static int TcpRecvData(int fd, char* databuf, int& len);

    /***
     *  @brief   �����׽���Ϊ������״̬
     *  @param   fd: �׽���
     *  @return   0: ok , -1: error
     ***/
    static int SetNonBlock(int fd);

    /***
     *  @brief   �����׽��ָ��ض�ѡ��,��:�Ƿ�����,send/recv buffer��
     *  @param   fd: �׽���
     *  @return   0: ok , -1: error
     ***/
    static int SetSocketOpt(int fd);

    /***
     *  @brief   �����׽��ֵ�ַ��Ϣ
     *  @param   ip: ip�ַ���
     *  @param   port: �˿ڵ�ַ
     *  @param   addr: �׽��ֵ�ַ
     *  @return    fd
     ***/
    static void SetAddress(const char* ip, unsigned short port, struct sockaddr_in* addr);

private:
    
    /***
     *  @brief   ��ȡ�ͻ�������
     *  @return   void
     ***/
    void GetClientMessage(void);

    /***
     *  @brief   ��ȡtcpsvr<-->gamesvr����Ϣ��������ȡ��������client������
     *  @return   void
     ***/
    void CheckWaitSendData(void);

    /***
     *  @brief   ���ͨѶ��ʱ��socket
     *  @return   void
     ***/
    void CheckTimeOut(void);

private:

    /***
     *  @brief   ��ʼ��tcpsvr����socket
     *  @param   ip: ip�ַ���
     *  @param   port: �˿ڵ�ַ
     *  @return   0: ok , -1: error
     ***/
    int InitListenSocket(const char* ip, unsigned short port);

    /***
     *  @brief   ��ͨ�Ŵ�������socket�����Ϣ
     *  @param   fd: socket���
     *  @param   type: ������
     *  @return   void
     ***/
    void ClearSocketInfo(int fd, ENMSocketErr type);

    /***
     *  @brief   ���տͻ������ӵ�����
     *  @param   fd: accept���ɵ�socket
     *  @return   0: ok , -1: error
     ***/
    int RecvClientData(int fd);

    /***
     *  @brief   ����/������������
     *  @return   
     ***/	
    void IncSockConn(void){++cur_conn_num_;}
    void DecSockConn(void){--cur_conn_num_;}

    /***
     *  @brief   ������Ϣ����
     *  @param   type : ��Ϣ��������
     *  @return   0: ok , -1: error
     ***/
    int CreateMQ(MQType type);
    
private:
    static char* current_shmptr_; // attath�����ڴ��ַ
   
    int run_flag_; // ���б�־
    SocketConnInfo client_sockconn_list_[MAX_SOCKET_NUM]; // �ͻ���socket������Ϣ�б�
    char send_client_buf_[SOCK_SEND_BUFFER]; // ������Ϣ��client��buf

    struct epoll_event  events_[MAX_SOCKET_NUM]; // epoll�¼�����
    
    int epoll_fd_; // ������epoll�����ľ��    
    int listen_fd_; // ����socket���
    int maxfds_; // ���sockt�����
    int epoll_timeout_; // epoll wait��ʱʱ��
    int cur_conn_num_; // ��ǰ������    

    CMsgQueue* mq_cs_; // tcpsvr->gamesvr��Ϣ����
    CMsgQueue* mq_sc_; // gamesvr->tcp��Ϣ����

    int send_pkg_count_;
    int recv_pkg_count_;
    int laststat_time_;
};





#endif



