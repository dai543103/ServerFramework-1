
#include "GameProtocol.hpp"
#include "Kernel/ModuleHelper.hpp"
#include "Kernel/ZoneMsgHelper.hpp"

#include "ChatUtility.h"

//发送走马灯
void CChatUtility::SendHorseLamp(std::vector<HorseLampData> vData)
{
	if (vData.size() == 0)
	{
		return;
	}

	static GameProtocolMsg stMsg;

	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_HORSELAMP_NOTIFY);

	Zone_HorseLamp_Notify* pstNotify = stMsg.mutable_stbody()->mutable_m_stzone_horselamp_notify();
	pstNotify->set_izoneid(CModuleHelper::GetZoneID());
	for (unsigned i = 0; i < vData.size(); ++i)
	{
		HorseLampInfo* pstOneInfo = pstNotify->add_stinfos();
		pstOneInfo->set_ilampid(vData[i].iLampID);
		pstOneInfo->set_iendtime(vData[i].iEndTime);
		pstOneInfo->set_iinterval(vData[i].iInterval);
		pstOneInfo->set_itimes(vData[i].iTimes);

		for (unsigned j = 0; j < vData[i].vParams.size(); ++j)
		{
			pstOneInfo->add_strparams(vData[i].vParams[j]);
		}
	}

	//先广播给本线
	CZoneMsgHelper::SendZoneMsgToZoneAll(stMsg);

	//转发世界服务器
	CZoneMsgHelper::SendZoneMsgToWorld(stMsg);

	return;
}
