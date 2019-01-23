#pragma once

#include <string>
#include <vector>

#include "Kernel/GameRole.hpp"

//走马灯信息
struct HorseLampData
{
	int iLampID;	//走马灯ID
	int iEndTime;	//结束时间，为0不生效
	int iInterval;	//播放时间间隔，为0不生效
	int iTimes;		//播放次数，为0不用管
	std::vector<std::string> vParams;	//走马灯参数

	HorseLampData()
	{
		Reset();
	}

	void Reset()
	{
		iLampID = 0;
		iEndTime = 0;
		iInterval = 0;
		iTimes = 0;
		vParams.clear();
	}
};

//聊天工具类
class CChatUtility
{
public:
	//发送走马灯
	static void SendHorseLamp(std::vector<HorseLampData> vData);
};
