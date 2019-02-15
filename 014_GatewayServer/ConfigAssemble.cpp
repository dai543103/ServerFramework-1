#include "ConfigAssemble.hpp"



CConfigAssemble::CConfigAssemble()
{

}

int CConfigAssemble::LoadConfig()
{
    int iRet = 0;

    //������
    iRet = m_stLotusConfigMng.LoadPrimaryConfig();
    if(iRet < 0)
    {
        return -3;
    }

    //CodeQueue����
    iRet = m_stLotusConfigMng.LoadCodeQueueConfig();
    if(iRet < 0)
    {
        return -4;
    }

    //������־Uin
    m_stLotusConfigMng.LoadUinConfig();

    //�����˿�
    iRet = m_stLotusConfigMng.LoadListenerConfig();
    if(iRet < 0)
    {
        return -5;
    }

    //Flash�����ļ�
    if(m_stLotusConfigMng.IsFlashCodeEnabled())
    {
        iRet = m_stLotusConfigMng.LoadFlashConfig();
        if(iRet < 0)
        {
            return -6;
        }
    }

    return 0;
}

int CConfigAssemble::ReloadConfig()
{
    int iRet = 0;

    //������
    iRet = m_stLotusConfigMng.LoadPrimaryConfig();
    if(iRet < 0)
    {
        return -4;
    }

    //CodeQueue����
    iRet = m_stLotusConfigMng.LoadCodeQueueConfig();
    if(iRet < 0)
    {
        return -5;
    }

    //������־Uin
    m_stLotusConfigMng.LoadUinConfig();

    return 0;
}
