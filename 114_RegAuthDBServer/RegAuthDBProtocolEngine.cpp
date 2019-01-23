
#include "GameProtocol.hpp"
#include "ThreadLogManager.hpp"

#include "RegAuthDBProtocolEngine.hpp"

//线程消息解码
int CRegAuthDBProtocolEngine::Decode(const unsigned char* pszCodeBuff, int iCodeLen, GameProtocolMsg& stMsg,
	int iThreadIndex, const TNetHead_V2* pNetHead, EGameServerID enMsgPeer)
{
    // 不使用NetHead
    if (!pszCodeBuff)
	{
		return -1;
	}

    // 2bytes(len) + 4bytes(hash) + msg
	int iBuffLen = iCodeLen - (sizeof(unsigned short) + sizeof(unsigned int));
	char* pszBuff = (char*)pszCodeBuff + (sizeof(unsigned short) + sizeof(unsigned int));

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
int CRegAuthDBProtocolEngine::Encode(const GameProtocolMsg& stMsg, unsigned char* pszCodeBuff, int iBuffLen, int& iCodeLen,
	int iThreadIndex, const TNetHead_V2* pNetHead, EGameServerID enMsgPeer)
{
    // 不使用NetHead
    if (!pszCodeBuff)
	{
		return -3;
	}

	//对protobuf进行编码
	int iProtoMsgLen = stMsg.ByteSize();
	if(iProtoMsgLen >= iBuffLen)
	{
		TRACE_THREAD(iThreadIndex, "Fail to parse from data array, real size %d, buff size %d\n", stMsg.ByteSize(), iBuffLen);
		return -4;
	}

    // 编码数据
	bool bRet = stMsg.SerializeToArray(pszCodeBuff +sizeof(unsigned short), iBuffLen);
	if(!bRet)
	{
		TRACE_THREAD(iThreadIndex, "Fail to serialize proto to array!\n");
		return -5;
	}

	iCodeLen = iProtoMsgLen + sizeof(unsigned short);
	pszCodeBuff[0] = iCodeLen / 256;
	pszCodeBuff[1] = iCodeLen % 256;

	DEBUG_THREAD(iThreadIndex, "Success to serialize proto to array!\n");

    return 0;
}
