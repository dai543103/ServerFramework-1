
#ifndef __ERROR_DEF_HPP__
#define __ERROR_DEF_HPP__

#include <string.h>
#include <assert.h>

namespace DeprecatedLib
{

	/**
	*@brief �����붨��
	*
	*	ǰ�����ֽڱ�ʾģ����ţ���0x0001XXXX��ʾ��־ģ��Ķ�Ӧ���󣬾����������С��0010��ʼ
	*/
	typedef enum enmErrorNO
	{
		EEN_LOGFILE = 0x00010000, //!<��־ģ��
		EEN_LOGFILE__NULL_POINTER = 0x00010010, //!<ָ��Ϊ��
		EEN_LOGFILE__OPEN_FILE_FAILED = 0x00010011, //!<���ļ�ʧ��
		EEN_LOGFILE__GET_FILE_STAT_FAILED = 0x00010012, //!<��ȡ�ļ�ͳ����Ϣʧ�ܣ�����stat
		EEN_LOGFILE__REMOVE_FILE_FAILED = 0x00010013, //!<ɾ���ļ�ʧ��
		EEN_LOGFILE__RENAME_FILE_FAILED = 0x00010014, //!<�ļ�������ʧ��
		//...�����־ģ������������

		EEN_SECTION_CONFIG = 0x00020000, //!<���ζ�����ģ��
		EEN_SECTION_CONFIG__NULL_POINTER = 0x00020010, //!<ָ��Ϊ��
		EEN_SECTION_CONFIG__OPEN_FILE_FAILED = 0x00020011, //!<open�ļ�ʧ��
		EEN_SECTION_CONFIG__FSEEK_FAILED = 0x00020012, //!<����fseekʧ��
		//...��Ӱ��ζ�����ģ������������

		EEN_STRING_SPLITTER = 0x00030000, //!<�ַ���������ģ��
		EEN_STRING_SPLITTER__NULL_POINTER = 0x00030010, //!<ָ��Ϊ��
		EEN_STRING_SPLITTER__BAD_TOKEN_BEG = 0x00030011, //!<Token��ʼָ�벻�Ϸ�
		EEN_STRING_SPLITTER__BAD_SEPARTOR = 0x00030012, //!<Token�ָ������Ϸ�
		EEN_STRING_SPLITTER__REACH_END = 0x00030013, //!<�ַ���������ĩβ��
		EEN_STRING_SPLITTER__STRSTR_FAILED = 0x00040014, //!<����strstrʧ��
		//...����ַ���������ģ������������

		EEN_LINE_CONFIG = 0x00040000, //!<���ж�����ģ��
		EEN_LINE_CONFIG__NULL_POINTER = 0x00040010, //!<ָ��Ϊ��
		EEN_LINE_CONFIG__OPEN_FILE_FAILED = 0x00040011, //!<open�ļ�ʧ��
		EEN_LINE_CONFIG__REACH_EOF = 0x00040012, //!<�ļ���ȡ��ĩβ
		//...��Ӱ��ж�ȡ����������������

		EEN_TCP_CONNECTOR = 0x00050000, //!<TCP������
		EEN_TCP_CONNECTOR__NULL_POINTER = 0x00050010, //!<ָ��Ϊ��
		EEN_TCP_CONNECTOR__INVALID_FD = 0x00050011, //!<�Ƿ���FD
		EEN_TCP_CONNECTOR__SOCKET_FAILED = 0x00050012, //!<����Socketʧ��
		EEN_TCP_CONNECTOR__BIND_SOCKET_FAILED = 0x00050013, //!<��IP��ַʧ��
		EEN_TCP_CONNECTOR__CONNECT_FAILED = 0x00050014, //!<����ʧ��
		EEN_TCP_CONNECTOR__CONFIG_USR_BUF_FAILED = 0x00050015, //!<�û�����������ʧ��
		EEN_TCP_CONNECTOR__USR_RECV_BUF_FULL = 0x00050016, //!<�û���������
		EEN_TCP_CONNECTOR__REMOTE_DISCONNECT = 0x00050017, //!<Զ�˹ر�����
		EEN_TCP_CONNECTOR__RECV_ERROR = 0x00050018, //!<����recvʧ�ܣ���EAGAIN
		EEN_TCP_CONNECTOR__SEND_ERROR = 0x00050019, //!<����sendʧ�ܣ���EAGAIN
		EEN_TCP_CONNECTOR__SYS_SEND_BUF_FULL = 0x0005001A, //!<ϵͳ���ͻ�������
		EEN_TCP_CONNECTOR__USR_SEND_BUF_FULL = 0x0005001B, //!<�û����ͻ�������
		EEN_TCP_CONNECTOR__NB_CONNECT_TIMEOUT = 0x0005001C, //!<���������ӳ�ʱ
		EEN_TCP_CONNECTOR__NB_CONNECT_FAILED = 0x0005001D, //!<����������ʧ��
		EEN_TCP_CONNECTOR__NB_CONNECT_REFUSED = 0x0005001E, //!<���������ӱ��ܾ�
		EEN_TCP_CONNECTOR__ALLOC_USR_BUF_FAILED = 0x0005001F, //!<�û������������ڴ�ʧ��
		EEN_TCP_CONNECTOR__BAD_USR_BUF_ALLOC_TYPE = 0x00050020, //!<������û����������䷽ʽ
		//...���TCP����������������

		EEN_TCP_LISTENER = 0x00060000, //!<TCP������
		EEN_TCP_LISTENER__NULL_POINTER = 0x00060010, //!<ָ��Ϊ��
		EEN_TCP_LISTENER__INVALID_FD = 0x00060011, //!<�Ƿ���FD
		EEN_TCP_LISTENER__SOCKET_FAILED = 0x00060012, //!<����Socketʧ��
		EEN_TCP_LISTENER__BIND_SOCKET_FAILED = 0x00060013, //!<��IP��ַʧ��
		EEN_TCP_LISTENER__REUSE_ADDR_FAILED = 0x00060014, //!<���õ�ַ����ʧ��
		EEN_TCP_LISTENER__LISTEN_FAILED = 0x00060015, //!<����ʧ��
		EEN_TCP_LISTENER__ACCEPT_FAILED = 0x00060016, //!<��������ʧ��
		//...���TCP����������������

		EEN_STATISTIC = 0x00070000, //!<����ͳ����
		EEN_STATISTIC__NULL_POINTER = 0x00070010, //!<ָ��Ϊ��
		EEN_STATISTIC__INVALID_SECTION_NUM = 0x00070011, //!<�Ƿ���ͳ�ƶθ���
		EEN_STATISTIC__NEW_SECTION_FAILED = 0x00070012, //!<��̬����Sectionʧ��
		EEN_STATISTIC__SECTION_NUM_REACH_UPPER_LIMIT = 0x00070013, //!<Section�����ﵽ����
		EEN_STATISTIC__INVALID_SECTION_INDEX = 0x00070014, //!<�Ƿ��Ķ�����
		EEN_STATISTIC__LOG_FILE_INIT_FAILED = 0x00070015, //!<ͳ�ƴ�ӡ���ó�ʼ��ʧ��
		EEN_STATISTIC__ITEM_NUM_REACH_UPPER_LIMIT = 0x00070016, //!<ͳ��������ﵽ����
		EEN_STATISTIC__INVALID_ITEM_INDEX = 0x00070017, //!<�Ƿ���������
		//...��ӻ���ͳ��������������

		EEN_MSG_STATISTIC = 0x00080000, //!<��Ϣͳ����
		EEN_MSG_STATISTIC__NULL_POINTER = 0x00080010, //!<ָ��Ϊ��
		EEN_MSG_STATISTIC__STATISTIC_INIT_FAILED = 0x00080011, //!<��Ϣͳ�����ʼ��CStatisticʧ��
		EEN_MSG_STATISTIC__MSG_ID_REACH_UPPER_LIMIT = 0x00080012, //!<ͳ����Ϣ�����ﵽ����
		EEN_MSG_STATISTIC__ADD_SECTION_FAILED = 0x00080013, //!<������Ϣͳ�ƶ�ʧ��
		EEN_MSG_STATISTIC__GET_SECTION_FAILED = 0x00080014, //!<��ȡ��Ϣͳ�ƶ�ָ��ʧ��
		//...�����Ϣͳ��������������

		EEN_CACHE_STATISTIC = 0x00090000, //!<Cacheͳ����
		EEN_CACHE_STATISTIC__NULL_POINTER = 0x00090010, //!<ָ��Ϊ��
		EEN_CACHE_STATISTIC__STATISTIC_INIT_FAILED = 0x00090011, //!<Cacheͳ�����ʼ��CStatisticʧ��
		EEN_CACHE_STATISTIC__CACHE_TYPE_REACH_UPPER_LIMIT = 0x00090012, //!<ͳ����Ϣ�����ﵽ����
		EEN_CACHE_STATISTIC__ADD_SECTION_FAILED = 0x00090013, //!<����Cacheͳ�ƶ�ʧ��
		EEN_CACHE_STATISTIC__GET_SECTION_FAILED = 0x00090014, //!<��ȡCacheͳ�ƶ�ָ��ʧ��
		//...���Cacheͳ��������������
		
		EEN_CODEQUEUE = 0x000A0000, //!<�������ģ��
		EEN_CODEQUEUE__NULL_POINTER = 0x000A0010, //!<ָ��Ϊ��
		EEN_CODEQUEUE__QUEUE_SIZE_TOO_LOW = 0x000A0011, //!<���л���������̫С
		EEN_CODEQUEUE__QUEUE_IS_FULL = 0x000A0012, //!<��������
		EEN_CODEQUEUE__DATA_OFFSET_INVALID = 0x000A0013, //!<����ƫ�Ʋ��Ϸ�
		EEN_CODEQUEUE__MID_OFFSET_INVALID = 0x000A0014, //!<�ж�ƫ�Ʋ��Ϸ�
		EEN_CODEQUEUE__FREE_LENGTH_INVALID = 0x000A0015, //!<���г��Ȳ��Ϸ����ڴ�д���ˣ�
		EEN_CODEQUEUE__USED_LENGTH_INVALID = 0x000A0016, //!<���ó��Ȳ��Ϸ����ڴ�д���ˣ�
		EEN_CODEQUEUE__CODE_LENGTH_INVALID = 0x000A0017, //!<���볤�Ȳ��Ϸ�
		EEN_CODEQUEUE__OUT_BUFFER_TOO_SMALL = 0x000A0018, //!<���������̫С
		//...���CodeQueue����������

		EEN_TEMPMEMORYMNG = 0x000B0000, //!<��ʱ�ڴ������
		EEN_TEMPMEMORYMNG__NULL_POINTER = 0x000B0010, //!<ָ��Ϊ��
		EEN_TEMPMEMORYMNG__CONFIG_OJB_FAILED = 0x000B0011, //!<���ö�����Ϣʧ��
		EEN_TEMPMEMORYMNG__INVALID_MEM_SIZE = 0x000B0012, //!<�Ƿ����ڴ��С
		EEN_TEMPMEMORYMNG__ALLOC_MEM_FAILED = 0x000B0013, //!<�����ڴ�ʧ��
		EEN_TEMPMEMORYMNG__BAD_MEM_ALLOC_TYPE = 0x000B0014, //!<������ڴ���䷽ʽ
		EEN_TEMPMEMORYMNG__OBJ_MEMORY_USED_UP = 0x000B0015, //!<�ڴ����ľ�
		EEN_TEMPMEMORYMNG__INVALID_OBJ_IDX = 0x000B0016, //!<�Ƿ����ڴ��������
		//...���CTempMemoryMng����������

		EEN_ENCODER = 0x000C0000, //!<������
		EEN_ENCODER__NULL_POINTER = 0x000C0010, //!<ָ��Ϊ��
		EEN_ENCODER__EXCEED_CODE_BUF_SIZE = 0x000C0010, //!<������������
		//...��ӱ���������������

		EEN_DECODER = 0x000D0000, //!<������
		EEN_DECODER__NULL_POINTER = 0x000D0010, //!<ָ��Ϊ��
		EEN_DECODER__EXCEED_CODE_BUF_SIZE = 0x000D0010, //!<������������
		//...��ӽ���������������

		EEN_HASH_MAP = 0x000E0000, //!<HashMap
		EEN_HASH_MAP__NULL_POINTER = 0x000E0010, //!<ָ��Ϊ��
		EEN_HASH_MAP__INVALID_INDEX = 0x000E0011, //!<��Ч������ֵ
		EEN_HASH_MAP__INSERT_FAILED_FOR_KEY_DUPLICATE = 0x000E0012, //!<��Ч������ֵ
		EEN_HASH_MAP__NODE_IS_FULL = 0x000E0013, //!<Map�ڵ�������
		EEN_HASH_MAP__NODE_NOT_EXISTED = 0x000E0014, //!<Map�ڵ㲻����
		//...���HashMap������������

	} ENMERRORNO;

	const int MAX_ERROR_STRING_LENGTH = 256; //!<������Ϣ����󳤶�

#define USING_ERROR_NO \
	public: \
		int GetErrorNO() const { return m_iErrorNO; } \
	private: \
		int m_iErrorNO; \
		void SetErrorNO(int iErrorNO) { m_iErrorNO = iErrorNO; }
}

#endif //__ERROR_DEF_HPP__
///:~
