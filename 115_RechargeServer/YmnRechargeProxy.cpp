#include <string.h>

#include "GameProtocol.hpp"
#include "ErrorNumDef.hpp"
#include "LogAdapter.hpp"
#include "lib_md5.hpp"
#include "URLCoder.hpp"
#include "ServerBusManager.h"
#include "TimeUtility.hpp"
#include "IOHandler.hpp"
#include "OrderManager.h"

#include "YmnRechargeProxy.h"

using namespace ServerLib;

//有猫腻 捕鱼AppKey
static const char* YMN_FISHGAME_APPKEY = "e8ee19a47d6c028e9ccdaf9701133769";

//有猫腻返回格式串
//注意：下面的串格式不能乱动
static const char* YMN_RESPONSE_FORMAT = "HTTP/1.1 200 OK\r\n\
Content-Type: text/html; charset=utf-8\r\n\
Content-Length: %u\r\n\
Connection: keep-alive\r\n\
Cache-Control: private\r\n\r\n%s";

static GameProtocolMsg stSendMsg;

//充值请求处理
int CYmnRechargeProxy::OnRechargeReq(const std::string& strURI, unsigned int uiSessionFD, const char* pszCodeBuff, int iCodeLen)
{
	if (!pszCodeBuff)
	{
		SendRechargeResponseToClient(uiSessionFD, false);
		return -1;
	}

	//打印收到的请求
	LOGDEBUG("Recv: %s\n", pszCodeBuff);

	//检查urlencode
	std::string strURLEncode;
	bool bIsEncoded = GetRechargeToken(pszCodeBuff, "x-www-form-urlencoded", strURLEncode);

	//签名校验算法：
	//1.按请求字段Key排列所有字段(排除sign),然后拼接appkey进行md5,与sign比较；
	//2.示例格式如下： a=1&b=2&c=3

	//解析参数
	m_mReqParams.clear();

	//找到请求消息体
	const char* pReqParams = strstr(pszCodeBuff, "\r\n\r\n");
	if (!pReqParams)
	{
		//没有消息体
		SendRechargeResponseToClient(uiSessionFD, false);
		return -2;
	}

	//跳到参数位置
	pReqParams += strlen("\r\n\r\n");

	//解析参数
	bool bIsValue = false;
	std::string strKey;
	std::string strValue;

	for (unsigned i = 0; i < strlen(pReqParams); ++i)
	{
		if (pReqParams[i] == '=')
		{
			//接下来的是value
			bIsValue = true;
			strValue.clear();
		}
		else if (pReqParams[i] == '&')
		{
			//新参数
			m_mReqParams[strKey] = strValue;

			bIsValue = false;
			strKey.clear();
		}
		else
		{
			//实际内容
			if (!bIsValue)
			{
				strKey.push_back(pReqParams[i]);
			}
			else
			{
				strValue.push_back(pReqParams[i]);
			}
		}
	}

	//最后一个参数
	m_mReqParams[strKey] = strValue;

	//生成校验串, 拼接成 key1=value1&key2=value2&app_key的形式，不包含sign
	std::string strCheckStr;
	for (std::map<std::string, std::string>::iterator it = m_mReqParams.begin(); it != m_mReqParams.end(); ++it)
	{
		if (it->first != "sign")
		{
			strCheckStr += it->first + "=" + URLEncode(it->second, bIsEncoded) + "&";
		}
	}

	//校验串加上appkey
	strCheckStr += YMN_FISHGAME_APPKEY;

	//校验签名
	unsigned char szDigest[64] = { 0 };
	MD5DigestHex((unsigned char*)strCheckStr.c_str(), strCheckStr.size(), szDigest, true);

	std::string strAppOrderID = m_mReqParams["app_order_id"];

	if (m_mReqParams["sign"].compare((char*)szDigest) != 0)
	{
		//验签失败
		LOGERROR("Failed to recharge, invalid sign, app order %s\n", strAppOrderID.c_str());
		SendRechargeResponseToClient(uiSessionFD, false);
		return -3;
	}

	//检查支付状态,为0表示支付失败
	if (atoi(m_mReqParams["pay_status"].c_str()) == 0)
	{
		LOGERROR("Failed to recharge, invalid pay status, app order %s\n", strAppOrderID.c_str());
		SendRechargeResponseToClient(uiSessionFD, false);
		return -2;
	}

	//解析cp_private
	int iWorldID = 0;
	unsigned int uin = 0;
	int iRechargeID = 0;

	//格式为 worldid_uin_rechargeid
	sscanf(m_mReqParams["cp_private"].c_str(), "%d_%u_%d", &iWorldID, &uin, &iRechargeID);

	//检查订单号
	int iTime = 0;
	unsigned uOrderUin = 0;
	sscanf(strAppOrderID.c_str(), "%10d%08u", &iTime, &uOrderUin);

	if (uOrderUin != uin || iTime < (CTimeUtility::GetNowTime() - 1 * 60 * 60))
	{
		//订单号校验失败
		SendRechargeResponseToClient(uiSessionFD, false);
		return -4;
	}

	//订单是否处理过
	if (COrderManager::Instance()->IsOrderIDExist(strAppOrderID))
	{
		//处理过或者处理中,直接返回成功
		SendRechargeResponseToClient(uiSessionFD, true);
		return -5;
	}

	//发送充值请求到World
	stSendMsg.Clear();
	stSendMsg.mutable_sthead()->set_uisessionfd(uiSessionFD);
	stSendMsg.mutable_sthead()->set_uin(uin);
	stSendMsg.mutable_sthead()->set_uimsgid(MSGID_WORLD_USERRECHARGE_REQUEST);

	World_UserRecharge_Request* pstReq = stSendMsg.mutable_stbody()->mutable_m_stworld_userrecharge_request();
	pstReq->set_uin(uin);
	pstReq->set_strorderid(strAppOrderID);
	pstReq->set_irmb(atoi(m_mReqParams["amount"].c_str()));
	pstReq->set_irechargeid(iRechargeID);
	pstReq->set_itime(atoi(m_mReqParams["pay_time"].c_str()));
	pstReq->set_iplatform(LOGIN_PLATFORM_YMN);

	//发送到对应服务器
	int iRet = SendRechargeMsgToWorld(stSendMsg, iWorldID);
	if (iRet)
	{
		LOGERROR("Failed to send recharge to world, ret %d, app order %s\n", iRet, strAppOrderID.c_str());
		SendRechargeResponseToClient(uiSessionFD, false);
		return -6;
	}

	//增加订单号记录
	OrderData stOneOrder;
	stOneOrder.strOrder = strAppOrderID;
	COrderManager::Instance()->AddOrder(stOneOrder, false);

	//清空参数
	m_mReqParams.clear();

	return 0;
}

//充值返回处理
int CYmnRechargeProxy::OnRechargeResp(const GameProtocolMsg& stMsg)
{
	//获取返回信息
	const World_UserRecharge_Response& stResp = stMsg.stbody().m_stworld_userrecharge_response();
	
	//充值失败
	if (stResp.iresult() != 0)
	{
		//从set中删除订单号
		COrderManager::Instance()->DeleteOrderID(stResp.strorderid());

		LOGERROR("Failed to user recharge, uin %u, recharge id %d, order id %s\n", stResp.uin(), stResp.irechargeid(), stResp.strorderid().c_str());
		SendRechargeResponseToClient(stMsg.sthead().uisessionfd(), false);
	}
	else
	{
		//充值成功

		//插入新订单记录
		OrderData stOneOrder;
		stOneOrder.strOrder = stResp.strorderid();
		stOneOrder.iTime = CTimeUtility::GetNowTime();
		stOneOrder.uiUin = stResp.uin();
		stOneOrder.iRechargeID = stResp.irechargeid();

		COrderManager::Instance()->AddOrder(stOneOrder, true);

		LOGDEBUG("Success to user recharge, uin %u, recharge id %d, order id %s\n", stResp.uin(), stResp.irechargeid(), stResp.strorderid().c_str());
		SendRechargeResponseToClient(stMsg.sthead().uisessionfd(), true);
	}

	return T_SERVER_SUCCESS;
}

//获取充值请求字段
bool CYmnRechargeProxy::GetRechargeToken(const char* pszCodeBuff, const char* pszKey, std::string& strValue)
{
	if (!pszCodeBuff || !pszKey)
	{
		return false;
	}

	strValue.clear();

	const char* pValue = strstr(pszCodeBuff, pszKey);
	if (pValue == NULL)
	{
		return false;
	}
	else
	{
		//跳过key
		pValue += strlen(pszKey);
		for (int i = 0; pValue[i] != '&' && pValue[i] != ' ' && pValue[i] != '\n' && pValue[i] != '\0'; i++)
		{
			strValue.push_back(pValue[i]);
		}
	}

	return true;
}

//发送返回
int CYmnRechargeProxy::SendRechargeResponseToClient(unsigned int uiSessionFD, bool bSuccess)
{
	const char* pResult = bSuccess ? "success" : "fail";

	m_iSendLen = SAFE_SPRINTF(m_szSendBuff, sizeof(m_szSendBuff)-1, YMN_RESPONSE_FORMAT, strlen(pResult), pResult);

	//发送返回
	int iRet = CIOHandler::Instance()->SendToExternalClient(uiSessionFD, m_szSendBuff, m_iSendLen);
	if (iRet)
	{
		LOGERROR("Failed to send ymn recharge response, ret %d, session fd %u\n", iRet, uiSessionFD);
		return iRet;
	}

	return 0;
}
