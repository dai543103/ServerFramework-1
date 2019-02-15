#ifndef __CODE_DISPATCHER_HPP__
#define __CODE_DISPATCHER_HPP__


#include "LotusDefine.hpp"
#include "CodeQueue.hpp"
#include "MsgStatistic.hpp"

class CIOHandler;
class CCodeQueueAssemble;
class CFDPool;

class CCodeDispatcher
{
public:
	int Initialize();

	//�ɷ������ⲿ����Ϣ
	int DispatchOneCode(const char* pszDispatchingBuffer, unsigned short ushCodeOffset,
		unsigned short ushCodeLength, int iSrcFD);

	//��Ĭ��Ŀ���ɷ�
	int DispatchToDefault(const char* pszDispatchingBuffer, unsigned short ushCodeOffset,
		unsigned short ushCodeLength, int iSrcFD);
	
	//��ָ��CodeQueue�ɷ�
	// @param uiRealUin: ʵ���ɷ�Uin ����ʹ��Fd��Uin 
	//                   ��Ϊ�ذ�ʱ Fd�е�Uin�����͸�uiRealUin��һ��һ��
	int DispatchToCodeQueue(int iCodeQueueID, const char* pszDispatchingBuffer,
		unsigned short ushCodeOffset, unsigned short ushCodeLength, int iSrcFD, 
		unsigned int uiRealUin);
	
	//��ָ���ڲ������ɷ�
	int DispatchToDstServer(const unsigned short ushServerType, unsigned short ushServerID,
		const char* pszDispatchingBuffer, unsigned short ushCodeOffset, unsigned short ushCodeLength, int iSrcFD, 
		unsigned int uiRealUin);

	//��װ��ʹ��NetHead_V2
	int PackNetHead_V2(char* pNetHead, int iSrcFD, unsigned int uiRealUin);

	int SendToInternalServer(TInternalServerSocket* pstSocket,
		int iCodeLength, const char* pszCodeBuffer);

	int SendRemain(TInternalServerSocket* pstSocket);

public:

    CIOHandler* m_pIOHandler;
    CConfigAssemble* m_pstConfigAssemble;
    CCodeQueueAssemble* m_pstCodeQueueAssemble;
    CFDPool* m_pstFDPool;
};

#endif
