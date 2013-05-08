/**
 *  @file   MyShmMgr.cpp
 *  @brief   �����ڴ����
 *  @author   stanjiang
 *  @date   2010-06-19
*/

#include	<sys/ipc.h>
#include	<sys/shm.h>
#include <algorithm>
#include "MyShmMgr.h"

using namespace google;

CMyShmMgr::CMyShmMgr()
{
}

CMyShmMgr::~CMyShmMgr()
{
}

void* CMyShmMgr::CreateShm(int nShmKey, int nShmSize, int nAssignSize)
{
    // �����Ƿ��Ѿ������и�key�Ĺ����ڴ�,���������ڸù����ڴ��Ϸ���ռ�
    SShmCreateInfo* pShmInfo = FindShmCreateInfo(nShmKey);
    if(NULL != pShmInfo)
    {
        // ��鵱ǰ�����ڴ��Ƿ��㹻����
        if((pShmInfo->m_nOffset + nAssignSize) > pShmInfo->m_nSize)
        {
            LOG(ERROR) << "shmmem isn't enough to assign: key=" << nShmKey << ", shmSize=" 
                << pShmInfo->m_nSize << ", assignSize=" << nAssignSize;
            return NULL;
        }

        void* pAssignShmMemAddr = (void*)((char*)pShmInfo->m_pMemAddr + pShmInfo->m_nOffset);
        pShmInfo->m_nOffset += nAssignSize;

        LOG(ERROR) << "shmmem isn't enough to assign: key=" << nShmKey << ", shmSize=" 
            << pShmInfo->m_nSize << ", assignSize=" << nAssignSize;

        return pAssignShmMemAddr;
    }

    // ȫ�´���key����Ӧ�Ĺ����ڴ�
    char* pShmMem = NULL;
    ENMShmMode enmStartMode = ESM_Init;
    int nShmID = shmget(nShmKey, nShmSize, IPC_CREAT|IPC_EXCL|0666);
    if(nShmID < 0)
    {
        if(errno != EEXIST)
        {
            LOG(ERROR) << "Can't create SHM , key=" << nShmKey << ", size=" << nShmSize << ", ErrMsg=" << strerror(errno);
            return NULL;
        }

        // �����ڴ��Ѿ�����
        enmStartMode = ESM_Resume;
        nShmID = shmget(nShmKey, nShmSize, 0666);
        if(-1 == nShmID)
        {
            LOG(ERROR) << "shmget get exist shm error, key=" << nShmKey << ", size=" << nShmSize;
            return NULL;
        }
    }

    pShmMem = (char*)shmat(nShmID, NULL, 0);
    if((NULL == pShmMem) || (-1 == *pShmMem))
    {
        LOG(ERROR) << "shmat error, key=" << nShmKey << ", size=" << nShmSize;
        return NULL;
    }

    // ����ɹ������Ĺ����ڴ���Ϣ
    SShmCreateInfo* pShmCreateMgr = new SShmCreateInfo;
    pShmCreateMgr->m_enmStartMode = enmStartMode;
    pShmCreateMgr->m_nKey = nShmKey;
    pShmCreateMgr->m_nSize = nShmSize;
    pShmCreateMgr->m_nShmID = nShmID;
    pShmCreateMgr->m_pMemAddr= (void*)pShmMem;
    pShmCreateMgr->m_nOffset = 0;
    m_vecShmCreateMgr.push_back(pShmCreateMgr);

    DLOG(INFO) << "shmmem create ok: key=" << nShmKey << ", shmSize=" << nShmSize << ", assignSize="
        << nAssignSize << ", shmBlockNum=" << m_vecShmCreateMgr.size();

    return (void*)pShmMem;
}


int CMyShmMgr::DestroyShm(int nShmKey)
{
    SShmCreateInfo* pShmInfo = NULL;
    if((pShmInfo = FindShmCreateInfo(nShmKey)) != NULL)
    {
        shmctl(pShmInfo->m_nShmID, IPC_RMID, NULL);

        remove(m_vecShmCreateMgr.begin(), m_vecShmCreateMgr.end(), pShmInfo);
        delete pShmInfo;
    }

    return -1;
}


