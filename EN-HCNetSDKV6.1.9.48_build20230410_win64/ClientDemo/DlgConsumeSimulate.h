#pragma once
#include "afxcmn.h"
#include <map>
#include "cjson/cJSON.h"

using namespace std;
// CDlgConsumeSimulate �Ի���

typedef struct tagCARDNO_STRUCT
{
    string strCardNo;
    int iCurBalance;
    int iCurTimes;
    tagCARDNO_STRUCT()
    {
        strCardNo = "";
        iCurBalance = 0;
        iCurTimes = 0;
    }
    tagCARDNO_STRUCT(string cardNo, int curBalance, int curTimes)
    {
        strCardNo = cardNo;
        iCurBalance = curBalance;
        iCurTimes = curTimes;
    }
}CARDNO_STRUCT, *LPCARDNO_STRUCT; 

typedef struct tagEMPLOTEENO_STRUCT
{
    string strEmployeeNo;
    int iCurBalance;
    int iCurTimes;
    tagEMPLOTEENO_STRUCT()
    {
        strEmployeeNo = "";
        iCurBalance = 0;
        iCurTimes = 0;
    }
    tagEMPLOTEENO_STRUCT(string employeeNo, int curBalance, int curTimes)
    {
        strEmployeeNo = employeeNo;
        iCurBalance = curBalance;
        iCurTimes = curTimes;
    }
}EMPLOTEENO_STRUCT, *LPEMPLOTEENO_STRUCT;

typedef struct tagTRANSRECORD_STRUCT
{
    string strSerialNo;   //���integer�����׼�¼��ˮ�ţ�����Ԥ����ȷ��������ˮ�ű���һ�£��뽻�׼�¼��ˮ�ű���һ�£�
    string strFrontSerialNo;//���integer����һ�����׼�¼��ˮ��
    string strName;//��ѡ��string��������128�ֽڣ�
    string strEmployeeNo;//��ѡ��string�����ţ�32�ֽڣ������źͿ��ű������ٷ���һ����������������أ��ϲ��Թ���Ϊ׼��
    string strCardNo;//��ѡ��string�����ţ�32�ֽڣ������źͿ��ű������ٷ���һ����������������أ��ϲ��Թ���Ϊ׼��
    string strModeType;//��ѡ��string������ģʽ���ͣ�current-ʵʱ���ף�offLine-���߼��˽���
    string strType;//��ѡ��string���������ͣ�transaction-���ף�refund-����
    string strRefundSerialNo;//������integer����Ҫ�������ˮ�ţ�typeΪrefund���ֶ���Ч���ұ��뷵�أ�
    string strVerifyMode;//��ѡ��string����֤��ʽ��card-ˢ����face-ˢ����
    string strMode;//��ѡ��string������ģʽ��amount-��quota-���count-�ƴΣ�
    int iTotalPayment;//��ѡ��Ӧ������λ���֣�
    int iActualPayment;//��ѡ��ʵ������λ���֣�
    int iBalance;//��ѡ������λ���֣�
    int iTimes;//��ѡ��integer�������Ѵ���
    int iRemainingTimes;//��ѡ��integer��ʣ�����
    string strContentType;//������string����Դ�������ͣ��������д���ͼƬʱ���ýڵ���뷵�أ�url-url��ʽ���䣬binary-�����Ʒ�ʽ����
    string strContent;//������string���ͣ���Դ��ʶID���������д���ͼƬʱ���ýڵ���뷵�أ���resourcesContentTypeΪbinaryʱ���ýڵ���ͼƬ��Content-ID�ϸ��Ӧ����resourcesContentTypeΪurlʱ,�ýڵ���д�����url
    int iPicturesNumber;//��ѡ��integer��ͼƬ����������������ͼƬ��Ŀ��ûͼƬʱ�����ֶ�Ϊ0�򲻷��أ�
}TRANSRECORD_STRUCT, *LPTRANSRECORD_STRUCT;

typedef struct tagSERIALNO_STRUCT
{
    string strCancel;
    string strSerialNo;
    string strEmployeeNo;
    string strCardNo;
    string strType;
    string strRefundSerialNo;
    string strMode;
    int iTotalPayment;
    int iCurBalance;
    int iCurTimes;
    tagSERIALNO_STRUCT()
    {
        strCancel = "";
        strSerialNo = "";
        strEmployeeNo = "";
        strCardNo = "";
        strType = "";
        strRefundSerialNo = "";
        strMode = "";
        iTotalPayment = 0;
        iCurBalance = 0;
        iCurTimes = 0;
    }
    
    tagSERIALNO_STRUCT(string cancelType, string SerialNo, string EmployeeNo, string CardNo, string transType, string RefundSerialNo, string mode, int totalPayment)
    {
        strCancel = cancelType;
        strSerialNo = SerialNo;
        strEmployeeNo = EmployeeNo;
        strCardNo = CardNo;
        strType = transType;
        strRefundSerialNo = RefundSerialNo;
        strMode = mode;
        iTotalPayment = totalPayment;
        iCurBalance = 0;
        iCurTimes = 0;
    }
}SERIALNO_STRUCT, *LPSERIALNO_STRUCT;

class CDlgConsumeSimulate : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgConsumeSimulate)

public:
	CDlgConsumeSimulate(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgConsumeSimulate();
    LONG m_lUserID;
    int m_iDeviceIndex;
// �Ի�������
	enum { IDD = IDD_DLG_CONSUME_SIMULATE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
    virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
    CString m_strEmployeeNo;
    CString m_strCardNo;
    DWORD m_iCurBalance;
    DWORD m_iCurTimes;
    CListCtrl m_personList;
    CListCtrl m_consumeResultList;
    CListCtrl m_transRecordList;
    map<string, CARDNO_STRUCT>     m_employeeToCardMap;  //��������Ϊ���ĵ���Ա��Ϣ
    map<string, EMPLOTEENO_STRUCT> m_cardToEmployeeMap;  //�������ݲ�ʹ��
    map<string, SERIALNO_STRUCT>   m_serialNoListMap;    //�����ȷ�ϵ���ˮ���¼���Ϣ
    map<string, SERIALNO_STRUCT>   m_serialNoTempListMap;   //���Ԥ�������ˮ���¼���Ϣ���ȴ�ȷ���¼��ϱ�
    LRESULT OnWMProcConsumeAlarm(WPARAM wParam, LPARAM lParam);
    LRESULT OnWMProcTransRecordAlarm(WPARAM wParam, LPARAM lParam);
    afx_msg void OnBnClickedBtnAddPerson();
    afx_msg void OnBnClickedBtnModifyPerson();
    afx_msg void OnBnClickedBtnDeletePerson();
    void ConsumeJsonToStruct(cJSON *pRoot, SERIALNO_STRUCT* pStruSerialNo);
    void TransRecordJsonToStruct(cJSON *pRoot, TRANSRECORD_STRUCT* pStruTransRecord);
};
