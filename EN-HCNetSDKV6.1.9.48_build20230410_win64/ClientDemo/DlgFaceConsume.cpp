// DlgFaceConsume.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ClientDemo.h"
#include "DlgFaceConsume.h"
#include "afxdialogex.h"
#include "DlgConsumeSimulate.h"

// CDlgFaceConsume �Ի���

IMPLEMENT_DYNAMIC(CDlgFaceConsume, CDialogEx)

CDlgFaceConsume::CDlgFaceConsume(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgFaceConsume::IDD, pParent)
{

}

CDlgFaceConsume::~CDlgFaceConsume()
{
}

void CDlgFaceConsume::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgFaceConsume, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_CONSUME_SIMULATE, &CDlgFaceConsume::OnBnClickedBtnConsumeSimulate)
END_MESSAGE_MAP()


// CDlgFaceConsume ��Ϣ�������


void CDlgFaceConsume::OnBnClickedBtnConsumeSimulate()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    CDlgConsumeSimulate dlg;
    g_pDlgConsumeSimulate = &dlg;
    dlg.m_lUserID = m_lUserID;
    dlg.m_iDeviceIndex = m_iDeviceIndex;
    dlg.DoModal();
    g_pDlgConsumeSimulate = NULL;
}
