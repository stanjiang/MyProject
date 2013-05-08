#include "GameSvr.h"
#include "TcpCode.h"
#include "MyShmMgr.h"
#include <string>

using namespace google;

char* CGameSvr::current_shmptr_;

// 日志文件名
const char* LOGFILE_INFO = "../log/gamesvr_info.log";
const char* LOGFILE_ERROR = "../log/gamesvr_error.log";

CGameSvr*  CGameSvr::CreateInstance(void)
{
    int shm_key = GAMESVR_SHM_KEY;
    int shm_size = sizeof(CGameSvr);
    int assign_size = shm_size;
    current_shmptr_ = (char*)CMyShmMgr::Instance().CreateShm(shm_key, shm_size, assign_size);

    CGameSvr* obj = new CGameSvr();
    return obj;
}

void* CGameSvr::operator new(size_t size)
{
    return (void*)current_shmptr_;
}

void CGameSvr::operator delete(void* mem)
{
}

int CGameSvr::Init(void)
{
    laststat_time_ = 0;
    send_pkg_count_ = 0;
    recv_pkg_count_ = 0;

    if(CreateMQ(MQ_CS) != 0)
    {
        return -1;
    }

    if(CreateMQ(MQ_SC) != 0)
    {
        return -1;
    }
    
    return 0;
}

int CGameSvr::RecvData(void)
{
    int size = 0;
    int ret = mq_cs_->Pop(recv_buff_, size);
    if(ret != 0)
    {
        return -1;
    }

    char* recv_data = &recv_buff_[0];
    CSPkgHead* head = (CSPkgHead*)recv_data;
    int fd = ntohl(head->fd);
    if(fd >= MAX_SOCKET_NUM)
    {
        LOG(ERROR) << "recv invalid client fd, fd=" << fd;
        return -1;
    }

    
    cspkg_head_[fd].fd = fd;
    cspkg_head_[fd].pkg_size = ntohl(head->pkg_size);
    cspkg_head_[fd].client_ip = ntohl(head->client_ip);

    const char* body = recv_data + sizeof(CSPkgHead);
    int pkg_body_size = CTcpCode::ConvertInt32(body);
    
    struct sockaddr_in client_addr;
    client_addr.sin_addr.s_addr = cspkg_head_[fd].client_ip;   

    std::string buf;
    buf.assign(body, pkg_body_size);
    google::protobuf::Message* msg = CTcpCode::Decode(buf);
    assert(NULL != msg);
    cspkg::AccountLoginReq* acc_login_req = dynamic_cast<cspkg::AccountLoginReq*>(msg);
    assert(acc_login_req != NULL);

    DLOG(INFO) << "recv client data, fd=" << cspkg_head_[fd].fd << ", ip=" << inet_ntoa(client_addr.sin_addr)
        << ", pkgsize=" << cspkg_head_[fd].pkg_size << ", bodysize=" << pkg_body_size; 

//    acc_login_req->PrintDebugString();
//    DLOG(INFO) << acc_login_req->DebugString();

    ++recv_pkg_count_;

    // 发送响应消息
    ret = AccountLoginRes(fd, acc_login_req->account());
    if(0 == ret)
    {
        DLOG(INFO) << "send to tcpsvr ok, fd=" << fd << ", uin=" << acc_login_req->account();
        ++send_pkg_count_;
    }
    else
    {
        DLOG(INFO) << "send to tcpsvr error, fd=" << fd << ", uin=" << acc_login_req->account() << ", ret=" << ret;
    }


    return 0;
}

void CGameSvr::TickHandle(void)
{
    /***加入统计信息***/
    time_t curtime = time(NULL);
    if(curtime >= laststat_time_)
    {
        recv_pkg_count_ /= STAT_TIME;
        send_pkg_count_ /= STAT_TIME;
        LOG(INFO) << "process pkg info, recvpkg=" << recv_pkg_count_ << ", sendpkg=" << send_pkg_count_;
        
        send_pkg_count_ = 0;
        recv_pkg_count_ = 0;
        laststat_time_ = curtime + STAT_TIME;
    }
}

void CGameSvr::Dispatch(int fd, int uin)
{

}

int CGameSvr::CreateMQ(MQType type)
{
    char* shmptr_ = NULL;
    if(MQ_CS == type)
    {
        shmptr_ = (char*)CMyShmMgr::Instance().CreateShm(CS_MQ_SHMKEY, MQ_SIZE, MQ_SIZE);
        CMsgQueue::InitShmAttr(shmptr_);
        mq_cs_ = new CMsgQueue;
        if(NULL == mq_cs_)
        {
            LOG(ERROR) << "new cs msg queue error, key=" << CS_MQ_SHMKEY << ", size=" << MQ_SIZE;
            return -1;
        }
        ENMShmMode mode = CMyShmMgr::Instance().GetShmMode(CS_MQ_SHMKEY);
        mq_cs_->Init(MQ_SIZE, mode);
        LOG(INFO) << "create cs mq ok, key=" << CS_MQ_SHMKEY << ", size=" << MQ_SIZE;
    }
    else if(MQ_SC == type)
    {
        shmptr_ = (char*)CMyShmMgr::Instance().CreateShm(SC_MQ_SHMKEY, MQ_SIZE, MQ_SIZE);
        CMsgQueue::InitShmAttr(shmptr_);
        mq_sc_ = new CMsgQueue;
        if(NULL == mq_sc_)
        {
            LOG(ERROR) << "new sc msg queue error, key=" << SC_MQ_SHMKEY << ", size=" << MQ_SIZE;
            return -1;
        }
        ENMShmMode mode = CMyShmMgr::Instance().GetShmMode(SC_MQ_SHMKEY);
        mq_sc_->Init(MQ_SIZE, mode);
        LOG(INFO) << "create sc mq ok, key=" << SC_MQ_SHMKEY << ", size=" << MQ_SIZE;
    }

    return 0;
}

int CGameSvr::AccountLoginRes(int fd, unsigned int uin)
{
    cspkg::AccountLoginRes acc_login_res;
    acc_login_res.set_account(uin);
    acc_login_res.set_result(0);

    std::string pkg = CTcpCode::Encode(acc_login_res);
    if(pkg.size() == 0)
    {
        return -1;
    }
    
    int pkg_head = 0;
    std::copy(pkg.begin(), pkg.begin() + sizeof(pkg_head), reinterpret_cast<char*>(&pkg_head));
    int len = ::ntohl(pkg_head);
    assert(len == pkg.size());

    std::string buf = pkg.substr(PKGHEAD_FIELD_SIZE);
    assert(len == buf.size()+PKGHEAD_FIELD_SIZE);

    DLOG(INFO) << "encode pkg ok, len=" << len << ", fd=" << fd << ", uin=" << uin;
    
    return SendMsgToClient(fd, pkg.c_str(), pkg.size());
}

int CGameSvr::SendMsgToClient(int fd, const char* msg, size_t size)
{
    assert(msg != NULL);

    // 检查gamesvr<-->tcpsvr的包头内容
    assert(fd == cspkg_head_[fd].fd);

    static char msg_buf[CSPKG_OPT_BUFFSIZE];
    char* msg_ptr = msg_buf;
    CSPkgHead* pkg_head = (CSPkgHead*)msg_ptr;
    
    pkg_head->pkg_size = htonl(size + sizeof(CSPkgHead));
    pkg_head->fd = htonl(fd);
    pkg_head->client_ip = htonl(cspkg_head_[fd].client_ip);

    msg_ptr += sizeof(CSPkgHead);
    memcpy(msg_ptr, msg, size);

    // 加入了gamesvr-->tcpsvr的消息队列中
    int ret = mq_sc_->Push(msg_buf, size + sizeof(CSPkgHead));

    return ret;
}

int main(int argc, char** argv)
{
    google::InitGoogleLogging(argv[0]);
    google::SetLogDestination(google::INFO, LOGFILE_INFO);
    google::SetLogDestination(google::ERROR, LOGFILE_ERROR);
 //   google::SetStderrLogging(google::ERROR); // 错误日志同时输出到console
    
//    google::FlushLogFiles(ERROR);

    CGameSvr* gamesvr = CGameSvr::CreateInstance();
    if(NULL == gamesvr)
    {
        return -1;
    }

    int ret = gamesvr->Init();
    if(ret < 0)
    {
        return -1;
    }

    for(; ;)
    {
        int ret = gamesvr->RecvData();
        if(ret != 0)
        {
            usleep(1000*20);
        }
        gamesvr->TickHandle();
        
        FLUSH();
    }

    google::ShutdownGoogleLogging();

    return 0;
}


