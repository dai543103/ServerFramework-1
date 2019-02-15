
#ifndef __CONFIGFILE_HPP__
#define __CONFIGFILE_HPP__


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


class ConfigFile
{
private:
    char    *m_pszContent;        // �����ļ���ԭʼ����
    char    *m_pszShadow;         // �����ļ�������ȫ��ת����Сд
    size_t  m_nSize;              // �����ļ����ݵĳ��ȣ�����������NULL

    unsigned int   m_bIsOpened;   // �����ļ��Ƿ�򿪳ɹ��ı�־

public:
    char    *m_pszFilename;       // �����Ҫ��ȡ�������ļ���

    ConfigFile();
    ~ConfigFile();

    int  OpenFile(const char* pszFilename);

    void CloseFile();

    unsigned int IsOpened();

    unsigned int GetItemValue( const char* pszSectionName,
                               const char* pszKeyName,
                               char* pszReturnedString,              //char*
                               unsigned int nSize );
    unsigned int GetItemValue( const char* pszSectionName,
                               const char* pszKeyName,
                               char* pszReturnedString,              //char* +default
                               unsigned int nSize,
                               const char* pszDefaultValue );
    unsigned int GetItemValue( const char* pszSectionName,
                               const char* pszKeyName,
                               int &lReturnedValue );                //int&
    unsigned int GetItemValue( const char* pszSectionName,
                               const char* pszKeyName,
                               int &lReturnedValue,                  //int& +default
                               int lDefaultValue );
    unsigned int GetItemValue( const char* pszSectionName,
                               const char* pszKeyName,
                               int64_t &lReturnedValue );            //int64_t&
    unsigned int GetItemValue( const char* pszSectionName,
                               const char* pszKeyName,
                               int64_t &lReturnedValue,              //int64_t& +default
                               int64_t lDefaultValue );
    unsigned int SetItemValue( const char* pszSectionName,
                               const char* pszKeyName,
                               int lKeyValue );                     //set int
    unsigned int SetItemValue( const char* pszSectionName,
                               const char* pszKeyName,
                               const char* pszKeyValue );           //set char*


private:
    unsigned int LocateSection( const char* pszSectionName,
                                char*  &pszSectionBegin,
                                char*  &pszSectionEnd );
    unsigned int LocateKey(     const char* pszKeyName,
                                const char* pszSectionBegin,
                                const char* pszSectionEnd,
                                char*  &pszKeyBegin,
                                char*  &pszKeyEnd );
    unsigned int LocateValue(   const char* pszKeyName,
                                const char* pszSectionBegin,
                                const char* pszSectionEnd,
                                char*  &pszValueBegin,
                                char*  &pszValueEnd );

    char* LocateStr(const char* pszStr, const char* pszBegin, const char* pszEnd);
    char* FindStr(const char* pszBegin, const char* pszStr);

    char* MapToContent(const char* p);
    char* MapToShadow(const char* p);

    void MyToLower(char*  pszSrc, size_t len);
};


#endif
