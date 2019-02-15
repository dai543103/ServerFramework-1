#ifndef __FDPOOL_HPP__
#define __FDPOOL_HPP__


#include "LotusDefine.hpp"
#include "ConfigDefine.hpp"
#include "InternalServerPool.hpp"
#include "ExternalClientPool.hpp"

class CFDPool
{
public:

	//��ʼ��External��Internal�׽��ֳ�
	int Initialize();


	//��ָ��FD����Ϊ��
	void SetFDInactive(int iFD);

	// ���ü���
	void SetFDActive(int iFD);

	//�Ƿ����ⲿ�����׽���
	bool IsListeningForExternalClient(int iFD);
	//����Ϊ�ⲿ�����׽���
	void SetListeningForExternalClient(int iFD);
	//�Ƿ����ڲ������׽���
	bool IsListeningForInternalServer(int iFD);
	//����Ϊ�ڲ������׽���
	void SetListeningForInternalServer(int iFD);


	//�Ƿ����ⲿ�����׽���
	bool IsConnectedByExternalClient(int iFD);
	//����Ϊ�ⲿ�����׽���
	void SetConnectedByExternalClient(int iFD);
	//�Ƿ����ڲ������׽���
	bool IsConnectedByInternalServer(int iFD);
	//����Ϊ�ڲ������׽���
	void SetConnectedByInternalServer(int iFD);


	//�����ڲ�Server��ַ
    int AddInternalServerIP(unsigned int uiInternalServerIP, unsigned short ushListenedPort, 
        unsigned short ushServerType, unsigned short ushServerID);
    int ClearInternalServerByIPAndPort(unsigned int uiInternalServerIP, unsigned short ushListenedPort);

	TInternalServerSocket* GetInternalSocketByTypeAndID(unsigned short ushServerType, unsigned short ushServerID);
	TInternalServerSocket* GetInternalSocketByInternalServerIP(unsigned int uiInternalServerIP, unsigned short ushListenedPort);
	TInternalServerSocket* GetInternalSocketByFD(int iFD);
	TExternalClientSocket* GetExternalSocketByFD(int iFD);
	TExternalClientSocket* GetExternalFirstSocket();


    //����Ĭ���ڲ��׽���
    int SetDefaultInternalSocket(unsigned short ushServerType, unsigned short ushServerID);
    //��ȡĬ���ڲ��׽���
    TInternalServerSocket* GetDefaultInternalSocket();
    //���Ĭ���׽���
    int ClearDefaultInternalSocket();
    
private:
	unsigned int m_auiSocketFlag[FD_SIZE];
	CInternalServerPool m_stInternalServerPool;
	CExternalClientPool m_stExternalClientPool;

};

#endif
