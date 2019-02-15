
#ifndef __STATISTIC_HPP__
#define __STATISTIC_HPP__

#include <time.h>

#include "ErrorDef.hpp"
#include "LotusLogEngine.hpp"
#include "LotusLogAdapter.hpp"

const int MAX_STAT_ITEM_NAME_LENGTH = 32; //!<ͳ�����������󳤶�
const int MAX_STAT_ITEM_NUM = 16; //!<ÿ��ͳ�ƶ�����ж��ٸ�ͳ����
const int MAX_STAT_SECTION_NAME_LENGTH = 64; //!<ͳ�ƶε�������󳤶�
const int MAX_STAT_SECTION_STRING_LENGTH = 1024; //!<ÿһ��ͳ�ƶδ�ӡ���ļ���ÿһ�е������

//!ͳ����,һ�����ݱ�ʾһ��ͳ����
typedef struct tagStatItem
{
	char m_szName[MAX_STAT_ITEM_NAME_LENGTH]; //!<ͳ������
	double m_dValue; //!<ͳ��ֵ
} TStatItem;

typedef enum enmStatSectionFlag
{
	ESSF_NOT_PRINT = 0x0001, //!<����ӡͳ��
} ENMSTATSECTIONFLAG;

//!ͳ�ƶΣ�һ��ͳ�ƶ��ж��ͳ���ͬһ��ͳ�ƶε����ݴ�ӡ��ͬһ����
typedef struct tagStatSection
{
	char m_szName[MAX_STAT_SECTION_NAME_LENGTH]; //!<ͳ�ƶ���
	int m_iStatFlag; //!<ͳ�ƶα�־��Ŀǰ�����Ƿ��ӡ��ͳ�ƶ�
	short m_shStatItemNum; //!<ͳ�������
	TStatItem m_astStatItem[MAX_STAT_ITEM_NUM];
} TStatSection;

class CStatistic
{	
public:
	CStatistic();
	~CStatistic();

public:
	/**
	*��ʼ��ͳ�ƶ�
	*@param[in] pszStatName ͳ������
	*@param[in] iMaxSectionNum ��Ҫ֧����ͳ�ƶθ���
	*@return 0 success
	*/
	int Initialize(const char* pszStatName, int iMaxSectionNum);

	/**
	*����ͳ�ƶ�
	*@param[in] pszSectionName ͳ�ƶ���
	*@param[out] riSectionIdx ��õ�ͳ�ƶ�����
	*@return 0 success
	*/
	int AddSection(const char* pszSectionName, int& riSectionIdx);

	/**
	*����ͳ����
	*@param[in] iSectionIdx ͳ�ƶ�����
	*@param[in] pszItemName ͳ������
	*@param[out] riItemIdx ͳ��������
	*@return 0 success
	*/
	int AddItem(int iSectionIdx, const char* pszItemName, int& riItemIdx);

	/**
	*����ĳ��ͳ��ֵ
	*@param[in] iSectionIdx ͳ�ƶ�����
	*@param[in] iItemIdx ͳ��������
	*@param[in] dValue ͳ��ֵ
	*@return 0 success
	*/
	int SetValue(int iSectionIdx, int iItemIdx, double dValue);

	/**
	*����ĳ��ͳ��ֵ
	*@param[in] iSectionIdx ͳ�ƶ�����
	*@param[in] iItemIdx ͳ��������
	*@param[in] dPlusValue Ҫ���ӵ�ͳ��ֵ
	*@return 0 success
	*/
	int AddValue(int iSectionIdx, int iItemIdx, double dPlusValue);

	/**
	*��ȡͳ�ƶΣ������ϲ�������ɲ���
	*@param[in] iSectionIdx ������
	*@return 0 success
	*/
	TStatSection* GetSection(int iSectionIdx);

	int GetSectionNum() const { return m_iCurSectionNum; }

	//!ͳ�Ƹ�λ
	void Reset();

	//!��ӡͳ��
	void Print();

	//!�ڽӿڷ��ش���ʱ���������������ȡ�����
	int GetErrorNO() const { return m_iErrorNO; }

private: 
	//!���ô����
	void SetErrorNO(int iErrorNO) { m_iErrorNO = iErrorNO; }

	//!�����ж�˫�������Ƿ������������Թ�С��С��0.0000001ʱ�޷���ȷ
	bool IsDoubleInt(double dValue);

private:
	char m_szStatName[MAX_STAT_SECTION_NAME_LENGTH];
	int m_iMaxSectionNum; //!<֧�ֵ�ͳ�ƶθ���
	int m_iCurSectionNum; //!<��ǰͳ�ƶθ���
	TStatSection* m_astStatSection;
	int m_iErrorNO; //!������

	time_t m_tLastReset; //!<���һ�θ�λͳ�Ƶ�ʱ��


};

#endif //__STATISTIC_HPP__
///:~
