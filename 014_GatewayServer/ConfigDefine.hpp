#ifndef __CONFIG_DEFINE_HPP__
#define __CONFIG_DEFINE_HPP__

#include "GlobalValue.hpp"

#define FILE_LOTUSCONFIG_FLASH_REQ          "../conf/FlashRequest.ini"
#define FILE_LOTUSCONFIG_FLASH_RSP          "../conf/FlashResponse.ini"

#define	FILE_LOTUSCONFIG_SERVER			    "../conf/LotusServer.tcm"
#define FILE_LOTUSCONFIG_CODEQUEUE		    "../conf/LotusCodeQueue.tcm"
#define FILE_LOTUSCONFIG_LISTENER		    "../conf/LotusListener.tcm"

#define FILE_LOTUSCONFIG_TRACE_UIN          "../conf/TraceUin.ini"
#define FILE_LOCALCONFIG_LOG                "../conf/LogSetting.tcm"

const unsigned int MAX_LISTEN_PORT_NUMBER = 16;
const unsigned int MAX_CODEQUEUE_NUMBER = 16;

#ifdef _DEBUG_
const unsigned int MAX_SERVER_ENTITY_NUMBER = 3;
#else
const unsigned int MAX_SERVER_ENTITY_NUMBER = 3;
#endif

//CodeQueue����
typedef struct tagCodeQueueConfig
{
    int m_iCodeQueueID;

    //��λȡֵ 0����ͨ����CodeQueue 1����ͨ���CodeQueue 2��Ĭ������CodeQueue
    unsigned int m_uiCodeQueueFlag;

    //CodeQueueKey�ļ�·��
    char m_szKeyFileName[MAX_FILENAME_LENGTH];
    unsigned char m_ucKeyPrjID;

    //CodeQueue��С
    unsigned int m_uiShmSize;

    // CodeQueue���ͣ���������CodeQueue�ҽ�Server������
    unsigned int m_uiCodeQueueType;

}TCodeQueueConfig;

typedef enum enSocketListenerType
{
	ESLT_ExternalClinet	= 1,	//�ⲿClient
	ESLT_InternalServer	= 2,	//�ڲ�Server

}ESocketListenerType;

typedef struct tagAddress
{
	unsigned int m_uiIP;
	unsigned short m_ushPort;

}TAddress;

typedef struct tagAddressGroup
{
	unsigned int m_uiNumber;
	TAddress m_astAddress[MAX_LISTEN_PORT_NUMBER];
}TAddressGroup;


typedef enum enCodeQueueFlag
{
	ECDF_INPUT	= (unsigned int)0x00000000,	// ����CodeQueue����ʼֵ���������ж�
	ECDF_OUTPUT	= (unsigned int)0x00000001,	// ���CodeQueue
}ECodeQueueFlag;

//���ֶ�ȡֵ��������CodeQueue�ҽ�Server������
typedef enum enmCodeQueueType
{
	ECQT_DEFAULT = 1,	// ��CodeQueue����Ĭ��ҵ�������
	ECQT_RECORD = 3,	// ��CodeQueue������Ϣ¼�Ʒ�����
}ECodeQueueType;

//Flash��U3D�����ļ��ߴ�
const int POLICY_FILE_SIZE = 1024;

typedef struct tagMsgConfigBuffer
{
    short m_shLength;
    char  m_szContent[POLICY_FILE_SIZE];
}TMsgConfigBuffer;


#endif
