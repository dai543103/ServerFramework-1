

#include "ConfigFile.hpp"



/*************************************************
  Function:     ConfigFile
  Description:
        ConfigFile�Ĺ��캯�������ڳ�ʼ����ĳ�Ա����
*************************************************/

ConfigFile::ConfigFile()
{
    m_pszFilename = NULL;
    m_pszContent = NULL;
    m_pszShadow = NULL;
    m_nSize = 0;

    m_bIsOpened = 0;
}

/*************************************************
  Function:     ~ConfigFile
  Description:
        ConfigFile�����������ͷ��������Դ
*************************************************/
ConfigFile::~ConfigFile()
{
    CloseFile();
}


/*************************************************
  Function:     OpenFile
  Description:
        ��ȡָ���������ļ���
        ����ļ���ȡ�ɹ�������m_bIsOpenedΪtrue
  Input:
        pszFilename     - ��NULL��β�������ļ���
  Output:
  Return:   �ɹ�����0, ʧ�ܷ��ط�0
*************************************************/
int ConfigFile::OpenFile(const char* pszFilename)
{
    FILE    *fp;
    size_t  len;
    int     result;

    // �ͷ���Դ
    CloseFile();

    if(NULL == pszFilename)
    {
        return -1;
    }

    m_pszFilename = strdup(pszFilename);

    fp = fopen(m_pszFilename, "rb");

    if(NULL == fp)
    {
        return -2;
    }

    result = fseek(fp, 0L, SEEK_END);
    if(0 != result)
    {
        fclose(fp);
        return -3;
    }

    len = (size_t)ftell(fp);
    m_pszContent = (char* )new char [len+1];
    m_pszShadow = (char* )new char [len+1];

    if((NULL == m_pszContent) || (NULL == m_pszShadow))
    {
        fclose(fp);
        return -4;
    }

    result = fseek(fp, 0L, SEEK_SET);
    if(0 != result)
    {
        fclose(fp);
        return -5;
    }

    m_nSize = fread(m_pszContent, 1, len, fp);
    m_pszContent[m_nSize] = '\0';

    // ����Ӱ���ڴ棬������ȫ����Сд���ַ���
    memcpy(m_pszShadow, m_pszContent, m_nSize + 1);
    MyToLower(m_pszShadow, m_nSize + 1);

    fclose(fp);

    m_bIsOpened = 1;

    return 0;
}


/*************************************************
  Function:     CloseFile
  Description:
        �ر������ļ��� �ͷŷ�����Դ��
  Input:
  Output:
  Return:
*************************************************/
void ConfigFile::CloseFile()
{
    // �ͷ���Դ
    if(m_pszFilename)
    {
        free(m_pszFilename);        //strdup����
        m_pszFilename = NULL;
    }

    if(m_pszContent)
    {
        delete [] m_pszContent;
        m_pszContent = NULL;
    }

    if(m_pszShadow)
    {
        delete [] m_pszShadow;
        m_pszShadow = NULL;
    }

    m_nSize = 0;
    m_bIsOpened = 0;
}



/*************************************************
  Function:     IsOpened
  Description:
        ���ض�ȡ�����ļ��Ƿ�ɹ��ı�־
  Input:
  Output:
  Return:       ��������ļ���ȡ�ɹ�������true�����򷵻�false
*************************************************/
unsigned int ConfigFile::IsOpened()
{
    return m_bIsOpened;
}


/*************************************************
  Function:     GetItemValue����
  Description:
        ���ڴ���ȡָ�����������͵ļ�ֵ����������ڣ���ʹ��ָ����ȱʡֵ
  Input:
        pszSectionName  - ��NULL��β���ַ���ָ�룬ָʾ����Key��Ƭ��
        pszKeyName      - ��NULL��β���ַ���ָ�룬ָʾ��Ҫ����ֵ��Key
        lDefaultValue   - ȡֵʧ�ܺ�ʹ�õ�ȱʡֵ
  Output:
        lReturnedValue  - ָ�����ڽ��ս���Ļ�������ַ
  Return:       �ɹ�����true, ʧ�ܷ���false
  Others:       ����������UNICODE�汾
*************************************************/
unsigned int ConfigFile::GetItemValue( const char* pszSectionName,
                                       const char* pszKeyName,
                                       int &lReturnedValue,
                                       int lDefaultValue )
{
    if(0 == GetItemValue(pszSectionName, pszKeyName, lReturnedValue))
    {
        lReturnedValue = lDefaultValue;
        return 0;
    }

    return 1;
}


unsigned int ConfigFile::GetItemValue( const char* pszSectionName,
                                       const char* pszKeyName,
                                       int &lReturnedValue )
{
    char szBuf[100];

    if(0 == GetItemValue(pszSectionName, pszKeyName, szBuf, 100))
    {
        return 0;
    }

    lReturnedValue = atol(szBuf);

    return 1;
}


/*************************************************
  Function:     GetItemValue����
  Description:
        ���ڴ���ȡָ�����������͵ļ�ֵ����������ڣ���ʹ��ָ����ȱʡֵ
  Input:
        pszSectionName  - ��NULL��β���ַ���ָ�룬ָʾ����Key��Ƭ��
        pszKeyName      - ��NULL��β���ַ���ָ�룬ָʾ��Ҫ����ֵ��Key
        lDefaultValue   - ȡֵʧ�ܺ�ʹ�õ�ȱʡֵ
  Output:
        lReturnedValue  - ָ�����ڽ��ս���Ļ�������ַ
  Return:       �ɹ�����true, ʧ�ܷ���false
  Others:       ����������UNICODE�汾
*************************************************/
unsigned int ConfigFile::GetItemValue( const char* pszSectionName,
                                       const char* pszKeyName,
                                       int64_t &lReturnedValue,
                                       int64_t lDefaultValue )
{
    if(0 == GetItemValue(pszSectionName, pszKeyName, lReturnedValue))
    {
        lReturnedValue = lDefaultValue;
        return 0;
    }

    return 1;
}

unsigned int ConfigFile::GetItemValue( const char* pszSectionName,
                                       const char* pszKeyName,
                                       int64_t &lReturnedValue )
{
    char szBuf[100];

    if(0 == GetItemValue(pszSectionName, pszKeyName, szBuf, 100))
    {
        return 0;
    }

    lReturnedValue = atoll(szBuf);

    return 1;
}



/*************************************************
  Function:     GetItemValue�ַ���
  Description:
        ���ڴ���ȡָ�����ַ������͵ļ�ֵ����������ڣ���ʹ��ָ����ȱʡֵ
  Input:
        pszSectionName  - ��NULL��β���ַ���ָ�룬ָʾ����Key��Ƭ��
        pszKeyName      - ��NULL��β���ַ���ָ�룬ָʾ��Ҫ����ֵ��Key
        nSize           - ָ�����ջ������Ĵ�С
        pszDefaultValue - ȡֵʧ�ܺ�ʹ�õ�ȱʡֵ
  Output:
        pszReturnedString - ָ�����ڽ��ս���Ļ�������ַ
  Return:       ���ػ������е���Ч�ַ��������������ַ�����β��NULL
  Others:       ����������UNICODE�汾
*************************************************/
unsigned int ConfigFile::GetItemValue( const char* pszSectionName,
                                       const char* pszKeyName,
                                       char* pszReturnedString,
                                       unsigned int nSize,
                                       const char* pszDefaultValue )
{
    unsigned int len;

    if(nSize <= 0)
    {
        return 0;
    }

    len = GetItemValue(pszSectionName, pszKeyName, pszReturnedString, nSize);
    if(0 != len)
    {
        return len;
    }

    strncpy(pszReturnedString, pszDefaultValue, nSize-1);
    pszReturnedString[nSize-1] = '\0';
    return strlen(pszReturnedString);
}


/*************************************************
  Function:     GetItemValue�ַ���
  Description:
        ���ڴ滺�������ҵ�KeyName����ֵ������ָ���Ŀռ䡣
        �������ֵ���ڿռ�Ĵ�С������ַ������н�β����
        ���ڻ����������һ���ֽڼ���NULL��
        ������������������ַ��Ǻ��ֱ���ʱ�����Զ���������������
  Input:
        pszSectionName  - ��NULL��β���ַ���ָ�룬ָʾ����Key��Ƭ��
        pszKeyName      - ��NULL��β���ַ���ָ�룬ָʾ��Ҫ����ֵ��Key
        nSize           - ָ�����ջ������Ĵ�С
  Output:
        pszReturnedString - ָ�����ڽ��ս���Ļ�������ַ
  Return:       ���ػ������е���Ч�ַ��������������ַ�����β��NULL
  Others:       ����������UNICODE�汾
*************************************************/
unsigned int ConfigFile::GetItemValue( const char* pszSectionName,
                                       const char* pszKeyName,
                                       char* pszReturnedString,
                                       unsigned int nSize )
{
    char* pszSectionBegin;
    char* pszSectionEnd;
    char* pszValueBegin;
    char* pszValueEnd;
    unsigned int dwCount;

    // �������Ƿ��ʼ���ɹ�
    if(0 == IsOpened())
    {
        return (unsigned int)0;
    }

    // ��鴫������Ϸ���
    if((NULL == pszSectionName)
            || (NULL == pszKeyName)
            || (NULL == pszReturnedString)
            || (0 == nSize))
    {
        return (unsigned int)0;
    }

    // ����SectionName����λSection�Ŀ�ʼ�ͽ�βָ��
    if(0 == LocateSection(pszSectionName, pszSectionBegin, pszSectionEnd))
    {
        return (unsigned int)0;
    }

    // ��ָ����Χ�ڶ�λKeyName��Value
    if(0 == LocateValue( pszKeyName,
                         pszSectionBegin,
                         pszSectionEnd,
                         pszValueBegin,
                         pszValueEnd ))
    {
        return (unsigned int)0;
    }

    // ����Ҫ��ֵ�������������У���ע�⻺��������
    dwCount = 0;

    for(; pszValueBegin < pszValueEnd && dwCount < (nSize-1); pszValueBegin++, dwCount++)
    {
        pszReturnedString[dwCount] = *pszValueBegin;
    }

    pszReturnedString[dwCount] = '\0';


    // �ַ������ضϣ��жϣ����һ���ַ��Ƿ�Ϊ˫�ֽ�
    if((dwCount == nSize-1) && ((unsigned char)(pszReturnedString[dwCount-1]) > 0x7f))
    {
        // ��˫�ֽڵ����һ���ַ�����Ϊ'\0'
        // Ϊ�˷�ֹ������������������Ժ���ַ�������Խ��
        //     "\xa9"

        pszReturnedString[dwCount-1] = '\0';
        dwCount --;
    }

    return (unsigned int)dwCount;
}


/*************************************************
  Function:     SetItemValue����
  Description:
        ��ָ�����������ͼ�ֵ���������ͬʱ�����ڴ�������ļ�
  Input:
        pszSectionName  - ��NULL��β���ַ���ָ�룬ָʾ����Key��Ƭ��
        pszKeyName      - ��NULL��β���ַ���ָ�룬ָʾ��Ҫ���õ�Key
        ulKeyValue      - �������ͣ�ָʾ��Ҫ���õ�ֵ
  Output:
  Return:       �����Ƿ�ɹ��ı�־���ɹ�������true�����򷵻�false
  Others:       ����������UNICODE�汾
*************************************************/
unsigned int ConfigFile::SetItemValue( const char* pszSectionName,
                                       const char* pszKeyName,
                                       int lKeyValue )
{
    char szBuf[100];

    sprintf(szBuf, "%d", lKeyValue);

    return SetItemValue(pszSectionName, pszKeyName, szBuf);
}


/*************************************************
  Function:     SetItemValue�ַ���
  Description:
        ���ڴ滺�������ҵ�KeyName����ֵ������ָ���Ŀռ䣬�����������ļ���
        �������ֵ���ڿռ�Ĵ�С������ַ������н�β����
        ���ڻ����������һ���ֽڼ���NULL��
        ������������������ַ��Ǻ��ֱ���ʱ�����Զ���������������
  Input:
        pszSectionName  - ��NULL��β���ַ���ָ�룬ָʾ����Key��Ƭ��
        pszKeyName      - ��NULL��β���ַ���ָ�룬ָʾ��Ҫ���õ�Key
        pszKeyValue     - ��NULL��β���ַ���ָ�룬ָʾ��Ҫ���õ�ֵ
  Output:
  Return:       �����Ƿ�ɹ��ı�־���ɹ�������true�����򷵻�false
  Others:       ����������UNICODE�汾
*************************************************/
unsigned int ConfigFile::SetItemValue( const char* pszSectionName,
                                       const char* pszKeyName,
                                       const char* pszKeyValue )
{
    char* pszSectionBegin;
    char* pszSectionEnd;
    char* pszKeyBegin;
    char* pszKeyEnd;
    char* pszContent;
    char* pszShadow;
    char* pszBuf;
    size_t len;

    // �������Ƿ��ʼ���ɹ�
    if(0 == IsOpened())
    {
        return 0;
    }

    // ��鴫��γ��Ϸ���
    if((NULL == pszSectionName)
            || (NULL == pszKeyName)
            || (NULL == pszKeyValue))
    {
        return 0;
    }


    // ����SectionName����λSection�Ŀ�ʼ�ͽ�βָ��
    if(0 == LocateSection(pszSectionName, pszSectionBegin, pszSectionEnd))
    {
        return 0;
    }

    // ��ָ����Χ������KeyName
    if(0 == LocateKey( pszKeyName,
                       pszSectionBegin,
                       pszSectionEnd,
                       pszKeyBegin,
                       pszKeyEnd ))
    {
        // Keyû�ҵ�����pszKeyBegin��pszKeyEnd����λ��Section�Ŀ�ʼ

        pszKeyBegin = pszSectionBegin;
        pszKeyEnd = pszSectionBegin;
    }

    // ���������ݵ��ַ���
#ifdef WIN32
    len = strlen(pszKeyName) + strlen(pszKeyValue) + 5;
#else
    len = strlen(pszKeyName) + strlen(pszKeyValue) + 4;
#endif

    // ������Դ
    pszBuf = (char*)new char [len + 1];
    pszContent = (char*)new char [m_nSize - (pszKeyEnd - pszKeyBegin) + len + 1];
    pszShadow = (char*)new char [m_nSize - (pszKeyEnd - pszKeyBegin) + len + 1];

    if((NULL == pszBuf) || (NULL == pszContent) || (NULL == pszShadow))
    {
        if(pszBuf)
        {
            delete [] pszBuf;
        }

        if(pszContent)
        {
            delete [] pszContent;
        }

        if(pszShadow)
        {
            delete [] pszShadow;
        }

        return 0;
    }

    memset(pszBuf, 0, len + 1);
    memset(pszContent, 0, len + 1);
    memset(pszShadow, 0, len + 1);

#ifdef WIN32
    sprintf(pszBuf, "%s = %s\r\n", pszKeyName, pszKeyValue);
#else
    sprintf(pszBuf, "%s = %s\n", pszKeyName, pszKeyValue);
#endif

    // ���µ������滻ԭ�е�����
    memcpy( (void *)pszContent,
            (void *)m_pszContent,
            (size_t)(pszKeyBegin - m_pszContent) );
    memcpy( (void *)(pszContent + (pszKeyBegin - m_pszContent)),
            (void *)pszBuf,
            len );
    memcpy( (void *)(pszContent + (pszKeyBegin - m_pszContent) + len),
            (void *)pszKeyEnd,
            m_nSize - (pszKeyEnd - m_pszContent) + 1 );

    delete [] pszBuf;
    delete [] m_pszContent;
    delete [] m_pszShadow;

    m_nSize = m_nSize - (pszKeyEnd - pszKeyBegin) + len;

    // �����ļ��ڴ�ӳ��
    m_pszContent = pszContent;

    // ����Ӱ���ڴ棬������ȫ����Сд���ַ���
    m_pszShadow = pszShadow;
    memcpy(m_pszShadow, m_pszContent, m_nSize + 1);
    MyToLower(m_pszShadow, m_nSize + 1);

    // �����ļ�����
    FILE *fp;

    if(NULL == m_pszFilename)
    {
        return 0;
    }

    fp = fopen(m_pszFilename, "wb");

    if(NULL == fp)
    {
        return 0;
    }

    len = fwrite(m_pszContent, 1, m_nSize, fp);

    // ��������룬���ò���
    if((size_t)len != m_nSize)
    {
        // ���ʵ��д����ֽ����������ĳ��Ȳ����
        fclose(fp);

        return 0;
    }

    fclose(fp);

    return 1;
}


/*************************************************
  Function:     LocateSection
  Description:
        ��ָ���Ļ�������Χ������Section��������Section�Ŀ�ʼ��ַ�ͽ�����ַ��
  Input:
        pszSectionName  - ��NULL��β���ַ���ָ�룬ָʾ��Ҫ����ֵ��Section
  Output:
        pszSectionBegin - ���շ���ֵ�Ŀ�ʼ��ַ
                          ָ��Section�е���һ�е�һ���ֽ�
        pszSectionEnd   - ���շ���ֵ�Ľ�����ַ
                          ָ�����һ����Ч�ֽڵ���һ����ַ
  Return:       ��λ�ɹ�������true��ʧ�ܣ�����false
  Others:       ����������UNICODE�汾
*************************************************/
unsigned int ConfigFile::LocateSection( const char* pszSectionName,
                                        char* &pszSectionBegin,
                                        char* &pszSectionEnd )
{
    char*   pszLowerSection;
    char*   pszSectionBeginOnShadow;
    unsigned int   bIsFirstValidCharOnLine;
    char*   pR;

    // �����Ϸ��Լ��
    if(NULL == pszSectionName)
    {
        return 0;
    }

    // ��������룬���ò���
    if((NULL == m_pszContent) || (NULL == m_pszShadow))
    {
        return 0;
    }

    // ��SectionNameת����Сд
    pszLowerSection = new char [strlen(pszSectionName) + 2 + 1];

    sprintf(pszLowerSection, "[%s]", pszSectionName);
    MyToLower(pszLowerSection, strlen(pszLowerSection));

    // ��Shadow�ж�λ��Ȼ�󣬼������ȷ��ָ��
    // �õ�Key��Shadow�е�λ��
    pszSectionBeginOnShadow = LocateStr( pszLowerSection,
                                         m_pszShadow,
                                         m_pszShadow + m_nSize );

    if(NULL == pszSectionBeginOnShadow)
    {
        // �ͷ���Դ
        delete [] pszLowerSection;
        return 0;
    }

    pszSectionBegin = MapToContent(pszSectionBeginOnShadow)
                      + strlen(pszLowerSection);

    //
    // ��SectionBeginָ��ָ��Section����һ�������ֽ�
    //

    // ������ĩ
    for(; pszSectionBegin < (m_pszContent + m_nSize); pszSectionBegin++)
    {
        if((*pszSectionBegin == '\r') || (*pszSectionBegin == '\n'))
        {
            break;
        }
    }

    // �����ǿ��ַ�
    for(; pszSectionBegin < (m_pszContent + m_nSize); pszSectionBegin++)
    {
        if((*pszSectionBegin != '\r')
                && (*pszSectionBegin != '\n')
                && (*pszSectionBegin != ' ')
                && (*pszSectionBegin != '\t'))
        {
            break;
        }
    }

    // �ͷ���Դ
    delete [] pszLowerSection;


    //
    // Ѱ����һ����Ч�ַ���'['��ͷ����
    //
    bIsFirstValidCharOnLine = true;

    pR = pszSectionBegin;
    for(; pR < (m_pszContent + m_nSize + 1); pR++)
    {
        if(bIsFirstValidCharOnLine && *pR == '[')
        {
            break;
        }

        if(*pR == '\0')
        {
            break;
        }

        if(*pR == '\r' || *pR == '\n')
        {
            bIsFirstValidCharOnLine = true;
        }
        else if((*pR != ' ') && (*pR != '\t'))
        {
            bIsFirstValidCharOnLine = false;
        }
    }

    pszSectionEnd = pR;

    return 1;
}


/*************************************************
  Function:     LocateKey
  Description:
        ��ָ���Ļ�������Χ������Key��������Keyƥ���ֵ�Ŀ�ʼ��ַ�ͽ�����ַ��
        ע�⣺ָ����Χ�Ľ�β��ַ�ͷ��ص�ȡֵ������ַ������ָ�����һ����Ч
        �ռ����ĵ�ַ��
  Input:
        pszKeyName      - ��NULL��β���ַ���ָ�룬ָʾ��Ҫ����ֵ��Key
        pszSectionBegin - ָ����Ҫ��������Ŀ�ʼ��ַ
        pszSectionEnd   - ָ����Ҫ������������һ���ַ�����һ����ַ
  Output:
        pszKeyBegin     - ����Key��Ŀ�ʼ��ַ
        pszKeyEnd       - ����Key�����һ�еĿ�ʼ�ֽڵ�ַ
                          ָ�����һ����Ч�ַ�����һ����ַ
  Return:       ��λ�ɹ�������1��ʧ�ܣ�����0
  Others:       ����������UNICODE�汾
*************************************************/
unsigned int ConfigFile::LocateKey( const char* pszKeyName,
                                    const char* pszSectionBegin,
                                    const char* pszSectionEnd,
                                    char* &pszKeyBegin,
                                    char* &pszKeyEnd )
{
    char* pszLowerKey;

    // �����Ϸ��Լ��
    if((NULL == pszKeyName)
            || (NULL == pszSectionBegin)
            || (NULL == pszSectionEnd))
    {
        return 0;
    }

    if(pszSectionBegin >= pszSectionEnd)
    {
        return 0;
    }

    // ��������룬���ò���
    if((NULL == m_pszContent) || (NULL == m_pszShadow))
    {
        return 0;
    }

    // ��KeyNameת����Сд
    pszLowerKey = strdup(pszKeyName);
    MyToLower(pszLowerKey, strlen(pszLowerKey));

    // ��Shadow�ж�λ��Ȼ�󣬼������ȷ��ָ��
    char*   pszKeyBeginOnShadow;

    // �õ�Key��Shadow�е�λ��
    pszKeyBeginOnShadow = LocateStr( pszLowerKey,
                                     MapToShadow(pszSectionBegin),
                                     MapToShadow(pszSectionEnd) );
    if(NULL == pszKeyBeginOnShadow)
    {
        // �ͷ���Դ
        free(pszLowerKey);
        return 0;
    }

    // �ͷ���Դ
    free(pszLowerKey);

    pszKeyBegin = MapToContent(pszKeyBeginOnShadow);

    pszKeyEnd = pszKeyBegin + strlen(pszKeyName);

    // ��ָ����Χ��Ѱ�ҹؼ��ֺ����'='
    for(; pszKeyEnd < pszSectionEnd; pszKeyEnd++)
    {
        if((*pszKeyEnd != ' ') && (*pszKeyEnd != '\t'))
        {
            break;
        }
    }

    if(*pszKeyEnd != '=')
    {
        // �ҵ����ַ������ǹؼ��֣����õݹ鷽ʽ����ָ����Χ�е���һ��λ��
        char* pszSearchBegin;       // ָʾ��������Ŀ�ʼλ��
        // ������LocateKey�У�
        // ��pszValueBegin�޸ĺ�Ӱ����������Ŀ�ʼλ��

        pszSearchBegin = pszKeyEnd;

        return LocateKey( pszKeyName,
                          pszSearchBegin,
                          pszSectionEnd,
                          pszKeyBegin,
                          pszKeyEnd );
    }

    // ����'='������ַ�
    for(pszKeyEnd++; pszKeyEnd < pszSectionEnd; pszKeyEnd++)
    {
        if((*pszKeyEnd == '\r') || (*pszKeyEnd == '\n'))
        {
            break;
        }
    }

    // ��λ�����ȡֵ��Χ
    for(; pszKeyEnd < pszSectionEnd; pszKeyEnd++)
    {
        if((*pszKeyEnd != '\r') && (*pszKeyEnd != '\n'))
        {
            break;
        }
    }

    if(pszKeyEnd > pszKeyBegin)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


/*************************************************
  Function:     LocateValue
  Description:
        ��ָ���Ļ�������Χ������Key��������Keyƥ���ֵ�Ŀ�ʼ��ַ�ͽ�����ַ��
        ע�⣺ָ����Χ�Ľ�β��ַ�ͷ��ص�ȡֵ������ַ������ָ�����һ����Ч
        �ռ����ĵ�ַ��
  Input:
        pszKeyName      - ��NULL��β���ַ���ָ�룬ָʾ��Ҫ����ֵ��Key
        pszSectionBegin - ָ����Ҫ��������Ŀ�ʼ��ַ
        pszSectionEnd   - ָ����Ҫ������������һ���ַ�����һ����ַ
  Output:
        pszValueBegin   - ���շ���ֵ�Ŀ�ʼ��ַ
        pszValueEnd     - ���շ���ֵ�Ľ�����ַ
                          ָ�����һ����Ч�ַ�����һ����ַ
  Return:       ��λ�ɹ�������1��ʧ�ܣ�����0
  Others:       ����������UNICODE�汾
*************************************************/
unsigned int ConfigFile::LocateValue( const char* pszKeyName,
                                      const char* pszSectionBegin,
                                      const char* pszSectionEnd,
                                      char* &pszValueBegin,
                                      char* &pszValueEnd )
{
    char* pszLowerKey;

    // �����Ϸ��Լ��
    if((NULL == pszKeyName)
            || (NULL == pszSectionBegin)
            || (NULL == pszSectionEnd))
    {
        return 0;
    }

    if(pszSectionBegin >= pszSectionEnd)
    {
        return 0;
    }

    // ��������룬���ò���
    if((NULL == m_pszContent) || (NULL == m_pszShadow))
    {
        return 0;
    }

    // ��KeyNameת����Сд
    pszLowerKey = strdup(pszKeyName);
    MyToLower(pszLowerKey, strlen(pszLowerKey));

    // ��Shadow�ж�λ��Ȼ�󣬼������ȷ��ָ��
    char    *pszKeyBeginOnShadow;

    // �õ�Key��Shadow�е�λ��
    pszKeyBeginOnShadow = LocateStr( pszLowerKey,
                                     MapToShadow(pszSectionBegin),
                                     MapToShadow(pszSectionEnd) );

    if (NULL == pszKeyBeginOnShadow)
    {
        // �ͷ���Դ
        free(pszLowerKey);
        return 0;
    }

    // �ͷ���Դ
    free(pszLowerKey);

    pszValueBegin = MapToContent(pszKeyBeginOnShadow) + strlen(pszKeyName);

    // ��ָ����Χ��Ѱ�ҹؼ��ֺ����'='
    for (; pszValueBegin < pszSectionEnd; pszValueBegin++)
    {
        if((*pszValueBegin != ' ') && (*pszValueBegin != '\t'))
        {
            break;
        }
    }

    if(*pszValueBegin != '=')
    {
        // �ҵ����ַ������ǹؼ��֣����õݹ鷽ʽ����ָ����Χ�е���һ��λ��
        char* pszSearchBegin;       // ָʾ��������Ŀ�ʼλ��
        // ������LocateValue�У�
        // ��pszValueBegin�޸ĺ�Ӱ����������Ŀ�ʼλ��

        pszSearchBegin = pszValueBegin;

        return LocateValue( pszKeyName,
                            pszSearchBegin,
                            pszSectionEnd,
                            pszValueBegin,
                            pszValueEnd );
    }

    // ����'='����Ŀո�
    for(pszValueBegin++; pszValueBegin < pszSectionEnd; pszValueBegin++)
    {
        if((*pszValueBegin != '\t') && (*pszValueBegin != ' '))
        {
            break;
        }
    }

    pszValueEnd = pszValueBegin;

    // ���˿ո���ҵ���β
    for(; pszValueEnd < pszSectionEnd; pszValueEnd++)
    {
        if((*pszValueEnd == '\r') || (*pszValueEnd == '\n'))
        {
            break;
        }
    }

    // ����������β�Ŀո�
    for(; pszValueEnd > pszValueBegin; pszValueEnd--)
    {
        if((*(pszValueEnd-1) != '\t') && (*(pszValueEnd-1) != ' '))
        {
            break;
        }
    }


    if(pszValueEnd > pszValueBegin)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


/*************************************************
  Function:     LocateStr
  Description:
        �ڻ�������ָ���ķ�Χ������CharSet������CharSet�ڻ������еĿ�ʼ��ַ��
        ������������ĸ�Ĵ�Сд
        ע�⣺����������Ӧ����NULL��β���ַ�����
  Input:
        pszStr      - ��NULL��β���ַ���ָ�룬ָʾ��Ҫ�������ַ���
        pszBegin        - ָ����Ҫ��������Ŀ�ʼ��ַ
        pszEnd          - ָ����Ҫ������������һ���ַ�����һ����ַ
  Output:
  Return:       �����ɹ�������һ����Ч��ָ�룻ʧ�ܣ�����NULL
*************************************************/
char* ConfigFile::LocateStr( const char* pszStr,
                             const char* pszBegin,
                             const char* pszEnd )
{
    char* pFind;

    // �����Ϸ��Լ��
    if((NULL == pszStr)
            || (NULL == pszBegin)
            || (NULL == pszEnd))
    {
        return NULL;
    }

    if(pszBegin >= pszEnd)
    {
        return NULL;
    }

    // �����ַ����ڻ������е�λ��
    pFind = FindStr(pszBegin, pszStr);

    if((NULL == pFind) || ((pFind + strlen(pszStr)) > pszEnd))
    {
        return NULL;
    }
    else
    {
        return pFind;
    }
}

char* ConfigFile::FindStr(const char* pszBegin, const char* pszStr)
{
    char* pFind;
    char* pEnd;
    const char* pTempBegin = pszBegin;

    while(1)
    {
        pFind = strstr(const_cast<char*>(pTempBegin), const_cast<char*>(pszStr));
        if(NULL == pFind)
        {
            return NULL;
        }

        pEnd = pFind + strlen(pszStr);
        if( '=' == *pEnd || ' ' == *pEnd || '\t' == *pEnd || '\r' == *pEnd|| '\n' == *pEnd )
        {
            return pFind;
        }

        pTempBegin = pFind + strlen(pszStr);
    }

    return NULL;
}


/*************************************************
  Function:     MapToContent
  Description:
        ��Shadow�ĵ�ַӳ�䵽Content��
  Input:
        p       ָ��Shadow�еĵ�ַ
  Output:
  Return:       ���p��Shadow�е���Ч��ַ������ָ��Content�еĶ�Ӧ��ַ
*************************************************/
char* ConfigFile::MapToContent(const char* p)
{
    return (m_pszContent + (p - m_pszShadow));
}


/*************************************************
  Function:     MapToShadow
  Description:
        ��Content�ĵ�ַӳ�䵽Shadow��
  Input:
        p       ָ��Content�еĵ�ַ
  Output:
  Return:       ���p��Content�е���Ч��ַ������ָ��Shadow�еĶ�Ӧ��ַ
*************************************************/
char* ConfigFile::MapToShadow(const char* p)
{
    return (m_pszShadow + (p - m_pszContent));
}


/*************************************************
  Function:     MyToLower
  Description:
        ���ַ����еĴ�д��ĸ���Сд��ĸ��
        strlwr�ڴ���ĳЩ���ֱ���ʱ����������º��ֱ��뱻�ı�
  Input:
        pszSrc  ��Ҫ������ַ�����ַ
        len     ��Ҫ����ĳ���
  Output:
        pszSrc  ��Ŵ�����ɷ��ص�����
*************************************************/
void ConfigFile::MyToLower(char* pszSrc, size_t len)
{
    unsigned char cb;
    size_t  i;

    if(NULL == pszSrc)
    {
        return;
    }

    for(i=0; i<len; i++)
    {
        cb = *(unsigned char* )(pszSrc + i);
        if(cb >='A' && cb<='Z')
        {
            *(unsigned char* )(pszSrc + i) = (unsigned char)(cb + 32);
        }
    }
}


