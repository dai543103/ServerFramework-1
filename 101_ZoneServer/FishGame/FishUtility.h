#pragma once

//捕鱼辅助工具类
class CFishpondObj;
class CFishUtility
{
public:
	//获取桌子ID
	static unsigned GetTableUniqID();

	//获取鱼唯一ID
	static unsigned GetFishUniqID();

	//获取子弹唯一ID
	static unsigned GetBulletUniqID();

	//获取红包唯一ID
	static unsigned GetReturnUniqID();

	//获取周期唯一ID
	static unsigned GetCycleUniqID();

	//获取鱼池对象
	static CFishpondObj* GetFishpondByID(unsigned uTableID);

	//判断时间是否同一天
	static bool IsSameDay(long lTime1, long lTime2);

private:

	//桌子唯一ID
	static unsigned uTableUniqID;

	//出鱼唯一ID
	static unsigned uFishUniqID;

	//子弹唯一ID
	static unsigned uBulletUniqID;

	//红包唯一ID
	static unsigned uReturnID;

	//周期唯一ID
	static unsigned uCycleUniqID;
};