// DlgConsumeSimulate.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ClientDemo.h"
#include "DlgConsumeSimulate.h"
#include "afxdialogex.h"


// CDlgConsumeSimulate �Ի���

IMPLEMENT_DYNAMIC(CDlgConsumeSimulate, CDialogEx)

CDlgConsumeSimulate::CDlgConsumeSimulate(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgConsumeSimulate::IDD, pParent)
    , m_strEmployeeNo(_T(""))
    , m_strCardNo(_T(""))
    , m_iCurBalance(0)
    , m_iCurTimes(0)
{

}

CDlgConsumeSimulate::~CDlgConsumeSimulate()
{
}



void CDlgConsumeSimulate::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_EMPLOYEENO, m_strEmployeeNo);
    DDX_Text(pDX, IDC_EDIT_CARDNO, m_strCardNo);
    DDX_Text(pDX, IDC_EDIT_CUR_BALANCE, m_iCurBalance);
    DDX_Text(pDX, IDC_EDIT_CUR_TIMES, m_iCurTimes);
    DDX_Control(pDX, IDC_LIST_PERSON, m_personList);
    DDX_Control(pDX, IDC_LIST_CONSUME_RESULT, m_consumeResultList);
    DDX_Control(pDX, IDC_LIST_TRANS_RECORD, m_transRecordList);
}


BEGIN_MESSAGE_MAP(CDlgConsumeSimulate, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_ADD_PERSON, &CDlgConsumeSimulate::OnBnClickedBtnAddPerson)
    ON_BN_CLICKED(IDC_BTN_MODIFY_PERSON, &CDlgConsumeSimulate::OnBnClickedBtnModifyPerson)
    ON_BN_CLICKED(IDC_BTN_DELETE_PERSON, &CDlgConsumeSimulate::OnBnClickedBtnDeletePerson)
    ON_MESSAGE(WM_PROC_CONSUME_ALARM, &CDlgConsumeSimulate::OnWMProcConsumeAlarm)
    ON_MESSAGE(WM_PROC_TRANS_RECORD_ALARM, &CDlgConsumeSimulate::OnWMProcTransRecordAlarm)
END_MESSAGE_MAP()


/*********************************************************
Function:	OnInitDialog
Desc:		Initialize the dialog
Input:
Output:
Return:
**********************************************************/
BOOL CDlgConsumeSimulate::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_personList.InsertColumn(0, _T("����"),LVCFMT_LEFT,80);
    m_personList.InsertColumn(1, _T("����"), LVCFMT_LEFT, 80);
    m_personList.InsertColumn(2, _T("��ǰ���"), LVCFMT_LEFT, 80);
    m_personList.InsertColumn(3, _T("��ǰ���"), LVCFMT_LEFT, 80);

    m_consumeResultList.InsertColumn(0, _T("��ˮ��"), LVCFMT_LEFT, 80);
    m_consumeResultList.InsertColumn(1, _T("����"), LVCFMT_LEFT, 80);
    m_consumeResultList.InsertColumn(2, _T("����"), LVCFMT_LEFT, 80);
    m_consumeResultList.InsertColumn(3, _T("��������"), LVCFMT_LEFT, 80);
    m_consumeResultList.InsertColumn(4, _T("����ģʽ"), LVCFMT_LEFT, 80);
    m_consumeResultList.InsertColumn(5, _T("ԭ���"), LVCFMT_LEFT, 80);
    m_consumeResultList.InsertColumn(6, _T("��ǰ���"), LVCFMT_LEFT, 80);
    m_consumeResultList.InsertColumn(7, _T("ԭ���"), LVCFMT_LEFT, 80);
    m_consumeResultList.InsertColumn(8, _T("��ǰ���"), LVCFMT_LEFT, 80);
   

    m_transRecordList.InsertColumn(0, _T("��ˮ��"), LVCFMT_LEFT, 80);
    m_transRecordList.InsertColumn(1, _T("������ˮ��"), LVCFMT_LEFT, 80);
    m_transRecordList.InsertColumn(2, _T("����"), LVCFMT_LEFT, 80);
    m_transRecordList.InsertColumn(3, _T("����"), LVCFMT_LEFT, 80);
    m_transRecordList.InsertColumn(4, _T("��������"), LVCFMT_LEFT, 80);
    m_transRecordList.InsertColumn(5, _T("Ӧ�����"), LVCFMT_LEFT, 80);
    m_transRecordList.InsertColumn(6, _T("ʵ�����"), LVCFMT_LEFT, 80);
    m_transRecordList.InsertColumn(7, _T("���"), LVCFMT_LEFT, 80);
    m_transRecordList.InsertColumn(8, _T("�����Ѵ���"), LVCFMT_LEFT, 80);
    m_transRecordList.InsertColumn(9, _T("ʣ�����"), LVCFMT_LEFT, 80);

    return TRUE;
}

void CDlgConsumeSimulate::TransRecordJsonToStruct(cJSON *pRoot, TRANSRECORD_STRUCT* pStruTransRecord)
{
    char byStr[32] = { 0 };

    memset(byStr, 0, 32);
    cJSON* pNode = cJSON_GetObjectItem(pRoot, "serialNo");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruTransRecord->strSerialNo = byStr;
    }

    memset(byStr, 0, 32);
    pNode = cJSON_GetObjectItem(pRoot, "frontSerialNo");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruTransRecord->strFrontSerialNo = byStr;
    }

    memset(byStr, 0, 32);
    pNode = cJSON_GetObjectItem(pRoot, "employeeNoString");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruTransRecord->strEmployeeNo = byStr;
    }


    memset(byStr, 0, 32);
    pNode = cJSON_GetObjectItem(pRoot, "cardNo");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruTransRecord->strCardNo = byStr;
    }

    memset(byStr, 0, 32);
    pNode = cJSON_GetObjectItem(pRoot, "modeType");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruTransRecord->strModeType = byStr;
    }

    memset(byStr, 0, 32);
    pNode = cJSON_GetObjectItem(pRoot, "type");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruTransRecord->strType = byStr;
    }


    memset(byStr, 0, 32);
    pNode = cJSON_GetObjectItem(pRoot, "refundSerialNo");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruTransRecord->strRefundSerialNo = byStr;
    }

    memset(byStr, 0, 32);
    pNode = cJSON_GetObjectItem(pRoot, "verifyMode");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruTransRecord->strVerifyMode = byStr;
    }

    memset(byStr, 0, 32);
    pNode = cJSON_GetObjectItem(pRoot, "mode");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruTransRecord->strMode = byStr;
    }


    memset(byStr, 0, 32);
    pNode = cJSON_GetObjectItem(pRoot, "totalPayment");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruTransRecord->iTotalPayment = atoi(byStr);
    }

    memset(byStr, 0, 32);
    pNode = cJSON_GetObjectItem(pRoot, "actualPayment");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruTransRecord->iActualPayment = atoi(byStr);
    }

    memset(byStr, 0, 32);
    pNode = cJSON_GetObjectItem(pRoot, "balance");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruTransRecord->iBalance = atoi(byStr);
    }

    memset(byStr, 0, 32);
    pNode = cJSON_GetObjectItem(pRoot, "times");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruTransRecord->iTimes = atoi(byStr);
    }

    memset(byStr, 0, 32);
    pNode = cJSON_GetObjectItem(pRoot, "remainingTimes");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruTransRecord->iRemainingTimes = atoi(byStr);
    }

    memset(byStr, 0, 32);
    pNode = cJSON_GetObjectItem(pRoot, "resourcesContentType");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruTransRecord->strContentType = byStr;
    }
    memset(byStr, 0, 32);
    pNode = cJSON_GetObjectItem(pRoot, "resourcesContent");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruTransRecord->strContent = byStr;
    }
    memset(byStr, 0, 32);
    pNode = cJSON_GetObjectItem(pRoot, "picturesNumber");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruTransRecord->iPicturesNumber = atoi(byStr);
    }
}

void CDlgConsumeSimulate::ConsumeJsonToStruct(cJSON *pRoot, SERIALNO_STRUCT* pStruSerialNo)
{
    char byStr[32] = { 0 };
    memset(byStr, 0, 32);
    cJSON *pNode = cJSON_GetObjectItem(pRoot, "cancel");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruSerialNo->strCancel = byStr;
    }

    memset(byStr, 0, 32);
    pNode = cJSON_GetObjectItem(pRoot, "serialNo");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruSerialNo->strSerialNo = byStr;
    }


    memset(byStr, 0, 32);
    pNode = cJSON_GetObjectItem(pRoot, "employeeNoString");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruSerialNo->strEmployeeNo = byStr;
    }


    memset(byStr, 0, 32);
    pNode = cJSON_GetObjectItem(pRoot, "cardNo");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruSerialNo->strCardNo = byStr;
    }


    memset(byStr, 0, 32);
    pNode = cJSON_GetObjectItem(pRoot, "type");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruSerialNo->strType = byStr;
    }


    memset(byStr, 0, 32);
    pNode = cJSON_GetObjectItem(pRoot, "refundSerialNo");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruSerialNo->strRefundSerialNo = byStr;
    }


    memset(byStr, 0, 32);
    pNode = cJSON_GetObjectItem(pRoot, "mode");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruSerialNo->strMode = byStr;
    }


    memset(byStr, 0, 32);
    pNode = cJSON_GetObjectItem(pRoot, "totalPayment");
    if (pNode != NULL && pNode->type == cJSON_String)
    {
        strncpy(byStr, pNode->valuestring, strlen(pNode->valuestring));
        pStruSerialNo->iTotalPayment = atoi(byStr);
    }
}

// CDlgConsumeSimulate ��Ϣ�������
LRESULT CDlgConsumeSimulate::OnWMProcConsumeAlarm(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    char byMinor[32] = { 0 }; //�����¼�������
    SERIALNO_STRUCT struSerialNo; //�����¼��ṹ�壬�洢jsonת���������
    cJSON *pRoot;
    pRoot = cJSON_Parse(g_struConsumeISAPIAlarm.pAlarmData);
    if (pRoot)
    {
        cJSON *pEventNode = cJSON_GetObjectItem(pRoot, "ConsumptionEvent");
        if (!pEventNode)
        {
            

            //����ISAPI����ת�ṹ��
            ConsumeJsonToStruct(pEventNode, &struSerialNo);
            
            cJSON *pMinorNode = cJSON_GetObjectItem(pEventNode, "minor");
            if (pMinorNode != NULL && pMinorNode->type == cJSON_String)
            {
                strncpy(byMinor, pMinorNode->valuestring, strlen(pMinorNode->valuestring));
            }
        }
    }


    //���Ϊ�����¼�Ԥ����
    //�ȶԵ�ǰ��Ա�б����Ƿ���ڶ�Ӧ��Ա����������������
    //������ڣ���ʹ����ˮ���б��ȡ��ˮ���Լ���صĽ����Ϣ��Ȼ���·��¼�ȷ��Э��
    if (strcmp(byMinor, "transactionPreprocessingRequest") == 0)
    {
        cJSON *pTemp = cJSON_CreateObject();
        if (pTemp == NULL)
        {
            return FALSE;
        }
        cJSON_AddStringToObject(pTemp, "serialNo", struSerialNo.strSerialNo.c_str());
        if (m_employeeToCardMap.find(struSerialNo.strEmployeeNo) != m_employeeToCardMap.end() 
            && (m_serialNoListMap.find(struSerialNo.strSerialNo) == m_serialNoListMap.end())) //��Ҫ�ж���ˮ���б����Ƿ���ڸ���ˮ�ţ�����������Ԥ����������Ӧ�ô���
        {
            if (struSerialNo.strType == "transaction") //��������Ϊ����ʱ��ҵ�����߼�
            {
                if (struSerialNo.strMode == "amount" || struSerialNo.strMode == "quota")
                {
                    if (m_employeeToCardMap[struSerialNo.strEmployeeNo].iCurBalance >= struSerialNo.iTotalPayment)
                    {
                        struSerialNo.iCurBalance = m_employeeToCardMap[struSerialNo.strEmployeeNo].iCurBalance - struSerialNo.iTotalPayment;
                        cJSON_AddStringToObject(pTemp, "result", "success");
                        cJSON_AddStringToObject(pTemp, "mode", struSerialNo.strMode.c_str());
                        cJSON_AddStringToObject(pTemp, "actualPayment", to_string(struSerialNo.iTotalPayment).c_str());
                        cJSON_AddStringToObject(pTemp, "balance", to_string(m_employeeToCardMap[struSerialNo.strEmployeeNo].iCurBalance).c_str());
                        cJSON_AddStringToObject(pTemp, "employeeNoString", struSerialNo.strEmployeeNo.c_str());
                        cJSON_AddStringToObject(pTemp, "cardNo", struSerialNo.strCardNo.c_str());
                    }
                    else
                    {
                        struSerialNo.iCurBalance = m_employeeToCardMap[struSerialNo.strEmployeeNo].iCurBalance;
                        cJSON_AddStringToObject(pTemp, "result", "balanceNotEnough");                                    
                    }                                
                }
                else if (struSerialNo.strMode == "count")
                {
                    //NET_DVR_STDXMLConfig
                    if (m_employeeToCardMap[struSerialNo.strEmployeeNo].iCurTimes >= 1)  //����ʣ������ͳɹ�
                    {
                        struSerialNo.iCurTimes = m_employeeToCardMap[struSerialNo.strEmployeeNo].iCurTimes - 1;
                        cJSON_AddStringToObject(pTemp, "result", "success");
                        cJSON_AddStringToObject(pTemp, "mode", struSerialNo.strMode.c_str());
                        cJSON_AddNumberToObject(pTemp, "times", 1);
                        cJSON_AddNumberToObject(pTemp, "remainingTimes", m_employeeToCardMap[struSerialNo.strEmployeeNo].iCurTimes);
                        cJSON_AddStringToObject(pTemp, "employeeNoString", struSerialNo.strEmployeeNo.c_str());
                        cJSON_AddStringToObject(pTemp, "cardNo", struSerialNo.strCardNo.c_str());
                    }
                    else
                    {
                        struSerialNo.iCurTimes = m_employeeToCardMap[struSerialNo.strEmployeeNo].iCurTimes;
                        cJSON_AddStringToObject(pTemp, "result", "remainingTimesNotEnough");
                    }
                }
            }
            else if (struSerialNo.strType == "refund") //��������Ϊ����ʱ��ҵ�����߼�
            {
                if (m_serialNoListMap.find(struSerialNo.strRefundSerialNo) != m_serialNoListMap.end() && m_serialNoTempListMap.find(struSerialNo.strRefundSerialNo) != m_serialNoTempListMap.end())   //�ж��Ѿ����׹�����ˮ���б����Ƿ��о�����ˮ��
                {
                    if (m_serialNoTempListMap[struSerialNo.strRefundSerialNo].strMode == "amount" || m_serialNoTempListMap[struSerialNo.strRefundSerialNo].strMode == "quota")
                    {//����������ˮ�����Խ��ף���ô��������ˮ�ŵ����ѽ��дΪ0������ԭ�������
                        m_serialNoTempListMap[struSerialNo.strRefundSerialNo].iCurBalance = m_serialNoTempListMap[struSerialNo.strRefundSerialNo].iCurBalance + m_serialNoTempListMap[struSerialNo.strRefundSerialNo].iTotalPayment;
                        m_serialNoTempListMap[struSerialNo.strRefundSerialNo].iTotalPayment = 0;
                    }
                    else if (m_serialNoTempListMap[struSerialNo.strRefundSerialNo].strMode == "count")
                    {//����������ˮ�����Դ������ף���ô��������ˮ�ŵĴ�����ԭ
                        m_serialNoTempListMap[struSerialNo.strRefundSerialNo].iCurTimes = m_serialNoTempListMap[struSerialNo.strRefundSerialNo].iCurTimes + 1;
                    }
                    cJSON_AddStringToObject(pTemp, "result", "success");
                }
            }

        }
        else  //���޴����߼�
        {
            cJSON_AddStringToObject(pTemp, "result", "noSuchPerson");
            //NET_DVR_STDXMLConfig  ���޴���
                        
        }

        //�·������¼�ȷ�ϱ���
        cJSON *root = cJSON_CreateObject();
        cJSON_AddItemToObject(root, "ConsumptionEventConfirm", pTemp);

        m_serialNoTempListMap[struSerialNo.strSerialNo] = struSerialNo;  //Ǩ�Ƶ�����
        char szUrl[256] = { 0 };
        char *pBuf = cJSON_Print(root);
        sprintf(szUrl, "PUT /ISAPI/Consume/consumptionEventConfirm?format=json\r\n");
        NET_DVR_XML_CONFIG_INPUT    struInput = { 0 };
        NET_DVR_XML_CONFIG_OUTPUT   struOuput = { 0 };
        struInput.dwSize = sizeof(struInput);
        struOuput.dwSize = sizeof(struOuput);
        struInput.lpRequestUrl = szUrl;
        struInput.dwRequestUrlLen = strlen(szUrl);
        struInput.lpInBuffer = pBuf;
        struInput.dwInBufferSize = strlen(pBuf);
        if (!NET_DVR_STDXMLConfig(m_lUserID, &struInput, &struOuput))
        {

        }
        return NULL;


    }
    else if (strcmp(byMinor, "transactionConfirmingRequest") == 0)
    {
        //���Ϊ�����¼�ȷ��
        //�ȶԵ�ǰ��Ա�б����Ƿ���ڶ�Ӧ��Ա����������������
        //������ڣ����жϵ�ǰ��ˮ���Ƿ�����ˮ���б��У�������ڣ��򱨴���Ϊ����������ܳ��֣�������Ҫ����
        //�����ˮ�Ŵ��ڣ�����пۿ���ز���
        if (m_employeeToCardMap.find(struSerialNo.strEmployeeNo) != m_employeeToCardMap.end()
            && (m_serialNoTempListMap.find(struSerialNo.strSerialNo) != m_serialNoTempListMap.end())) //��Ҫ�ж���ˮ����ʱ�б����Ƿ���ڸ���ˮ�ţ�����Ӧ�ô���
        {
            if (struSerialNo.strCancel == "false")//����ȡ��
            {
                m_serialNoTempListMap.erase(struSerialNo.strSerialNo);
            }
            else
            {
                //����ǽ��н��ף���ô�޸���Ա�б�ؼ��Լ�m_employeeToCardMap����Ϣ������Σ�����������ˮ�����ӵ�������ɵ��б�m_serialNoListMap�У����ɾ��m_serialNoTempListMap�е���ˮ��
                //����ǽ��о�����ô�޸���Ա�б�ؼ��Լ�m_employeeToCardMap����Ϣ������Σ������ɾ��m_serialNoTempListMap�е���ˮ�ţ�ɾ��m_serialNoListMap�еľ�����ˮ��
                SERIALNO_STRUCT* struLastSerialNo = &m_serialNoTempListMap[struSerialNo.strSerialNo];
                m_employeeToCardMap[struLastSerialNo->strEmployeeNo].iCurBalance = struLastSerialNo->iCurBalance;
                m_employeeToCardMap[struLastSerialNo->strEmployeeNo].iCurTimes = struLastSerialNo->iCurTimes;
                int count = m_personList.GetItemCount();
                for (int i = 0; i < count; i++)
                {
                    CString temp = m_personList.GetItemText(i,0);
                    if (temp.GetBuffer() == struLastSerialNo->strEmployeeNo)
                    {
                        m_personList.SetItemText(i, 2, to_string(struLastSerialNo->iCurBalance).c_str());
                        m_personList.SetItemText(i, 3, to_string(struLastSerialNo->iCurTimes).c_str());
                        break;
                    }
                }
                            

                //�����ѿؼ��б���ʾ���Ѿ�ȷ�ϵ������¼�
                if (struLastSerialNo->strType == "transaction")
                {
                    int iConsumeResultSize = m_consumeResultList.GetItemCount();
                    m_consumeResultList.InsertItem(iConsumeResultSize, struLastSerialNo->strSerialNo.c_str());
                    m_consumeResultList.SetItemText(iConsumeResultSize, 1, struLastSerialNo->strEmployeeNo.c_str());
                    m_consumeResultList.SetItemText(iConsumeResultSize, 2, struLastSerialNo->strCardNo.c_str());
                    m_consumeResultList.SetItemText(iConsumeResultSize, 3, struLastSerialNo->strType.c_str());
                    m_consumeResultList.SetItemText(iConsumeResultSize, 4, struLastSerialNo->strMode.c_str());
                    m_consumeResultList.SetItemText(iConsumeResultSize, 5, to_string(struLastSerialNo->iCurBalance + struLastSerialNo->iTotalPayment).c_str());
                    m_consumeResultList.SetItemText(iConsumeResultSize, 6, to_string(struLastSerialNo->iCurBalance).c_str());
                    m_consumeResultList.SetItemText(iConsumeResultSize, 7, to_string(struLastSerialNo->iCurTimes + 1).c_str());
                    m_consumeResultList.SetItemText(iConsumeResultSize, 8, to_string(struLastSerialNo->iCurTimes).c_str());
                                
                }
                else if (struLastSerialNo->strType == "refund")
                {
                    int iConsumeResultSize = m_consumeResultList.GetItemCount();
                    m_consumeResultList.InsertItem(iConsumeResultSize, struLastSerialNo->strSerialNo.c_str());
                    m_consumeResultList.SetItemText(iConsumeResultSize, 1, struLastSerialNo->strEmployeeNo.c_str());
                    m_consumeResultList.SetItemText(iConsumeResultSize, 2, struLastSerialNo->strCardNo.c_str());
                    m_consumeResultList.SetItemText(iConsumeResultSize, 3, struLastSerialNo->strType.c_str());
                    m_consumeResultList.SetItemText(iConsumeResultSize, 4, "");
                    m_consumeResultList.SetItemText(iConsumeResultSize, 5, "");
                    m_consumeResultList.SetItemText(iConsumeResultSize, 6, "");
                    m_consumeResultList.SetItemText(iConsumeResultSize, 7, "");
                    m_consumeResultList.SetItemText(iConsumeResultSize, 8, "");
                                
                }
                //��ȷ�ϵ���ˮ�����������ӵ�ǰ�����¼�
                m_serialNoListMap[struSerialNo.strSerialNo] = *struLastSerialNo;
            }
        }
        else
        {
            //�޲�����Ӧ�ñ���
        }
    }
    else
    {
        return NULL;
    }
    
    return NULL;
}

LRESULT CDlgConsumeSimulate::OnWMProcTransRecordAlarm(WPARAM wParam, LPARAM lParam)
{
    //g_struConsumeISAPIAlarm
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    TRANSRECORD_STRUCT struTransRecord; //���׼�¼�¼��ṹ�壬�洢jsonת���������
    cJSON *pRoot;
    pRoot = cJSON_Parse(g_struConsumeISAPIAlarm.pAlarmData);
    if (pRoot)
    {
        cJSON *pEventNode = cJSON_GetObjectItem(pRoot, "TransactionRecordEvent");
        if (!pEventNode)
        {
            //����ISAPI����ת�ṹ��
            TransRecordJsonToStruct(pEventNode, &struTransRecord);
        }
    }
    int iCount = m_transRecordList.GetItemCount();
    m_transRecordList.InsertItem(iCount, struTransRecord.strSerialNo.c_str());
    m_transRecordList.SetItemText(iCount, 1, struTransRecord.strFrontSerialNo.c_str());
    m_transRecordList.SetItemText(iCount, 2, struTransRecord.strEmployeeNo.c_str());
    m_transRecordList.SetItemText(iCount, 3, struTransRecord.strCardNo.c_str());
    m_transRecordList.SetItemText(iCount, 4, struTransRecord.strType.c_str());
    m_transRecordList.SetItemText(iCount, 5, to_string(struTransRecord.iTotalPayment).c_str());
    m_transRecordList.SetItemText(iCount, 6, to_string(struTransRecord.iActualPayment).c_str());
    m_transRecordList.SetItemText(iCount, 7, to_string(struTransRecord.iBalance).c_str());
    m_transRecordList.SetItemText(iCount, 8, to_string(struTransRecord.iTimes).c_str());
    m_transRecordList.SetItemText(iCount, 9, to_string(struTransRecord.iRemainingTimes).c_str());
    //���Ϊ���׼�¼�¼�
    //�ظ�ȷ�ϱ��ģ�������¼������ʾ
    //�·������¼�ȷ�ϱ���
    cJSON *pTemp = cJSON_CreateObject();
    if (pTemp == NULL)
    {
        return FALSE;
    }
    cJSON_AddStringToObject(pTemp, "serialNo", struTransRecord.strSerialNo.c_str());
    cJSON_AddStringToObject(pTemp, "result", "success");
    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "TransactionRecordEventConfirm", pTemp);

    char szUrl[256] = { 0 };
    char *pBuf = cJSON_Print(root);
    sprintf(szUrl, "/ISAPI/Consume/transactionRecordEventConfirm?format=json\r\n");
    NET_DVR_XML_CONFIG_INPUT    struInput = { 0 };
    NET_DVR_XML_CONFIG_OUTPUT   struOuput = { 0 };
    struInput.dwSize = sizeof(struInput);
    struOuput.dwSize = sizeof(struOuput);
    struInput.lpRequestUrl = szUrl;
    struInput.dwRequestUrlLen = strlen(szUrl);
    struInput.lpInBuffer = pBuf;
    struInput.dwInBufferSize = strlen(pBuf);
    if (!NET_DVR_STDXMLConfig(m_lUserID, &struInput, &struOuput))
    {

    }
    return NULL;
}

void CDlgConsumeSimulate::OnBnClickedBtnAddPerson()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    UpdateData(TRUE);
    if (m_strEmployeeNo.IsEmpty() || m_strCardNo.IsEmpty())
    {
        return;
    }

    //auto iter = m_employeeToCardMap.find(m_strEmployeeNo.GetBuffer());
    if (m_employeeToCardMap.find(m_strEmployeeNo.GetBuffer()) != m_employeeToCardMap.end() || m_cardToEmployeeMap.find(m_strCardNo.GetBuffer()) != m_cardToEmployeeMap.end())
    {
        return;
    }
    //auto iter1 = m_cardToEmployeeMap.find(m_strCardNo.GetBuffer());
    EMPLOTEENO_STRUCT struEmployeeNo(m_strEmployeeNo.GetBuffer(), m_iCurBalance, m_iCurTimes);
    CARDNO_STRUCT struCardNo(m_strCardNo.GetBuffer(),m_iCurBalance,m_iCurTimes);
    

    m_employeeToCardMap[m_strEmployeeNo.GetBuffer()] = struCardNo;
    m_cardToEmployeeMap[m_strEmployeeNo.GetBuffer()] = struEmployeeNo;

    int size = m_personList.GetItemCount();
    m_personList.InsertItem(size, m_strEmployeeNo);
    m_personList.SetItemText(size, 1, m_strCardNo);
    CString str;
    str.Format(_T("%d"), m_iCurBalance);
    m_personList.SetItemText(size, 2, str);
    str.Format(_T("%d"), m_iCurTimes);
    m_personList.SetItemText(size, 3, str);
    UpdateData(FALSE);
}


void CDlgConsumeSimulate::OnBnClickedBtnModifyPerson()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
}


void CDlgConsumeSimulate::OnBnClickedBtnDeletePerson()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
}
