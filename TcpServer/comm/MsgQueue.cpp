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
    int rear = rear_; // ����rear_����,�Է��Զ˽����ڴ��ڼ��޸�

    DLOG(INFO) << "before push info, front=" << front_ << ", rear=" << rear <<", datasize=" << size << ", mqsize=" << size_;

    // ����ָ���ڶ�βǰ������
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
                // ��������
                LOG(ERROR) << "mq is full, pos=" << pos << ", front=" << front_<< ", rear=" << rear << ", count=" << count_;
                return -1;
            }
            else
            {
                // �����ο���,ֻ�п��������,���޸�front_����
                int rest =  size_ - front_;
                memcpy(cur_data, data, rest);
                memcpy(data_begin, data+rest, pos); // ��MQ������ʼλ�ÿ�ʼ����
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
            // ��������
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
    int front = front_; // ���渱��,�Է�push�����ڴ��ڼ��޸ĸ�ֵ

    // �����п�(�пռ���������ǰ��)
    if(rear_ == front)
    {
        DLOG(INFO) << "pop mq is empty";
        return -1;
    }

    // ��Ҫ������β����4�ֽڵ����,����ų��ȵ��ֶ���һ�����ڶ���β,��һ�����ڶ���ͷ
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

    // ��Ϣ�����ȼ��
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
        // ������Ϣ�Ƿ�����δ洢
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
            
            // �����δ洢
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

