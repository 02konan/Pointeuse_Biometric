// OpenSourceCERT.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ClientDemo.h"
#include "OpenSourceCERT.h"
#include "afxdialogex.h"


// COpenSourceCERT �Ի���

IMPLEMENT_DYNAMIC(COpenSourceCERT, CDialogEx)

COpenSourceCERT::COpenSourceCERT(CWnd* pParent /*=NULL*/)
	: CDialogEx(COpenSourceCERT::IDD, pParent)
    , m_csOpenSouceCertSavePath(_T(""))
    , m_iDownloadHandle(-1)
    , m_bDownloading(FALSE)
{

}

COpenSourceCERT::~COpenSourceCERT()
{
}

void COpenSourceCERT::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_SAVE_PATH, m_csOpenSouceCertSavePath);
    DDX_Control(pDX, IDC_PROGRESS, m_ctrlProgress);
    DDX_Control(pDX, IDC_STATIC_PROGRESS, m_statProgress);
}


BEGIN_MESSAGE_MAP(COpenSourceCERT, CDialogEx)
    ON_BN_CLICKED(IDOK, &COpenSourceCERT::OnBnClickedOk)
    ON_BN_CLICKED(IDC_BTN_EXPORT_SCENE_FILE, &COpenSourceCERT::OnBnClickedBtnExportSceneFile)
    ON_WM_TIMER()
END_MESSAGE_MAP()


// COpenSourceCERT ��Ϣ�������


void COpenSourceCERT::OnBnClickedOk()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    CDialogEx::OnOK();
}


void COpenSourceCERT::OnBnClickedBtnExportSceneFile()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    CString strFilter = _T("All Files(*.*)|*.*||");
    CFileDialog fileChose(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter);

    if (fileChose.DoModal() != IDOK)
    {
        return;
    }
    m_csOpenSouceCertSavePath = fileChose.GetPathName();

    GetDlgItem(IDC_EDIT_SAVE_PATH)->SetWindowText(m_csOpenSouceCertSavePath);

    UpdateData(TRUE);
    m_iDownloadHandle = NET_DVR_StartDownload(m_lUserID, NET_SDK_DOWNLOAD_OPEN_SOURCE_CERT, NULL, 0, m_csOpenSouceCertSavePath.GetBuffer(0));
    if (m_iDownloadHandle == -1)
    {
        g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_StartDownload NET_SDK_DOWNLOAD_OPEN_SOURCE_CERT");
        return;
    }
    g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_StartDownload NET_SDK_DOWNLOAD_OPEN_SOURCE_CERT");

    m_bDownloading = TRUE;
    m_ctrlProgress.SetPos(0);
    m_statProgress.SetWindowText(_T("0%"));
    SetTimer(TIMER_DOWNLOAD_OPENSOURCE_CERT_PROGRESS, 100, NULL);

}


void COpenSourceCERT::OnTimer(UINT_PTR nIDEvent)
{
    // TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
    if (nIDEvent == TIMER_DOWNLOAD_OPENSOURCE_CERT_PROGRESS)
    {
        DWORD dwProgress = 0;
        LONG lStatus = -1;
       if (m_bDownloading)
        {
            lStatus = NET_DVR_GetDownloadState(m_iDownloadHandle, &dwProgress);
            if (lStatus == -1)
            {
                g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_GetDownloadState");
                if (!NET_DVR_StopDownload(m_iDownloadHandle))
                {
                    g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_StopDownload");
                }
                else
                {
                    g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_StopDownload");
                }
            }
            else
            {
                g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_GetDownloadState");
            }
        }
        switch (lStatus)
        {
        case 1: //�ϴ������سɹ�
        {
            m_ctrlProgress.SetPos(dwProgress);
            CString str;
            str.Format("%d", dwProgress);
            str += _T("%");
            m_statProgress.SetWindowText(str);
           if (m_bDownloading)
            {
                if (!NET_DVR_StopDownload(m_iDownloadHandle))
                {
                    g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_StopDownload");
                }
                else
                {
                    g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_StopDownload");
                }
                m_bDownloading = FALSE;
            }

            KillTimer(TIMER_DOWNLOAD_OPENSOURCE_CERT_PROGRESS);
            char sTitle[64] = { 0 };
            char sMsg[64] = { 0 };
            g_StringLanType(sTitle, "��Դ�ļ�", "open souce cert file");
            g_StringLanType(sMsg, "�ϴ�/�������", "Upload/Download finished.");
            MessageBox(sMsg, sTitle, MB_OK | MB_ICONWARNING);
            break;
        }
        case 2: //�����ϴ�������
        {
            m_ctrlProgress.SetPos(dwProgress);
            CString str;
            str.Format("%d", dwProgress);
            str += _T("%");
            m_statProgress.SetWindowText(str);
            break;
        }
        case 3: //�ϴ�������ʧ��
        {
            if (m_bDownloading)
            {
                if (!NET_DVR_StopDownload(m_iDownloadHandle))
                {
                    g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_StopDownload");
                }
                else
                {
                    g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_StopDownload");
                }
                m_bDownloading = FALSE;
            }
            m_iDownloadHandle = -1;
            KillTimer(TIMER_DOWNLOAD_OPENSOURCE_CERT_PROGRESS);
            char sTitle[64] = { 0 };
            char sMsg[64] = { 0 };
            g_StringLanType(sTitle, "��Դ�ļ�", "open souce cert file");
            g_StringLanType(sMsg, "�ϴ�/����ʧ��", "Upload/Download failed.");
            MessageBox(sMsg, sTitle, MB_OK | MB_ICONWARNING);
            break;
        }
        case 4: //����Ͽ�
        {
             if (m_bDownloading)
            {
                if (!NET_DVR_StopDownload(m_iDownloadHandle))
                {
                    g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_StopDownload");
                }
                else
                {
                    g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_StopDownload");
                }
                m_bDownloading = FALSE;
            }
             m_iDownloadHandle = -1;
            KillTimer(TIMER_DOWNLOAD_OPENSOURCE_CERT_PROGRESS);
            char sTitle[64] = { 0 };
            char sMsg[64] = { 0 };
            g_StringLanType(sTitle, "��Դ�ļ�", "open souce cert file");
            g_StringLanType(sMsg, "����Ͽ�", "Network disconnection.");
            MessageBox(sMsg, sTitle, MB_OK | MB_ICONWARNING);
            break;
        }
        }
    }


    CDialogEx::OnTimer(nIDEvent);
}


BOOL COpenSourceCERT::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  �ڴ���Ӷ���ĳ�ʼ��
    m_ctrlProgress.SetRange(0, 100);
    m_ctrlProgress.SetStep(1);
    m_ctrlProgress.SetPos(0);
    m_statProgress.SetWindowText(_T("0%"));
    m_iDeviceIndex = g_pMainDlg->GetCurDeviceIndex();
    return TRUE;  // return TRUE unless you set the focus to a control
    // �쳣:  OCX ����ҳӦ���� FALSE
}
