#include "afxcmn.h"
#include "afxwin.h"
#if !defined(AFX_DLGUPLOADBLOCKLISTPICTURE_H__CB382A4E_8DAE_44E3_9FEA_3C69466D3BAB__INCLUDED_)
#define AFX_DLGUPLOADBLOCKLISTPICTURE_H__CB382A4E_8DAE_44E3_9FEA_3C69466D3BAB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CDlgUploadBlockListPicture �Ի���

class CDlgUploadBlockListPicture : public CDialogEx
{
    DECLARE_DYNAMIC(CDlgUploadBlockListPicture)

public:
    CDlgUploadBlockListPicture(CWnd* pParent = NULL);   // ��׼���캯��
    virtual ~CDlgUploadBlockListPicture();

// �Ի�������
    enum { IDD = IDD_DLG_ACS_BLOCKLIST_PICTURE_CFG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

    DECLARE_MESSAGE_MAP()

public:
    LONG m_lServerID;
    int m_iDeviceIndex;
    LONG m_lChannel;
    int GetExistItem(const NET_DVR_BLOCKLIST_PICTURE_CFG *lpCardCfg);
    char * ReadPictureFromFile(CString csFileName);
    void Clear();
    BOOL SendFirstPicture();
    BOOL SendNextPicture();
    DWORD m_dwSendIndex;       //�·����
    LONG m_lUploadPicCfgHandle;
    void ProcessUploadPictureCfgCallbackData(DWORD dwType, void* lpBuffer, DWORD dwBufLen);

    CComboBox m_comboSex;
    CString m_szName;
    CString m_szPictureName;
    DWORD m_dwPictureLen;
    DWORD m_dwLen;
    CListCtrl m_listPicture;
    CString m_szCardNumber;
    afx_msg void OnBnClickedButtonAdd();
    afx_msg void OnBnClickedButtonClear();
    afx_msg void OnBnClickedButtonUpload();
    afx_msg void OnBnClickedOk();
    afx_msg void OnDestroy();
    afx_msg LRESULT OnMsgUploadPictureCfgFinish(WPARAM wParam, LPARAM lParam);
    afx_msg void OnBnClickedButtonBrowse();
    virtual BOOL OnInitDialog();
    DWORD m_dwPictureNum;
    afx_msg void OnBnClickedButtonDelete();
    BOOL m_bIsValid;
    afx_msg void OnBnClickedCheckIsdelete();
};

#endif // !defined(AFX_DLGUPLOADBLOCKLISTPICTURE_H__CB382A4E_8DAE_44E3_9FEA_3C69466D3BAB__INCLUDED_)
