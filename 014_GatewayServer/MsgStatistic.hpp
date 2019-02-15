
#ifndef __MSG_STATISTIC_HPP__
#define __MSG_STATISTIC_HPP__

#include "Statistic.hpp"
#include "SingletonTemplate.hpp"

const int MAX_STAT_MSG_NUM = 128; //���ͳ�Ƶ���Ϣ������

//!����ͨ��MsgID���㶨λ����CStatistic�е�Section����
typedef struct tagStatMsgInfo
{
	int m_iMsgID; //!<��ϢID
	int m_iMsgIndex; //!<��Ϣ��CStaitistic�е�Section����
} TStatMsgInfo;

typedef enum enmStatMsgResult
{
	ESMR_SUCCEED = 0, //!<��Ϣ����ɹ�
	ESMR_FAILED = 1, //!<��Ϣ����ʧ��
	ESMR_TIMEOUT = 2, //!<��Ϣ����ʱ
} ENMSTATMSGRESULT;

/**
*@brief ��Ϣ��Ҫͳ�Ƶ�ͳ����
*
*	
*/
typedef enum enmStatMsgItemIndex
{
	ESMI_SUCCESS_MSG_NUM_IDX = 0, //!<��Ϣ����ɹ��ĸ���
	ESMI_FAILED_MSG_NUM_IDX, //!<��Ϣ����ʧ�ܵĸ��� 
	ESMI_TIMEOUT_MSG_NUM_IDX, //!<��Ϣ����ʱ�ĸ���
	ESMI_SUM_PROCESSTIME_IDX, //!<�ܴ����ʱ
	ESMI_MAX_PROCESSTIME_IDX, //!<������ʱ
	ESMI_AVA_PROCESSTIME_IDX, //!<ƽ�������ʱ
	ESMI_MAX_ITEM_NUM //!<�ܹ���Ҫͳ�Ƶ�ͳ�����������֤���ֵ���ᳬ��Section�����ɵ�Item���ֵ
} ENMSTATMSGITEMINDEX;

extern const char* g_apszMsgItemName[ESMI_MAX_ITEM_NUM];

class CMsgStatistic
{
public:
	CMsgStatistic();
	~CMsgStatistic();

public:
	/**
	*��ʼ�����ڳ�ʼ��ʱ������ڴ��CStatistic���е�Section
	*@param[in] pszStatPath ͳ���ļ�·����Ĭ����../stat/
	*@param[in] pszStatFileName ͳ���ļ�����Ĭ����s
	*@return 0 success
	*/
	int Initialize(const char* pszStatPath = NULL, const char* pszStatFileName = NULL);

	//!���ͳ����Ϣ
	int AddMsgStat(int iMsgID, short shResult, timeval tvProcessTime);

	//!��ӡͳ����Ϣ
	void Print();

	//!���ͳ����Ϣ
	void Reset();

	//!�ڽӿڷ��ش���ʱ���������������ȡ�����
	int GetErrorNO() const { return m_iErrorNO; }

private:
	//!ͨ��MsgIDѰ�ҵ���Ӧ��TStatMsgInfo�ṹ
	TStatMsgInfo* GetStatMsgInfo(int iMsgID);

	//!����MsgID��ͳ��
	int AddMsgInfo(int iMsgID);

	//!���ô����
	void SetErrorNO(int iErrorNO) { m_iErrorNO = iErrorNO; }

private:
	CStatistic m_stStatistic;
	int m_iErrorNO; //!������

	short m_shMsgNum;
	TStatMsgInfo m_astMsgInfo[MAX_STAT_MSG_NUM];
};

//!һ����˵ֻ���õ�һ��CMsgStatistic�࣬����ʵ��һ����������ʹ��
typedef Singleton<CMsgStatistic> MsgStatisticSingleton;


#endif //__MSG_STATISTIC_HPP__
///:~
