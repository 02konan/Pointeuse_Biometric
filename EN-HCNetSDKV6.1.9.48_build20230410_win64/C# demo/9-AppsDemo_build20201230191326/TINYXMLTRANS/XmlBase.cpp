
#include "XmlBase.h"

/****************************************************
������:     CXmlBase
��������: ���캯��
�������: ��  
�������: ��  
����ֵ:   ��
*****************************************************/
CXmlBase::CXmlBase()
{
    m_pDoc = new TiXmlDocument();
    m_pBase = NULL;
}

/****************************************************
������:   ~CXmlBase
��������: ��������
�������: ��  
�������: ��  
����ֵ:   ��
*****************************************************/
CXmlBase::~CXmlBase()
{
    m_pBase = NULL;
    if (m_pDoc != NULL)
    {
        delete m_pDoc;
        m_pDoc = NULL;
    }
}

/****************************************************
������:   LoadFile
��������: ����XML�ļ�
�������:   
pFileName: XML�ļ���
�������:  ��
����ֵ:  
true������ɹ�,false������ʧ�� 
*****************************************************/
bool CXmlBase::LoadFile(const char* pFileName)
{
    m_pDoc->Clear();
    return m_pDoc->LoadFile(pFileName);
}
/****************************************************
������:   FindElemFromBegin
��������: ���ļ�ͷ��ʼ����ָ���Ľڵ㣬���Ǵӵ�ǰλ�ÿ�ʼ���ң������һЩ�����ڵ�˳���붨�岻һ��
�������:
pName: �ڵ�����
�������:  ��
����ֵ:
true�����ҵ�,false��δ�ҵ�
*****************************************************/
bool CXmlBase::FindElemFromBegin(const char* pName)
{
    if (m_pBase == NULL)
    {
       m_pBase = m_pDoc->RootElement();
    }

    if (m_pBase == NULL)
    {
        return false;
    }
    TiXmlElement* pTmp = m_pBase;

    TiXmlNode* pPreSiblingNode = m_pBase->PreviousSibling();
    while (pPreSiblingNode != NULL)
    {
        m_pBase = pPreSiblingNode->ToElement();
        if (m_pBase != NULL)
        {
            pPreSiblingNode = m_pBase->PreviousSibling();
        }
        else
        {
            m_pBase = pTmp;
            break;
        }
    }

    const char* str = NULL;
    while (m_pBase != NULL)
    {
        str = m_pBase->Value();
        if (strcmp(str, pName) == 0)
        {
            return true;
        }
       m_pBase = m_pBase->NextSiblingElement();
    }
    m_pBase = pTmp;
    return false;
}
/****************************************************
������:   FindElem
��������: �ڵ�ǰ���в��ҽڵ�
�������:   
pName: �ڵ�����
�������:  ��
����ֵ:  
true�����ҵ�,false��δ�ҵ� 
*****************************************************/
bool CXmlBase::FindElem(const char* pName)
{
    if (m_pBase == NULL)
    {
        m_pBase = m_pDoc->RootElement();
//        m_pBase = m_pBase->FirstChildElement();
    }
    TiXmlElement* pTmp = m_pBase;
    while(m_pBase != NULL)
    {                
        string str = m_pBase->Value();
        if (str.compare(pName) == 0)
        {            
            return true;
        }
        m_pBase = m_pBase->NextSiblingElement();
    }
    m_pBase = pTmp;
    return false;
}

/****************************************************
������:   IntoElem
��������: ָ��ǰ�ڵ�ĵ�һ���ӽڵ�
�������:  ��
�������:  ��
����ֵ:  
true�����ӽڵ㲢��ָ����ӽڵ�,false�����ӽڵ� 
*****************************************************/
bool CXmlBase::IntoElem()
{
    if (m_pBase == NULL)
    {
        return false;
    }
    TiXmlElement *pTmp = m_pBase->FirstChildElement();    
    if (pTmp == NULL)
    {
        return false;
    }
    m_pBase = pTmp;
    return true;
}

/****************************************************
������:   OutOfElem
��������: ָ��ǰ�ڵ�ĸ��ڵ�
�������:  ��
�������:  ��
����ֵ:  
true���и��ڵ㲢��ָ�򸸽ڵ�,false���޸��ڵ� 
*****************************************************/
bool CXmlBase::OutOfElem()
{
    if (m_pBase == NULL)
    {
        return false;
    }
    m_pBase = m_pBase->Parent()->ToElement();
    if (m_pBase == NULL)
    {
        return false;
    }
    return true;
}

/****************************************************
������:   GetData
��������: ��ȡ��ǰ�ڵ��ı�����
�������:  ��
�������:  ��
����ֵ:  ��ǰ�ڵ��ı����� 
*****************************************************/
string  CXmlBase::GetData() const
{
    string str = "";
    if (m_pBase == NULL)
    {
        return str;
    }

    if (m_pBase->GetText() == NULL)
    {
        return str;
    }

    return  m_pBase->GetText();
}

/****************************************************
������:   GetNodeName
��������: ��ȡ��ǰ�ڵ�����
�������:  ��
�������:  ��
����ֵ:  ��ǰ�ڵ�����
*****************************************************/
string    CXmlBase::GetNodeName() const
{
    if (m_pBase == NULL)
    {
        return NULL;
    }
    return m_pBase->Value();
}

/****************************************************
������:   ModifyData
��������: �޸Ľڵ��ı�����
�������: 
strElem:  �ڵ���
pData:      ���ı�����
�������: ��
����ֵ:   true-�޸ĳɹ�,false-�޴˽ڵ�
*****************************************************/
bool    CXmlBase::ModifyData(const string strElem, const char *pData)
{
    if (!FindElem(strElem.c_str()))
    {
        return false;
    }
    TiXmlNode *pNode = m_pBase->FirstChild();
    if (pNode == NULL)
    {
        TiXmlNode *pTmp = new  TiXmlText(pData);
        m_pBase->LinkEndChild(pTmp);
        return true;
    }
    pNode->SetValue(pData);
    return true;
}

/****************************************************
������:   SetRoot
��������: ���ø��ڵ�
�������: ��
�������: ��
����ֵ:   true--�ɹ�,false--ʧ��
*****************************************************/
bool CXmlBase::SetRoot()
{
    m_pBase = m_pDoc->RootElement();
    if (m_pBase == NULL)
    {
        return false;
    }
    return true;
}

/****************************************************
������:   Parse
��������: ����XML�ı����ݣ�����DOM��
�������: 
pBuf:      XML�ı�����
�������: ��
����ֵ:   
*****************************************************/
const void*    CXmlBase::Parse(const char *pBuf)
{
    m_pDoc->Clear();
    return m_pDoc->Parse(pBuf);
}

/****************************************************
������:   WriteToFile
��������: ���ݴ��ļ�
�������: 
pFileName:�ļ���
�������: ��
����ֵ:   true������ɹ�,false������ʧ��
*****************************************************/
bool    CXmlBase::WriteToFile(const char* pFileName) const
{     
    return m_pDoc->SaveFile(pFileName);
}

/****************************************************
������:   AddNode
��������: �ڵ�ǰ�ڵ�����ӽڵ㣬��ָ����ӽڵ�
�������: 
strElem:  �ڵ���
strTxt:   �ڵ��ı�
�������: ��
����ֵ:   true������ɹ�,false������ʧ��
*****************************************************/
bool    CXmlBase::AddNode(const string &strElem, const string &strTxt)
{
    if (m_pBase == NULL)
    {
        return false;
    }
    TiXmlNode *pNode = m_pBase->LinkEndChild(new TiXmlElement(strElem.c_str()));
    pNode->LinkEndChild(new TiXmlText(strTxt.c_str()));
    m_pBase = pNode->ToElement();
    return true;
}

/****************************************************
������:   AddNode
��������: �ڵ�ǰ�ڵ�����ӽڵ㣬��ָ����ӽڵ�
�������: 
strElem:  �ڵ���
�������: ��
����ֵ:   true������ɹ�,false������ʧ��
*****************************************************/
bool    CXmlBase::AddNode(const string &strElem)
{
    if (m_pBase == NULL)
    {
        return false;
    }
    TiXmlNode *pNode = m_pBase->LinkEndChild(new TiXmlElement(strElem.c_str()));
    m_pBase = pNode->ToElement();
    return true;
}

/****************************************************
������:   AddNode
��������: �ڵ�ǰ�ڵ�����ӽڵ㣬��ָ����ӽڵ�
�������: 
abtNode:  CXmlBase����,�����ж༶�ڵ�
�������: ��
����ֵ:   true������ɹ�,false������ʧ��
*****************************************************/
bool    CXmlBase::AddNode(const CXmlBase &abtNode)
{
    if (m_pBase == NULL)
    {
        return false;
    }
    TiXmlNode *pNode = m_pBase->LinkEndChild(new TiXmlElement(*abtNode.m_pBase));
    m_pBase = pNode->ToElement();
    return true;
}

/****************************************************
������:   WriteToBuf
��������: ���ȫ��XML������buf
�������: 
pBuf:      ������ָ��
dwBufSize:��������С
�������: 
dwReturn: XML����ʵ��ռ�ô�С
����ֵ:   true���ɹ�,false��ʧ��
*****************************************************/
bool    CXmlBase::WriteToBuf(char* pBuf, size_t dwBufSize, int &dwReturn) const
{
    TiXmlPrinter printer;
    m_pDoc->Accept( &printer );
    if (dwBufSize < printer.Size())
    {
        return false;
    }
    size_t dwSize = printer.Size();
    memcpy(pBuf,printer.CStr(),dwSize);
    dwReturn = printer.Size();
    return true;
}

/****************************************************
������:   GetChildren
��������: �����ǰ�ڵ㡢�ӽڵ㼰���ӽڵ�������buf
�������: 
pBuf:      ������ָ��
dwBufSize:��������С
�������: 
dwReturn: ����ʵ��ռ�ô�С
����ֵ:   
*****************************************************/
size_t    CXmlBase::GetChildren(char* pBuf, size_t dwBufSize, size_t &dwReturn) const
{
    if (m_pBase == NULL)
    {
        return false;
    }
    TiXmlPrinter printer;
    m_pBase->Accept(&printer);
    if (dwBufSize < printer.Size())
    {
        return true;
    }
    memset(pBuf,0,dwBufSize);
    size_t dwSize = printer.Size();
    memcpy(pBuf,printer.CStr(),dwSize);
    dwReturn = printer.Size();
    return false;
}

/****************************************************
������:   GetChildren
��������: ��ȡ��ǰ�ڵ㡢�ӽڵ㼰���ӽڵ������ı�
�������: ��
�������: ��
����ֵ:   string���ı�����
*****************************************************/
string CXmlBase::GetChildrenText() const
{
    if (m_pBase == NULL)
    {
        return "";
    }
    TiXmlPrinter printer;
    m_pBase->Accept(&printer);
    return printer.CStr();
}


/****************************************************
������:   RemoveNode
��������: ɾ����ǰ�ڵ�,ɾ����ָ�򸸽ڵ�
�������: ��
�������: ��
����ֵ:   true-ɾ���ɹ�,false���޴˽ڵ�
*****************************************************/
bool    CXmlBase::RemoveNode()
{
    if (m_pBase == NULL)
    {
        return false;
    }
    TiXmlNode *pNode = m_pBase->Parent();
    pNode->RemoveChild(m_pBase);
    m_pBase = pNode->ToElement();
    return true;
}

/****************************************************
������:   RemoveNode
��������: ɾ���ӽڵ�
�������: 
strElem:  �ӽڵ���
strTxt:      �ӽڵ��ı�����
�������: ��
����ֵ:   true-ɾ���ɹ�,false��ʧ��
****************************************************/
bool    CXmlBase::RemoveNode(const string &strElem,const string &strTxt)
{
    if (m_pBase == NULL)
    {
        return false;
    }
    TiXmlElement* pTmp = m_pBase;
    pTmp = pTmp->FirstChildElement();
    while(pTmp != NULL)
    {                
        string str = pTmp->Value();
        if (str == strElem && strTxt.compare(pTmp->GetText()) == 0)
        {            
            return m_pBase->RemoveChild(pTmp);
        }
        pTmp = pTmp->NextSiblingElement();
    }
    return false;
}

/****************************************************
������:   RemoveNode
��������: ɾ���ӽڵ�
�������: 
strElem:  �ӽڵ���
�������: ��
����ֵ:   true-ɾ���ɹ�,false��ʧ��
****************************************************/
bool    CXmlBase::RemoveNode(const string &strElem)
{
    if (m_pBase == NULL)
    {
        return false;
    }
    TiXmlElement* pTmp = m_pBase;
    pTmp = pTmp->FirstChildElement();
    while(pTmp != NULL)
    {                
        string str = pTmp->Value();
        if (str == strElem)
        {            
            return m_pBase->RemoveChild(pTmp);
        }
        pTmp = pTmp->NextSiblingElement();
    }
    return false;
}

/****************************************************
������:   RemoveNode
��������: ɾ���ӽڵ�
�������: 
nIndex:   �ӽڵ����
�������: ��
����ֵ:   true-ɾ���ɹ�,false��ʧ��
****************************************************/
bool    CXmlBase::RemoveNode(size_t nIndex)
{
    if (m_pBase == NULL)
    {
        return false;
    }
    TiXmlElement *pTmp = m_pBase->FirstChildElement(); 
    size_t i;
    for (i = 1; i < nIndex; i++)
    {
        if (pTmp != NULL)
        {
            pTmp = pTmp->NextSiblingElement();
        }    
    }
    m_pBase->RemoveChild(pTmp);

    return true;
}

/****************************************************
������:   NextSibElem
��������: ָ����һ���ֵܽڵ�
�������: ��
�������: ��
����ֵ:   true-�ɹ�,false��ʧ��
****************************************************/
bool    CXmlBase::NextSibElem()
{
    if (m_pBase == NULL)
    {
        return false;
    }
    TiXmlElement *pElem = m_pBase->NextSiblingElement();
    if (!pElem)
    {
        return false;
    }
    m_pBase = pElem;
    return     true;
}

/****************************************************
������:   PreSibElem
��������: ָ����һ���ֵܽڵ�
�������: ��
�������: ��
����ֵ:   true-�ɹ�,false��ʧ��
****************************************************/
bool    CXmlBase::PreSibElem()
{
    if (m_pBase == NULL)
    {
        return false;
    }
    TiXmlNode *pNode = m_pBase->PreviousSibling();
    if (!pNode)
    {
        return false;
    }    
    m_pBase = pNode->ToElement();    
    return true;
}

/****************************************************
������:   RemovePreSibElem
��������: ɾ����һ���ֵܽڵ�
�������: ��
�������: ��
����ֵ:   true-�ɹ�,false��ʧ��
****************************************************/
bool    CXmlBase::RemovePreSibElem()
{
    if (m_pBase == NULL)
    {
        return false;
    }
    TiXmlNode *pNode = m_pBase->PreviousSibling();
    if (pNode == NULL)
    {
        return false;
    }    
    TiXmlNode *pNodeParent = m_pBase->Parent();
    return pNodeParent->RemoveChild(pNode);    
}

/****************************************************
������:   RemoveNextSilElem
��������: ɾ����һ���ֵܽڵ�
�������: ��
�������: ��
����ֵ:   true-�ɹ�,false��ʧ��
****************************************************/
bool    CXmlBase::RemoveNextSilElem()
{
    if (m_pBase == NULL)
    {
        return false;
    }
    TiXmlElement *pElem = m_pBase->NextSiblingElement();
    if (pElem == NULL)
    {
        return false;
    }
    TiXmlNode *pNode = m_pBase->Parent();
    return pNode->RemoveChild(pElem);
}

/****************************************************
������:   GetFirstAttributeValue
��������: ��ȡ��ǰ�ڵ�ĵ�һ������ֵ
�������: ��
�������: ��
����ֵ:   string������ֵ
****************************************************/
string    CXmlBase::GetFirstAttributeValue()
{
    if (m_pBase == NULL)
    {
        return NULL;
    }
    TiXmlAttribute *pAttr = NULL;
    pAttr = m_pBase->FirstAttribute();
    if (pAttr == NULL)
    {
        return "";
    }
    return pAttr->Value();
}
//��ø��ڵ�����
string    CXmlBase::GetRootName()
{
    if (m_pBase == NULL)
    {
        return NULL;
    }
    TiXmlElement *pAttr = NULL;
    pAttr = m_pDoc->RootElement();
    if (pAttr == NULL)
    {
        return "";
    }
    return pAttr->Value();
}
/****************************************************
������:   GetAttributeValue
��������: ��ȡָ�����Ե�����ֵ
�������: ��
�������: ��
����ֵ:   string������ֵ
****************************************************/
string    CXmlBase::GetAttributeValue(const string &strAttriName) const
{
    if (m_pBase == NULL)
    {
        return "";
    }
    return m_pBase->Attribute(strAttriName.c_str());
}

/****************************************************
������:   SetAttribute
��������: ���õ�ǰ�ڵ�����
�������: 
pName:      ��������
pValue:      ����ֵ
�������: ��
����ֵ:   true���ɹ�,false��ʧ��
****************************************************/
bool    CXmlBase::SetAttribute(const char* pName, const char* pValue)
{
    if (m_pBase == NULL)
    {
        return false;
    }
    m_pBase->SetAttribute(pName,pValue);
    return true;
}

/****************************************************
������:   CreateRoot
��������: �������ڵ�
�������: 
strRoot:  ���ڵ�����
�������: ��
����ֵ:   ��
****************************************************/
void    CXmlBase::CreateRoot(const string &strRoot)
{
    TiXmlElement* rootElem = new TiXmlElement(strRoot.c_str());
    m_pDoc->LinkEndChild(rootElem);
    m_pBase = m_pDoc->RootElement();
    //2012-01-31 ����Ҫdelete,m_doc������ʱ�������ڵ����delete
    rootElem = NULL;//lint !e423
}

void    CXmlBase::Clean()
{
    if (m_pDoc != NULL)
    {
        delete m_pDoc;
        m_pDoc = NULL;
    }
}