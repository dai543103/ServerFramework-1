#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "StringUtility.hpp"
#include "HashUtility.hpp"
#include "ConfigManager.hpp"
#include "ErrorNumDef.hpp"
#include "libghttp/ghttp.h"
#include "pugixml.hpp"
#include "lib_md5.hpp"
#include "TimeUtility.hpp"
#include "ExchangeMsgProcessor.hpp"
#include "ThreadLogManager.hpp"

#include "OnlineExchangeHandler.hpp"

using namespace ServerLib;

static const char* REGULAR_ONLINE_EXCHANGE_URL = "http://api2.ofpay.com/";	//正式地址
//static const char* TEST_ONLINE_EXCHANGE_URL = "http://apitest.ofpay.com/";	//测试地址
//static const char* SP_ACCOUNT_CODE = "A1406907";							//SP编码,对应账号 xiadaobuyu
static const char* SP_ACCOUNT_CODE = "A1406742";							//SP编码,对应账号 13761609301
static const char* SP_ACCOUNT_PASSWORD = "112233qqwwee";					//SP密码
static const char* SP_ACCOUNT_KEY = "A9jwB7Oyt5Eq10MHStw4xX2fFZ5kNoPH";		//SP秘钥

COnlineExchangeHandler::COnlineExchangeHandler()
{

}

//第三个参数传空
void COnlineExchangeHandler::OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult, TNetHead_V2* pstNetHead)
{
	ASSERT_AND_LOG_RTN_VOID(pstHandleResult);

	m_pstRequestMsg = &stReqMsg;
	switch (m_pstRequestMsg->sthead().uimsgid())
	{
	case MSGID_WORLD_ONLINEEXCHANGE_REQUEST:
	{
		//在线兑换
		OnRequestOnlineExchange(*pstHandleResult);
		return;
	}
	break;

	default:
	{

	}
	break;
	}

	return;
}

//设置线程index
void COnlineExchangeHandler::SetThreadIdx(int iThreadIndex)
{
	m_iThreadIndex = iThreadIndex;
}

//在线兑换
int COnlineExchangeHandler::OnRequestOnlineExchange(SHandleResult& stHandleResult)
{
	//获取请求
	const World_OnlineExchange_Request& stReq = m_pstRequestMsg->stbody().m_stworld_onlineexchange_request();

	//返回消息
	GenerateMsgHead(stHandleResult.stResponseMsg, 0, MSGID_WORLD_ONLINEEXCHANGE_RESPONSE, 0);
	World_OnlineExchange_Response* pstResp = stHandleResult.stResponseMsg.mutable_stbody()->mutable_m_stworld_onlineexchange_response();
	pstResp->set_ifromzoneid(stReq.ifromzoneid());
	pstResp->set_uin(stReq.uin());
	pstResp->mutable_stinfo()->CopyFrom(stReq.stinfo());

	if (stReq.stinfo().itype() <= ONLINE_EXCHANGE_INVALID || stReq.stinfo().itype() >= ONLINE_EXCHANGE_MAX)
	{
		TRACE_THREAD(m_iThreadIndex, "Failed to online exchange, invaid type %d, uin %u\n", stReq.stinfo().itype(), stReq.uin());

		pstResp->set_iresult(T_EXCHANGE_PARA_ERROR);
		return -1;
	}

	//下标0： 非法的接口
	//下标1： 话费充值接口
	static const char* pszAPI[ONLINE_EXCHANGE_MAX] = {"Invalid", "onlineorder.do"};

	//封装请求
	std::string strParams;
	switch (stReq.stinfo().itype())
	{
	case ONLINE_EXCHANGE_BILL:
	{
		strParams = PackBillRequestParams(stReq.stinfo());
	}
	break;

	default:
		break;
	}

	if (strParams.size() == 0)
	{
		//封装失败
		TRACE_THREAD(m_iThreadIndex, "Failed to pack online exchange param, uin %u, type %d\n", stReq.uin(), stReq.stinfo().itype());

		pstResp->set_iresult(T_EXCHANGE_PARA_ERROR);
		return -2;
	}

	//处理请求
	int iRet = ProcessOnlineExchange(stReq.uin(), pszAPI[stReq.stinfo().itype()], strParams);
	if (iRet)
	{
		//处理充值失败
		TRACE_THREAD(m_iThreadIndex, "Failed to process online exchange, ret %d, uin %u, type %d, param %s\n", iRet, stReq.uin(), stReq.stinfo().itype(), strParams.c_str());

		pstResp->set_iresult(iRet);
		return -3;
	}

	DEBUG_THREAD(m_iThreadIndex, "Success to online exchange, uin %d, exchange type %d, num %d\n", stReq.uin(), stReq.stinfo().itype(), stReq.stinfo().iexchangenum());

	pstResp->set_iresult(T_SERVER_SUCCESS);

	return T_SERVER_SUCCESS;
}

//封装在线兑换参数
std::string COnlineExchangeHandler::PackBillRequestParams(const OnlineExchange& stInfo)
{
	//1.参数格式为: userid=XXX&userpwd=XXX&carid=XX&cardnum=XX&mctype=XX&sporderid=XXX&sporder_time=XXX&game_userid=XXX&md5_str=XXX&ret_url=&version=XXX&buyNum=XXX
	//2.MD5计算方式: 包体=userid+userpws+cardid+cardnum+sporder_id+sporder_time+ game_userid
	//				 对“包体 + KeyStr” 这个串进行md5 的32位值.结果大写
	//				KeyStr(秘钥) 必须由客户提供欧飞商务进行绑定

	std::string strParams;
	std::string strCheckStr;

	//userid
	strParams += std::string("userid=") + SP_ACCOUNT_CODE;
	strCheckStr += SP_ACCOUNT_CODE;

	//userpws, md5原密码，小写
	char szParam[128] = { 0 };
	MD5DigestHex((unsigned char*)SP_ACCOUNT_PASSWORD, strlen(SP_ACCOUNT_PASSWORD), (unsigned char*)szParam, true);
	strParams += std::string("&userpws=") + szParam;
	strCheckStr += szParam;

	//cardid, 快充140101，慢充170101,用快充
	strParams += std::string("&cardid=") + std::string("140101");
	strCheckStr += "140101";

	//cardnum, todo jasonxiong 检查参数
	SAFE_SPRINTF(szParam, sizeof(szParam)-1, "%d", stInfo.iexchangenum());
	strParams += std::string("&cardnum=") + szParam;
	strCheckStr += szParam;

	//mctype,快充不传
	strParams += std::string("&mctype=");

	//sporder_id,商户订单号
	std::string strOrderID = GetSpOrderID();
	strParams += std::string("&sporder_id=") + strOrderID;
	strCheckStr += strOrderID;

	//sporder_time,订单时间
	CTimeUtility::ConvertUnixTimeToTimeString(CTimeUtility::GetNowTime(), szParam, true);
	strParams += std::string("&sporder_time=") + szParam;
	strCheckStr += szParam;

	//game_userid, 手机号码
	strParams += std::string("&game_userid=") + stInfo.strphonenum();
	strCheckStr += stInfo.strphonenum();

	//md5_str
	//md5(userid + userpws + cardid + cardnum + sporder_id + sporder_time + game_userid + app_key)
	//这个串进行md5 的32位值.结果大写
	strCheckStr += SP_ACCOUNT_KEY;
	MD5DigestHex((unsigned char*)strCheckStr.c_str(), strCheckStr.size(), (unsigned char*)szParam, false);
	strParams += std::string("&md5_str=") + szParam;

	//ret_url
	strParams += std::string("&ret_url=");

	//version,默认6.0
	strParams += std::string("&version=") + "6.0";

	//buyNum
	strParams += std::string("&buyNum=");

	//返回封装参数
	return strParams;
}

//处理在线兑换请求
int COnlineExchangeHandler::ProcessOnlineExchange(unsigned uin, const std::string& strAPI, const std::string& strParams)
{
	//创建请求
	ghttp_request* pstRequest = ghttp_request_new();

	//封装URL
	if (ghttp_set_uri(pstRequest, (char*)(std::string(REGULAR_ONLINE_EXCHANGE_URL)+strAPI).c_str()) == -1)
	{
		ghttp_request_destroy(pstRequest);

		return -1;
	}

	if (ghttp_set_type(pstRequest, ghttp_type_post) == -1)
	{
		ghttp_request_destroy(pstRequest);
		return -2;
	}

	ghttp_set_sync(pstRequest, ghttp_sync);

	//设置请求头
	ghttp_set_header(pstRequest, "Content-Type", "application/x-www-form-urlencoded");

	//设置超时
	ghttp_set_header(pstRequest, http_hdr_Timeout, "5");

	//封装请求Body
	ghttp_set_body(pstRequest, (char*)strParams.c_str(), strParams.size());

	//发起请求
	ghttp_prepare(pstRequest);
	ghttp_status iStatus = ghttp_process(pstRequest);
	if (iStatus == ghttp_error)
	{
		ghttp_request_destroy(pstRequest);
		return -3;
	}

	//获取返回
	char* pstRespBody = ghttp_get_body(pstRequest);
	if (!pstRespBody)
	{
		ghttp_request_destroy(pstRequest);
		return -4;
	}

	DEBUG_THREAD(m_iThreadIndex, "Response: %s\n", pstRespBody);

	//处理返回 XML格式
	pugi::xml_document doc;
	if (!doc.load(pstRespBody))
	{
		ghttp_request_destroy(pstRequest);
		return -5;
	}

	int iRetCode = doc.child("orderinfo").child("retcode").text().as_int();
	std::string strOrderID = doc.child("orderinfo").child("orderid").text().as_string();
	std::string strSpOrderID = doc.child("orderinfo").child("sporder_id").text().as_string();
	int iGameStat = doc.child("orderinfo").child("game_state").text().as_int();
	float fOrderCash = doc.child("orderinfo").child("ordercash").text().as_float();

	if (iRetCode != 1)
	{
		//充值失败
		TRACE_THREAD(m_iThreadIndex, "Online Exchange Failed, uin %u, ret %d, order %s, sporder %s, stat %d, cash %f\n", uin, iRetCode, strOrderID.c_str(), strSpOrderID.c_str(), iGameStat, fOrderCash);
		
		ghttp_request_destroy(pstRequest);
		return -6;
	}
	
	//充值成功
	DEBUG_THREAD(m_iThreadIndex, "Online Exchange Success, uin %u, order %s, sporder %s, cash %f\n", uin, strOrderID.c_str(), strSpOrderID.c_str(), fOrderCash);

	//销毁请求
	ghttp_request_destroy(pstRequest);

	return 0;
}

//获取SP订单号
const std::string COnlineExchangeHandler::GetSpOrderID()
{
	static int aiIndex[WORK_THREAD_NUM] = {0};

	char szParam[128] = { 0 };
	CTimeUtility::ConvertUnixTimeToTimeString(CTimeUtility::GetNowTime(), szParam, true);

	//0-999
	aiIndex[m_iThreadIndex]++;
	aiIndex[m_iThreadIndex] %= 1000;
	SAFE_SPRINTF(szParam+strlen(szParam), sizeof(szParam)-strlen(szParam)-1, "%d", aiIndex[m_iThreadIndex]);

	return szParam;
}
