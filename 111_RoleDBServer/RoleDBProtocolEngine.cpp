
#include "GameProtocol.hpp"
#include "ThreadLogManager.hpp"
#include "RoleDBProtocolEngine.hpp"
#include "StringUtility.hpp"

//线程消息解码
int CRoleDBProtocolEngine::Decode(const unsigned char* pszCodeBuff, int iCodeLen, GameProtocolMsg& stMsg, 
	int iThreadIndex, const TNetHead_V2* pNetHead, EGameServerID enMsgPeer)
{
    // 不使用NetHead
    if (!pszCodeBuff)
	{
		return -1;
	}

	// 网络数据,跳过2字节len + 4字节 uin
	int iBuffLen = iCodeLen - sizeof(unsigned short) - sizeof(unsigned int);
	char* pszBuff = (char*)pszCodeBuff + sizeof(unsigned short) + sizeof(unsigned int);

    DEBUG_THREAD(iThreadIndex, "Decode Buff len: %d\n", iBuffLen);

	//解码
	bool bRet = stMsg.ParseFromArray(pszBuff, iBuffLen);
	if(!bRet)
	{
		TRACE_THREAD(iThreadIndex, "Fail to parse proto from buff!\n");
		return -2;
	}

	DEBUG_THREAD(iThreadIndex, "Success to parse proto data!\n");

    return 0;
}

//线程消息编码
int CRoleDBProtocolEngine::Encode(const GameProtocolMsg& stMsg, unsigned char* pszCodeBuff, int iBuffLen, int& iCodeLen, 
	int iThreadIndex, const TNetHead_V2* pNetHead, EGameServerID enMsgPeer)
{
    // 不使用NetHead
    if (!pszCodeBuff)
	{
		return -1;
	}

	//对protobuf进行编码
	int iMsgLen = stMsg.ByteSize();
	if(iMsgLen >= iBuffLen)
	{
		TRACE_THREAD(iThreadIndex, "Fail to parse from data array, real size %d, buff size %d\n", iMsgLen, iBuffLen);
		return -4;
	}

    // 编码数据
	bool bRet = stMsg.SerializeToArray(pszCodeBuff +sizeof(unsigned short), iBuffLen);
	if(!bRet)
	{
		TRACE_THREAD(iThreadIndex, "Fail to serialize proto to array!\n");
		return -5;
	}

	iCodeLen = iMsgLen + sizeof(unsigned short);
	pszCodeBuff[0] = iCodeLen / 256;
	pszCodeBuff[1] = iCodeLen % 256;

	DEBUG_THREAD(iThreadIndex, "Success to serialize proto to array!\n");

    return 0;
}
