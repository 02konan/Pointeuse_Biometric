// DlgCVRPassbackBasicCfg.cpp : 实现文件
//

#include "stdafx.h"
#include "ClientDemo.h"
#include "DlgCVRPassbackBasicCfg.h"
#include "afxdialogex.h"


// CDlgCVRPassbackBasicCfg 对话框

IMPLEMENT_DYNAMIC(CDlgCVRPassbackBasicCfg, CDialogEx)

CDlgCVRPassbackBasicCfg::CDlgCVRPassbackBasicCfg(CWnd* pParent /*=NULL*/)
: CDialogEx(CDlgCVRPassbackBasicCfg::IDD, pParent)
, m_iMaxTotalConcurrenceNum(0)
, m_iMaxDvrConcurrenceNum(0)
, m_lServerID(0)
, m_iDevIndex(0)
, m_dwStartTime(0)
, m_dwStopTime(0)
, m_dwSyncSpeed(0)
{
    memset(m_szStatusBuf, 0, ISAPI_STATUS_LEN);
}

CDlgCVRPassbackBasicCfg::~CDlgCVRPassbackBasicCfg()
{
}

void CDlgCVRPassbackBasicCfg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_MAX_DVR_CONCURRENCE_NUM, m_iMaxDvrConcurrenceNum);
    DDX_Text(pDX, IDC_EDIT_MAX_TOTAL_CONCURRENCE_NUM, m_iMaxTotalConcurrenceNum);
    DDX_Text(pDX, IDC_EDIT_START_TIME, m_dwStartTime);
    DDX_Text(pDX, IDC_EDIT_STOP_TIME, m_dwStopTime);
    DDX_Text(pDX, IDC_EDIT_SYNC_SPEED, m_dwSyncSpeed);
    DDX_Control(pDX, IDC_COMBO_RECORD_TYPE, m_comRecordType);
}


BEGIN_MESSAGE_MAP(CDlgCVRPassbackBasicCfg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_GET, &CDlgCVRPassbackBasicCfg::OnBnClickedButtonGet)
    ON_BN_CLICKED(IDC_BUTTON_SET, &CDlgCVRPassbackBasicCfg::OnBnClickedButtonSet)
END_MESSAGE_MAP()


// CDlgCVRPassbackBasicCfg 消息处理程序

BOOL CDlgCVRPassbackBasicCfg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  在此添加额外的初始化

    UpdateData(FALSE);
    return TRUE;  // return TRUE unless you set the focus to a control
    // 异常:  OCX 属性页应返回 FALSE
}

void CDlgCVRPassbackBasicCfg::OnBnClickedButtonGet()
{
    // TODO:  在此添加控件通知处理程序代码
    UpdateData(TRUE);
    NET_DVR_REC_PASSBACK_BASIC_CFG struRecPassbackBasicCfg = { 0 };
    struRecPassbackBasicCfg.dwSize = sizeof(struRecPassbackBasicCfg);

    NET_DVR_STD_CONFIG struCfg = { 0 };
    struCfg.lpCondBuffer = NULL;
    struCfg.dwCondSize = 0;
    struCfg.lpOutBuffer = &struRecPassbackBasicCfg;
    struCfg.dwOutSize = sizeof(struRecPassbackBasicCfg);
    memset(m_szStatusBuf, 0, ISAPI_STATUS_LEN);
    struCfg.lpStatusBuffer = m_szStatusBuf;
    struCfg.dwStatusSize = ISAPI_STATUS_LEN;
    if (!NET_DVR_GetSTDConfig(m_lServerID, NET_DVR_GET_RECORD_PASSBACK_BASIC_CFG, &struCfg))
    {
        OutputDebugString(m_szStatusBuf);
        g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "NET_DVR_GET_RECORD_PASSBACK_BASIC_CFG");
    }
    else
    {
        g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, "NET_DVR_GET_RECORD_PASSBACK_BASIC_CFG");
        m_dwStartTime = struRecPassbackBasicCfg.dwStartTime;
        m_dwStopTime = struRecPassbackBasicCfg.dwStopTime;
        m_iMaxTotalConcurrenceNum = struRecPassbackBasicCfg.wMaxTotalConcurrenceNum;
        m_iMaxDvrConcurrenceNum = struRecPassbackBasicCfg.wMaxDvrConcurrenceNum;
        m_dwSyncSpeed = struRecPassbackBasicCfg.dwSyncSpeed;
        DWORD m_dwRecordType;
        m_dwRecordType = struRecPassbackBasicCfg.dwRecordType;
        switch (m_dwRecordType)
        {
             case 0xffffffff:
                 m_comRecordType.SetCurSel(0);
                 break;
             case 0:
                 m_comRecordType.SetCurSel(1);
                 break;
             case 1:
                 m_comRecordType.SetCurSel(2);
                 break;
             case 2:
                 m_comRecordType.SetCurSel(3);
                 break;
             case 0xff:
                 m_comRecordType.SetCurSel(4);
                 break;
             default:
                 break;
        }
    }
    UpdateData(FALSE);
}


void CDlgCVRPassbackBasicCfg::OnBnClickedButtonSet()
{
    // TODO:  在此添加控件通知处理程序代码
    UpdateData(TRUE);

    NET_DVR_REC_PASSBACK_BASIC_CFG struRecPassbackBasicCfg = { 0 };
    struRecPassbackBasicCfg.dwSize = sizeof(struRecPassbackBasicCfg);
    struRecPassbackBasicCfg.dwStartTime = m_dwStartTime;
    struRecPassbackBasicCfg.dwStopTime = m_dwStopTime;
    struRecPassbackBasicCfg.wMaxTotalConcurrenceNum = m_iMaxTotalConcurrenceNum;
    struRecPassbackBasicCfg.wMaxDvrConcurrenceNum = m_iMaxDvrConcurrenceNum;
	struRecPassbackBasicCfg.dwSyncSpeed = m_dwSyncSpeed;
	int chooseIndex = m_comRecordType.GetCurSel();
    DWORD m_dwRecordType;
    switch (chooseIndex)
    {
    case 0:
        m_dwRecordType = 0xffffffff;
        break;
    case 1:
        m_dwRecordType = 0;
        break;
    case 2:
        m_dwRecordType = 1;
        break;
    case 3:
        m_dwRecordType = 2;
        break;
    case 4:
        m_dwRecordType = 0xff;
        break;
    default:
        break;
    }
	struRecPassbackBasicCfg.dwRecordType = m_dwRecordType;
    NET_DVR_STD_CONFIG struCfg = { 0 };
    struCfg.lpCondBuffer = NULL;
    struCfg.dwCondSize = 0;
    struCfg.lpInBuffer = &struRecPassbackBasicCfg;
    struCfg.dwInSize = sizeof(struRecPassbackBasicCfg);
    memset(m_szStatusBuf, 0, ISAPI_STATUS_LEN);
    struCfg.lpStatusBuffer = m_szStatusBuf;
    struCfg.dwStatusSize = ISAPI_STATUS_LEN;

    if (!NET_DVR_SetSTDConfig(m_lServerID, NET_DVR_SET_RECORD_PASSBACK_BASIC_CFG, &struCfg))
    {
        OutputDebugString(m_szStatusBuf);
        g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "NET_DVR_SET_RECORD_PASSBACK_BASIC_CFG");
    }
    else
    {
        g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, "NET_DVR_SET_RECORD_PASSBACK_BASIC_CFG");
    }

    UpdateData(FALSE);
}

