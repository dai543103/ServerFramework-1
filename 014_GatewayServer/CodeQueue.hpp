#ifndef __CODEQUEUE_HPP__
#define __CODEQUEUE_HPP__

#include "base.hpp"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const int QUEUE_RESERVE_LENGTH = 8; //��Ϣ����Ԥ�����ֵĳ��ȣ���ֹ��β���

typedef enum enmStatusFlag
{
    ESF_NORMAL_STATUS = 0, //!<����״̬
    ESF_DISORDER_STATUS = 1, //!<����״̬�������״̬��ʱ����п����Ѿ������ϲ㽨�鲻Ӧ����Pop��Push��Pop���������״̬��Push������״̬
} ENMRESETFLAG;

typedef enum enmQueueAllocType
{
    EQT_NEW_BY_SELF = 0, //!<ͨ��New��̬������
    EQT_ALLOC_BY_SHARED_MEMEORY = 1, //!<ͨ�������ڴ洴����
} ENMQUEUEALLOCTYPE;

class CCodeQueue
{
private:
    CCodeQueue();

public:
    /**
    *�ö�̬����������������У���Ҫָ�����г���
    *@param[in] uiMaxQueueLength �����Ķ��г���
    *@return 0 success
    */
    CCodeQueue(unsigned int uiMaxQueueLength);
    ~CCodeQueue();

    /**
    *ͨ�������ڴ�������CodeQueue��ע������������CodeQueue�����ʼ��������ı����ڹ����ڴ��ֵ��
    *@param[in] pszKeyFileName �����ڴ�Attach���ļ���
    *@param[in] ucKeyPrjID �����ڴ��ProjectID
    *@param[in] uiMaxQueueLength ���Ķ��г���
    *@return 0 success
    */
    static CCodeQueue* CreateBySharedMemory(const char* pszKeyFileName,
        const unsigned char ucKeyPrjID,
        unsigned int uiMaxQueueLength);

public:
    /**
    *��ʼ���������
    *@return 0 success
    */
    int Initialize();

    /**
    *����һ������
    *@param[in] pucInCode ����ı���ָ��
    *@param[in] iCodeLength ����ı��볤��
    *@return 0 success
    */
    int PushOneCode(const unsigned char* pucInCode, int iCodeLength);

    /**
    *ȡ��һ�����뿽����pucOutCode������ƶ��������ͷָ��
    *@param[in] pucOutCode ��ȡ���ı��뿽�������������
    *@param[in] iMaxOutCodeLen ��ű���Ļ�������󳤶�
    *#param[out] riCodeLength ȡ���ı���ʵ�ʳ���
    *@return 0 success
    */
    int PopOneCode(unsigned char* pucOutCode, int iMaxOutCodeLen, int& riCodeLength);

    /**
    *ȡ��һ�����룬����ƶ��������ͷָ��
    *@return 0 success
    */
    int PopOneCode();

    /**
    *��ȡһ�������ͷָ��
    *@param[out] rpucOutCode ��ȡ�ı���ͷָ��
    *@param[out] riCodeLength ȡ���ı���ʵ�ʳ���
    *@return 0 success
    */
    int GetOneCode(unsigned char*& rpucOutCode, int& riCodeLength);

    //!��ȡ�����������ͷָ��
    unsigned char* GetHeadCodeQueue() const;

    //!��ȡ�����������βָ��
    unsigned char* GetTailCodeQueue() const;

    /**
    *�жϱ�������Ƿ��Ѿ���
    *@return true ����
    */
    bool IsQueueFull();

    //!���ö��У���յ�ǰ���е�ͷָ���βָ�룬����Ϊ��ʼ״̬
    void Reset();

    //!����״̬
    void SetStatus(int iStatus);

    //!��ȡ����״̬
    int GetStatus() const { return m_iCodeStatus; }

    //!�ڽӿڷ��ش���ʱ���������������ȡ�����
    int GetErrorNO() const { return m_iErrorNO; }

    /**
    *��ȡ������е�ǰ���೤��
    *@return ���೤��
    */
    unsigned int GetFreeLength();

    /**
    *��ȡ������е�ǰ���ó���
    *@return ���ó���
    */
    unsigned int GetUsedLength();

private:
    //!���ô����
    void SetErrorNO(int iErrorNO) { m_iErrorNO = iErrorNO; }

    //!�ж�Offset�Ƿ�Ϸ�
    bool IsDataOffsetValid(unsigned int uiDataOffset);

    //!��ȡ��ǰ������ƫ��
    int GetDataOffset(unsigned int& ruiBegOffset, unsigned int& ruiEndOffset);

    //!��ȡCodeQueue��Bufferָ��
    int GetQueueBuf(unsigned char*& rucQueue) const;

    //!���ñ�����п�ʼƫ����
    int SetBeginOffset(unsigned int uiBegOffset);

    //!���ñ�����н���ƫ����
    int SetEndOffset(unsigned int uiEndOffset);

    //!��Pop��Push��ʱ�򶼵�������������㳤�ȣ�������������Ϊ����û�������ڶ���̵���ʱ��m_uiBegOffset��m_uiEndOffset��ʱ���ܱ仯
    unsigned int GetUsedLength(unsigned int uiBegOffset, unsigned int uiEndOffset,
        bool bMidOffsetValid, unsigned int uiMidOffset);

    //!��Pop��Push��ʱ�򶼵�������������㳤�ȣ�������������Ϊ����û�������ڶ���̵���ʱ��m_uiBegOffset��m_uiEndOffset��ʱ���ܱ仯
    unsigned int GetFreeLength(unsigned int uiBegOffset, unsigned int uiEndOffset);

    //!ʹ��MidOffset
    int EnableMidOffset(unsigned int uiMidOffset);

    //!��ʹ��MidOffset
    int DisableMidOffset();

    //!��ȡMidOffset
    int GetMidOffset(bool& rbMidOffsetValid, unsigned int& ruiMidOffset);

private:
    int m_iErrorNO; //!<������
    int m_iCodeStatus; //!<�������״̬
    short m_shQueueAllocType; //!<��������������
    unsigned char* m_aucQueue; //!<������л�����������ʹ�ö�̬����ʱʹ��
    size_t m_uiQueueOffset; //!<������л�����ƫ�ƣ������thisָ�룩����ʹ�ù����ڴ�ʱʹ��
    unsigned int m_uiMaxQueueLength; //!<���������󳤶�
    unsigned int m_uiBegOffset; //!<��ǰ���������ָ��ƫ�ƣ�Popһ�������ʱ���������ֵ
    unsigned int m_uiEndOffset; //!<��ǰ�������βָ��ƫ�ƣ�Pushһ�������ʱ���������ֵ
    bool m_bMidOffsetValid; //!<�ж�ƫ���Ƿ���Ч
    unsigned int m_uiMidOffset; //!<��m_uiEndOffset�ӽ�������β��ʱ������������һ�����룬����m_uiMidOffset������ʱ��m_uiEndOffset��m_uiEndOffset��0��ʼ
};

#endif


