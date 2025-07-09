#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "afxdtctl.h"


// CDlgBatchUploadBlockList �Ի���

class CDlgBatchUploadBlockList : public CDialog
{
	DECLARE_DYNAMIC(CDlgBatchUploadBlockList)

public:
	CDlgBatchUploadBlockList(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgBatchUploadBlockList();

// �Ի�������
	enum { IDD = IDD_DLG_BATCH_UPLOAD_BLOCKLIST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

public:
    CListCtrl m_listViewOfBlockList;
    CString m_csName;
    CComboBox m_comboxSex;
    CComboBox m_comboxNation;
    CDateTimeCtrl m_dtcBirthday;
    BOOL m_bIsLongTermValid;
    CDateTimeCtrl m_dtcValidPeriodStart;
    CDateTimeCtrl m_dtcValidPeriodEnd;
    CString m_csAddress;
    CString m_csIDNumber;
    CString m_csIssuingAuthority;
    afx_msg void OnBnClickedButtonUploadList();
    afx_msg void OnBnClickedButtonDeleteByid();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedButtonAdd();
    afx_msg void OnBnClickedButtonDelete();
    afx_msg void OnBnClickedButtonClear();
    virtual BOOL OnInitDialog();
    afx_msg void OnClose();

public:
    LONG m_lServerID;
    int m_iDeviceIndex;

    BOOL m_bValid;
    int GetExistItem(const NET_DVR_UPLOAD_ID_BLOCKLIST_CFG *lpCardCfg);
    void ProcessUploadCfgCallbackData(DWORD dwType, void* lpBuffer, DWORD dwBufLen);
    BOOL SendFirst();
    BOOL SendNext();
    void Clear();
    DWORD m_dwSendIndex;       //�·����
    LONG m_lUploadCfgHandle;
    DWORD m_dwNumOfBlockID;
protected:
    afx_msg LRESULT OnMsgUploadFinish(WPARAM wParam, LPARAM lParam);
};
