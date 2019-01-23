
#ifndef __RANDOM_HPP__
#define __RANDOM_HPP__

#include <algorithm>
#include "LogAdapter.hpp"


const int MAX_DROP_RATE = 100000000;
const int MAX_RANDOM_NUMBER = 32767;
const int CONST_INT_TEN_THOUSAND = 10000;

typedef struct
{
    int iIdx;
    int iWeight;
} IdxWeight;

using namespace ServerLib;
class CRandomCalculator
{

public:
    static int Initialize();

    static int GetMyRand();

    // �������0~MAX_DROP_RATE֮���һ����
    static int  GetRandomInRangeHundredMillion();

    //����0 ~ 1000 ��Χ�ڵ������
    static int GetRandomInRangeThousand();

	//����0 ~ 10000 ��Χ�ڵ������
	static int GetRandomInRangeTenThousand();

	//����[0,range) �����ڵ�һ����
	static int GetRandomNumberInRange(int iRange);

    // �������һ������ֵ
    static bool GetRandomBool();

    // �������һ��[min, max]������
    static int  GetRandomNumber(int iMin, int iMax);

    // �������һ������p=0.5�Ķ���ֲ���[min, max]������
    static int  GetBinoRandNum(int iMin, int iMax);

    // ����Ƿ���Ҽ��Χ��
    static bool IsInRangeTenThousand(int iIn);

    // ����Ƿ���Ҽ�ڷ�Χ��
    static bool IsInRangeHundredMillion(int iIn);

    // ����Ƿ���ָ����Χ
    static bool IsInGivenRange(int iRange, int iIn);

    // ����[iLow, iHigh]֮������ظ���iCount�������
    static int GetDistinctRandNumber(const int iLow, const int iHigh,
                                     const int iCount, int aiNumber[]);

    // ����[iLow, iHigh]֮��Ŀ��ظ���iCount�������
    static int GetManyRandNumber(const int iLow, const int iHigh,
                                 const int iCount, int aiNumber[]);


    //�����Ƿ��ɹ�
    static bool TestSuccess(int iSuccessRate);

    //����Ȩ�ش�TotalNum ��ѡ��SelectNum��ͨ��Ʒ����
    static int GetNotSameRand(int iTotalNum, int iSelectNum, int *aiLoot);

    //����Ȩ�ش�TotalNum ��ѡ��SelectNum����ͬ��Ʒ����, �洢��aiLoot��
    //ʱ�临�Ӷ�O(TotalNum * SelectNum)
    static int GetNotSameRandByWeight(int *aiWeight, int iTotalNum, int iSelectNum, int *aiLoot);

    //ϴ���㷨
    template<typename T>
    static void Shuffle(T *array, int iNum)
    {
        ASSERT_AND_LOG_RTN_VOID(array);
        for (int i = 0; i < iNum; i++)
        {
            int iRanPos = GetRandomNumber(i, iNum - 1);
            std::swap(array[i], array[iRanPos]);
        }
    }
private:
#ifdef GET_RAND_FROM_SHM
    static int* m_piRandRingBuff;
    static int m_iRandRingBuffIdx;
#endif
};


#endif