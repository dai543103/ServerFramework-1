#pragma once

class CCacheTime
{
public:
    void SetCreatedTime(int iTime);
    int GetCreatedTime();
    void GetCreatedTime(char* szTimeString);

protected:
	CCacheTime();

private:
	// 缓存结点创建的时间
	int m_iCreatedTime;
};
