/**
*  @file   MyMemMgr.h
*  @brief   系统统一内存管理，内存池中包含固定大小不等的内存块，
*               而每个内存块包含了固定数量和大小相等的内存单元
*  @author   stanjiang
*  @date   2010-09-24
*/
#ifndef _MYGAME_MYMEMMGR_H_
#define _MYGAME_MYMEMMGR_H_

#include <new>
#include "TcpComm.h"

enum MemBlockUseFlag
{
    EMBU_FREE = 0,
    EMBU_USED,
    EMBU_MAX
};

// 内存池中每个单元块的类型
enum MemBlockType
{
    EMBT_HOUSE = 0, // 住宅建筑对象池类型
    EMBT_BUSINESS, // 商业建筑对象池

    EMBT_MAX
};


// 内存块头节点
struct tagMemBlockHead
{
    unsigned int uiBlockUnitSize;
    unsigned int uiBlockUnitNum;
    MemBlockType eBlockType;
    unsigned long long ullOffset; // 内存块在内存池中的偏移量
    unsigned int uiUsedNum;
    int iQueueFront; // 内存块用队列来组织，此为队列头
    int iQueueTail; // 队列尾
    unsigned int* pUnitIndex; // 内存单位索引号数组
    char* pUnitUsedFlag; // 内存单位使用情况数组
    char* pData; // 内存块存放数据起始地址
};


class CMyMemoryBlock
{
public:
    CMyMemoryBlock() {}
    ~CMyMemoryBlock() {}

    /***
    *  @brief   初始化内存块
    *  @param   void
    *  @return  0: 分配成功，-1：分配失败
    ***/
    int Init(MemBlockType eBlockType,unsigned int uiBlockUnitSize, unsigned int uiBlockUnitNum, unsigned long long ullOffset);

    /***
    *  @brief   获取指定类型内存块空闲的内存单位
    *  @param   uiMemUnitIndex: 内存单位索引
    *  @return  返回新分配的内存单位地址,null为失败
    ***/
    char* GetFreeObj(unsigned int& uiMemUnitIndex);

    /***
    *  @brief   释放指定类型的内存块的内存单位
    *  @param   uiMemUnitIndex: 内存单位索引
    *  @return  0: 成功，-1：失败
    ***/
    int ReleaseObj(unsigned int uiMemUnitIndex);

    /***
    *  @brief   获取指定类型内存块已存在的内存单位
    *  @param   uiMemUnitIndex: 内存单位索引
    *  @return  返回内存单位地址,null为失败
    ***/
    char* GetObj(unsigned int uiMemUnitIndex);

    /***
    *  @brief   判定内存块队列为空
    *  @param   void
    *  @return  true:ok ,false:error
    ***/
    bool IsEmpty(void);

    /***
    *  @brief   判定内存块队列已满
    *  @param   void
    *  @return  true:ok ,false:error
    ***/
    bool IsFull(void);

private:
    friend class CMyMemoryPool;
    tagMemBlockHead* m_pMemBlockHead;
};

inline bool CMyMemoryBlock::IsEmpty()
{
    if(NULL == m_pMemBlockHead)
    {
        LOG(ERROR) << "block head is null";
        return false;
    }

    if(m_pMemBlockHead->iQueueFront == m_pMemBlockHead->iQueueTail)
    {
        return true;
    }

    return false;
}

inline bool CMyMemoryBlock::IsFull()
{
    if(NULL == m_pMemBlockHead)
    {
        LOG(ERROR) << "block head is null";
        return false;
    }

    if(0 == m_pMemBlockHead->uiBlockUnitNum)
    {
        LOG(ERROR) << "mempool is broken";
        return false;
    }
    int iNextIndex = (m_pMemBlockHead->iQueueFront + 1)% m_pMemBlockHead->uiBlockUnitNum;
    if(iNextIndex == m_pMemBlockHead->iQueueTail)
    {
        return true;
    }

    return false;
}


// 内存池中内存块的基本信息
struct MemBlockInfo
{
    CMyMemoryBlock blockObj;  // 内存块对象
    unsigned long long ullBlockOffset; // 内存块相对于内存池的地址偏移
};

class CMyMemoryPool
{
public:
    static CMyMemoryPool& Instance(void)
    {
        static CMyMemoryPool s_pool;
        return s_pool;
    }

    ~CMyMemoryPool();

    /***
    *  @brief   初始化内存池
    *  @param   ullPoolSize: 内存池大小
    *  @return  0: 分配成功，-1：分配失败
    ***/
    int Init(void);

    /***
    *  @brief   分配内存池中每个
    *  @param   eBlockType: 内存块类型
    *  @param   uiBlockUnitSize: 内存块中每个单位的大小
    *  @param   uiBlockUnitNum: 内存块中的单位数量
    *  @return  0: 分配成功，-1：分配失败
    ***/
    int Alloc(MemBlockType eBlockType, unsigned int uiBlockUnitSize, unsigned int uiBlockUnitNum);

    /***
    *  @brief   获取指定类型内存块空闲的内存单位
    *  @param   eBlockType: 内存块类型
    *  @param   uiMemUnitIndex: 内存单位索引
    *  @return  返回新分配的内存单位地址,null为失败
    ***/
    char* GetFreeObj(MemBlockType eBlockType, unsigned int& uiMemUnitIndex);

    /***
    *  @brief   释放指定类型的内存块的内存单位
    *  @param   eBlockType: 内存块类型
    *  @param   uiMemUnitIndex: 内存单位索引
    *  @return  0: 成功，-1：失败
    ***/
    int ReleaseObj(MemBlockType eBlockType, unsigned int uiMemUnitIndex);

    /***
    *  @brief   获取指定类型内存块已存在的内存单位
    *  @param   eBlockType: 内存块类型
    *  @param   uiMemUnitIndex: 内存单位索引
    *  @return  返回内存单位地址,null为失败
    ***/
    char* GetObj(MemBlockType eBlockType, unsigned int uiMemUnitIndex);

    /***
    *  @brief   获取内存池起始地址
    *  @param   void
    *  @return  内存池起始地址
    ***/
    char* GetMemPoolBaseAttr(void)
    {
        return m_pMemPoolBaseAttr;
    }

    /***
     *  @brief   获取玩家对象池头指针
     *  @param   void
     *  @return   头指针
     ***/
    tagMemBlockHead* GetPoolObjHead(MemBlockType type);


private:
    CMyMemoryPool() {}
    CMyMemoryPool(const CMyMemoryPool&);
    CMyMemoryPool& operator=(const CMyMemoryPool&);

private:
    unsigned long long m_ullPoolSize; // 内存池总尺寸
    unsigned long long m_ullFreeSize; // 内存池空闲尺寸
    unsigned long long m_ullCurOffset; // 当前内存池偏移位置
    unsigned short m_usBlockNum; // 内存块个数
    MemBlockInfo m_stBlockInfo[EMBT_MAX];
    char* m_pMemPoolBaseAttr; // 内存池的起始地址

};


inline char* CMyMemoryPool::GetFreeObj(MemBlockType eBlockType, unsigned int &uiMemUnitIndex)
{
    return m_stBlockInfo[eBlockType].blockObj.GetFreeObj(uiMemUnitIndex);
}

inline int CMyMemoryPool::ReleaseObj(MemBlockType eBlockType, unsigned int uiMemUnitIndex)
{
    return m_stBlockInfo[eBlockType].blockObj.ReleaseObj(uiMemUnitIndex);
}

inline char* CMyMemoryPool::GetObj(MemBlockType eBlockType, unsigned int uiMemUnitIndex)
{
    return m_stBlockInfo[eBlockType].blockObj.GetObj(uiMemUnitIndex);
}

inline tagMemBlockHead* CMyMemoryPool::GetPoolObjHead(MemBlockType type)
{
    if(type >= EMBT_MAX)
    {
        LOG(ERROR) << "invalid type, type=" << type;
        return NULL;
    }

    return m_stBlockInfo[type].blockObj.m_pMemBlockHead;
}


#endif
