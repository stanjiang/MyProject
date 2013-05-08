/**
 *  @file   TcpCode.h
 *  @brief   Ӧ��protobuf��CS��SSͨ�Ű����мӽ���
 *  @author   stanjiang
 *  @date   2012-04-27
*/
#ifndef _ZONE_TCPCODE_H_
#define _ZONE_TCPCODE_H_

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <string>
#include <arpa/inet.h>

/********************proto�����ʽ˵��********************/
// ���ܳ�+protobuf  message name����+message name+protobuf data
// ע:��message name��Ϊ��Ϣ������
// ע:�����Զ�������cmdid��ver
/****************************************************************/

class CTcpCode
{
public:
    CTcpCode(){}
    ~CTcpCode(){}

    /***
     *  @brief   protobuf��Ϣ���
     *  @param   message: �������protobuf message
     *  @return   ������string��
     ***/
    static std::string Encode(const google::protobuf::Message& message);

    /***
     *  @brief   protobuf��Ϣ���
     *  @param   buf: ���������Ϣ��
     *  @return   ������protobuf message
     ***/
    static google::protobuf::Message* Decode(const std::string& buf);

    /***
     *  @brief   ����protobuf message typename����message
     *  @param   type_name: protobuf message typename
     *  @return   protobuf message
     ***/
    static google::protobuf::Message* CreateMessage(const std::string& type_name);

    /***
     *  @brief   ����Ϣ��ǰ�ĸ��ֽ�ת��Ϊ�����ֽ����int����
     *  @param   buf: ���������Ϣ��
     *  @return   ת�����int����
     ***/
    static int ConvertInt32(const char* buf);

private:

};


#endif

