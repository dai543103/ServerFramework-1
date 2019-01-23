#include "pugixml.hpp"

#include "ErrorNumDef.hpp"
#include "StringUtility.hpp"
#include "LogAdapter.hpp"

#include "DBConfigManager.h"

using namespace pugi;
using namespace ServerLib;

DBConfigManager::DBConfigManager()
{
	m_vDBConfigInfos.clear();
}

DBConfigManager::~DBConfigManager()
{
	m_vDBConfigInfos.clear();
}

int DBConfigManager::LoadDBConfig(const char* pszFilePath)
{
	if (!pszFilePath)
	{
		TRACESVR("Failed to load db info config, invalid parameter!\n");
		return -1;
	}

	LOGDEBUG("Load db info config!\n");

	m_vDBConfigInfos.clear();

	xml_document oXmlDoc;
	xml_parse_result oResult = oXmlDoc.load_file(pszFilePath);
	if (!oResult)
	{
		TRACESVR("Failed to open xml config file : %s\n", pszFilePath);
		return -2;
	}

	xml_node oDBInfoNodes = oXmlDoc.child("ALLDBMSINFOS");
	OneDBInfo stOneDBInfo;
	for (xml_node oOneInfoNode = oDBInfoNodes.child("DBMSInfo"); oOneInfoNode; oOneInfoNode = oOneInfoNode.next_sibling())
	{
		stOneDBInfo.Reset();

		stOneDBInfo.iDBMajorVersion = oOneInfoNode.child("m_iMajorVersion").text().as_uint();
		stOneDBInfo.iDBMinVersion = oOneInfoNode.child("m_iMinVersion").text().as_uint();
		SAFE_STRCPY(stOneDBInfo.szDBEngine, oOneInfoNode.child("m_szDBMSName").text().as_string(), sizeof(stOneDBInfo.szDBEngine) - 1);
		SAFE_STRCPY(stOneDBInfo.szDBHost, oOneInfoNode.child("m_szDBMSConnectionInfo").text().as_string(), sizeof(stOneDBInfo.szDBHost) - 1);
		SAFE_STRCPY(stOneDBInfo.szUserName, oOneInfoNode.child("m_szDBMSUser").text().as_string(), sizeof(stOneDBInfo.szUserName) - 1);
		SAFE_STRCPY(stOneDBInfo.szUserPasswd, oOneInfoNode.child("m_szDBMSPassword").text().as_string(), sizeof(stOneDBInfo.szUserPasswd) - 1);
		SAFE_STRCPY(stOneDBInfo.szDBName, oOneInfoNode.child("m_szDBMSCurDatabaseName").text().as_string(), sizeof(stOneDBInfo.szDBName) - 1);

		m_vDBConfigInfos.push_back(stOneDBInfo);
	}

	return T_SERVER_SUCCESS;
}

const OneDBInfo* DBConfigManager::GetDBConfigByIndex(int iDBIndex)
{
	if (iDBIndex >= (int)m_vDBConfigInfos.size() || iDBIndex<0)
	{
		TRACESVR("Failed to get one db config, invalid index %d\n", iDBIndex);
		return NULL;
	}

	return &m_vDBConfigInfos[iDBIndex];
}