/**
 *  @file   TcpClient.h
 *  @brief   tcpͨѶ�ͻ���
 *  @author   stanjiang
 *  @date   2012-04-04
*/
#ifndef _ZONE_TCPCLIENT_H_
#define _ZONE_TCPCLIENT_H_

#include "TcpComm.h"
#include <string>

struct ClientOptInfo
{
    int fd; // socket
    bool send_flag; // ���ݷ��ͱ�־,true:���Է�������,false:�����Է���
    bool recv_flag; // ���ݽ��ձ�־
    int recv_bytes; // ���յ��ֽ���
    char recv_buf[RECV_BUF_LEN];  //���յ���client�����       
    int uin;
};

class CTcpClient
{
public:
    CTcpClient(){}
    ~CTcpClient(){Destroy();}

    /***
     *  @brief   ����ʵ��,���ڴ��й��ڹ����ڴ���
     *  @param   ��
     *  @return   ʵ��ָ��
     ***/
    static CTcpClient*  CreateInstance(void);

    /***
     *  @brief   ����new��delete����,ʹ������ڹ����ڴ��з�������տռ�
     *  @return   void
     ***/
    static void* operator new(size_t size);
    static void operator delete(void* mem);


    /***
     *  @brief   ��ʼ��
     *  @param   port: �˿ڵ�ַ
     *  @param   addr: �׽��ֵ�ַ     
     *  @return    void
     ***/
    int Init(const char* ip, unsigned short port);

    /***
     *  @brief   ����ͻ����¼�ѭ��
     *  @return    void
     ***/
    void Run(void);

private:
    /***
     *  @brief   �����ͻ����׽���
     *  @param   port: �˿ڵ�ַ
     *  @param   addr: �׽��ֵ�ַ
     *  @return    fd
     ***/
    int CreateSocket(const char* ip, unsigned short port);

    /***
     *  @brief   Ԥ����IO�¼�
     *  @return    void
     ***/
    void PrepareIOEvent(void);

    /***
     *  @brief   ��ʱ����
     *  @return    void
     ***/
    void TickHandle(void);
    
    /***
     *  @brief   ����CS��Ϣ
     *  @return    void
     ***/
    void SendDataToServera(void);

    /***
     *  @brief   ����CS��Ϣ
     *  @return    void
     ***/
    void RecvDataFromServer(void);

    /***
     *  @brief   ��ӡ��Ϣ������
     *  @param   addr: �׽��ֵ�ַ
     *  @return    void
     ***/
    void PrintMsg(const std::string buf);

    /***
     *  @brief   ����AccountLoginReq��Ϣ
     *  @param   fd: client�׽���
     *  @param   uin: client �˺�
     *  @return    void
     ***/
    int SendAccountLoginReq(int fd, unsigned int uin);

    /***
     *  @brief   ����AccountLoginRes��Ϣ
     *  @param   fd: client�׽���
     *  @return    void
     ***/
    int RecvAccountLoginRes(int fd, const char* pkg, int len);

    /***
     *  @brief   �����˳�ǰ��������
     *  @return    void
     ***/
    void Destroy(void);
    
private:
    static char* current_shmptr_; // �ͻ��˹����ڴ��ַ
    
    int client_conn_[MAX_CLIENT_CONN];
    int maxfd_;
    ClientOptInfo client_opt_[MAX_CLIENT_CONN+1]; 
    int client_epoll_fd_;  // �ͻ���epoll�����ľ��
    struct epoll_event client_events_[MAX_CLIENT_CONN];    
    int cur_conn_num_;

    int send_pkg_count_;
    int recv_pkg_count_;
    time_t laststat_time_;
};



#endif
