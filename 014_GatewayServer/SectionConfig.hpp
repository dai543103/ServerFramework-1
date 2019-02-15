
#ifndef __SECTION_CONFIG_HPP__
#define __SECTION_CONFIG_HPP__

#include <stdint.h>

#include "ErrorDef.hpp"
//#define BYTE unsigned char

namespace ServerLib
{
class CSectionConfig
{
public:
    char    *m_pszFilename;     //!<�����Ҫ��ȡ�������ļ���

    //!���캯������ʼ����ĳ�Ա����ȡ�����ļ�
    CSectionConfig();
    ~CSectionConfig();

    //!���������ļ���ȡ״̬
    unsigned int IsOpen();

    //!ȡָ���ļ�ֵ
    unsigned int GetItemValue( const char *pszSectionName,
                               const char *pszKeyName,
                               char *pszReturnedString,
                               unsigned int nSize );

    //!���ڴ滺�������ҵ�KeyName����ֵ������ָ���Ŀռ�
    unsigned int SetItemValue( const char *pszSectionName,
                               const char *pszKeyName,
                               const char *pszKeyValue );
    unsigned int GetItemValue( const char *pszSectionName,
                               const char *pszKeyName,
                               int &lReturnedValue );
    unsigned int SetItemValue( const char *pszSectionName,
                               const char *pszKeyName,
                               int lKeyValue );
    unsigned int GetItemValue( const char *pszSectionName,
                               const char *pszKeyName,
                               int &lReturnedValue,
                               int lDefaultValue );
    unsigned int GetItemValue( const char *pszSectionName,
                               const char *pszKeyName,
                               short &rshReturnedValue,
                               short shDefaultValue );
    unsigned int GetItemValue( const char *pszSectionName,
                               const char *pszKeyName,
                               int64_t &lReturnedValue );
    unsigned int GetItemValue( const char *pszSectionName,
                               const char *pszKeyName,
                               int64_t &lReturnedValue,
                               int64_t lDefaultValue );
    unsigned int GetItemValue( const char *pszSectionName,
                               const char *pszKeyName,
                               char *pszReturnedString,
                               unsigned int nSize,
                               const char *pszDefaultValue );

    //!���������ļ����ڴ�
    int  OpenFile(const char *pszFilename);

    //!�ͷ������ļ����ص��ڴ��ռ�õ���Դ
    void CloseFile();

    //!�ڽӿڷ��ش���ʱ���������������ȡ�����
    int GetErrorNO() const
    {
        return m_iErrorNO;
    }

private:

    //!��λsection�Ŀ�ʼ��ַ�ͽ�����ַ
    unsigned int LocateSection(const char *pszSectionName,
                               char * &pszSectionBegin,
                               char * &pszSectionEnd);

    //!��ָ���Ļ�������Χ������Key��������Keyƥ���ֵ�Ŀ�ʼ��ַ�ͽ�����ַ
    unsigned int LocateKeyRange(const char *pszKeyName,
                                const char *pszSectionBegin,
                                const char *pszSectionEnd,
                                char * &pszKeyBegin,
                                char * &pszKeyEnd);

    //!��ָ���Ļ�������Χ������Key��������Keyƥ���ֵ�Ŀ�ʼ��ַ�ͽ�����ַ
    unsigned int LocateKeyValue(const char *pszKeyName,
                                const char *pszSectionBegin,
                                const char *pszSectionEnd,
                                char * &pszValueBegin,
                                char * &pszValueEnd);

    //!��һ���ַ�����������һ���ַ���
    const char *LocateStr(    const char *pszCharSet,
                              const char *pszBegin,
                              const char *pszEnd );

    const char *SearchMarchStr(const char *pszBegin, const char *pszCharSet);

    //!��Shadow�еĵ�ַӳ�䵽Content��
    char *MapToContent(const char *p);

    //!��Content�еĵ�ַӳ�䵽Shadow��
    char *MapToShadow(const char *p);

    //!���ַ����еĴ�д��ĸת����Сд��ĸ
    void ToLower( char * pszSrc, size_t len);

    //!���ô����
    void SetErrorNO(int iErrorNO)
    {
        m_iErrorNO = iErrorNO;
    }

private:
    char    *m_pszContent;      //!<�����ļ���ԭʼ����
    char    *m_pszShadow;       //!<�����ļ�������ȫ��ת����Сд
    size_t  m_nSize;            //!<�����ļ����ݵĳ��ȣ�����������NULL
    short   m_bIsOpen;         //!<�����ļ��Ƿ�򿪳ɹ��ı�־
    int m_iErrorNO; //!������
};
}

#endif //__SECTION_CONFIG_HPP__
