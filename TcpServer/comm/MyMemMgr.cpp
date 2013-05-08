#include "MyMemMgr.h"
#include "MyShmMgr.h"

CMyMemoryPool::~CMyMemoryPool()
{

}



int CMyMemoryPool::Init(void)
{
    int nShmKey = 111;
    int nShmSize = 128;
    int nAssignSize = nShmSize;
    m_pMemPoolBaseAttr = (char*)CMyShmMgr::Instance().CreateShm(nShmKey, nShmSize, nAssignSize);
    if(NULL == m_pMemPoolBaseAttr)
    {
        return -1;
    }

    m_ullPoolSize = nShmSize;
    m_ullFreeSize = m_ullPoolSize;
    m_usBlockNum = 0;
    m_ullCurOffset = 0;

    DLOG(INFO) << "mempool init ok: size=" << nShmSize << ", blocknum=" << m_usBlockNum<< ", offset=" << m_ullCurOffset;

    return 0;
}

int CMyMemoryPool::Alloc(MemBlockType eBlockType, unsigned int uiBlockUnitSize, unsigned int uiBlockUnitNum)
{
    if(eBlockType >= EMBT_MAX || eBlockType < 0)
    {
        return -1;
    }

    unsigned long long ullNeedSize = sizeof(tagMemBlockHead) + (sizeof(unsigned int) + sizeof(char) + uiBlockUnitSize)*uiBlockUnitNum;
    if(ullNeedSize > m_ullFreeSize)
    {
        LOG(ERROR) << "mempool not enough mem: need=" << ullNeedSize << ", free=" << m_ullFreeSize;
        return -1;
    }

    if(m_stBlockInfo[eBlockType].blockObj.Init(eBlockType, uiBlockUnitSize, uiBlockUnitNum, m_ullCurOffset) != 0)
    {
        return -1;
    }

    m_stBlockInfo[eBlockType].ullBlockOffset = m_ullCurOffset;
    m_ullCurOffset += ullNeedSize;
    m_ullFreeSize -= ullNeedSize;

    return 0;
}


int CMyMemoryBlock::Init(MemBlockType eBlockType,unsigned int uiBlockUnitSize, unsigned int uiBlockUnitNum, unsigned long long ullOffset)
{
    char* pMemBaseAttr = CMyMemoryPool::Instance().GetMemPoolBaseAttr();
    pMemBaseAttr += ullOffset;

    m_pMemBlockHead = (tagMemBlockHead*)pMemBaseAttr;
    if(NULL == m_pMemBlockHead)
    {
        return -1;
    }


    // 需要区分启动模式
    int nShmKey = 111;
    ENMShmMode mode = CMyShmMgr::Instance().GetShmMode(nShmKey);

    m_pMemBlockHead->pUnitIndex = (unsigned int*)(pMemBaseAttr + sizeof(tagMemBlockHead)); // 索引数组紧随内存块头结点之后
    m_pMemBlockHead->pUnitUsedFlag = pMemBaseAttr + sizeof(tagMemBlockHead) + sizeof(unsigned int)*uiBlockUnitNum;
    m_pMemBlockHead->pData = pMemBaseAttr + sizeof(tagMemBlockHead) + (sizeof(unsigned int) + sizeof(char))*uiBlockUnitNum; // 存放数据的起始地址

    if(ESM_Init == mode)
    {
        // 填充内存块头的基本信息
        m_pMemBlockHead->uiBlockUnitSize = uiBlockUnitSize;
        m_pMemBlockHead->uiBlockUnitNum = uiBlockUnitNum;
        m_pMemBlockHead->eBlockType = eBlockType;
        m_pMemBlockHead->ullOffset = ullOffset;
        m_pMemBlockHead->uiUsedNum = 0;
        m_pMemBlockHead->iQueueFront = 0;
        m_pMemBlockHead->iQueueTail = 0;

        // 初始化内存块索引数组，用作将来待分配对象的唯一标识
        for(unsigned int i = 0; i < uiBlockUnitNum; ++i)
        {
            m_pMemBlockHead->pUnitIndex[i] = i;
        }

        // 初始化内存块已使用标识数组
        memset(m_pMemBlockHead->pUnitUsedFlag, EMBU_FREE, uiBlockUnitNum);
    }
    else
    {
        // 必要的检查
        if(m_pMemBlockHead->uiBlockUnitSize != uiBlockUnitSize || m_pMemBlockHead->uiBlockUnitNum != uiBlockUnitNum\
                || m_pMemBlockHead->eBlockType != eBlockType)
        {
            LOG(ERROR) << "invalid shmmem, unitsize=" << m_pMemBlockHead->uiBlockUnitSize << ", num="
                                << m_pMemBlockHead->uiBlockUnitNum << ", type=" << m_pMemBlockHead->eBlockType;
            return -1;
        }
    }

    DLOG(INFO) << "init mempool, offset=" << ullOffset << ", head=" << sizeof(tagMemBlockHead) << ", mode=" << mode;

    return 0;
}

char* CMyMemoryBlock::GetFreeObj(unsigned int& uiMemUnitIndex)
{
    if(NULL == m_pMemBlockHead)
    {
        LOG(ERROR) << "block head is null";
        return NULL;
    }

    // 判定内存块队列是否已满
    if(IsFull())
    {
        LOG(ERROR) << "MemBlock is full: totolNum=" << m_pMemBlockHead->uiBlockUnitNum 
                            << ", usedNum=" << m_pMemBlockHead->uiUsedNum;
        return NULL;
    }

    // 获取待分配内存对象的索引号
    m_pMemBlockHead->iQueueFront = (m_pMemBlockHead->iQueueFront + 1) % m_pMemBlockHead->uiBlockUnitNum;
    uiMemUnitIndex = m_pMemBlockHead->pUnitIndex[m_pMemBlockHead->iQueueFront];

    m_pMemBlockHead->pUnitUsedFlag[uiMemUnitIndex] = EMBU_USED;
    ++m_pMemBlockHead->uiUsedNum;

    static unsigned int s_uiCount = 0;
    ++s_uiCount;

    DLOG(INFO) << "get free obj ok, type=" << m_pMemBlockHead->eBlockType << ", index=" << uiMemUnitIndex<< ", front="
           << m_pMemBlockHead->iQueueFront << ", usednum=" << m_pMemBlockHead->uiUsedNum << ", totalFreeObj=" << s_uiCount;

    return m_pMemBlockHead->pData+uiMemUnitIndex*m_pMemBlockHead->uiBlockUnitSize;
}

int CMyMemoryBlock::ReleaseObj(unsigned int uiMemUnitIndex)
{
    if(NULL == m_pMemBlockHead)
    {
        LOG(ERROR) << "block head is null";
        return -1;
    }

    if(uiMemUnitIndex >= m_pMemBlockHead->uiBlockUnitNum)
    {
        LOG(ERROR) << "MemUnitIndex beyond bound: curIndex=" << uiMemUnitIndex << ", maxIndex=" << m_pMemBlockHead->uiBlockUnitNum;
        return -1;
    }

    // 判定内存块队列是否为空
    if(IsEmpty())
    {
        LOG(ERROR) << "MemBlock is empty!";
        return -1;
    }

    // 计算存放当前释放对象索引号的队列位置
    m_pMemBlockHead->pUnitIndex[m_pMemBlockHead->iQueueTail] = uiMemUnitIndex;
    m_pMemBlockHead->iQueueTail = (m_pMemBlockHead->iQueueTail + 1) % m_pMemBlockHead->uiBlockUnitNum;
    m_pMemBlockHead->pUnitUsedFlag[uiMemUnitIndex] = EMBU_FREE;
    --m_pMemBlockHead->uiUsedNum;

    static unsigned int s_uiCount = 0;
    ++s_uiCount;

    DLOG(INFO) << "release obj ok, type=" << m_pMemBlockHead->eBlockType << ", index=" << uiMemUnitIndex 
           <<", tail=" << m_pMemBlockHead->iQueueTail << ", totalReleaseObj=" << s_uiCount;

    return 0;
}

char* CMyMemoryBlock::GetObj(unsigned int uiMemUnitIndex)
{
    if(NULL == m_pMemBlockHead)
    {
        LOG(ERROR) << "block head is null";
        return NULL;
    }

    if(uiMemUnitIndex >= m_pMemBlockHead->uiBlockUnitNum)
    {
        LOG(ERROR) << "MemUnitIndex beyond bound: curIndex=" << uiMemUnitIndex << ", maxIndex=" 
            << m_pMemBlockHead->uiBlockUnitNum;
        return NULL;
    }

    if(m_pMemBlockHead->pUnitUsedFlag[uiMemUnitIndex] != EMBU_USED)
    {
        LOG(ERROR) << "MemBlock use flag error: flag=" << m_pMemBlockHead->pUnitUsedFlag[uiMemUnitIndex]
            << ", index=" << uiMemUnitIndex;
        return NULL;
    }

    return m_pMemBlockHead->pData + uiMemUnitIndex*m_pMemBlockHead->uiBlockUnitSize;
}

