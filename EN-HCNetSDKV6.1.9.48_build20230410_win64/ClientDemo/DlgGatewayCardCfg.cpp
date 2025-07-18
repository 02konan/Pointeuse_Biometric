// DlgGatewayCardCfg.cpp : implementation file
//

#include "stdafx.h"
#include "clientdemo.h"
#include "DlgGatewayCardCfg.h"
#include "DlgACCardParam.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgGatewayCardCfg dialog

void CALLBACK g_fSetGatewayCardCallback(DWORD dwType, void* lpBuffer, DWORD dwBufLen, void* pUserData);
void CALLBACK g_fGetGatewayCardCallback(DWORD dwType, void* lpBuffer, DWORD dwBufLen, void* pUserData);
void CALLBACK g_fCardParamSyncCallback(DWORD dwType, void* lpBuffer, DWORD dwBufLen, void* pUserData);
UINT __cdecl  g_fSendCardCfgThread(LPVOID pParam);
UINT __cdecl  g_fShowCardListThread(LPVOID pParam);
#define WM_MSG_SETCARD_FINISH 1002
#define WM_MSG_GETCARD_FINISH 1003
#define WM_MSG_ADD_CARDCFG_TOLIST 1004
#define WM_MSG_UPDATEDATA_INTERFACE   1005
#define WM_MSG_CARD_PARAM_SYNC_FINISH   1006
#define WM_MSG_UPDATECARD 1007


CDlgGatewayCardCfg::CDlgGatewayCardCfg(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgGatewayCardCfg::IDD, pParent)
    , m_dwNowSendItem(0)
    , m_iSelListItem(-1)
    , m_lpRecordCardCfg(NULL)
    ,m_pDisplayListThread(NULL)
    ,m_dwBatchAddNum(0)
	,m_lpNowSendCard(NULL)
	,m_byLastCardSendTime(0)
    , m_iLocalControllerNo(0)
    , m_szCardNum(_T(""))
{
	//{{AFX_DATA_INIT(CDlgGatewayCardCfg)
	m_BcheckCardNo = TRUE;
	m_csBatchFlag = _T("Ready to Add");
	//}}AFX_DATA_INIT
	m_lServerID = -1;
	m_iDevIndex = -1;
	m_lSetCardCfgHandle = -1;
	m_lGetCardCfgHandle = -1;
    m_lCardParamSyncHandle = -1;
	m_hStopProcEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	m_pProcThread = NULL;
    m_BSendSel = FALSE; 
    m_lpSelSendCardCfg = NULL; 
}

CDlgGatewayCardCfg::~CDlgGatewayCardCfg()
{
    CloseHandle(m_hStopProcEvent);
    
    if (m_pProcThread != NULL && !m_pProcThread->m_bAutoDelete)
    {
        delete m_pProcThread;
    }

	if ( m_lpRecordCardCfg )
    {
		PULONG_PTR lpArr = m_lpRecordCardCfg;
// 		for (int i=0; i<m_dwCardNum; i++)
// 		{
// 			delete (LPNET_DVR_CARD_CFG_V50)lpArr[i]; 
// 		}
        delete [] m_lpRecordCardCfg; 
    }
}

void CDlgGatewayCardCfg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDlgGatewayCardCfg)
    DDX_Control(pDX, IDC_LIST_BELONGGROUP, m_listBelongGroup);
    DDX_Control(pDX, IDC_LIST_CARDRIGHTPLAN, m_listCardRightPlan);
    DDX_Control(pDX, IDC_LIST_DOORRIGHT, m_listDoorRight);
    DDX_Control(pDX, IDC_LIST_CARD, m_listCard);
    DDX_Check(pDX, IDC_CHK_ACGC_CHECKCARDNO, m_BcheckCardNo);
    DDX_Text(pDX, IDC_STATIC_BATCH_ADD_FLAG, m_csBatchFlag);
    //}}AFX_DATA_MAP
    DDX_Text(pDX, IDC_EDIT_LOCALCONTROLLER_NO, m_iLocalControllerNo);
    DDX_Text(pDX, IDC_EDIT_CardSearch, m_szCardNum);
}


BEGIN_MESSAGE_MAP(CDlgGatewayCardCfg, CDialog)
	//{{AFX_MSG_MAP(CDlgGatewayCardCfg)
	ON_BN_CLICKED(IDC_BTN_ADD_TO_LIST, OnBtnAddToList)
	ON_NOTIFY(LVN_DELETEITEM, IDC_LIST_CARD, OnDeleteitemListCard)
	ON_BN_CLICKED(IDC_BTN_MODIFY_TO_LIST, OnBtnModifyToList)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_CARD, OnDblclkListCard)
	ON_BN_CLICKED(IDC_BTN_DEL_CARD, OnBtnDelCard)
	ON_BN_CLICKED(IDC_BTN_SET_CARDCFG, OnBtnSetCardcfg)
	ON_BN_CLICKED(IDC_BTN_GET_ALL_CARD, OnBtnGetAllCard)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_MSG_SETCARD_FINISH,OnMsgSetcardCfgFinish)
	ON_MESSAGE(WM_MSG_GETCARD_FINISH,OnMsgGetcardCfgFinish)
    ON_MESSAGE(WM_MSG_CARD_PARAM_SYNC_FINISH, OnMsgCardParamSyncFinish)
    ON_MESSAGE(WM_MSG_ADD_CARDCFG_TOLIST,OnMsgAddCardCfgToList)
    ON_MESSAGE(WM_MSG_UPDATEDATA_INTERFACE,OnMsgUpdateData)
	ON_BN_CLICKED(IDC_BTN_CLEAR_ALL_CARD, OnBtnClearAllCard)
	ON_NOTIFY(NM_CLICK, IDC_LIST_CARD, OnClickListCardCfg)
	ON_BN_CLICKED(IDC_BUT_SIMULATION_ADD, OnButSimulationAdd)
	ON_BN_CLICKED(IDC_BTN_GET_SELECT, OnBtnGetSel)
	ON_BN_CLICKED(IDC_BTN_SET_SELECT, OnBtnSetSel)
	ON_BN_CLICKED(IDC_BUT_BATCH_ADD, OnButBatchAdd)
	ON_BN_CLICKED(IDC_BTN_STOP_SEND, OnBtnStopSend)
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BTN_CARD_PARAM_SYNC, &CDlgGatewayCardCfg::OnBnClickedBtnCardParamSync)
    ON_BN_CLICKED(IDC_BTN_CardSearch, &CDlgGatewayCardCfg::OnBnClickedBtnCardsearch)
    ON_MESSAGE(WM_MSG_UPDATECARD, &CDlgGatewayCardCfg::OnMsgUpdatecard)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgGatewayCardCfg message handlers

BOOL CDlgGatewayCardCfg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	InitCtrlState();

	return TRUE; 
}

void CDlgGatewayCardCfg::OnClose()
{
	static BOOL bNotifyQuit = FALSE;
	if ((m_pProcThread   != NULL &&  WaitForSingleObject(m_pProcThread->m_hThread,0) != WAIT_OBJECT_0))
	{  
		if (!bNotifyQuit)
		{
			SetEvent(m_hStopProcEvent);
			if (m_pProcThread != NULL)
			{
				m_pProcThread->SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL);
			}
			
			PostMessage(WM_CLOSE);
			bNotifyQuit = TRUE;
		}
		PostMessage(WM_CLOSE);
		return;
	}

	bNotifyQuit = FALSE;
    if (m_lGetCardCfgHandle != -1)
    {
        NET_DVR_StopRemoteConfig(m_lGetCardCfgHandle);
        m_lGetCardCfgHandle = -1;
    }

    if (m_lSetCardCfgHandle != -1)
    {
        NET_DVR_StopRemoteConfig(m_lSetCardCfgHandle);
        m_lSetCardCfgHandle = -1;
    }
	CDialog::OnClose();
}

void CDlgGatewayCardCfg::OnDestroy()
{
	StopProcThread();
	CDialog::OnDestroy();
}

LRESULT CDlgGatewayCardCfg::OnMsgSetcardCfgFinish(WPARAM wParam,LPARAM lParam)
{   
	NET_DVR_StopRemoteConfig(m_lSetCardCfgHandle);
	m_lSetCardCfgHandle = -1;
    g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, "NET_DVR_SET_CARD_CFG_V50 Set finish");
    ShowSelListItem(); 
	return 0;
}

LRESULT CDlgGatewayCardCfg::OnMsgGetcardCfgFinish(WPARAM wParam,LPARAM lParam)
{
	NET_DVR_StopRemoteConfig(m_lGetCardCfgHandle);
	m_lGetCardCfgHandle = -1;
    g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, "NET_DVR_GET_CARD_CFG_V50 Get finish");
    ShowSelListItem(); 
	return 0;
}

LRESULT CDlgGatewayCardCfg::OnMsgCardParamSyncFinish(WPARAM wParam, LPARAM lParam)
{
    if (m_lCardParamSyncHandle != -1)
    {
        NET_DVR_StopRemoteConfig(m_lCardParamSyncHandle);
        m_lCardParamSyncHandle = -1;
    }
    g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, "Card Param Sync finish");
    return 0;
}

LRESULT CDlgGatewayCardCfg::OnMsgAddCardCfgToList(WPARAM wParam,LPARAM lParam)
{
    char *pCardNo; 
    LPNET_DVR_CARD_CFG_V50 lpCardCfg = (LPNET_DVR_CARD_CFG_V50)wParam;
    if ( lpCardCfg->dwSize == 0)
    {
        return 0; 
    }
    pCardNo = (char *)lpCardCfg->byCardNo; 
    AddToCardList(*lpCardCfg, pCardNo); 
    delete lpCardCfg; 
    return 0; 
}

LRESULT CDlgGatewayCardCfg::OnMsgUpdateData(WPARAM wParam,LPARAM lParam)
{
    DWORD dwTrue = (DWORD)wParam; 
    UpdateData(dwTrue); 
    return 0; 
}


void CDlgGatewayCardCfg::InitCtrlState()
{
    int iIndex = 0;
    char szLan[128];
    g_StringLanType(szLan, "编号", "number");
	m_listCard.InsertColumn( iIndex++,szLan,LVCFMT_LEFT,60);
    g_StringLanType(szLan, "卡号", "car number");
	m_listCard.InsertColumn( iIndex++,szLan,LVCFMT_LEFT,100); //1
    g_StringLanType(szLan, "卡有效", "CardValid");
    m_listCard.InsertColumn( iIndex++,szLan,LVCFMT_LEFT,60);
    g_StringLanType(szLan, "卡密码", "CardPassword");
    m_listCard.InsertColumn( iIndex++,szLan,LVCFMT_LEFT,60);
    g_StringLanType(szLan, "卡类型", "CardType");
    m_listCard.InsertColumn( iIndex++,szLan,LVCFMT_LEFT,60);

    g_StringLanType(szLan, "首卡", "LeaderCard");
    m_listCard.InsertColumn( iIndex++,szLan,LVCFMT_LEFT,60); //5
    g_StringLanType(szLan, "最大刷卡", "MaxSwipeTime");
    m_listCard.InsertColumn( iIndex++,szLan,LVCFMT_LEFT,80);
    g_StringLanType(szLan, "已刷卡", "dwSwipeTime");
    m_listCard.InsertColumn( iIndex++,szLan,LVCFMT_LEFT,80);
    g_StringLanType(szLan, "卡有效期使能", "valid enable");
    m_listCard.InsertColumn( iIndex++,szLan,LVCFMT_LEFT,100);  //8
    g_StringLanType(szLan, "卡有效开始时间", "BeginTime");
    m_listCard.InsertColumn( iIndex++,szLan,LVCFMT_LEFT,150);
    g_StringLanType(szLan, "卡有效结束时间", "EndTime");
    m_listCard.InsertColumn( iIndex++,szLan,LVCFMT_LEFT,150);
    g_StringLanType(szLan, "发送状态", "send status");
    m_listCard.InsertColumn( iIndex++,szLan,LVCFMT_LEFT,150);
    g_StringLanType(szLan, "房间号", "room number");
    m_listCard.InsertColumn( iIndex++,szLan,LVCFMT_LEFT,150);
    g_StringLanType(szLan, "层号", "floor number");
    m_listCard.InsertColumn( iIndex++,szLan,LVCFMT_LEFT,150);
    g_StringLanType(szLan, "工号", "employee number");
    m_listCard.InsertColumn(iIndex++, szLan, LVCFMT_LEFT, 150);
    g_StringLanType(szLan, "姓名", "name");
    m_listCard.InsertColumn(iIndex++, szLan, LVCFMT_LEFT, 150);
    g_StringLanType(szLan, "部门编号", "department number");
    m_listCard.InsertColumn(iIndex++, szLan, LVCFMT_LEFT, 150);
    g_StringLanType(szLan, "排班计划编号", "schedule plan number");
    m_listCard.InsertColumn(iIndex++, szLan, LVCFMT_LEFT, 150);
    g_StringLanType(szLan, "权限", "right");
    m_listCard.InsertColumn(iIndex++, szLan, LVCFMT_LEFT, 150);
    g_StringLanType(szLan, "排班计划类型", "schedule plan type");
    m_listCard.InsertColumn(iIndex++, szLan, LVCFMT_LEFT, 150);
    g_StringLanType(szLan, "下发权限类型", "RightType");
    m_listCard.InsertColumn(iIndex++, szLan, LVCFMT_LEFT, 150);  //20
    g_StringLanType(szLan, "手机号", "SIM No");
    m_listCard.InsertColumn(iIndex++, szLan, LVCFMT_LEFT, 150);  //21
    g_StringLanType(szLan, "用户类型", "User Type");
    m_listCard.InsertColumn(iIndex++, szLan, LVCFMT_LEFT, 150);  //22

    m_byListSendStatus = iIndex - 1 ; 
	m_listCard.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);

    g_StringLanType(szLan, "门编号", "door number");
    m_listDoorRight.InsertColumn(0, szLan, LVCFMT_LEFT,60);
    g_StringLanType(szLan, "有权限", "right");
    m_listDoorRight.InsertColumn(1, szLan, LVCFMT_LEFT,60);
    
    m_listDoorRight.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
    g_StringLanType(szLan, "群组号", "group number");
    m_listBelongGroup.InsertColumn(0, szLan, LVCFMT_LEFT,60);
    g_StringLanType(szLan, "属于", "belong");
    m_listBelongGroup.InsertColumn(1, szLan, LVCFMT_LEFT,60);
    m_listBelongGroup.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
    g_StringLanType(szLan, "门下标", "door index");
    m_listCardRightPlan.InsertColumn(0, szLan, LVCFMT_LEFT,55);
    g_StringLanType(szLan, "卡权限下标", "card right index");
    m_listCardRightPlan.InsertColumn(1, szLan, LVCFMT_LEFT,70);
    g_StringLanType(szLan, "计划模板编号", "plan index");
    m_listCardRightPlan.InsertColumn(2, szLan, LVCFMT_LEFT,150);
    m_listCardRightPlan.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
}

void CDlgGatewayCardCfg::OnBtnAddToList() 
{
//	CDlgGatewayCardInfo dlg;
    CDlgACCardParam  dlg; 
    NET_DVR_CARD_CFG_V50 struCardCfg = { 0 };
    dlg.SetCardCfg(struCardCfg);     
	if (dlg.DoModal() == IDOK)
	{
        char szCardNo[ACS_CARD_NO_LEN]; 
		struCardCfg = dlg.GetCardCfg(szCardNo);
		AddToCardList(struCardCfg, szCardNo);
        UpdateSubList(&struCardCfg);
	}
}

void CDlgGatewayCardCfg::OnDeleteitemListCard(NMHDR* pNMHDR, LRESULT* pResult) 
{
// 	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
// 	GATEWAY_CARD_INFO* pCardInfo = (GATEWAY_CARD_INFO*)pNMListView->lParam;
// 	if (pCardInfo != NULL)
// 	{
// 		delete pCardInfo;
// 	}	
// 	*pResult = 0;
}


int CDlgGatewayCardCfg::GetExistItem(const NET_DVR_CARD_CFG_V50 *lpCardCfg)
{
    int nItemCount = m_listCard.GetItemCount();
    int i = 0; 
    LPNET_DVR_CARD_CFG_V50 lpTemp = NULL;
    for (i=0; i<nItemCount; i++)
    {
        lpTemp = (LPNET_DVR_CARD_CFG_V50)m_listCard.GetItemData(i);
        if ( ! lpCardCfg )
        {
            continue; 
        }
        if (strncmp((char *)lpCardCfg->byCardNo, (char *)lpTemp->byCardNo, 32) == 0)
        {
            return i;
        }
        //if ( strcmp((char *)lpCardCfg->byCardNo, (char *)lpTemp->byCardNo) == 0)
        //{
        //    return i; 
        //}
    }
    return -1; 
} 


void CDlgGatewayCardCfg::AddToCardList(const NET_DVR_CARD_CFG_V50& struCardInfo, const char * pCardNo)
{
    LPNET_DVR_CARD_CFG_V50 pCardInfo = NULL;
    int iItemIndex   = GetExistItem(&struCardInfo); 
    if ( iItemIndex == -1)
    {
        pCardInfo = new NET_DVR_CARD_CFG_V50;
        int iSize = sizeof(NET_DVR_CARD_CFG_V50);
        int nItemCount = m_listCard.GetItemCount();
    
        CString strItem = "";
        strItem.Format("%d",nItemCount + 1);
        iItemIndex = m_listCard.InsertItem(nItemCount,strItem);
		m_listCard.SetItemData(nItemCount, (ULONG_PTR)pCardInfo);
    }
    else
    {
        pCardInfo = (LPNET_DVR_CARD_CFG_V50)m_listCard.GetItemData(iItemIndex);
    }
    memcpy(pCardInfo,&struCardInfo,sizeof(struCardInfo));
    UpdateList(iItemIndex, pCardNo, *pCardInfo); 
	PULONG_PTR lpArr = GetCardCfgPoint();
	lpArr[iItemIndex] = (ULONG_PTR)pCardInfo;

}

void CDlgGatewayCardCfg::ModifyToCardList(int nItemIndex, const NET_DVR_CARD_CFG_V50& struCardInfo)
{
//    UpdateList(nItemIndex, struCardInfo); 
}





void CDlgGatewayCardCfg::OnBtnModifyToList() 
{
	int nSelIndex = m_listCard.GetNextItem(-1,LVNI_SELECTED);
	if (nSelIndex == -1)
	{
		return;
	}

    NET_DVR_CARD_CFG_V50* pCardInfo = (NET_DVR_CARD_CFG_V50*)m_listCard.GetItemData(nSelIndex);
	if (pCardInfo == NULL)
	{
		return;
	}
    CString csCardNo; 
    csCardNo = m_listCard.GetItemText(nSelIndex, 1); //卡编号 
    char szCardNo[ACS_CARD_NO_LEN]; 
    strncpy(szCardNo, (LPCTSTR)csCardNo, sizeof(szCardNo));
	CDlgACCardParam dlg;
	dlg.SetCardCfg(*pCardInfo, szCardNo);
	if (dlg.DoModal() == IDOK)
    {
		*pCardInfo = dlg.GetCardCfg (szCardNo);
        UpdateList(nSelIndex, szCardNo, *pCardInfo); 
        UpdateSubList(pCardInfo);
	}
}

void CDlgGatewayCardCfg::OnDblclkListCard(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnBtnModifyToList();
	*pResult = 0;
}

void CDlgGatewayCardCfg::OnBtnDelCard() 
{
	// TODO: Add your control notification handler code here
	
}



void CDlgGatewayCardCfg::OnBtnSetCardcfg() 
{
	if (m_lSetCardCfgHandle != -1)
	{
		NET_DVR_StopRemoteConfig(m_lSetCardCfgHandle);
	}
    UpdateData(TRUE);
    m_dwCardNum = m_listCard.GetItemCount();
	NET_DVR_CARD_CFG_COND struCond = {0};
	struCond.dwSize  = sizeof(struCond);
	struCond.dwCardNum = m_dwCardNum;
    //struCond.dwCardNum = 3;
    struCond.byCheckCardNo = (BYTE)m_BcheckCardNo;
    struCond.wLocalControllerID = m_iLocalControllerNo;

    m_lSetCardCfgHandle = NET_DVR_StartRemoteConfig(m_lServerID, NET_DVR_SET_CARD_CFG_V50, &struCond, sizeof(struCond), g_fSetGatewayCardCallback, this);
	if (m_lSetCardCfgHandle == -1)
	{
    	g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "NET_DVR_SET_CARD_CFG_V50");
		return;
	}
	else
	{
		g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, "NET_DVR_SET_CARD_CFG_V50");
	}
//	StartProcThread();
	//发送第一张卡, 失败关闭连接
	if ( !SendFirstCard())
	{
		NET_DVR_StopRemoteConfig(m_lSetCardCfgHandle);
		m_lSetCardCfgHandle = -1;
	}
	
}


void CDlgGatewayCardCfg::OnBtnGetAllCard() 
{
	if (m_lSetCardCfgHandle != -1)
	{
		NET_DVR_StopRemoteConfig(m_lSetCardCfgHandle);
	}
    //清除所有卡项
//    ClearList(); 
    m_listCard.DeleteAllItems(); 
    UpdateData(TRUE); 
	NET_DVR_CARD_CFG_COND struCond = {0};
	struCond.dwSize  = sizeof(struCond);
	struCond.dwCardNum = 0xffffffff;
	struCond.byCheckCardNo = (BYTE)m_BcheckCardNo; 
    struCond.wLocalControllerID = m_iLocalControllerNo;

    m_lGetCardCfgHandle = NET_DVR_StartRemoteConfig(m_lServerID, NET_DVR_GET_CARD_CFG_V50, &struCond, sizeof(struCond), g_fGetGatewayCardCallback, this);
	if (m_lGetCardCfgHandle == -1)
	{
		g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "NET_DVR_GET_CARD_CFG_V50");
		return;
	}
	else
	{
		g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, "NET_DVR_GET_CARD_CFG_V50");
	}
}

void CDlgGatewayCardCfg::SendCardData(LPNET_DVR_CARD_CFG_V50 lpCardCfg, DWORD dwDiffTime)
{
    if ( m_lSetCardCfgHandle == -1)
    {
        return; 
    }
    if (!NET_DVR_SendRemoteConfig(m_lSetCardCfgHandle,3/*ENUM_ACS_SEND_DATA*/, (char *)lpCardCfg ,sizeof(*lpCardCfg)))
    {
        g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "Send Fail,CardNO: %s", m_lpNowSendCard->byCardNo);    
        //发送失败  关闭连接
        PostMessage(WM_MSG_SETCARD_FINISH,0,0);
        return ; 
    }
	return ; 
}

void CDlgGatewayCardCfg::SendCardData()
{   
	if (m_lSetCardCfgHandle == -1)
	{
		return;
	}
    LPNET_DVR_CARD_CFG_V50 lpCardCfg = NULL;
    if ( m_BSendSel )
    {
//         lpCardCfg = GetSelItem();
        SendCardData( m_lpSelSendCardCfg ); 
        m_BSendSel = FALSE; 
        m_lpSelSendCardCfg = NULL; 
        return; 
    }
	int nItemCount = m_listCard.GetItemCount();
	PULONG_PTR lpArr = GetCardCfgPoint();
	DWORD beforeWait = 0;  
	DWORD afterWait = 0;
	for (int i = 0; i < nItemCount; i++)
	{
	
//        lpCardCfg = (LPNET_DVR_CARD_CFG_V50) m_listCard.GetItemData(i);
        lpCardCfg = (LPNET_DVR_CARD_CFG_V50)lpArr[i];
        if ( ! lpCardCfg )
        {
            continue; 
        }
//		beforeWait = GetTickCount(); 
        if ( i%10 == 9 && WaitForSingleObject(m_hStopProcEvent,0) == WAIT_OBJECT_0)
        {
            break;
		}
//		afterWait = GetTickCount(); 
        m_dwNowSendItem = i; 
        SendCardData(lpCardCfg, afterWait-beforeWait); 
	}
}

void CDlgGatewayCardCfg::ProcessSetCardCfgCallbackData(DWORD dwType, void* lpBuffer, DWORD dwBufLen)
{   
	CString strItem = "";
	if (dwType != NET_SDK_CALLBACK_TYPE_STATUS)
	{
		g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, "recv unknow type[%d]", dwType);
		return;
	}

	DWORD dwStatus = *(DWORD*)lpBuffer;


	if (dwStatus == NET_SDK_CALLBACK_STATUS_PROCESSING)
	{   
		char szCardNumber[ACS_CARD_NO_LEN + 1] = "\0";
		strncpy(szCardNumber,(char*)(lpBuffer) + 4,ACS_CARD_NO_LEN);
        g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, "SetCard PROCESSING %s", szCardNumber);
		//SetCardCfgState(szCardNumber,TRUE);
		SendNextCard(); 
	}
	else if (dwStatus == NET_SDK_CALLBACK_STATUS_FAILED)
	{
		char szCardNumber[ACS_CARD_NO_LEN + 1] = "\0";
		DWORD dwErrCode = *((DWORD*)lpBuffer + 1);
		strncpy(szCardNumber,(char*)(lpBuffer) + 8,ACS_CARD_NO_LEN);
		g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "SetCard Err:%d,CardNo:%s", dwErrCode,  szCardNumber);
		if (m_lpNowSendCard !=NULL)
		{
            if (ReSendLastCard())
            {
                return ; 
		    }
		}
        
        SendNextCard(); 
	}
	//下面两个关闭长连接
	else if (dwStatus == NET_SDK_CALLBACK_STATUS_SUCCESS)
	{
        g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, "SetCard SUCCESS");
		PostMessage(WM_MSG_SETCARD_FINISH,0,0);

	}
	else if (dwStatus == NET_SDK_CALLBACK_STATUS_EXCEPTION)
	{
		PostMessage(WM_MSG_SETCARD_FINISH,0,0);
	}
}

void CDlgGatewayCardCfg::ProcessGetCardCfgCallbackData(DWORD dwType, void* lpBuffer, DWORD dwBufLen)
{   
	CString strItem = "";
	if (dwType == NET_SDK_CALLBACK_TYPE_DATA)
	{
        LPNET_DVR_CARD_CFG_V50 lpCardCfg = new NET_DVR_CARD_CFG_V50;
        memcpy(lpCardCfg, lpBuffer, sizeof(*lpCardCfg)); 
        if (g_struDeviceInfo[m_iDevIndex].byCharaterEncodeType == 6)
        {
            BYTE tempName[NAME_LEN + 1] = { 0 };
            memcpy(tempName, lpCardCfg->byName, sizeof(lpCardCfg->byName));
            string str_gb2312 = UTF2GB((const char *)tempName);
            memset(lpCardCfg->byName, 0, sizeof(lpCardCfg->byName));
            strncpy((char *)lpCardCfg->byName, str_gb2312.c_str(), str_gb2312.length());
        }
		PostMessage(WM_MSG_ADD_CARDCFG_TOLIST, (WPARAM)lpCardCfg,0);
	}
	else if (dwType == NET_SDK_CALLBACK_TYPE_STATUS)
	{
		DWORD dwStatus = *(DWORD*)lpBuffer;
		if (dwStatus == NET_SDK_CALLBACK_STATUS_SUCCESS)
		{
			PostMessage(WM_MSG_GETCARD_FINISH,0,0);
		}
		else if ( dwStatus == NET_SDK_CALLBACK_STATUS_FAILED )
		{
			char szCardNumber[ACS_CARD_NO_LEN + 1] = "\0";
			DWORD dwErrCode = *(DWORD*)((char *)lpBuffer + 4); 
			strncpy(szCardNumber,(char*)(lpBuffer) + 8,ACS_CARD_NO_LEN);
			g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "GetCard STATUS_FAILED, Error code %d, Card Number %s", dwErrCode,  szCardNumber);
		}
	}
}

void CDlgGatewayCardCfg::ProcessCardParamSyncCallbackData(DWORD dwType, void* lpBuffer, DWORD dwBufLen)
{
    CString strItem = "";

    if (dwType == NET_SDK_CALLBACK_TYPE_STATUS)
    {
        DWORD dwStatus = *(DWORD*)lpBuffer;
        if (dwStatus == NET_SDK_CALLBACK_STATUS_PROCESSING)
        {
            ;
        }
        //返回下面状态时关闭长连接
        else if (dwStatus == NET_SDK_CALLBACK_STATUS_FAILED)
        {
            g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "Card Param Sync Failed");
            PostMessage(WM_MSG_CARD_PARAM_SYNC_FINISH, 0, 0);
        }
        else if (dwStatus == NET_SDK_CALLBACK_STATUS_SUCCESS)
        {
            g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, "Card Param Sync Success");
            PostMessage(WM_MSG_CARD_PARAM_SYNC_FINISH, 0, 0);
        }
        else if (dwStatus == NET_SDK_CALLBACK_STATUS_EXCEPTION)
        {
            g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "Card Param Sync Exception");
            PostMessage(WM_MSG_CARD_PARAM_SYNC_FINISH, 0, 0);
        }
        else
        {
            g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "Card Param Sync Other Exception");
            PostMessage(WM_MSG_CARD_PARAM_SYNC_FINISH, 0, 0);
        }
    }
    else if (dwType == NET_SDK_CALLBACK_TYPE_DATA)
    {
        cJSON *root, *json_CardParamSync, *json_progressValue;
        root = cJSON_Parse((const char*)lpBuffer);
        json_CardParamSync = cJSON_GetObjectItem(root, "CardParamSync");
        json_progressValue = cJSON_GetObjectItem(json_CardParamSync, "progressValue");
        int iProgressValue = json_progressValue->valueint;
        g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, "Card Param Sync Progress:[%d]", iProgressValue);
        cJSON_Delete(root);

        if (iProgressValue != 100)
        {
            cJSON* pRoot = cJSON_CreateObject();
            cJSON* pItem = cJSON_CreateObject();
            cJSON_AddItemToObject(pRoot, "CardParamSyncCond", pItem);
            cJSON_AddStringToObject(pItem, "command", "getProgress");
            cJSON_AddNumberToObject(pItem, "maxResults", 30);

            Sleep(2000);    //避免获取进度过快，休眠2s

            char *out = cJSON_Print(pRoot);
            if (!NET_DVR_SendRemoteConfig(m_lCardParamSyncHandle, ENUM_SEND_JSON_DATA, out, strlen(out)))
            {
                g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "Get Card Param Sync Failed");
                PostMessage(WM_MSG_CARD_PARAM_SYNC_FINISH, 0, 0);
            }

            cJSON_Delete(pRoot);
            free(out);
        }
        else
        {
            PostMessage(WM_MSG_CARD_PARAM_SYNC_FINISH, 0, 0);
        }
    }

}

BOOL CDlgGatewayCardCfg::StartProcThread()
{
	if (m_pProcThread != NULL)
	{
		StopProcThread();
	}
	m_pProcThread = AfxBeginThread(g_fSendCardCfgThread,this,THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED,NULL);
	if (m_pProcThread != NULL)
	{
		m_pProcThread->m_bAutoDelete = FALSE;
		m_pProcThread->ResumeThread();
	}
	BOOL bResult =  m_pProcThread != NULL;
	return bResult;
}

BOOL CDlgGatewayCardCfg::StopProcThread()
{
	BOOL bResult = TRUE;	
	if (m_pProcThread != NULL)
	{   
		DWORD dwResult = WaitForSingleObject(m_pProcThread->m_hThread,5 * 1000);
		if (dwResult == WAIT_TIMEOUT)
		{   
			DWORD dwExitCode = 0;
			if (GetExitCodeThread(m_pProcThread->m_hThread,&dwExitCode) && dwExitCode == STILL_ACTIVE)
			{   
				bResult = TerminateThread(m_pProcThread->m_hThread,0);
			}
		}		
		if (bResult)
		{
			if (!m_pProcThread->m_bAutoDelete)
			{
				delete m_pProcThread;
				m_pProcThread = NULL;
			}
		}
	}
	if (m_pDisplayListThread != NULL)
	{   
		DWORD dwResult = WaitForSingleObject(m_pDisplayListThread->m_hThread,0);
		if (dwResult == WAIT_TIMEOUT)
		{   
			DWORD dwExitCode = 0;
			if (GetExitCodeThread(m_pDisplayListThread->m_hThread,&dwExitCode) && dwExitCode == STILL_ACTIVE)
			{   
				bResult = TerminateThread(m_pDisplayListThread->m_hThread,0);
			}
		}		
		if (bResult)
		{
			if (!m_pDisplayListThread->m_bAutoDelete)
			{
				delete m_pDisplayListThread;
				m_pDisplayListThread = NULL;
			}
		}
	}
	return bResult;
}

void CDlgGatewayCardCfg::SetCardCfgState(CString strCardNumber,BOOL bSucc,CString strUserDefineState /*= ""*/)
{   
	CString strCardNO = "";
    int nItemCount = m_listCard.GetItemCount();
    //先尝试匹配现在发送记录项
    if ( nItemCount > m_dwNowSendItem)
    {
        strCardNO = m_listCard.GetItemText(m_dwNowSendItem,1);
        if (strCardNO == strCardNumber)
		{   
//			g_pMainDlg->AddLog(-1, OPERATION_SUCC_T, "match now Send Item"); 
            if (strUserDefineState.IsEmpty())
            {
                m_listCard.SetItemText(m_dwNowSendItem, m_byListSendStatus,bSucc ? "OK" : "Error");
            }
            else
            {
				m_listCard.SetItemText(m_dwNowSendItem, m_byListSendStatus, strUserDefineState);
			}
			return; 
        }
    }
    //记录项不准确时候，循环查询
    for (int i = 0; i < nItemCount; i++)
	{
		strCardNO = m_listCard.GetItemText(i,1);
		if (strCardNO == strCardNumber)
		{   
			if (strUserDefineState.IsEmpty())
			{
				m_listCard.SetItemText(i, m_byListSendStatus,bSucc ? "OK" : "Error");
			}
			else
			{
				m_listCard.SetItemText(i, m_byListSendStatus, strUserDefineState);
			}
			break;
		}
	}
}

void CDlgGatewayCardCfg::OnBtnClearAllCard() 
{   
	char szCn[128] = "";
	char szEn[128] = "";
	char szErrInfo[128] = "";
	sprintf(szCn,"%s","此操作将清空所有的门禁卡信息，请确认?");
	sprintf(szEn,"%s","Note:This operation will clear all of the access card information, continue? ");
    g_StringLanType(szErrInfo,szCn,szEn);
	if (AfxMessageBox(szErrInfo,MB_OKCANCEL) != IDOK)
	{
		return;
	}

	NET_DVR_ACS_PARAM_TYPE struAcsParamType = {0};

	memset(&struAcsParamType,0,sizeof(struAcsParamType));
	struAcsParamType.dwSize = sizeof(struAcsParamType);
	struAcsParamType.dwParamType |= ACS_PARAM_CARD;

	if (NET_DVR_RemoteControl(m_lServerID,NET_DVR_CLEAR_ACS_PARAM,&struAcsParamType,sizeof(struAcsParamType)))
	{
		g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, "NET_DVR_CLEAR_ACS_PARAM");
	}
	else
	{
		g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "NET_DVR_CLEAR_ACS_PARAM");
	}
}

void CDlgGatewayCardCfg::UpdateSubList(const NET_DVR_CARD_CFG_V50 *lpCardCfg)
{
    if ( ! lpCardCfg )
    {
        return; 
    }
    m_listDoorRight.DeleteAllItems(); 
    m_listBelongGroup.DeleteAllItems(); 
    m_listCardRightPlan.DeleteAllItems(); 
    int i=0; 
    CString csTemp; 
//     if ( lpCardCfg->dwModifyParamType & 0x8 ) //门权限 
//     {
    
        for ( i =0; i<MAX_DOOR_NUM_256; i++)
        {
            csTemp.Format("%d", i+1); 
            m_listDoorRight.InsertItem(i, csTemp);
            m_listDoorRight.SetItemText(i, 1, (lpCardCfg->byDoorRight[i])>0 ? "Yes" : "No");
        }
//     }
//     if ( lpCardCfg->dwModifyParamType & 0x40 ) //所属群组参数
//     {
    
        for (i = 0; i<MAX_GROUP_NUM_128; i++)
        {
            csTemp.Format("%d", i+1); 
            m_listBelongGroup.InsertItem(i, csTemp);
            m_listBelongGroup.SetItemText(i, 1, (lpCardCfg->byBelongGroup[i] >0 ? "Yes" : "No"));
        }
//     }
//     if ( lpCardCfg->dwModifyParamType & 0x100 ) //卡权限计划参数
//     {
//     
        for (i = 0; i<MAX_DOOR_NUM_256; i++)
        {
            for (int j=0; j<MAX_CARD_RIGHT_PLAN_NUM; j++)
            {
                csTemp.Format("%d", i+1); 
                m_listCardRightPlan.InsertItem(i*MAX_CARD_RIGHT_PLAN_NUM+j, csTemp);
                csTemp.Format("%d", j+1); 
                m_listCardRightPlan.SetItemText(i*MAX_CARD_RIGHT_PLAN_NUM+j, 1, csTemp); 
                csTemp.Format("%d", lpCardCfg->wCardRightPlan[i][j]);
                m_listCardRightPlan.SetItemText(i*MAX_CARD_RIGHT_PLAN_NUM+j, 2, csTemp); 
            }
        }
//     }
    UpdateData(FALSE);     
}
void CDlgGatewayCardCfg::UpdateList(int iInsertIndex, const char * pCardNo, const NET_DVR_CARD_CFG_V50& struCardInfo)
{ 
    CString strItem = "";
    strItem.Format("%d",iInsertIndex + 1);
//    m_listCard.InsertItem(iInsertIndex,strItem);
    
    m_listCard.SetItemText(iInsertIndex,1, (char *)struCardInfo.byCardNo);
//     if ( struCardInfo.dwModifyParamType &0x1)
//     {
        
        strItem = (struCardInfo.byCardValid )?"Yes":"No";
        m_listCard.SetItemText(iInsertIndex,2, strItem);
//    }
//     if (struCardInfo.dwModifyParamType &0x2)
//     {
        strItem = (struCardInfo.struValid.byEnable )?"Yes":"No";
        m_listCard.SetItemText(iInsertIndex,8, strItem);
        if ( struCardInfo.struValid.byEnable )
        {
            const NET_DVR_TIME_EX *lpTimeEx = & struCardInfo.struValid.struBeginTime; 
            strItem.Format("%d-%d-%d, %2d:%2d:%2d", lpTimeEx->wYear, lpTimeEx->byMonth, lpTimeEx->byDay, lpTimeEx->byHour, lpTimeEx->byMinute, lpTimeEx->bySecond);
            m_listCard.SetItemText(iInsertIndex, 9, strItem); 
            lpTimeEx = & struCardInfo.struValid.struEndTime;
            strItem.Format("%d-%d-%d, %2d:%2d:%2d", lpTimeEx->wYear, lpTimeEx->byMonth, lpTimeEx->byDay, lpTimeEx->byHour, lpTimeEx->byMinute, lpTimeEx->bySecond);
            m_listCard.SetItemText(iInsertIndex, 10, strItem); 
        }
//    }
//     if(struCardInfo.dwModifyParamType &0x4)
//     {
        char *p ; 
        if (struCardInfo.byCardType == 0 || struCardInfo.byCardType > 15)
        {
            p = pCardType[0];
        }
        else 
            p = pCardType[struCardInfo.byCardType];
        m_listCard.SetItemText(iInsertIndex, 4, p); 
//    }
//     if(struCardInfo.dwModifyParamType &0x10)
//     {
        strItem = (struCardInfo.byLeaderCard )?"Yes":"No";
        m_listCard.SetItemText(iInsertIndex, 5, strItem);

//    }
//     if (struCardInfo.dwModifyParamType &0x20)
//     {
        strItem.Format("%d", struCardInfo.dwMaxSwipeTime); 
        m_listCard.SetItemText(iInsertIndex, 6, strItem);
//    }
        strItem.Format("%d", struCardInfo.dwSwipeTime); 
        m_listCard.SetItemText(iInsertIndex, 7, strItem);  
//     if (struCardInfo.dwModifyParamType &0x80)
//     {
        char szCardPassTemp[CARD_PASSWORD_LEN+1] = {0}; 
        memcpy(szCardPassTemp, struCardInfo.byCardPassword, sizeof(struCardInfo.byCardPassword)); 
        strItem.Format("%s", szCardPassTemp); 
        m_listCard.SetItemText(iInsertIndex, 3, strItem);

        strItem.Format("%d", struCardInfo.wRoomNumber); 
        m_listCard.SetItemText(iInsertIndex, 12, strItem);

        strItem.Format("%d", struCardInfo.wFloorNumber); 
        m_listCard.SetItemText(iInsertIndex, 13, strItem);

        strItem.Format("%d", struCardInfo.dwEmployeeNo);
        m_listCard.SetItemText(iInsertIndex, 14, strItem);

        string str_gb2312 = UTF2GB((char*)struCardInfo.byName);
        strItem.Format("%s", (void*)str_gb2312.c_str());
        m_listCard.SetItemText(iInsertIndex, 15, strItem);

        strItem.Format("%d", struCardInfo.wDepartmentNo);
        m_listCard.SetItemText(iInsertIndex, 16, strItem);

        strItem.Format("%d", struCardInfo.wSchedulePlanNo);
        m_listCard.SetItemText(iInsertIndex, 17, strItem);

        /*if (struCardInfo.byRight == 1)
        {
            strItem = "super admin";
        }
        else if (struCardInfo.byRight == 2)
        {
            strItem = "ordinary admin";
        }
        else if (struCardInfo.byRight == 3)
        {
            strItem = "greffier";
        }
        else if (struCardInfo.byRight == 4)
        {
            strItem = "user";
        }*/
        strItem = "no right";
        m_listCard.SetItemText(iInsertIndex, 18, strItem);

        if (struCardInfo.bySchedulePlanType == 1)
        {
            strItem = "personal";
        }
        else if (struCardInfo.bySchedulePlanType == 2)
        {
            strItem = "department";
        }
        else
        {
            strItem = "no type";
        }
        m_listCard.SetItemText(iInsertIndex, 19, strItem);

        if (struCardInfo.byRightType == 0)
        {
            strItem = "card right";
        }
        else if (struCardInfo.byRightType == 1)
        {
            strItem = "QR code right";
        }
        else
        {
            strItem = "bluetooth right";
        }
        m_listCard.SetItemText(iInsertIndex, 20, strItem);
        char szSimNoTemp[NAME_LEN + 1] = { 0 };
        memcpy(szSimNoTemp, struCardInfo.bySIMNum, sizeof(struCardInfo.bySIMNum));
        strItem.Format("%s", szSimNoTemp);
        m_listCard.SetItemText(iInsertIndex, 21, strItem);

        if (struCardInfo.byUserType == 0)
        {
            strItem = "normal";
        }
        else if (struCardInfo.byUserType == 1)
        {
            strItem = "admin";
        }
        else
        {
            strItem = "unknown data";
        }
        m_listCard.SetItemText(iInsertIndex, 22, strItem);
//    }

}



void CALLBACK g_fSetGatewayCardCallback(DWORD dwType, void* lpBuffer, DWORD dwBufLen, void* pUserData)
{
	CDlgGatewayCardCfg* pDlg = (CDlgGatewayCardCfg*)pUserData;
	if (pDlg == NULL)
	{
		return;
	}
	pDlg->ProcessSetCardCfgCallbackData(dwType,lpBuffer,dwBufLen);
}

void CALLBACK g_fGetGatewayCardCallback(DWORD dwType, void* lpBuffer, DWORD dwBufLen, void* pUserData)
{
	CDlgGatewayCardCfg* pDlg = (CDlgGatewayCardCfg*)pUserData;
	if (pDlg == NULL)
	{
		return;
	}
	pDlg->ProcessGetCardCfgCallbackData(dwType,lpBuffer,dwBufLen);
}
void CALLBACK g_fCardParamSyncCallback(DWORD dwType, void* lpBuffer, DWORD dwBufLen, void* pUserData)
{
    CDlgGatewayCardCfg* pDlg = (CDlgGatewayCardCfg*)pUserData;
    if (pDlg == NULL)
    {
        return;
    }
    pDlg->ProcessCardParamSyncCallbackData(dwType, lpBuffer, dwBufLen);
}


UINT __cdecl  g_fSendCardCfgThread(LPVOID pParam)
{
	CDlgGatewayCardCfg* pDlg = (CDlgGatewayCardCfg*)pParam;
	if (pParam != NULL)
	{
		pDlg->SendCardData();
	}
	return 0;
}

UINT __cdecl  g_fShowCardListThread(LPVOID pParam)
{
    CDlgGatewayCardCfg* pDlg = (CDlgGatewayCardCfg*)pParam;
    if (pParam != NULL)
    {
        pDlg->BatchAddCardToList();
    }
	return 0;    	
}

void CDlgGatewayCardCfg::OnClickListCardCfg(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
    LPNET_DVR_CARD_CFG_V50 lpCardCfg  = GetSelItem(); 
    UpdateSubList(lpCardCfg);  
	*pResult = 0;
}

void CDlgGatewayCardCfg::OnButSimulationAdd() 
{
	// TODO: Add your control notification handler code here
	int iAddNum = 10; 
    NET_DVR_CARD_CFG_V50 struCardCfg = { 0 };
    struCardCfg.dwSize = sizeof(struCardCfg); 
    struCardCfg.dwModifyParamType = 1;    //卡权限
    struCardCfg.dwModifyParamType |= 0x8;  //门权限
    struCardCfg.byDoorRight[0] = 1;
    struCardCfg.byCardValid = 1; 
    int i; 
    char szCardNo[128]; 
    for (i=0; i<iAddNum ; i++)
    {
        sprintf(szCardNo, "%d%d%d",i,i+1, i+2); 
        strncpy((char*)struCardCfg.byCardNo, szCardNo, sizeof(struCardCfg.byCardNo));
        AddToCardList(struCardCfg, szCardNo);
    }

    
}

LPNET_DVR_CARD_CFG_V50 CDlgGatewayCardCfg::GetSelItem(char *lpCardNo)
{
    UpdateData(TRUE);
    POSITION  iPos = m_listCard.GetFirstSelectedItemPosition();
    if (iPos == NULL)
    {
        return NULL;
    }
    int iCurSel = m_listCard.GetNextSelectedItem(iPos);
    m_iSelListItem = iCurSel;
    m_listCard.SetItemState( m_iSelListItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED); 
    if ( lpCardNo )
    {
        m_listCard.GetItemText(iCurSel, 1, lpCardNo,  ACS_CARD_NO_LEN);
    }
    LPNET_DVR_CARD_CFG_V50 lpCardCfg = (LPNET_DVR_CARD_CFG_V50)m_listCard.GetItemData(iCurSel);
    return lpCardCfg; 
} 

void CDlgGatewayCardCfg::OnBtnGetSel() 
{
	// TODO: Add your control notification handler code here
    if (m_lGetCardCfgHandle != -1)
    {
        NET_DVR_StopRemoteConfig(m_lGetCardCfgHandle);
	}
    //发送选择项
    NET_DVR_CARD_CFG_SEND_DATA struSendData = {0}; 
    struSendData.dwSize = sizeof(struSendData);
    if ( !GetSelItem((char *)struSendData.byCardNo) )
    {
        MessageBox("Select list item first"); 
        return ; 
    }

    UpdateData(TRUE); 
    NET_DVR_CARD_CFG_COND struCond = {0};
    struCond.dwSize  = sizeof(struCond);
    struCond.dwCardNum = 1;
    struCond.byCheckCardNo = (BYTE)m_BcheckCardNo; 
    struCond.wLocalControllerID = m_iLocalControllerNo;

    m_lGetCardCfgHandle = NET_DVR_StartRemoteConfig(m_lServerID, NET_DVR_GET_CARD_CFG_V50, &struCond, sizeof(struCond), g_fGetGatewayCardCallback, this);
    if (m_lGetCardCfgHandle == -1)
    {
        g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "NET_DVR_GET_CARD_CFG_V50");
      
    }
    else
    {
        g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, "NET_DVR_GET_CARD_CFG_V50");
       
        if (!NET_DVR_SendRemoteConfig(m_lGetCardCfgHandle, ENUM_ACS_SEND_DATA, (char *)(&struSendData), sizeof(struSendData)))
        {
            g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "NET_DVR_SendRemoteConfig ENUM_ACS_SEND_DATA");
        }
        else
        {
            g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, "NET_DVR_SendRemoteConfig ENUM_ACS_SEND_DATA");
        }                
	}
    ShowSelListItem(); 
}

void CDlgGatewayCardCfg::OnBtnSetSel() 
{
	// TODO: Add your control notification handler code here
    if (m_lSetCardCfgHandle != -1)
    {
        NET_DVR_StopRemoteConfig(m_lSetCardCfgHandle);
	}
    m_lpSelSendCardCfg = GetSelItem();
    if ( ! m_lpSelSendCardCfg )
    {
        MessageBox("Select list item first"); 
        return ; 
    }
    UpdateData(TRUE);  
    NET_DVR_CARD_CFG_COND struCond = {0};
    struCond.dwSize  = sizeof(struCond);
    struCond.dwCardNum = 1;
    struCond.byCheckCardNo = (BYTE)m_BcheckCardNo;
    m_BSendSel = TRUE; 
    struCond.wLocalControllerID = m_iLocalControllerNo;
   
    m_lSetCardCfgHandle = NET_DVR_StartRemoteConfig(m_lServerID, NET_DVR_SET_CARD_CFG_V50, &struCond, sizeof(struCond), g_fSetGatewayCardCallback, this);
    if (m_lSetCardCfgHandle == -1)
    {
        g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "NET_DVR_SET_CARD_CFG_V50");
        return;
    }
    else
    {
        g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, "NET_DVR_SET_CARD_CFG_V50");
    }
	if (m_lSetCardCfgHandle == -1)
	{
		return;
	}
    LPNET_DVR_CARD_CFG_V50 lpCardCfg = NULL;
    if ( m_BSendSel )
    {
        m_dwCardNum = 1;
        m_dwSendIndex = 0;
        SendCardData( m_lpSelSendCardCfg ); 
        m_BSendSel = FALSE; 
        m_lpSelSendCardCfg = NULL; 
        return; 
    }
}


void CDlgGatewayCardCfg::ClearList()
{
}

void CDlgGatewayCardCfg::OnButBatchAdd() 
{
	// TODO: Add your control notification handler code here
    CDlgACCardParam  dlg; 
    NET_DVR_CARD_CFG_V50 struCardCfg = { 0 };
    dlg.SetAddMode(BATCH_ADD); 
    dlg.SetCardCfg(struCardCfg);     
    if (dlg.DoModal() == IDOK)
    {
        char szCardNo[ACS_CARD_NO_LEN];
        struCardCfg = dlg.GetCardCfg(szCardNo);
        m_dwCardNum = dlg.GetAddNum(); 
        CreateSameCfgCard(m_dwCardNum, struCardCfg); 
        if ( m_pDisplayListThread )
        {
            //停止之前的线程
        }
        m_pDisplayListThread = AfxBeginThread(g_fShowCardListThread,this,THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED,NULL);
        if (m_pDisplayListThread)
        {
            //m_pDisplayListThread->m_bAutoDelete = FALSE;
            m_pDisplayListThread->ResumeThread();
	    }
	}
}
void CDlgGatewayCardCfg::ShowSelListItem()
{
}

PULONG_PTR CDlgGatewayCardCfg::GetCardCfgPoint()
{
    if ( !m_lpRecordCardCfg)
    {
        const int iMaxCardNum = 100100; 
		m_lpRecordCardCfg = new ULONG_PTR[iMaxCardNum];
    }
    return m_lpRecordCardCfg; 
} 

void CDlgGatewayCardCfg::CreateSameCfgCard(DWORD dwNum, const NET_DVR_CARD_CFG_V50 &struCardCfg)
{
    int i = 0;
	PULONG_PTR lpArr = GetCardCfgPoint();
    LPNET_DVR_CARD_CFG_V50 pCardInfo = NULL;
    for (i=0; i<dwNum; i++)
    {
        pCardInfo = new NET_DVR_CARD_CFG_V50;
        memcpy(pCardInfo, &struCardCfg, sizeof(struCardCfg)); 
        if (dwNum == 1)
        {
            sprintf((char *)pCardInfo->byCardNo, "%s", (char *)struCardCfg.byCardNo);
            sprintf((char *)pCardInfo->byName, "%s", (char *)struCardCfg.byName);
        }
        else
        {
            sprintf((char *)pCardInfo->byCardNo, "%s%d", (char *)struCardCfg.byCardNo, i);
            sprintf((char *)pCardInfo->byName, "%s%d", (char *)struCardCfg.byName, i);
        }
        pCardInfo->dwEmployeeNo += i;
        int dwCardNumNow = m_listCard.GetItemCount();
        lpArr[dwCardNumNow + i] = (ULONG_PTR)pCardInfo;
    }
}

void CDlgGatewayCardCfg::BatchAddCardToList()
{
    LPNET_DVR_CARD_CFG_V50 pCardInfo = NULL;
	PULONG_PTR lpArr = GetCardCfgPoint();
    int i = 0; 
    int nItemCount = m_listCard.GetItemCount();   
    CString strItem ;
    m_csBatchFlag.Format("adding"); 
    PostMessage(WM_MSG_UPDATEDATA_INTERFACE, FALSE,0);
    m_dwCardNum += nItemCount;//保证新添加的卡位于列表底部
    for (i = nItemCount; i < m_dwCardNum; i++)
    {
        pCardInfo = (LPNET_DVR_CARD_CFG_V50)lpArr[i];
        strItem.Format("%d",nItemCount + 1);
        int iItemIndex = m_listCard.InsertItem(nItemCount, strItem);
        PostMessage(WM_MSG_UPDATECARD, iItemIndex, (LPARAM)pCardInfo);
        nItemCount ++; 
    }
    m_csBatchFlag.Format("finish add"); 
    PostMessage(WM_MSG_UPDATEDATA_INTERFACE, FALSE,0);
}

void CDlgGatewayCardCfg::OnBtnStopSend() 
{
	// TODO: Add your control notification handler code here
	StopProcThread(); 
}


BOOL CDlgGatewayCardCfg::SendFirstCard()
{
	if ( m_lSetCardCfgHandle == -1)
	{
		return FALSE; 
	}
	if ( m_BSendSel )
    {
		//         lpCardCfg = GetSelItem();
        SendCardData( m_lpSelSendCardCfg ); 
        m_BSendSel = FALSE; 
        m_lpSelSendCardCfg = NULL; 
        return TRUE; 
    }
	m_dwSendIndex = 0; 
	//一张卡没有，返回FALSE
	if( m_dwCardNum < 1)
	{
		return FALSE; 
	}
	PULONG_PTR lpArr = GetCardCfgPoint();
    m_lpNowSendCard = (LPNET_DVR_CARD_CFG_V50)lpArr[m_dwSendIndex];
	if (!NET_DVR_SendRemoteConfig(m_lSetCardCfgHandle,3/*ENUM_ACS_SEND_DATA*/, (char *)m_lpNowSendCard ,sizeof(*m_lpNowSendCard)))
    {
        g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "Send Fail,CardNO: %s", m_lpNowSendCard->byCardNo);        
		return FALSE; 
    }
	return TRUE; 
} 

//重发卡
BOOL CDlgGatewayCardCfg::ReSendLastCard()
{
	if ( m_lSetCardCfgHandle == -1)
	{
		return FALSE; 
	}
	m_byLastCardSendTime ++; 
	if ( m_byLastCardSendTime >= MAX_RESEND_CARD_TIME)
	{
		return FALSE; 
	}
	if (!NET_DVR_SendRemoteConfig(m_lSetCardCfgHandle,3/*ENUM_ACS_SEND_DATA*/, (char *)m_lpNowSendCard ,sizeof(*m_lpNowSendCard)))
    {
        g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "Send Fail,CardNO: %s", m_lpNowSendCard->byCardNo);        
		return FALSE; 
    }
	return TRUE; 
}     

//发送下一张卡
BOOL CDlgGatewayCardCfg::SendNextCard()
{
	if ( m_lSetCardCfgHandle == -1)
	{
		return FALSE;
	}
	m_dwSendIndex++;
	if ( m_dwSendIndex >= m_dwCardNum)
	{
		//PostMessage(WM_MSG_SETCARD_FINISH,0,0);
		return TRUE; 
	}
	PULONG_PTR lpArr = GetCardCfgPoint();
    m_lpNowSendCard = (LPNET_DVR_CARD_CFG_V50)lpArr[m_dwSendIndex];
    if (!NET_DVR_SendRemoteConfig(m_lSetCardCfgHandle,3/*ENUM_ACS_SEND_DATA*/, (char *)m_lpNowSendCard ,sizeof(*m_lpNowSendCard)))
    {
        g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "Send Fail,CardNO: %s", m_lpNowSendCard->byCardNo);
        //发送失败  关闭连接
        PostMessage(WM_MSG_SETCARD_FINISH,0,0);
        return FALSE; 
    }
	
	return TRUE; 
}      

void CDlgGatewayCardCfg::OnBnClickedBtnCardParamSync()
{
    // TODO:  同步卡参数到智能锁
    if (-1 != m_lCardParamSyncHandle)
    {
        if (NET_DVR_StopRemoteConfig(m_lCardParamSyncHandle))
        {
            m_lCardParamSyncHandle = -1;
        }
    }

    m_lCardParamSyncHandle = NET_DVR_StartRemoteConfig(m_lServerID, NET_DVR_JSON_CONFIG, "POST /ISAPI/SmartLock/CardParam/Sync?format=json", strlen("POST /ISAPI/SmartLock/CardParam/Sync?format=json"), g_fCardParamSyncCallback, this);

    if (m_lCardParamSyncHandle == -1)
    {
        g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "Start Card Param Sync Failed");
        return;
    }
    else
    {
        g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, "Start Card Param Sync Success");
    }

    cJSON* pRoot = cJSON_CreateObject();
    cJSON* pItem = cJSON_CreateObject();
    cJSON_AddItemToObject(pRoot, "CardParamSyncCond", pItem);
    cJSON_AddStringToObject(pItem, "command", "startSync");
    cJSON_AddNumberToObject(pItem, "maxResults", 30);

    char *out = cJSON_Print(pRoot);


    if (!NET_DVR_SendRemoteConfig(m_lCardParamSyncHandle, ENUM_SEND_JSON_DATA, out, strlen(out)))
    {
        g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "Get Card Param Sync Failed");
        PostMessage(WM_MSG_CARD_PARAM_SYNC_FINISH, 0, 0);
    }

    cJSON_Delete(pRoot);
    free(out);

    return;
}


void CDlgGatewayCardCfg::OnBnClickedBtnCardsearch()
{
    UpdateData(TRUE);

    for (int i = 0; i < m_listCard.GetItemCount(); i++)
    {
        m_listCard.SetItemState(i, 0, -1);
    }

    bool bSearchRst = false;

    for (int i = 0; i < m_listCard.GetItemCount(); i++)
    {
        CString szCardNum = m_listCard.GetItemText(i, 1);

        if (szCardNum == m_szCardNum.Trim())
        {
            m_listCard.SetFocus();
            m_listCard.SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
            m_listCard.EnsureVisible(i, TRUE);
            bSearchRst = true;
            break;
        }
    }

    if (!bSearchRst)
    {
        MessageBox("not find card!!!!");
    }


    UpdateData(FALSE);
}


afx_msg LRESULT CDlgGatewayCardCfg::OnMsgUpdatecard(WPARAM wParam, LPARAM lParam)
{
    LPNET_DVR_CARD_CFG_V50 pCardInfo = (LPNET_DVR_CARD_CFG_V50)lParam;
    int iItemIndex = (int)wParam;
    m_listCard.SetItemData(iItemIndex, (DWORD_PTR)pCardInfo);
    UpdateList(iItemIndex, (char *)pCardInfo->byCardNo, *pCardInfo);

    return 0;
}
