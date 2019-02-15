
#ifndef __OVERLOAD_SAFEGUARD_HPP__
#define __OVERLOAD_SAFEGUARD_HPP__

class COverloadSafeguard;

typedef struct tagSafeguardStrategyItem
{
	int m_iItemValue; //��ֵ
	bool m_bAcceptConnect; 
	bool m_bAcceptReceive;
	bool m_bNeedCloseConnect;

} TSafeguardStrategyItem;

const int MAX_STRATEGY_ITEM_NUM = 10;

class COverloadSafeguardStrategy
{
public:
	COverloadSafeguardStrategy();
	virtual ~COverloadSafeguardStrategy();

	virtual int Initialize(COverloadSafeguard* pstSafeguard, const char* pszStrategyCfg);

	virtual bool IsAcceptConnect();
	virtual bool IsAcceptReceive();
	virtual bool IsNeedCloseConnect();

	bool IsOpenSafeguard() { return m_bOpenSafeguard; }

	void CalculateStrategyVal();
	int FindStrategyItem(int iStrategyValue);

private:
	COverloadSafeguard* m_pstSafegurad;

	bool m_bOpenSafeguard;

	int m_iConnectCoefficient; //��ǰ����ϵ��
	int m_iStableConnectNum; //�ȶ����Ӹ���

	int m_iNewConnectCoefficient; //������ϵ��
	int m_iStableNewConnectNum; //�ȶ������Ӹ���

	int m_iUpPackCoefficient; //���а���ϵ��
	int m_iStableUpPackNum; //�ȶ����а�����

	int m_iCurStrategyVal; //��ǰ����ֵ

	short m_shStrategyNum; //������Ը���
	TSafeguardStrategyItem m_astStrategy[MAX_STRATEGY_ITEM_NUM];
};

class COverloadSafeguard
{
public:
	COverloadSafeguard();
	~COverloadSafeguard();

	void SetStrategy(COverloadSafeguardStrategy* pstStrategy) { m_pstStrategy = pstStrategy; }
	int Reset();

	bool IsAcceptConnect();
	bool IsAcceptReceive();
	bool IsNeedCloseConnect();

	void IncreaseRefuseAcceptNum() { ++m_iRefuseAcceptNum; }
	void IncreaseRefuseReciveNum() { ++m_iRefuseReceiveNum; }
    void IncreaseCloseNewConnectNum() { ++m_iCurNewCloseConnectNum; }
    
	void IncreaseUpPacketNum() { ++m_iCurUpPackCounter; }
	void IncreaseDownPacketNum() { ++m_iCurDownPacketCounter;}

	void IncreaseExternalConnectNum() { ++m_iCurExternalConnectNum; }
	void DecreaseExternalConnectNum() { --m_iCurExternalConnectNum; }

    void IncreaseNewConnectNum() { ++m_iCurNewConnectNum; }

	int Timeout();

public:
	//���ڷ�����������ͳ��
	int m_iRefuseAcceptNum; //�ܾ���Accept
	int m_iRefuseReceiveNum; //�ܾ���Receive
    int m_iCurNewCloseConnectNum; //��ǰ�ر�������

	int m_iCurUpPackCounter; //��ǰ�����а���
	int m_iCurDownPacketCounter; //��ǰ�����а���

	int m_iCurExternalConnectNum; //��ǰ�ⲿ������
	int m_iCurNewConnectNum; //��ǰ����������
	
	COverloadSafeguardStrategy* m_pstStrategy;
};

#endif

