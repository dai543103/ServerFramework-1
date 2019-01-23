#include <stdlib.h>

#include "lz4.hpp"
#include "ErrorNumDef.hpp"
#include "StringUtility.hpp"

#include "ProtoUnitTest.hpp"

using namespace ServerLib;

void ProtoUnitTest::SetUp()
{
    return;
}

void ProtoUnitTest::TearDown()
{
    return;
}

TEST_F(ProtoUnitTest, GameProtoTest)
{
    GameProtocolMsg stMsg;

    GameCSMsgHead* pMsgHead = stMsg.mutable_sthead();
    pMsgHead->set_m_strsessionkey("abcde");
    pMsgHead->set_uimsgid(MSGID_ACCOUNT_CREATEROLE_REQUEST);
    pMsgHead->set_uin(280251149);

    CreateRole_Account_Request* pstRequest = stMsg.mutable_stbody()->mutable_m_staccountcreaterolerequest();
    //pstRequest->set_sznickname("jason1");
    pstRequest->set_uin(280251149);

    char szBuff[10240] = {0};
    EXPECT_EQ(true, stMsg.SerializeToArray(szBuff, sizeof(szBuff)));

    //测试protobuf repeated结构为空时size的情况
    stMsg.Clear();
    EXPECT_EQ(0, stMsg.stbody().m_staccountlistroleresponse().roles_size());
}

TEST_F(ProtoUnitTest, LZ4ProtoTest)
{
    GameProtocolMsg stMsg;

    GameCSMsgHead* pMsgHead = stMsg.mutable_sthead();
    pMsgHead->set_m_strsessionkey("abcde");
    pMsgHead->set_uimsgid(MSGID_ACCOUNT_CREATEROLE_REQUEST);
    pMsgHead->set_uin(280251149);

    CreateRole_Account_Request* pstRequest = stMsg.mutable_stbody()->mutable_m_staccountcreaterolerequest();
    //pstRequest->set_sznickname("jason1");
    pstRequest->set_uin(280251149);

    char szTmpBuff[10240] = {0};
    EXPECT_EQ(true, stMsg.SerializeToArray(szTmpBuff, sizeof(szTmpBuff)));
}
