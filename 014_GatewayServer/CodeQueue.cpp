#include "CodeQueue.hpp"
#include "LotusLogAdapter.hpp"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <assert.h>

#include "ErrorDef.hpp"
#include "SharedMemory.hpp"

using namespace DeprecatedLib;

CCodeQueue::CCodeQueue()
{
}

CCodeQueue::CCodeQueue(unsigned int uiMaxQueueLength)
{
    assert(uiMaxQueueLength > 0);

    m_aucQueue = NULL;
    m_uiMaxQueueLength = uiMaxQueueLength;
    m_shQueueAllocType = EQT_NEW_BY_SELF;
    m_iErrorNO = 0;
    m_aucQueue = new unsigned char[uiMaxQueueLength];
    assert(m_aucQueue);

    Initialize();
}

CCodeQueue::~CCodeQueue()
{
    if(m_shQueueAllocType == EQT_NEW_BY_SELF)
    {
        if(m_aucQueue)
        {
            delete []m_aucQueue;
            m_aucQueue = NULL;
        }
    }
}

CCodeQueue* CCodeQueue::CreateBySharedMemory(const char* pszKeyFileName, 
                                             const unsigned char ucKeyPrjID, 
                                             unsigned int uiMaxQueueLength)
{
    if(pszKeyFileName == NULL || uiMaxQueueLength <= 0)
    {
        return NULL;
    }

    CSharedMemory stCodeQueueShm;
    unsigned int uiSharedMemorySize = sizeof(CCodeQueue) + uiMaxQueueLength;

    int iRet = stCodeQueueShm.CreateShmSegment(pszKeyFileName, ucKeyPrjID, 
        (int)uiSharedMemorySize);

    if(iRet)
    {
        return NULL;
    }

    //�ڹ����ڴ�ĵ�ַ�Ϸ���CodeQueue
    CCodeQueue* pstCodeQueue = (CCodeQueue*)stCodeQueueShm.GetFreeMemoryAddress();

    if(!pstCodeQueue)
    {
        return NULL;
    }

    pstCodeQueue->m_uiMaxQueueLength = uiMaxQueueLength;
    pstCodeQueue->m_shQueueAllocType = EQT_ALLOC_BY_SHARED_MEMEORY;
    pstCodeQueue->m_aucQueue = NULL;
    //����CodeQueueƫ��
    pstCodeQueue->m_uiQueueOffset = sizeof(CCodeQueue);

    return pstCodeQueue;
}

int CCodeQueue::Initialize()
{
    m_iErrorNO = 0;

    Reset();

    return 0;
}

void CCodeQueue::Reset()
{
    m_uiBegOffset = 0;
    m_uiEndOffset = 0;
    m_bMidOffsetValid = false;
    m_uiMidOffset = 0;
    m_iCodeStatus = 0;
}

void CCodeQueue::SetStatus(int iStatus)
{
    m_iCodeStatus = iStatus;
}

int CCodeQueue::GetQueueBuf(unsigned char*& rucQueue) const
{
    if(m_shQueueAllocType == EQT_NEW_BY_SELF)
    {
        rucQueue = m_aucQueue;
    }
    else if(m_shQueueAllocType == EQT_ALLOC_BY_SHARED_MEMEORY)
    {
        rucQueue = (unsigned char*)this + m_uiQueueOffset;
    }

    return 0;
}

unsigned char* CCodeQueue::GetHeadCodeQueue() const
{
    unsigned char* pcQueueHead;
    GetQueueBuf(pcQueueHead);

    return pcQueueHead;
}

unsigned char* CCodeQueue::GetTailCodeQueue() const
{
    unsigned char* pcQueueHead;
    GetQueueBuf(pcQueueHead);

    return pcQueueHead + m_uiMaxQueueLength;
}

int CCodeQueue::PushOneCode(const unsigned char* pucInCode, int iCodeLength)
{
    if(pucInCode == NULL || iCodeLength <= 0)
    {
        SetErrorNO(EEN_CODEQUEUE__NULL_POINTER);

        return -1;
    }

    unsigned int uiBegOffset = 0;
    unsigned int uiEndOffset = 0;
    GetDataOffset(uiBegOffset, uiEndOffset);

    if(!IsDataOffsetValid(uiBegOffset) || !IsDataOffsetValid(uiEndOffset))
    {
        SetErrorNO(EEN_CODEQUEUE__DATA_OFFSET_INVALID);

        return -2;
    }

    unsigned int uiFreeLength = GetFreeLength(uiBegOffset, uiEndOffset);

    if(uiFreeLength > m_uiMaxQueueLength)
    {
        SetErrorNO(EEN_CODEQUEUE__FREE_LENGTH_INVALID);

        return -3;
    }

    //�Ƿ��������
    if(uiFreeLength < (unsigned int)iCodeLength + sizeof(int))
    {
        SetErrorNO(EEN_CODEQUEUE__QUEUE_IS_FULL);

        return -3;
    }

    unsigned int uiTailLength = m_uiMaxQueueLength - uiEndOffset;

    //���������Ҫ�ӽ�������βʱ���������һ�����룬��Ҫ�Ͽ�����MidOffset���´�ʱ��λ�ã�EndOffset��0��ʼ
    if(uiTailLength < (unsigned int)iCodeLength + sizeof(int))
    {
        //��ʱ�����ܻ���MidOffset
        if(m_bMidOffsetValid)
        {
            SetErrorNO(EEN_CODEQUEUE__MID_OFFSET_INVALID);

            return -4;
        }

        //��ʱ������BegOffset��EndOffset��
        if(uiBegOffset > uiEndOffset)
        {
            SetErrorNO(EEN_CODEQUEUE__DATA_OFFSET_INVALID);

            return -5;
        }

        //��ʱβ���Ŀ��л��������ã����г������¼���
        uiFreeLength = (int)uiBegOffset;

        //�Ƿ��������
        if(uiFreeLength < (unsigned int)iCodeLength + sizeof(int))
        {
            SetErrorNO(EEN_CODEQUEUE__QUEUE_IS_FULL);

            return -6;
        }

        EnableMidOffset(uiEndOffset);
        uiEndOffset = 0; //������ָ���0��ʼ
    }

    unsigned char* pucQueueBuf = NULL;
    GetQueueBuf(pucQueueBuf);

    if(pucQueueBuf == NULL)
    {
        SetErrorNO(EEN_CODEQUEUE__NULL_POINTER);

        return -7;
    }

    unsigned char* pucCodeOffset = &pucQueueBuf[uiEndOffset];

    //�ȿ���Length
    unsigned int i;
    for(i = 0; i < sizeof(int); ++i)
    {
        pucCodeOffset[i] = ((char*)&iCodeLength)[i];
    }

    uiEndOffset += sizeof(int);
    pucCodeOffset = &pucQueueBuf[uiEndOffset];

    //��������
    memcpy(pucCodeOffset, pucInCode, iCodeLength);
    uiEndOffset += (unsigned int)iCodeLength;

    //�����µ�Offset
    SetEndOffset(uiEndOffset);

    return 0;
}

int CCodeQueue::GetMidOffset(bool& rbMidOffsetValid, unsigned int& ruiMidOffset)
{
    rbMidOffsetValid = m_bMidOffsetValid;
    ruiMidOffset = m_uiMidOffset;

    return 0;
}

int CCodeQueue::GetOneCode(unsigned char*& rpucOutCode, int& riCodeLength)
{
    unsigned int uiBegOffset = 0;
    unsigned int uiEndOffset = 0;
    unsigned int uiMidOffset = 0;
    bool bMidOffsetValid = false;
    GetDataOffset(uiBegOffset, uiEndOffset);
    GetMidOffset(bMidOffsetValid, uiMidOffset);

    if(!IsDataOffsetValid(uiBegOffset) || !IsDataOffsetValid(uiEndOffset))
    {
        SetErrorNO(EEN_CODEQUEUE__DATA_OFFSET_INVALID);

        return -2;
    }

    unsigned int uiUsedLength = GetUsedLength(uiBegOffset, uiEndOffset, 
        bMidOffsetValid, uiMidOffset);

    if(uiUsedLength > m_uiMaxQueueLength)
    {
        SetErrorNO(EEN_CODEQUEUE__USED_LENGTH_INVALID);

        return -3;
    }

    //ľ�б���
    if(uiUsedLength == 0)
    {
        rpucOutCode = NULL;
        riCodeLength = 0;

        return 0;
    }

    if(uiUsedLength <= sizeof(int))
    {
        SetErrorNO(EEN_CODEQUEUE__USED_LENGTH_INVALID);

        return -4;
    }

    unsigned char* pucQueueBuf = NULL;
    GetQueueBuf(pucQueueBuf);

    if(pucQueueBuf == NULL)
    {
        SetErrorNO(EEN_CODEQUEUE__NULL_POINTER);

        return -7;
    }

    if(bMidOffsetValid && uiBegOffset == uiMidOffset)
    {
        uiBegOffset = 0;
    }

    int iCodeLength = 0;
    unsigned char* pucCodeOffset = &pucQueueBuf[uiBegOffset];

    unsigned int i;
    //��ȡLength
    for(i = 0; i < sizeof(int); i++)
    {
        ((char* )&iCodeLength)[i] = pucCodeOffset[i];
    }

    //�Ƿ���Length
    if(iCodeLength <= (int)sizeof(int) || 
        (unsigned int)iCodeLength > uiUsedLength - sizeof(int))
    {
        SetErrorNO(EEN_CODEQUEUE__CODE_LENGTH_INVALID);

        return -5;
    }

    uiBegOffset += sizeof(int);
    pucCodeOffset = &pucQueueBuf[uiBegOffset];
    rpucOutCode = pucCodeOffset;
    riCodeLength = iCodeLength;

    return 0;
}

int CCodeQueue::PopOneCode(unsigned char* pucOutCode, int iMaxOutCodeLen, int& riCodeLength)
{
    if(pucOutCode == NULL || iMaxOutCodeLen <= 0)
    {
        SetErrorNO(EEN_CODEQUEUE__NULL_POINTER);

        return -1;
    }

    unsigned int uiBegOffset = 0;
    unsigned int uiEndOffset = 0;
    unsigned int uiMidOffset = 0;
    bool bMidOffsetValid = false;
    GetDataOffset(uiBegOffset, uiEndOffset);
    GetMidOffset(bMidOffsetValid, uiMidOffset);

    if(!IsDataOffsetValid(uiBegOffset) || !IsDataOffsetValid(uiEndOffset))
    {
        SetErrorNO(EEN_CODEQUEUE__DATA_OFFSET_INVALID);

        return -2;
    }

    unsigned int uiUsedLength = GetUsedLength(uiBegOffset, uiEndOffset, 
        bMidOffsetValid, uiMidOffset);

    if(uiUsedLength > m_uiMaxQueueLength)
    {
        SetErrorNO(EEN_CODEQUEUE__USED_LENGTH_INVALID);

        return -3;
    }

    //ľ�б���
    if(uiUsedLength == 0)
    {
        riCodeLength = 0;

        return 0;
    }

    if(uiUsedLength <= sizeof(int))
    {
        SetErrorNO(EEN_CODEQUEUE__USED_LENGTH_INVALID);

        return -4;
    }

    unsigned char* pucQueueBuf = NULL;
    GetQueueBuf(pucQueueBuf);

    if(pucQueueBuf == NULL)
    {
        SetErrorNO(EEN_CODEQUEUE__NULL_POINTER);

        return -7;
    }

    if(bMidOffsetValid && uiBegOffset == uiMidOffset)
    {
        DisableMidOffset();
        uiBegOffset = 0;
    }

    int iCodeLength = 0;
    unsigned char* pucCodeOffset = &pucQueueBuf[uiBegOffset];

    unsigned int i;
    //��ȡLength
    for(i = 0; i < sizeof(int); i++)
    {
        ((char* )&iCodeLength)[i] = pucCodeOffset[i];
    }

    //�Ƿ���Length
    if(iCodeLength <= (int)sizeof(int) || 
        (unsigned int)iCodeLength > uiUsedLength - sizeof(int))
    {
        SetErrorNO(EEN_CODEQUEUE__CODE_LENGTH_INVALID);

        return -5;
    }

    //�������������
    if(iCodeLength > iMaxOutCodeLen)
    {
        SetErrorNO(EEN_CODEQUEUE__OUT_BUFFER_TOO_SMALL);

        return -6;
    }

    uiBegOffset += sizeof(int);
    pucCodeOffset = &pucQueueBuf[uiBegOffset];
    memcpy(pucOutCode, pucCodeOffset, iCodeLength);
    riCodeLength = iCodeLength;
    uiBegOffset += iCodeLength;

    //�����µ�Offset
    SetBeginOffset(uiBegOffset);

    return 0;
}

int CCodeQueue::EnableMidOffset(unsigned int uiMidOffset)
{
    m_uiMidOffset = uiMidOffset;
    m_bMidOffsetValid = true;

    return 0;
}

int CCodeQueue::DisableMidOffset()
{
    m_bMidOffsetValid = false;

    return 0;
}

int CCodeQueue::PopOneCode()
{
    unsigned int uiBegOffset = 0;
    unsigned int uiEndOffset = 0;
    unsigned int uiMidOffset = 0;
    bool bMidOffsetValid = false;
    GetDataOffset(uiBegOffset, uiEndOffset);
    GetMidOffset(bMidOffsetValid, uiMidOffset);

    if(!IsDataOffsetValid(uiBegOffset) || !IsDataOffsetValid(uiEndOffset))
    {
        SetErrorNO(EEN_CODEQUEUE__DATA_OFFSET_INVALID);

        return -2;
    }

    unsigned int uiUsedLength = GetUsedLength(uiBegOffset, uiEndOffset, 
        bMidOffsetValid, uiMidOffset);

    if(uiUsedLength > m_uiMaxQueueLength)
    {
        SetErrorNO(EEN_CODEQUEUE__USED_LENGTH_INVALID);

        return -3;
    }

    //ľ�б���
    if(uiUsedLength == 0)
    {
        return 0;
    }

    if(uiUsedLength <= sizeof(int))
    {
        SetErrorNO(EEN_CODEQUEUE__USED_LENGTH_INVALID);

        return -4;
    }

    unsigned char* pucQueueBuf = NULL;
    GetQueueBuf(pucQueueBuf);

    if(pucQueueBuf == NULL)
    {
        SetErrorNO(EEN_CODEQUEUE__NULL_POINTER);

        return -7;
    }

    if(bMidOffsetValid && uiBegOffset == uiMidOffset)
    {
        DisableMidOffset();
        uiBegOffset = 0;
    }

    int iCodeLength = 0;
    unsigned char* pucCodeOffset = &pucQueueBuf[uiBegOffset];

    unsigned int i;
    //��ȡLength
    for(i = 0; i < sizeof(int); i++)
    {
        ((char* )&iCodeLength)[i] = pucCodeOffset[i];
    }

    //�Ƿ���Length
    if(iCodeLength <= (int)sizeof(int) || 
        (unsigned int)iCodeLength > uiUsedLength - sizeof(int))
    {
        SetErrorNO(EEN_CODEQUEUE__CODE_LENGTH_INVALID);

        return -5;
    }

    uiBegOffset += sizeof(int);
    uiBegOffset += iCodeLength;

    //�����µ�Offset
    SetBeginOffset(uiBegOffset);

    return 0;
}

int CCodeQueue::GetDataOffset(unsigned int& ruiBegOffset, unsigned int& ruiEndOffset)
{
    ruiBegOffset = m_uiBegOffset;
    ruiEndOffset = m_uiEndOffset;

    return 0;
}

int CCodeQueue::SetBeginOffset(unsigned int uiBegOffset)
{
    if(!IsDataOffsetValid(uiBegOffset))
    {
        return -1;
    }

    m_uiBegOffset = uiBegOffset;

    return 0;
}

int CCodeQueue::SetEndOffset(unsigned int uiEndOffset)
{
    if(!IsDataOffsetValid(uiEndOffset))
    {
        return -1;
    }

    m_uiEndOffset = uiEndOffset;

    return 0;
}

bool CCodeQueue::IsQueueFull()
{
    return GetFreeLength() == 0;
}

bool CCodeQueue::IsDataOffsetValid(unsigned int uiDataOffset)
{
    if(uiDataOffset >= m_uiMaxQueueLength)
    {
        return false;
    }

    return true;
}

unsigned int CCodeQueue::GetFreeLength()
{
    return GetFreeLength(m_uiBegOffset, m_uiEndOffset);
}

unsigned int CCodeQueue::GetUsedLength()
{
    return GetUsedLength(m_uiBegOffset, m_uiEndOffset, m_bMidOffsetValid, m_uiMidOffset);
}

unsigned int CCodeQueue::GetFreeLength(unsigned int uiBegOffset, unsigned int uiEndOffset)
{
    unsigned uiFreeLength = 0;

    if(uiBegOffset == uiEndOffset)
    {
        uiFreeLength = m_uiMaxQueueLength;
    }
    else if(uiBegOffset < uiEndOffset)
    {
        uiFreeLength = m_uiMaxQueueLength - (uiEndOffset - uiBegOffset);
    }
    else
    {
        uiFreeLength = uiBegOffset - uiEndOffset;
    }

    if(uiFreeLength < (unsigned int)QUEUE_RESERVE_LENGTH)
    {
        return 0;
    }

    //��Ҫ����󳤶�Ӧ�ü�ȥԤ�����ֳ��ȣ���֤��β�������
    uiFreeLength -= QUEUE_RESERVE_LENGTH;

    return uiFreeLength;
}

unsigned int CCodeQueue::GetUsedLength(unsigned int uiBegOffset, unsigned int uiEndOffset, 
                                       bool bMidOffsetValid, unsigned int uiMidOffset)
{
    unsigned int uiUsedLength = 0;

    if(uiBegOffset == uiEndOffset)
    {
        uiUsedLength = 0;
    }
    else if(uiBegOffset < uiEndOffset)
    {
        uiUsedLength = uiEndOffset - uiBegOffset;
    }
    else
    {
        uiUsedLength = m_uiMaxQueueLength - uiBegOffset + uiEndOffset;

        if(bMidOffsetValid)
        {
            uiUsedLength -= (m_uiMaxQueueLength - uiMidOffset);
        }
    }

    return uiUsedLength;
}


