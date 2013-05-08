/**
 *  @file   MyShmMgr.cpp
 *  @brief   共享内存管理
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
    // 查找是否已经创建有该key的共享内存,若有则尝试在该共享内存上分配空间
    SShmCreateInfo* pShmInfo = FindShmCreateInfo(nShmKey);
    if(NULL != pShmInfo)
    {
        // 检查当前共享内存是否足够分配
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

    // 全新创建key所对应的共享内存
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

        // 共享内存已经创建
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

    // 保存成功创建的共享内存信息
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


