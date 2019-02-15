#ifndef __LOCALCONFIGMNG_HPP__
#define __LOCALCONFIGMNG_HPP__

#include "GlobalValue.hpp"
#include "ConfigDefine.hpp"

class CLotusConfigMng
{
public:
    CLotusConfigMng();

public:

    int LoadPrimaryConfig();
    int LoadCodeQueueConfig();
    int LoadListenerConfig();
    int LoadUinConfig();
    int LoadFlashConfig();

    unsigned short GetServerType();
    unsigned short GetServerID();
    int GetIdleConnectionTimeout();
    int GetIdleClientTimeout();
    int GetIdleServerTimeout();
    int GetHelloTimeGap();
    int GetClientPacketMaxFrequencyNum();
    unsigned int GetInternalBufferSize();
    unsigned int GetExternalRCVBufferSize();
    unsigned int GetExternalSNDBufferSize();
    unsigned int GetClientPacketMaxLength();
    unsigned int GetBillFlag();

    bool IsRecordEnabled();
    bool IsNeedCheckNetHead() { return m_bIsNeedCheckNetHead; }

    bool IsFlashCodeEnabled();
    const TMsgConfigBuffer& GetFlashReqBuf();
    const TMsgConfigBuffer& GetFlashRspBuf();

    const TAddressGroup& GetExternalListenerAddress();
    const TAddressGroup& GetInternalListenerAddress();

    const int GetCodeQueueNumber();
    const TCodeQueueConfig& GetCodeQueueByIndex(int iCodeQueueIndex);
    const TCodeQueueConfig& GetCodeQueueByID(int iCodeQueueID);

private:
    //�ж�ָ��CodeQueue�����Ƿ����
    bool IsCodeQueueAvailable(TCodeQueueConfig& rstCodeQueueConfig);
private:
    //Primary
    unsigned short m_ushServerType;
    unsigned short m_ushServerID;

    //�ڲ��������ߴ�
    unsigned int m_uiInternalSocketBufferSize;

    // �ⲿ��������С
    // SO_RCVBUF
    unsigned int m_uiExternalSocketRCVBufferSize;
    // SO_SNDBUF
     unsigned int m_uiExternalSocketSNDBufferSize;

    //�������б��ĳ�������
    unsigned int m_uiClientPacketMaxLength;

    //��鳬ʱ��ʱ��
    int m_iIdleConnectionTimeout; //���ӳ�ʱʱ��
    int m_iIdleClientTimeout; //û���յ��ͻ��˰���ʱʱ��
    int m_iIdleServerTimeout; //û���յ�Svr�ذ��ĳ�ʱʱ��

    //Hello�����ʱ��
    int m_iHelloTimeGap;

    //��������Ƶ������
    int m_iClientPacketMaxFrequency;

    //�Ƿ����NetHead�����ӽ���Ч��
    bool m_bIsNeedCheckNetHead;

    //�˵���־,�����Ҫ��ӡʲô�˵�
    unsigned int m_uiBillFlag;

    //�ⲿ������ַ����
    TAddressGroup m_stClientListenerAddress;
    //�ڲ�������ַ����
    TAddressGroup m_stServerListenerAddress;

    //CodeQueue����
    int m_iCodeQueueNumber;
    //CodeQueue����
    TCodeQueueConfig m_astCodeQueueConfig[MAX_CODEQUEUE_NUMBER];
    int m_aiCodeQueueIDToIndex[MAX_CODEQUEUE_NUMBER];

    //Flash��ȫ���Կ���
    bool m_bIsFlashCodeEnabled;

    //Flash �����������Ӧ
    TMsgConfigBuffer m_stFlashReqConfigBuffer;
    TMsgConfigBuffer m_stFlashRspConfigBuffer;

    //Ĭ��¼�ƿ���
    bool m_bIsRecordEnable;
};

#endif
