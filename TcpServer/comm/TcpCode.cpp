#include "TcpCode.h"
#include "TcpComm.h"

using namespace std;

std::string CTcpCode::Encode(const google::protobuf::Message& message)
{
    std::string result;    
    result.resize(PKGHEAD_FIELD_SIZE); //  Ԥ����Ϣͷ��ǰ�ĸ��ֽ�

    // �����Ϣ���ͳ��Ⱥ���Ϣ���;�������
    const std::string& type_name = message.GetTypeName();
    int name_len = static_cast<int>(type_name.size()+1);
    int be32 = ::htonl(name_len);
    result.append(reinterpret_cast<char*>(&be32), sizeof(be32));
    result.append(type_name.c_str(), name_len);

    // ��protobuf messageЭ�����������result��
    bool succeed = message.AppendToString(&result);
    if(succeed)
    {
        // ������Ϣ���ܳ���ֵ,���������Ϣͷ����
        int len = ::htonl(result.size());
        std::copy(reinterpret_cast<char*>(&len), reinterpret_cast<char*>(&len) + sizeof(len), result.begin());
        DLOG(INFO) << "encode message ok, name=" << type_name.c_str() << ", len=" << len;
    }
    else
    {
        LOG(ERROR) << "encode message error, name=" << type_name.c_str();
        result.clear();        
    }

    return result;
}

google::protobuf::Message* CTcpCode::Decode(const std::string& buf)
{
    google::protobuf::Message* result = NULL;
    int len = static_cast<int>(buf.size()); // message ��Ϣ���ܳ���
    DLOG(INFO) << "decode message info, pkglen=" << len;
    
    if(len >= 2*PKGHEAD_FIELD_SIZE)
    {
        int name_len = ConvertInt32(buf.c_str()+PKGHEAD_FIELD_SIZE);
        DLOG(INFO) << "decod message name len, len=" << name_len;
            
        if(name_len >= 2 && name_len <= len - 2*PKGHEAD_FIELD_SIZE)
        {
            std::string type_name(buf.begin() + 2*PKGHEAD_FIELD_SIZE, buf.begin() + 2*PKGHEAD_FIELD_SIZE + name_len-1);
            google::protobuf::Message* message = CreateMessage(type_name);
            if(message != NULL)
            {
                const char* data = buf.c_str() + 2*PKGHEAD_FIELD_SIZE + name_len;
                int data_len = len - name_len - 2*PKGHEAD_FIELD_SIZE;
                if(message->ParseFromArray(data, data_len))
                {
                    result = message;
                    DLOG(INFO) << "parse message ok, name=" << type_name.c_str() << ", len=" << len;
                }
                else
                {
                    // protobuf message��������
                    LOG(ERROR) << "parse message error, name=" << type_name.c_str();
                    delete message;
                }                
            }
            else
            {
                LOG(ERROR) << "unknown message type!";
            }
        }
        else
        {
            LOG(ERROR) << "invalid message len, len=" << len << ", namelen=" << name_len;
        }
    }

    return result;
}

google::protobuf::Message* CTcpCode::CreateMessage(const std::string& type_name)
{
    google::protobuf::Message* message = NULL;
    const google::protobuf::Descriptor* descriptor =
        google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);
    if(descriptor != NULL)
    {
        const google::protobuf::Message* prototype =
            google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
        if(prototype != NULL)
        {
            message = prototype->New();
        }
    }
    return message;
}

int CTcpCode::ConvertInt32(const char* buf)
{
    int be32 = 0;
    ::memmove(&be32, buf, sizeof(be32));
    return ::ntohl(be32);
}


