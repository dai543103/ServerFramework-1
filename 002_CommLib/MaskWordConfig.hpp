#pragma once

#include <string>
#include <string.h>

#include "json/value.h"

//屏蔽字配置

//屏蔽字节点数量
static const int MAX_MASKWORD_NODE_NUM = 20000;

struct MaskNode
{
	char cData;				//字符
	bool bIsLast;			//是否最后一个
	MaskNode* pSiblingNode;	//兄弟节点
	MaskNode* pNextNode;	//下一个节点

	MaskNode()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

class MaskWordConfig
{
public:
	MaskWordConfig();
	virtual ~MaskWordConfig();

	//加载屏蔽字
	int LoadConfig(const char* szFile);

public:

	//是否包含屏蔽字
	bool HasMaskWord(const std::string& strMaskWord);

	//重置
	void Reset();

private:

	//节点是否已存在
	MaskNode* IsNodeExist(MaskNode* pCurrentNode, char cData);

private:

	//当前已使用节点数量
	int m_iUsedNum;

	//屏蔽字节点
	MaskNode m_astNodes[MAX_MASKWORD_NODE_NUM];
};
