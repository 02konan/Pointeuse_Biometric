#pragma once


// CDlgFaceConsume �Ի���

class CDlgFaceConsume : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgFaceConsume)

public:
	CDlgFaceConsume(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgFaceConsume();
    LONG m_lUserID;
    int m_iDeviceIndex;
// �Ի�������
	enum { IDD = IDD_DLG_FACE_CONSUME };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBtnConsumeSimulate();
};
