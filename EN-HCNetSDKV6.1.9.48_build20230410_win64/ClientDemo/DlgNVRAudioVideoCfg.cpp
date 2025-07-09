// DlgNVRAudioVideoCfg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ClientDemo.h"
#include "DlgNVRAudioVideoCfg.h"
#include "afxdialogex.h"


// CDlgNVRAudioVideoCfg �Ի���

IMPLEMENT_DYNAMIC(CDlgNVRAudioVideoCfg, CDialogEx)

CDlgNVRAudioVideoCfg::CDlgNVRAudioVideoCfg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgNVRAudioVideoCfg::IDD, pParent)
    , m_bOpen(FALSE)
    , m_iAudioChannel(0)
    , m_iVideoChannel(0)
    , m_lUserID(0)
    , m_iDeviceIndex(0)
{

}

CDlgNVRAudioVideoCfg::~CDlgNVRAudioVideoCfg()
{
}

void CDlgNVRAudioVideoCfg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_CHECK_OPEN, m_bOpen);
    DDX_Text(pDX, IDC_EDIT_AOCH, m_iAudioChannel);
    DDX_Text(pDX, IDC_EDIT_CH, m_iVideoChannel);
    DDX_Control(pDX, IDC_COMBO_LINK, m_comboInterface);
    DDX_Control(pDX, IDC_COMBO_SPLIT_NUM, m_comboSplitNum);
    DDX_Control(pDX, IDC_COMBO_TYPE_AV, m_comboType);
}


BEGIN_MESSAGE_MAP(CDlgNVRAudioVideoCfg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_SET, &CDlgNVRAudioVideoCfg::OnBnClickedBtnSet)
    ON_CBN_SELCHANGE(IDC_COMBO_TYPE_AV, &CDlgNVRAudioVideoCfg::OnCbnSelchangeComboTypeAv)
    ON_BN_CLICKED(IDC_BTN_EXIT, &CDlgNVRAudioVideoCfg::OnBnClickedBtnExit)
END_MESSAGE_MAP()

// CDlgNVRAudioVideoCfg ��Ϣ�������
BOOL CDlgNVRAudioVideoCfg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  �ڴ���Ӷ���ĳ�ʼ��
    m_comboInterface.SetCurSel(0);
    m_comboSplitNum.SetCurSel(0);
    m_comboType.SetCurSel(0);

    m_iAudioChannel = 2;
    m_iVideoChannel = 1;
    UpdateData(FALSE);
    showDlgView();
    return TRUE;  // return TRUE unless you set the focus to a control
    // �쳣:  OCX ����ҳӦ���� FALSE
}

/** @fn void CDlgNVRAudioVideoCfg::OnBnClickedBtnSet()
 *  @brief ��������Ƶ���ò���
 *  @return void
 */
void CDlgNVRAudioVideoCfg::OnBnClickedBtnSet()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    UpdateData(TRUE);

    NET_DVR_AUTOTEST_CFG_HEAD struAudoTestCfg = { 0 };
    struAudoTestCfg.dwSize = sizeof(NET_DVR_AUTOTEST_CFG_HEAD);
    int iType = m_comboType.GetCurSel();
    struAudoTestCfg.dwRetResult = 0;
    
    if (iType == 0) //��Ƶ
    {
        NET_DVR_AUTOTEST_AUDIO_CFG struAudio = { 0 };
        struAudio.dwOpen = m_bOpen;
        struAudio.dwVoCh = m_iAudioChannel;

        struAudoTestCfg.dwInfoType = 3;
        struAudoTestCfg.lpDataBody = &struAudio;
        struAudoTestCfg.dwDataBodySize = sizeof(NET_DVR_AUTOTEST_AUDIO_CFG);
    }
    else if (iType == 1) //��Ƶ
    {
        NET_DVR_AUTOTEST_VIDEO_CFG struVideo = { 0 };
        struVideo.dwVoCh = m_iVideoChannel;
        CString strNum;
        GetDlgItem(IDC_COMBO_SPLIT_NUM)->GetWindowText(strNum);
        struVideo.dwSplitScreenNums = atoi(strNum);
        struVideo.dwInterface = m_comboInterface.GetCurSel() + 1;

        struAudoTestCfg.dwInfoType = 1;
        struAudoTestCfg.lpDataBody = &struVideo;
        struAudoTestCfg.dwDataBodySize = sizeof(NET_DVR_AUTOTEST_VIDEO_CFG);
    }

    if (!NET_DVR_SetDVRConfig(m_lUserID, NET_DVR_SET_START_VIDEOAUDIO, 1, &struAudoTestCfg, sizeof(struAudoTestCfg) + struAudoTestCfg.dwDataBodySize))
    {
        g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_SET_START_VIDEOAUDIO");
    }
    else
    {
        g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_SET_START_VIDEOAUDIO");
    }
    return;
}

/** @fn void CDlgNVRAudioVideoCfg::OnCbnSelchangeComboTypeAv()
 *  @brief �����л�����Ƶ��ʾ
 *  @return void
 */
void CDlgNVRAudioVideoCfg::OnCbnSelchangeComboTypeAv()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    showDlgView();
}


/** @fn void CDlgNVRAudioVideoCfg::showDlgView()
 *  @brief �����л�����Ƶ��ʾ
 *  @return void
 */
void CDlgNVRAudioVideoCfg::showDlgView()
{
    int iType = m_comboType.GetCurSel();
    if (iType == 0) //��Ƶ
    {
        GetDlgItem(IDC_STATIC_VIDEO_CODING_TYPE)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_COMBO_SPLIT_NUM)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_EDIT_CH)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_COMBO_LINK)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_STATIC_SPLIT_NUM)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_STATIC_CH)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_STATIC_LINK)->ShowWindow(SW_HIDE);

        GetDlgItem(IDC_STATIC_AUDIO)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_EDIT_AOCH)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_STATIC_AOCH)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_CHECK_OPEN)->ShowWindow(SW_SHOW);
    }
    else if (iType == 1) //��Ƶ
    {
        GetDlgItem(IDC_STATIC_VIDEO_CODING_TYPE)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_COMBO_SPLIT_NUM)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_EDIT_CH)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_COMBO_LINK)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_STATIC_SPLIT_NUM)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_STATIC_CH)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_STATIC_LINK)->ShowWindow(SW_SHOW);

        GetDlgItem(IDC_STATIC_AUDIO)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_EDIT_AOCH)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_STATIC_AOCH)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_CHECK_OPEN)->ShowWindow(SW_HIDE);
    }
}

void CDlgNVRAudioVideoCfg::OnBnClickedBtnExit()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    CDialog::OnCancel();
}
