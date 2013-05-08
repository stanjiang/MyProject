/**
 *  @file   MyShmMgr.h
 *  @brief   共享内存管理
 *  @author   stanjiang
 *  @date   2010-06-19
*/
#ifndef _MYGAME_MYSHMMGR_H_
#define _MYGAME_MYSHMMGR_H_

#include <vector>
#include "TcpComm.h"

using std::vector;

// 共享内存基本信息
struct SShmCreateInfo
{
    int m_nKey; //共享内存key
    int m_nSize; //共享内存大小
    int m_nShmID; // 生成的共享内存ID
    ENMShmMode m_enmStartMode;  //启动模式
    void* m_pMemAddr; //共享内存首地址
    int m_nOffset; //当前共享内存已分配的偏移
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
     *  @brief   创建系统共享内存
     *  @param   nShmKey: 共享内存key
     *  @param   nShmSize: 共享内存尺寸大小
     *  @param   nAssignSize: 本次申请所需大小
     *  @return   返回调用者申请的共享内存地址
     ***/
    void* CreateShm(int nShmKey, int nShmSize, int nAssignSize);

    /***
     *  @brief   删除共享内存
     *  @param   nShmKey: 共享内存key
     *  @return   0:ok, -1: error
     ***/
    int DestroyShm(int nShmKey);

    /***
     *  @brief   获取共享内存创建模式
     *  @param   nShmKey: 共享内存key
     *  @return   返回调用者申请的共享内存地址
     ***/
    ENMShmMode GetShmMode(int nShmKey);

private:
    CMyShmMgr();
    CMyShmMgr(const CMyShmMgr&);
    CMyShmMgr& operator=(const CMyShmMgr&);

    /***
     *  @brief   查找共享内存创建信息
     *  @param   nShmKey: 共享内存key
     *  @return   返回共享内存创建信息地址
     ***/
    SShmCreateInfo* FindShmCreateInfo(int nShmKey);

private:
    vector<SShmCreateInfo*> m_vecShmCreateMgr; // 共享内存创建信息

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

