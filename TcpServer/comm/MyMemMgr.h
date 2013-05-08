/**
*  @file   MyMemMgr.h
*  @brief   ϵͳͳһ�ڴ�����ڴ���а����̶���С���ȵ��ڴ�飬
*               ��ÿ���ڴ������˹̶������ʹ�С��ȵ��ڴ浥Ԫ
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

// �ڴ����ÿ����Ԫ�������
enum MemBlockType
{
    EMBT_HOUSE = 0, // סլ�������������
    EMBT_BUSINESS, // ��ҵ���������

    EMBT_MAX
};


// �ڴ��ͷ�ڵ�
struct tagMemBlockHead
{
    unsigned int uiBlockUnitSize;
    unsigned int uiBlockUnitNum;
    MemBlockType eBlockType;
    unsigned long long ullOffset; // �ڴ�����ڴ���е�ƫ����
    unsigned int uiUsedNum;
    int iQueueFront; // �ڴ���ö�������֯����Ϊ����ͷ
    int iQueueTail; // ����β
    unsigned int* pUnitIndex; // �ڴ浥λ����������
    char* pUnitUsedFlag; // �ڴ浥λʹ���������
    char* pData; // �ڴ����������ʼ��ַ
};


class CMyMemoryBlock
{
public:
    CMyMemoryBlock() {}
    ~CMyMemoryBlock() {}

    /***
    *  @brief   ��ʼ���ڴ��
    *  @param   void
    *  @return  0: ����ɹ���-1������ʧ��
    ***/
    int Init(MemBlockType eBlockType,unsigned int uiBlockUnitSize, unsigned int uiBlockUnitNum, unsigned long long ullOffset);

    /***
    *  @brief   ��ȡָ�������ڴ����е��ڴ浥λ
    *  @param   uiMemUnitIndex: �ڴ浥λ����
    *  @return  �����·�����ڴ浥λ��ַ,nullΪʧ��
    ***/
    char* GetFreeObj(unsigned int& uiMemUnitIndex);

    /***
    *  @brief   �ͷ�ָ�����͵��ڴ����ڴ浥λ
    *  @param   uiMemUnitIndex: �ڴ浥λ����
    *  @return  0: �ɹ���-1��ʧ��
    ***/
    int ReleaseObj(unsigned int uiMemUnitIndex);

    /***
    *  @brief   ��ȡָ�������ڴ���Ѵ��ڵ��ڴ浥λ
    *  @param   uiMemUnitIndex: �ڴ浥λ����
    *  @return  �����ڴ浥λ��ַ,nullΪʧ��
    ***/
    char* GetObj(unsigned int uiMemUnitIndex);

    /***
    *  @brief   �ж��ڴ�����Ϊ��
    *  @param   void
    *  @return  true:ok ,false:error
    ***/
    bool IsEmpty(void);

    /***
    *  @brief   �ж��ڴ���������
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


// �ڴ�����ڴ��Ļ�����Ϣ
struct MemBlockInfo
{
    CMyMemoryBlock blockObj;  // �ڴ�����
    unsigned long long ullBlockOffset; // �ڴ��������ڴ�صĵ�ַƫ��
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
    *  @brief   ��ʼ���ڴ��
    *  @param   ullPoolSize: �ڴ�ش�С
    *  @return  0: ����ɹ���-1������ʧ��
    ***/
    int Init(void);

    /***
    *  @brief   �����ڴ����ÿ��
    *  @param   eBlockType: �ڴ������
    *  @param   uiBlockUnitSize: �ڴ����ÿ����λ�Ĵ�С
    *  @param   uiBlockUnitNum: �ڴ���еĵ�λ����
    *  @return  0: ����ɹ���-1������ʧ��
    ***/
    int Alloc(MemBlockType eBlockType, unsigned int uiBlockUnitSize, unsigned int uiBlockUnitNum);

    /***
    *  @brief   ��ȡָ�������ڴ����е��ڴ浥λ
    *  @param   eBlockType: �ڴ������
    *  @param   uiMemUnitIndex: �ڴ浥λ����
    *  @return  �����·�����ڴ浥λ��ַ,nullΪʧ��
    ***/
    char* GetFreeObj(MemBlockType eBlockType, unsigned int& uiMemUnitIndex);

    /***
    *  @brief   �ͷ�ָ�����͵��ڴ����ڴ浥λ
    *  @param   eBlockType: �ڴ������
    *  @param   uiMemUnitIndex: �ڴ浥λ����
    *  @return  0: �ɹ���-1��ʧ��
    ***/
    int ReleaseObj(MemBlockType eBlockType, unsigned int uiMemUnitIndex);

    /***
    *  @brief   ��ȡָ�������ڴ���Ѵ��ڵ��ڴ浥λ
    *  @param   eBlockType: �ڴ������
    *  @param   uiMemUnitIndex: �ڴ浥λ����
    *  @return  �����ڴ浥λ��ַ,nullΪʧ��
    ***/
    char* GetObj(MemBlockType eBlockType, unsigned int uiMemUnitIndex);

    /***
    *  @brief   ��ȡ�ڴ����ʼ��ַ
    *  @param   void
    *  @return  �ڴ����ʼ��ַ
    ***/
    char* GetMemPoolBaseAttr(void)
    {
        return m_pMemPoolBaseAttr;
    }

    /***
     *  @brief   ��ȡ��Ҷ����ͷָ��
     *  @param   void
     *  @return   ͷָ��
     ***/
    tagMemBlockHead* GetPoolObjHead(MemBlockType type);


private:
    CMyMemoryPool() {}
    CMyMemoryPool(const CMyMemoryPool&);
    CMyMemoryPool& operator=(const CMyMemoryPool&);

private:
    unsigned long long m_ullPoolSize; // �ڴ���ܳߴ�
    unsigned long long m_ullFreeSize; // �ڴ�ؿ��гߴ�
    unsigned long long m_ullCurOffset; // ��ǰ�ڴ��ƫ��λ��
    unsigned short m_usBlockNum; // �ڴ�����
    MemBlockInfo m_stBlockInfo[EMBT_MAX];
    char* m_pMemPoolBaseAttr; // �ڴ�ص���ʼ��ַ

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
