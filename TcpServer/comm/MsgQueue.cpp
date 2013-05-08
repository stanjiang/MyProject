#include "MsgQueue.h"
#include "MyShmMgr.h"
#include "TcpCode.h"

using namespace google;

char* CMsgQueue::current_shmptr_ = NULL;

void CMsgQueue::Init(int size, ENMShmMode mode)
{
    if(ESM_Init == mode)
    {
        count_ = 0;
        front_ = 0;
        rear_ = 0;
    }
    size_ = size - sizeof(*this);
    char* data = current_shmptr_ + sizeof(*this);

    DLOG(INFO) << "init msg queue info, totalsize=" << size <<", datasize=" << size_ << ", head=" << sizeof(*this)
        << ", mode=" << mode <<",count=" << count_ << ", front=" << front_ << ", rear=" << rear_;

    Trace("msg queue init info, dataptr=%p, shmptr=%p", data, current_shmptr_);
}

int CMsgQueue::InitShmAttr(char* attr_ptr)
{
    current_shmptr_ = attr_ptr;
    return 0;
}

void* CMsgQueue::operator new(size_t size)
{
    return (void*)current_shmptr_;
}

void CMsgQueue::operator delete(void* mem)
{
}

int CMsgQueue::Push(char* data, int size)
{
    if(NULL == data || size > size_)
    {
        LOG(ERROR) << "invalid push param, datasize=" << size << ", queuesize=" << size_;
        return -1;
    }

    char* cur_data = (char*)this + sizeof(*this) + front_;
    char* data_begin = (char*)this + sizeof(*this); 
    int rear = rear_; // 保存rear_副本,以防对端进程在此期间修改

    DLOG(INFO) << "before push info, front=" << front_ << ", rear=" << rear <<", datasize=" << size << ", mqsize=" << size_;

    // 队首指针在队尾前面的情况
    if(front_ >= rear)
    {
        int pos = front_ + size;
        if(pos <= size_)
        {            
            memcpy(cur_data, data, size);
            front_ = pos;
            DLOG(INFO) << "push data ok, front=" << front_ << ", rear=" << rear;
        }
        else
        {
            pos %= size_;
            if(pos >= rear)
            {
                // 队列满了
                LOG(ERROR) << "mq is full, pos=" << pos << ", front=" << front_<< ", rear=" << rear << ", count=" << count_;
                return -1;
            }
            else
            {
                // 分两段拷贝,只有拷贝完成了,才修改front_索引
                int rest =  size_ - front_;
                memcpy(cur_data, data, rest);
                memcpy(data_begin, data+rest, pos); // 从MQ数据起始位置开始拷贝
                front_ = pos;
                
                DLOG(INFO) << "push data ok to both side, front=" << front_ << ", rear=" << rear << ", rest=" << rest;
            }
        }
    }
    else 
    {
        int pos = front_ + size ;
        if(pos < rear)
        {
            memcpy(cur_data, data, size);
            front_ = pos;
            DLOG(INFO) << "push data ok, front=" << front_ << ", rear=" << rear;
        }
        else
        {
            // 队列满了
            LOG(ERROR) << "mq is full, pos=" << pos << ", front=" << front_<< ", rear=" << rear << ", count=" << count_;
            return -1;        
        }
    }

    ++count_;
    DLOG(INFO) << "push msg over, count=" << count_;
    
    return 0;
}

int CMsgQueue::Pop(char data[MAX_CSPKG_LEN], int & size)
{
    if(count_ <= 0)
    {
        return -1;
    }

    char* cur_data = (char*)this + sizeof(*this) + rear_;
    char* data_begin = (char*)this + sizeof(*this); 
    int front = front_; // 保存副本,以防push进程在此期间修改该值

    // 队列判空(判空检查需放在最前面)
    if(rear_ == front)
    {
        DLOG(INFO) << "pop mq is empty";
        return -1;
    }

    // 需要检查队列尾不足4字节的情况,即存放长度的字段有一部分在队列尾,另一部分在队列头
    if((size_ - rear_) < PKGHEAD_FIELD_SIZE)
    {
        int first_part = size_ - rear_;
        int second_part = PKGHEAD_FIELD_SIZE - first_part;
        char pkg_size[PKGHEAD_FIELD_SIZE];

        memcpy(pkg_size, cur_data, first_part);
        memcpy(pkg_size+first_part, data_begin, second_part);
        size = CTcpCode::ConvertInt32(pkg_size);        

        DLOG(INFO) << "pkg size are both side, rear=" << rear_ << ", datasize=" << size_ 
            << ", first=" << first_part << ", second=" << second_part;
    }
    else
    {
        size = CTcpCode::ConvertInt32(cur_data);
        DLOG(INFO) << "normal parse pkg size, size=" << size;
    }
    
    DLOG(INFO) << "before pop info, front=" << front << ", rear=" << rear_ << ", datasize=" << size << ", mqsize=" << size_;

    // 消息包长度检查
    if(size > size_)
    {
        LOG(ERROR) << "pop invalid pkg, size=" << size << ", size_=" << size_;
        return -1;
    }

    if(rear_ < front)
    {
        int pos = rear_ + size;
        if(pos > front)
        {
            LOG(ERROR) << "pop beyond data area";
            return -1;
        }
        else
        {
            memcpy(data, cur_data, size);
            rear_ = pos;
            DLOG(INFO) << "pop msg ok, size=" << size << ", front=" << front << ", rear=" << rear_;
        }
    }
    else
    {
        int pos = rear_ + size;
        // 检查该消息是否分两段存储
        if(pos <= size_)
        {
            memcpy(data, cur_data, size);
            rear_ = pos;
            DLOG(INFO) << "pop msg ok, size=" << size << ", front=" << front << ", rear=" << rear_;
        }
        else
        {
            pos %= size_;
            if(pos > front)
            {
                LOG(ERROR) << "pop beyond data area";
                return -1;
            }
            
            // 分两段存储
            int rest = size_ - rear_;
            memcpy(data, cur_data, rest);
            memcpy(data+rest, data_begin, pos);
            rear_ = pos;
            DLOG(INFO) << "pop msg ok from both side, size=" << size << ", front=" << front << ", rear=" << rear_ << ", rest=" << rest;
        }
    }

    --count_;
    DLOG(INFO) << "pop msg over, count=" << count_;
    
    return 0;
}

