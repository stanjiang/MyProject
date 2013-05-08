/**
 *  @file   MyShmMgr.h
 *  @brief   �����ڴ����
 *  @author   stanjiang
 *  @date   2010-06-19
*/
#ifndef _MYGAME_MYSHMMGR_H_
#define _MYGAME_MYSHMMGR_H_

#include <vector>
#include "TcpComm.h"

using std::vector;

// �����ڴ������Ϣ
struct SShmCreateInfo
{
    int m_nKey; //�����ڴ�key
    int m_nSize; //�����ڴ��С
    int m_nShmID; // ���ɵĹ����ڴ�ID
    ENMShmMode m_enmStartMode;  //����ģʽ
    void* m_pMemAddr; //�����ڴ��׵�ַ
    int m_nOffset; //��ǰ�����ڴ��ѷ����ƫ��
};

class CMyShmMgr
{
public:
    ~CMyShmMgr();
    static CMyShmMgr& Instance(void)
    {
        static CMyShmMgr shm;
        return shm;
    }

    /***
     *  @brief   ����ϵͳ�����ڴ�
     *  @param   nShmKey: �����ڴ�key
     *  @param   nShmSize: �����ڴ�ߴ��С
     *  @param   nAssignSize: �������������С
     *  @return   ���ص���������Ĺ����ڴ��ַ
     ***/
    void* CreateShm(int nShmKey, int nShmSize, int nAssignSize);

    /***
     *  @brief   ɾ�������ڴ�
     *  @param   nShmKey: �����ڴ�key
     *  @return   0:ok, -1: error
     ***/
    int DestroyShm(int nShmKey);

    /***
     *  @brief   ��ȡ�����ڴ洴��ģʽ
     *  @param   nShmKey: �����ڴ�key
     *  @return   ���ص���������Ĺ����ڴ��ַ
     ***/
    ENMShmMode GetShmMode(int nShmKey);

private:
    CMyShmMgr();
    CMyShmMgr(const CMyShmMgr&);
    CMyShmMgr& operator=(const CMyShmMgr&);

    /***
     *  @brief   ���ҹ����ڴ洴����Ϣ
     *  @param   nShmKey: �����ڴ�key
     *  @return   ���ع����ڴ洴����Ϣ��ַ
     ***/
    SShmCreateInfo* FindShmCreateInfo(int nShmKey);

private:
    vector<SShmCreateInfo*> m_vecShmCreateMgr; // �����ڴ洴����Ϣ

};

inline SShmCreateInfo* CMyShmMgr::FindShmCreateInfo(int nShmKey)
{
    vector<SShmCreateInfo*>::iterator it = m_vecShmCreateMgr.begin();
    for(; it != m_vecShmCreateMgr.end(); ++it)
    {
        if((*it)->m_nKey == nShmKey)
        {
            DLOG(INFO) << "CMyShmMgr::FindShmCreateInfo: find exist key info: key=" << nShmKey;
            return *it;
        }
    }

    return NULL;
}

inline ENMShmMode CMyShmMgr::GetShmMode(int nShmKey)
{
    SShmCreateInfo* pMemInfo = NULL;
    if((pMemInfo = FindShmCreateInfo(nShmKey)) != NULL)
    {
        return pMemInfo->m_enmStartMode;
    }

    return ESM_None;
}

#endif

