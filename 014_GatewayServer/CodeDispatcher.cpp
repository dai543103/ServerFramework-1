#include "IOHandler.hpp"
#include "ConfigAssemble.hpp"
#include "CodeQueueAssemble.hpp"
#include "FDPool.hpp"
#include "CodeDispatcher.hpp"
#include "SocketOperator.hpp"
#include "LotusLogAdapter.hpp"
#include "BillWriter.hpp"

int CCodeDispatcher::Initialize()
{
    return 0;
}

int CCodeDispatcher::DispatchOneCode(const char* pszDispatchingBuffer, unsigned short ushCodeOffset,
                                     unsigned short ushCodeLength, int iSrcFD)
{
    int iRet = 0;

    TRACESVR(LOG_LEVEL_DETAIL, "*************************step 1*****************************\n");
    TRACESVR(LOG_LEVEL_DETAIL, "CCodeDispatcher::DispatchOneCode, CodeLength;%d CodeBin:\n", ushCodeLength);
    TRACEBIN(LOG_LEVEL_DETAIL, (char*)pszDispatchingBuffer + ushCodeOffset, ushCodeLength);

    TExternalClientSocket* pstExternalClientSocket = m_pstFDPool->GetExternalSocketByFD(iSrcFD);
    if(!pstExternalClientSocket)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "In CCodeDispatcher::DispatchOneCode GetExternalSocketByFD(%d) is NULL\n", iSrcFD);
        return -1;
    }

    // ���ط����������κ���֤��ֱ��ת������Ϸ�߼�������
    iRet = DispatchToDefault(pszDispatchingBuffer, ushCodeOffset, ushCodeLength, iSrcFD);
    if(iRet < 0)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "*************************step 2*****************************\n");
        TRACESVR(LOG_LEVEL_NOTICE, "In CCodeDispatcher::DispatchOneCode DispatchToDefault Ret:%d\n", iRet);
        TRACESVR(LOG_LEVEL_DETAIL, "*************************step end*****************************\n");

        return -6;
    }

    //��¼��ˮ
    if(m_pstConfigAssemble->GetLotusConfigMng().GetBillFlag() &  EBF_RECV_PACKET)
    {
        CBillWriter::WriteSocketBill(pstExternalClientSocket, EBEI_RECV_PACKET);
    }

    TRACESVR(LOG_LEVEL_ALERT, "connected: FD = %d\n", pstExternalClientSocket->m_iSocketFD);

    return 0;
}

int CCodeDispatcher::DispatchToDefault(const char* pszDispatchingBuffer, unsigned short ushCodeOffset,
                                       unsigned short ushCodeLength, int iSrcFD)
{
    int iRet = 0;

    TExternalClientSocket *pstExternalClientSocket = m_pstFDPool->GetExternalSocketByFD(iSrcFD);
    if (NULL == pstExternalClientSocket)
    {
        //TRACESVR(LOG_LEVEL_DETAIL, "In CCodeDispatcher::DispatchToDefault DispatchToDstServer NULL == pstExternalClientSocket \n");
        return -1;
    }

    //���͸�Ĭ��InternalSocket
    TInternalServerSocket* pDefaultSocket = m_pstFDPool->GetDefaultInternalSocket();
    if(pDefaultSocket)
    {
        iRet = DispatchToDstServer(pDefaultSocket->m_ushServerType, pDefaultSocket->m_ushServerID,
            pszDispatchingBuffer, ushCodeOffset, ushCodeLength, iSrcFD, pstExternalClientSocket->m_uiUin);
        //TRACESVR(LOG_LEVEL_DETAIL, "In CCodeDispatcher::DispatchToDefault DispatchToDstServer: iRet:%d\n", iRet);
        return iRet;
    }

    //û��Ĭ��InternalSocket�����͸�Ĭ��ҵ��CodeQueue
    char* pNetHeadBuffer = (char*)pszDispatchingBuffer + ushCodeOffset - NETHEAD_V2_SIZE;
    PackNetHead_V2(pNetHeadBuffer, iSrcFD, pstExternalClientSocket->m_uiUin);
    iRet = m_pstCodeQueueAssemble->PushToDefault(ushCodeLength + NETHEAD_V2_SIZE, pNetHeadBuffer,
        m_pstConfigAssemble->GetLotusConfigMng().IsRecordEnabled());

    return iRet;
}

int CCodeDispatcher::DispatchToCodeQueue(int iCodeQueueID, const char* pszDispatchingBuffer,
                                         unsigned short ushCodeOffset, unsigned short ushCodeLength,
                                         int iSrcFD, unsigned int uiRealUin)
{
    //const TCodeQueueConfig& rstCodeQConfig = m_pstConfigAssemble->GetLotusConfigMng().GetCodeQueueByID(iCodeQueueID);

    int iNetHeadLength = sizeof(TNetHead_V2);

    char* pNetHeadBuffer = (char*)pszDispatchingBuffer + ushCodeOffset - iNetHeadLength;

    // ֻ�������֧��֧�ֶ���NetHead
    PackNetHead_V2(pNetHeadBuffer, iSrcFD, uiRealUin);

    int iRet = 0;
    iRet = m_pstCodeQueueAssemble->PushToSpecific(iCodeQueueID, ushCodeLength + iNetHeadLength, pNetHeadBuffer,
        m_pstConfigAssemble->GetLotusConfigMng().IsRecordEnabled());

    return iRet;
}

int CCodeDispatcher::DispatchToDstServer(unsigned short ushServerType, unsigned short ushServerID,
                                         const char* pszDispatchingBuffer, unsigned short ushCodeOffset,
                                         unsigned short ushCodeLength, int iSrcFD, unsigned int uiRealUin)
{
    TInternalServerSocket* pstSocket = m_pstFDPool->GetInternalSocketByTypeAndID(ushServerType, ushServerID);
    if(!pstSocket)
    {
        //TRACESVR(LOG_LEVEL_DETAIL, "In CCodeDispatcher::DispatchToDstServer pstSocket is NULL\n");
        return -1;
    }

    int iRet = 0;

    // ����NetHeadVer��ִͬ�в�ͬPack
    int iNetHeadLength = sizeof(TNetHead_V2);

    char* pNetHeadBuffer = (char*)pszDispatchingBuffer + ushCodeOffset - iNetHeadLength;

    // ֻ�������֧��֧�ֶ���NetHead
    PackNetHead_V2(pNetHeadBuffer, iSrcFD, uiRealUin);

    unsigned short ushTotalLength = 0;
    ushTotalLength = ushCodeLength + iNetHeadLength + sizeof(ushTotalLength);
    char* pLengthBuffer = (char*)pszDispatchingBuffer + ushCodeOffset - iNetHeadLength - sizeof(ushTotalLength);
    *(unsigned short*)pLengthBuffer = htons(ushTotalLength);

    //��·
    if(pstSocket->m_iSocketFD < 0 || pstSocket->m_iSocketFD >= (int)FD_SIZE)
    {
        //TRACESVR(LOG_LEVEL_DETAIL, "In CCodeDispatcher::DispatchToDstServer SocketFD:%d is Not Valid\n", pstSocket->m_iSocketFD);
        iRet = -1;
    }
    else
    {
        iRet = SendToInternalServer(pstSocket, ushTotalLength, pLengthBuffer);
        if(iRet < 0)
        {
            m_pIOHandler->Reset(pstSocket->m_iSocketFD, SEND_TO_INTERNAL_SVR_FAIL);
            pstSocket->m_iSocketFD = -1;
        }
    }

    return iRet;
}


//-X��ʾʧ�� 0��ʾ�ɹ� 1��ʾbuff��
int CCodeDispatcher::SendToInternalServer(TInternalServerSocket* pstSocket, int iCodeLength, const char* pszCodeBuffer)
{
    if(!pstSocket)
    {
        //TRACESVR(LOG_LEVEL_DETAIL, "In CCodeDispatcher::SendToInternalServer pstSocket:NULL\n");
        return -1;
    }

    int iRet = 0;

    //��ʣ��Code
    if(pstSocket->m_uiSendEndOffset > 0)
    {
        iRet = SendRemain(pstSocket);
        if(iRet < 0)
        {
            return -2;
        }

        //ʣ�µ�Codeû�з���
        if(iRet > 0)
        {
            if(pstSocket->m_uiSendEndOffset + iCodeLength > sizeof(pstSocket->m_szSendBuffer))
            {
                //TRACESVR(LOG_LEVEL_DETAIL, "In CCodeDispatcher::SendToInternalServer Buffer is Full, SendEndOffset:%u CodeLength:%d\n",
                //  pstSocket->m_uiSendEndOffset, iCodeLength);
                return 1;
            }

            memcpy(&pstSocket->m_szSendBuffer[pstSocket->m_uiSendEndOffset], pszCodeBuffer, iCodeLength);
            pstSocket->m_uiSendEndOffset += iCodeLength;

            return 0;
        }
    }

    iRet = CSocketOperator::Send(pstSocket->m_iSocketFD, iCodeLength, pszCodeBuffer);
    if(iRet < 0)
    {
        return -3;
    }

    //����һ����Code
    if(iRet < iCodeLength)
    {
        //CodeLength��Ȼ����SendBuffer,����Ӧ�ò����ߵ�
        if( iCodeLength - iRet > (int)sizeof(pstSocket->m_szSendBuffer))
        {
            //TRACESVR(LOG_LEVEL_DETAIL, "In CCodeDispatcher::SendToInternalServer, iRet:%d CodeLength:%d\n", iRet, iCodeLength);
            return -4;
        }

        memcpy(&pstSocket->m_szSendBuffer[0], (void*)&pszCodeBuffer[iRet], iCodeLength - iRet);
        pstSocket->m_uiSendEndOffset = iCodeLength - iRet;
    }

    return 0;
}

//-X.��ʾ���� 0.��ʾ���� 1.��ʾ���˲���
int CCodeDispatcher::SendRemain(TInternalServerSocket* pstSocket)
{
    if(!pstSocket)
    {
        //TRACESVR(LOG_LEVEL_DETAIL, "In CCodeDispatcher::SendRemain pstSocket:NULL\n");
        return -1;
    }

    if(pstSocket->m_uiSendEndOffset == 0)
    {
        return 0;
    }

    int iRet = 0;
    iRet = CSocketOperator::Send(pstSocket->m_iSocketFD, pstSocket->m_uiSendEndOffset, &pstSocket->m_szSendBuffer[0]);
    if(iRet < 0)
    {
        //TRACESVR(LOG_LEVEL_DETAIL, "In CCodeDispatcher::SendRemain Send:%d\n", iRet);
        return -2;
    }

    //TRACESVR(LOG_LEVEL_DETAIL, "In CCodeDispatcher::SendRemain, iRet:%d SendEndOffset:%u\n",
    //  iRet, pstSocket->m_uiSendEndOffset);

    //����һ����ʣ�µ�Code
    if(iRet < (int)pstSocket->m_uiSendEndOffset)
    {
        pstSocket->m_uiSendEndOffset -= iRet;
        memmove(&pstSocket->m_szSendBuffer[0], &pstSocket->m_szSendBuffer[iRet], pstSocket->m_uiSendEndOffset);
        return 1;
    }

    //������ʣ�µ�Code
    pstSocket->m_uiSendEndOffset = 0;

    return 0;
}

int CCodeDispatcher::PackNetHead_V2(char* pNetHeadBuffer, int iSrcFD, unsigned int uiRealUin)
{
    TNetHead_V2* pstNetHead = (TNetHead_V2*)pNetHeadBuffer;
    TExternalClientSocket* pstExternalClientSocket = m_pstFDPool->GetExternalSocketByFD(iSrcFD);
    if(!pstExternalClientSocket)
    {
        return -1;
    }

    // ע��: m_ushReservedValue01��m_uiCodeFlag��������, ��������������
    pstNetHead->m_uiSrcIP = pstExternalClientSocket->m_uiSrcIP_NBO;
    pstNetHead->m_uiSocketTime = pstExternalClientSocket->m_uiCreateTime_NBO;
    pstNetHead->m_ushSrcPort = pstExternalClientSocket->m_ushSrcPort_NBO;
    pstNetHead->m_ushReservedValue01 = 0;
    pstNetHead->m_uiSocketFD = pstExternalClientSocket->m_uiSocketFD_NBO;
    pstNetHead->m_uiCodeTime = htonl(time(NULL));
    pstNetHead->m_uiCodeFlag = uiRealUin; /*pstExternalClientSocket->m_uiUin;*/

    return 0;
}
