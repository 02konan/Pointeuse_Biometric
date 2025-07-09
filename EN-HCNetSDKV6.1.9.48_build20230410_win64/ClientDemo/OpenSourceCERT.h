#pragma once
#include "afxcmn.h"


// COpenSourceCERT �Ի���
#define TIMER_DOWNLOAD_OPENSOURCE_CERT_PROGRESS 1 //�ϴ����ؽ��ȶ�ʱ��
class COpenSourceCERT : public CDialogEx
{
	DECLARE_DYNAMIC(COpenSourceCERT)

public:
	COpenSourceCERT(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~COpenSourceCERT();

// �Ի�������
    enum { IDD = IDD_DLG_OPEN__SOURCE_CERT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
    LONG m_lUserID;
    int m_iDeviceIndex;
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedBtnExportSceneFile();
    CString m_csOpenSouceCertSavePath;
    CProgressCtrl m_ctrlProgress;
    int m_iDownloadHandle;
    BOOL m_bDownloading;      //��������
    
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    CStatic m_statProgress;
    virtual BOOL OnInitDialog();
};
