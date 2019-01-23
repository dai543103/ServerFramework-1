#include <assert.h>

#include "GameProtocol.hpp"
#include "LogAdapter.hpp"

#include "RegAuthProtocolEngine.hpp"

using namespace ServerLib;

int CRegAuthProtocolEngine::Decode(const unsigned char* pszCodeBuff, int iCodeLen, GameProtocolMsg& stMsg, 
	int iThreadIndex, const TNetHead_V2* pNetHead, EGameServerID enMsgPeer)
{
	if (!pszCodeBuff)
	{
		return -1;
	}

	int iRealCodeOffset = 0;
	if (enMsgPeer == GAME_SERVER_LOTUS)
	{
		//网关的包
		if (!pNetHead)
		{
			return -2;
		}

		//填充NetHead
		memcpy((void*)pNetHead, pszCodeBuff, NETHEAD_V2_SIZE);

		// 空包，客户端连接断开
		if (NETHEAD_V2_SIZE == iCodeLen)
		{
			stMsg.mutable_sthead()->set_uimsgid(MSGID_LOGOUTSERVER_REQUEST);
			return 0;
		}
		
		iRealCodeOffset = NETHEAD_V2_SIZE + sizeof(unsigned short);
	}
	else
	{
		//内网服务器包
		iRealCodeOffset = sizeof(unsigned short);
	}

	// 网络数据
	int iBuffLen = iCodeLen - iRealCodeOffset;
	char* pszBuff = (char*)pszCodeBuff + iRealCodeOffset;

	// 解码
	if (!stMsg.ParseFromArray(pszBuff, iBuffLen))
	{
		return -3;
	}

	return 0;
}

int CRegAuthProtocolEngine::Encode(const GameProtocolMsg& stMsg, unsigned char* pszCodeBuff, int iBuffLen, int& iCodeLen, 
	int iThreadIndex, const TNetHead_V2* pNetHead, EGameServerID enMsgPeer)
{
	if (!pszCodeBuff)
	{
		return -1;
	}

	unsigned char* pszRealBuff = pszCodeBuff;
	if (enMsgPeer == GAME_SERVER_LOTUS)
	{
		//发到网关服务器, NETHEAD_V2_SIZE(NetHead) + 2bytes(len) + msg
		if (!pNetHead)
		{
			return -2;
		}

		// 编码NetHead
		memcpy(pszCodeBuff, pNetHead, NETHEAD_V2_SIZE);

		//缓冲区信息
		pszRealBuff = pszCodeBuff + NETHEAD_V2_SIZE + sizeof(unsigned short);

		//填充长度
		iCodeLen = stMsg.ByteSize() + sizeof(unsigned short);
		pszCodeBuff[NETHEAD_V2_SIZE] = iCodeLen / 256;
		pszCodeBuff[NETHEAD_V2_SIZE + 1] = iCodeLen % 256;

		//加上网络头长度
		iCodeLen += NETHEAD_V2_SIZE;
	}
	else if (enMsgPeer == GAME_SERVER_REGAUTHDB)
	{
		//发到RegAuthDB的包 2bytes(len) + 4bytes(uin) + msg

		//缓冲区信息
		pszRealBuff = pszCodeBuff + sizeof(unsigned short) + sizeof(unsigned int);

		//填充长度
		iCodeLen = stMsg.ByteSize() + sizeof(unsigned int) + sizeof(unsigned short);
		pszCodeBuff[0] = iCodeLen / 256;
		pszCodeBuff[1] = iCodeLen % 256;

		//填充uin
		*((unsigned int*)(pszCodeBuff + sizeof(unsigned short))) = stMsg.sthead().uin();
	}
	else
	{
		//发到其他服务的包 2bytes(len) + msg

		//缓冲区信息
		pszRealBuff = pszCodeBuff + sizeof(unsigned short);

		//填充长度
		iCodeLen = stMsg.ByteSize() + sizeof(unsigned short);
		pszCodeBuff[0] = iCodeLen / 256;
		pszCodeBuff[1] = iCodeLen % 256;
	}

	bool bRet = stMsg.SerializeToArray((char*)pszRealBuff, iBuffLen);
	if (!bRet)
	{
		return -3;
	}

	return 0;
}
