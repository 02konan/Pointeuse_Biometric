/**********************************************************
FileName:    DlgPlayRemoteFile.cpp
Description: play remote file    
Date:        2008/05/17
Note: 		 <global>struct, refer to GeneralDef.h, global variants and functions refer to ClientDemo.cpp   
Modification History:      
    <version> <time>         <desc>
    <1.0    > <2008/05/17>       <created>
***********************************************************/


#include "stdafx.h"
#include "ClientDemo.h"
#include "DlgPlayRemoteFile.h"
#include ".\dlgplayremotefile.h"


// CDlgPlayRemoteFile dialog
//BOOL g_bRmtCapPic = FALSE;
CDlgPlayRemoteFile* g_pDlgRemoteFile;
HANDLE g_hESFileHandle = NULL;
//#define CALLBACK_PLAY
void CALLBACK PlayESCallBack_V40(LONG lRealHandle, NET_DVR_PACKET_INFO_EX *struPackInfo, void *pUser)
{
    BOOL bRet = FALSE;
    int i = 0;
    LONG lIndex = g_pDlgRemoteFile->m_lPort;
    
    char szLan[64] = {0};

    sprintf(szLan, "lRealHandle--dwDataType--dwBufSize:[%d][%d][%d]", lRealHandle, struPackInfo->dwPacketType, struPackInfo->dwPacketSize);
    OutputDebugString(szLan);
    
    //memcpy(g_pDlgRemoteFile->m_pBufferTemp, struPackInfo->pPacketBuffer, struPackInfo->dwPacketSize);
    
    DWORD dwReturn = 0;	  
    
    switch (struPackInfo->dwPacketType)
    {
    case FILE_HEAD:
        OutputDebugString("FILE_HEAD");
        //写入4字节长度
        //WriteFile(g_hESFileHandle, &struPackInfo->dwPacketSize, 4, &dwReturn, NULL);
        //写入文件头
        WriteFile(g_hESFileHandle, struPackInfo->pPacketBuffer, struPackInfo->dwPacketSize, &dwReturn, NULL);
        //OutputDebugString(g_pDlgRemoteFile->m_pBufferTemp);
        //WriteFile(g_pDlgRemoteFile->lWriteHandle, struPackInfo->pPacketBuffer, struPackInfo->dwPacketSize, &dwReturn, NULL);
        break;
    case VIDEO_I_FRAME:
    case VIDEO_B_FRAME:
    case VIDEO_P_FRAME:
    //case AUDIO_PACKET:
    //case PRIVT_PACKET:
        //case HIK_H264_E_FRAME:
        OutputDebugString("stream");
        //写入4字节长度
        //WriteFile(g_hESFileHandle, &struPackInfo->dwPacketSize, 4, &dwReturn, NULL);
        //写入文件内容
        WriteFile(g_hESFileHandle, struPackInfo->pPacketBuffer, struPackInfo->dwPacketSize, &dwReturn, NULL);
        //OutputDebugString(g_pDlgRemoteFile->m_pBufferTemp);
        //WriteFile(g_pDlgRemoteFile->lWriteHandle, struPackInfo->pPacketBuffer, struPackInfo->dwPacketSize, &dwReturn, NULL);
        break;
    default:
        OutputDebugString("error frame packet");
        //OutputDebugString(g_pDlgRemoteFile->m_pBufferTemp);
        break;
    }
    
    return;
}
/*********************************************************
Function:	PlayDataCallBack
Desc:		callback play data
Input:		lPlayHandle,play handle;dwDataType,data type or file stream;pBuffer,data;dwBufSize,data length;dwUser,user address;
Output:	
Return:	
**********************************************************/
void CALLBACK PlayDataCallBack_V40(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer,DWORD dwBufSize,void *pUser)
{
#ifndef CALLBACK_PLAY
		return;
#endif

	BOOL bRet = FALSE;
	int i = 0;
	LONG lIndex = g_pDlgRemoteFile->m_lPort;
	switch (dwDataType)
	{
	case NET_DVR_SYSHEAD:
		if (!PlayM4_GetPort(&lIndex))
		{
			g_pDlgRemoteFile->m_lPort = -1;
			break;
		}
		g_pDlgRemoteFile->m_lPort = lIndex;
		//set stream secret key
		if(strcmp(g_sSecretKey, "StreamNotEncrypt") != 0)
		{
			if(!PlayM4_SetSecretKey(lIndex, 1, g_sSecretKey, 128))
			{
				g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), PLAY_SUCC_T, "PlayM4_SetSecretKey err [%d]!", PlayM4_GetLastError(lIndex));
			}
		}

		bRet = PlayM4_SetOverlayMode(lIndex, FALSE, COLORREF(0));//not play on overplay
		bRet = PlayM4_SetStreamOpenMode(lIndex, STREAME_FILE);
		
		if (dwBufSize > 0)
		{
			bRet = PlayM4_OpenStream(lIndex, pBuffer, dwBufSize, 600*1000);//SOURCE_BUF_MIN50*1000
			if (bRet)
			{
				bRet = PlayM4_Play(lIndex, g_pDlgRemoteFile->m_hPlayWnd);
				if (!bRet)
				{
					g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), PLAY_FAIL_T, "PlayM4_Play err [%d]!", PlayM4_GetLastError(lIndex));
				}
				else
				{
					g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), PLAY_SUCC_T, "PlayM4_Play err [%d]!", PlayM4_GetLastError(lIndex));
				}
			}
			else
			{
				g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), PLAY_FAIL_T, "PlayM4_OpenStream err[%d]!", PlayM4_GetLastError(lIndex));
			}
		}
		break;
	case NET_DVR_AUDIOSTREAMDATA:
		if (dwBufSize > 0 && lIndex >=0)
		{
			bRet = PlayM4_InputVideoData(lIndex, pBuffer, dwBufSize);
			if (!bRet)
			{
				g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), PLAY_FAIL_T, "PlayM4_InputVideoData err[%d]!", PlayM4_GetLastError(lIndex));
			}
		}
		break;
    case NET_DVR_VOD_DRAW_DATA:
	case NET_DVR_STREAMDATA:
		if (dwBufSize > 0 && lIndex >=0)
		{
			for (i=0; i<1000; i++)
			{
				bRet = PlayM4_InputData(lIndex, pBuffer, dwBufSize);
				if (!bRet)
				{
					if ( i >=999)
					{
						g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), PLAY_FAIL_T, "PlayM4_InputData err[%d]!", PlayM4_GetLastError(lIndex));
					}
					
					Sleep(1);
				}
				else
				{
					break;
				}
			}
		}
		break;
	case NET_DVR_PLAYBACK_ALLFILEEND:
		{
			g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), PLAY_SUCC_T, "PlayBack All File End!");
		}
		break;
	}
	return;
}

/*********************************************************
Function:	GetFileThread
Desc:		get recorded file list thread
Input:		pParam,pointer to parameters
Output:	
Return:	
**********************************************************/
UINT GetFileThread(LPVOID pParam)
{
	LONG lRet = -1;
    NET_DVR_FINDDATA_V50 struFileInfo = { 0 };
    //memset(&struFileInfo, 0, sizeof(struFileInfo));
    //NET_DVR_FINDDATA_V40 struFileInfo;
    //memset(&struFileInfo, 0, sizeof(struFileInfo));
	CString csTmp;
	char m_szFileName[100];
	char szLan[128] = {0};

	while (g_pDlgRemoteFile->m_bQuit)
	{
        //lRet = NET_DVR_FindNextFile_V40(g_pDlgRemoteFile->m_lFileHandle, &struFileInfo);
        lRet = NET_DVR_FindNextFile_V50(g_pDlgRemoteFile->m_lFileHandle, &struFileInfo);
		if (lRet == NET_DVR_FILE_SUCCESS)
		{			
            memcpy(&g_pDlgRemoteFile->m_struFindData, &struFileInfo, sizeof(struFileInfo)); 

			strcpy(m_szFileName, struFileInfo.sFileName);
			g_pDlgRemoteFile->m_listRemoteFile.InsertItem(g_pDlgRemoteFile->m_iFileNum,m_szFileName,0);			
			if (struFileInfo.dwFileSize / 1024 == 0)
			{
				csTmp.Format("%d",struFileInfo.dwFileSize);
			}
			else if (struFileInfo.dwFileSize / 1024 > 0 && struFileInfo.dwFileSize / (1024*1024) == 0)
			{
				csTmp.Format("%dK",struFileInfo.dwFileSize / 1024);
			}
			else// if ()
			{
				csTmp.Format("%dM",struFileInfo.dwFileSize / 1024/1024);//different from hard disk capacity, files need tranformation
			}
			//csTmp.Format("%d",struFileInfo.dwFileSize);
			g_pDlgRemoteFile->m_listRemoteFile.SetItemText(g_pDlgRemoteFile->m_iFileNum, 1, csTmp);
            if (struFileInfo.struStartTime.byLocalOrUTC)
            {
                csTmp.Format("%04d%02d%02d%02d%02d%02dT%02d:%02d", struFileInfo.struStartTime.wYear, \
                    struFileInfo.struStartTime.byMonth, struFileInfo.struStartTime.byDay, \
                    struFileInfo.struStartTime.byHour, struFileInfo.struStartTime.byMinute, \
                    struFileInfo.struStartTime.bySecond, struFileInfo.struStartTime.cTimeDifferenceH, struFileInfo.struStartTime.cTimeDifferenceM);
            }
            else
            {
                csTmp.Format("%04d%02d%02d%02d%02d%02d", struFileInfo.struStartTime.wYear, \
                    struFileInfo.struStartTime.byMonth, struFileInfo.struStartTime.byDay, \
                    struFileInfo.struStartTime.byHour, struFileInfo.struStartTime.byMinute, \
                    struFileInfo.struStartTime.bySecond);
            }
            g_pDlgRemoteFile->m_listRemoteFile.SetItemText(g_pDlgRemoteFile->m_iFileNum, 2, csTmp);
            if (struFileInfo.struStartTime.byLocalOrUTC)
            {
                csTmp.Format("%04d%02d%02d%02d%02d%02dT%02d:%02d", struFileInfo.struStopTime.wYear, struFileInfo.struStopTime.byMonth, \
                    struFileInfo.struStopTime.byDay, struFileInfo.struStopTime.byHour, \
                    struFileInfo.struStopTime.byMinute, struFileInfo.struStopTime.bySecond, struFileInfo.struStartTime.cTimeDifferenceH, struFileInfo.struStartTime.cTimeDifferenceM);
            }
            else
            {
                csTmp.Format("%04d%02d%02d%02d%02d%02d", struFileInfo.struStopTime.wYear, struFileInfo.struStopTime.byMonth, \
                    struFileInfo.struStopTime.byDay, struFileInfo.struStopTime.byHour, \
                    struFileInfo.struStopTime.byMinute, struFileInfo.struStopTime.bySecond);
            }
			g_pDlgRemoteFile->m_listRemoteFile.SetItemText(g_pDlgRemoteFile->m_iFileNum, 3, csTmp);

            //csTmp.Format("%04d%02d%02d%02d%02d%02d", struFileInfo.struStartTime.dwYear, \
            //    struFileInfo.struStartTime.dwMonth, struFileInfo.struStartTime.dwDay, \
            //    struFileInfo.struStartTime.dwHour, struFileInfo.struStartTime.dwMinute, \
            //    struFileInfo.struStartTime.dwSecond);
            //g_pDlgRemoteFile->m_listRemoteFile.SetItemText(g_pDlgRemoteFile->m_iFileNum, 2, csTmp);
            //csTmp.Format("%04d%02d%02d%02d%02d%02d", struFileInfo.struStopTime.dwYear, struFileInfo.struStopTime.dwMonth, \
            //    struFileInfo.struStopTime.dwDay, struFileInfo.struStopTime.dwHour, \
            //    struFileInfo.struStopTime.dwMinute, struFileInfo.struStopTime.dwSecond);
            //g_pDlgRemoteFile->m_listRemoteFile.SetItemText(g_pDlgRemoteFile->m_iFileNum, 3, csTmp);

			if (struFileInfo.byLocked == 1)
			{
				g_StringLanType(szLan, "锁定", "Lock");
				csTmp.Format(szLan);
			}
			else if(struFileInfo.byLocked == 0)
			{
				g_StringLanType(szLan, "未锁", "Unlock");
				csTmp.Format(szLan);
			}
			g_pDlgRemoteFile->m_listRemoteFile.SetItemText(g_pDlgRemoteFile->m_iFileNum, 4, csTmp);
			g_pDlgRemoteFile->m_listRemoteFile.SetItemData(g_pDlgRemoteFile->m_iFileNum, struFileInfo.byLocked);

            csTmp.Format("%d",struFileInfo.dwFileIndex);
			g_pDlgRemoteFile->m_listRemoteFile.SetItemText(g_pDlgRemoteFile->m_iFileNum, 5, csTmp);

			csTmp.Format("%d",struFileInfo.byFileType);
			g_pDlgRemoteFile->m_listRemoteFile.SetItemText(g_pDlgRemoteFile->m_iFileNum, 6, csTmp);
			
			if(struFileInfo.byStreamType == 0)
			{
				g_StringLanType(szLan, "主码流", "Main Stream");
				csTmp.Format(szLan);
			}
			else if (struFileInfo.byStreamType == 1)
			{
				g_StringLanType(szLan, "子码流", "Sub Stream");
				csTmp.Format(szLan);
			}
			else if (struFileInfo.byStreamType == 2)
			{
				g_StringLanType(szLan, "三码流", "Three Stream");
				csTmp.Format(szLan);
			}
			g_pDlgRemoteFile->m_listRemoteFile.SetItemText(g_pDlgRemoteFile->m_iFileNum, 7, csTmp);
            if (strcmp(struFileInfo.struAddr.struIP.sIpV4, "") && struFileInfo.struAddr.struIP.sIpV4 != NULL)
            {
                csTmp.Format("%s", struFileInfo.struAddr.struIP.sIpV4);
            }
            else
            {
                csTmp.Format("%s", (char *)struFileInfo.struAddr.struIP.byIPv6);
            }
            g_pDlgRemoteFile->m_listRemoteFile.SetItemText(g_pDlgRemoteFile->m_iFileNum, 8, csTmp);
            csTmp.Format("%d", struFileInfo.struAddr.wPort);
            g_pDlgRemoteFile->m_listRemoteFile.SetItemText(g_pDlgRemoteFile->m_iFileNum, 9, csTmp);
			g_pDlgRemoteFile->m_iFileNum++;
		}
		else
		{
			if (lRet == NET_DVR_ISFINDING)
			{
                g_pMainDlg->AddLog(g_pDlgRemoteFile->m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_FindNextFile_V50, NET_DVR_ISFINDING, Sleep 1000");
                Sleep(2000);
				continue;
			}
			if ((lRet == NET_DVR_NOMOREFILE) || (lRet == NET_DVR_FILE_NOFIND))
			{
				g_StringLanType(szLan, "查找", "Search");
				g_pDlgRemoteFile->GetDlgItem(IDC_BTN_REMOTE_SEARCH_LIST)->SetWindowText(szLan);
				g_pDlgRemoteFile->m_bSearching = FALSE;
				(g_pDlgRemoteFile->GetDlgItem(IDC_STATIC_REMOTE_SEARCH_STATE))->ShowWindow(SW_HIDE);
				g_pMainDlg->AddLog(g_pDlgRemoteFile->m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_FindNextFile_V50 file num[%d]", g_pDlgRemoteFile->m_iFileNum);
// 				if (g_pDlgRemoteFile->m_iFileNum > 0)
// 				{
// 					g_pMainDlg->AddLog(g_pDlgRemoteFile->m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_FindNextFile_V30 file num[%d]", g_pDlgRemoteFile->m_iFileNum);
// 					g_StringLanType(szLan, "获取文件列表结束，文件已经全部列出", "Finish to get file list, and documents have all been listed");
// 					AfxMessageBox(szLan);
// 				}
// 				else
// 				{
// 					g_StringLanType(szLan, "获取文件列表结束，没有录像文件", "Finish to get file list, and There is no record file");
// 					AfxMessageBox(szLan);
// 				}
				
				g_pDlgRemoteFile->m_iFileNum = 0;
				if (g_pDlgRemoteFile->m_nPlayHandle == -1)
				{
					g_pDlgRemoteFile->SetStopState();
					g_pDlgRemoteFile->GetDlgItem(IDC_BTN_REMOTE_FILE_PLAY)->EnableWindow(FALSE);
				}
				break;
			}
			else
			{
                if (lRet == NET_DVR_FIND_TIMEOUT)
                {
                    g_pMainDlg->AddLog(g_pDlgRemoteFile->m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_FindNextFile_V50, NET_DVR_FIND_TIMEOUT");
                }

                g_pMainDlg->AddLog(g_pDlgRemoteFile->m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_FindNextFile_V50, lRet[%d]", lRet);

				g_pDlgRemoteFile->GetDlgItem(IDC_BTN_REMOTE_SEARCH_LIST)->SetWindowText("查找");
				g_pDlgRemoteFile->m_bSearching = FALSE;
				(g_pDlgRemoteFile->GetDlgItem(IDC_STATIC_REMOTE_SEARCH_STATE))->ShowWindow(SW_HIDE);
                g_pMainDlg->AddLog(g_pDlgRemoteFile->m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_FindNextFile_V50");
				g_StringLanType(szLan, "由于服务器忙,或网络故障,获取文件列表异常终止", "Since the server is busy, or network failure, abnormal termination of access to the file list");
				AfxMessageBox(szLan);
				g_pDlgRemoteFile->m_iFileNum = 0;
				if (g_pDlgRemoteFile->m_nPlayHandle == -1)
				{
					g_pDlgRemoteFile->SetStopState();
					g_pDlgRemoteFile->GetDlgItem(IDC_BTN_REMOTE_FILE_PLAY)->EnableWindow(FALSE);
				}
				break;
			}
		}
	}
	CloseHandle(g_pDlgRemoteFile->m_hFileThread);
	g_pDlgRemoteFile->m_hFileThread = NULL;
	NET_DVR_FindClose_V30(g_pDlgRemoteFile->m_lFileHandle);
	g_pDlgRemoteFile->m_lFileHandle = -1;
	g_pDlgRemoteFile->m_bSearching = FALSE;
	return 0;
}

/*********************************************************
  Function:	CDlgPlayRemoteFile
  Desc:		Constructor
  Input:	
  Output:	
  Return:	
**********************************************************/
IMPLEMENT_DYNAMIC(CDlgPlayRemoteFile, CDialog)
CDlgPlayRemoteFile::CDlgPlayRemoteFile(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgPlayRemoteFile::IDD, pParent)
	, m_csCardNum(_T(""))
	, m_bChkCardNum(FALSE)
	, m_ctDateStart(0)
	, m_ctTimeStart(0)
	, m_ctDateStop(0)
	, m_ctTimeStop(0)
	, m_csDownloadStat(_T(""))
	, m_iDeviceIndex(-1)
	, m_lLoginID(-1)
	, m_iChanIndex(-1)
	, m_nPlayHandle(-1)
	, m_bDownloading(FALSE)
	, m_bSearching(FALSE)
	, m_bFullScreen(FALSE)
	, m_hFileThread(NULL)
	, m_lDeviceType(0)
	, m_bGetMaxTime(FALSE)
	, m_hPareTabWnd(NULL)
	, m_hPareDlgWnd(NULL)
	, m_lDownloadHandle(-1)
	, m_bPause(FALSE)
	, m_bSound(FALSE)
	, m_hPlayWnd(NULL)
	, m_iFileNum(0)
	, m_iChanShowNum(0)
	, m_iFileType(0)
	, m_lFileHandle(-1)
	, m_nFileTime(0)
	, m_nTotalFrames(0)
	, m_nTotalSecond(0)
	, m_nTotalMinute(0)
	, m_nTotalHour(0)
	, m_lStartChan(0)
	, m_bSaveFile(FALSE)
	, m_bSetFrame(FALSE)
	, m_iChanNum(0)
	, m_iItemSel(-1)
	, m_iOffset(0)
	, m_lPort(-1)
	, m_bChangeSpeed(FALSE)
    , m_hBackupThread(NULL)
    , m_lBackupHandle(-1)
    , m_ctTimeSet(0)
    , m_ctDateSet(0)
    , m_bBackuping(FALSE)
	,m_csAddLabelName(_T(""))
	, m_bDrawFrame(FALSE)
	, m_bQuickSearch(FALSE)
	,m_dwVolumeNum(0)
	, m_bQuit(TRUE)
	,m_byGuid1(0)
	,m_byGuid2(0)
	,m_byGuid3(0)
	,m_byGuid4(0)
	,m_byGuid5(0)
	,m_byGuid6(0)
	,m_byGuid7(0)
	,m_byGuid8(0)
	,m_byGuid9(0)
	,m_byGuid10(0)
	,m_byGuid11(0)
	,m_byGuid12(0)
	,m_byGuid13(0)
	,m_byGuid14(0)
	,m_byGuid15(0)
	,m_byGuid16(0)
    ,m_byCountMutilDownload(0)
    ,m_iMoney(0)
    , m_drawdate(COleDateTime::GetCurrentTime())
    , m_drawtime(COleDateTime::GetCurrentTime())
    , m_bEnableUTC(FALSE)
    , m_bDownload(FALSE)
    , m_dwFindTimeout(0)
{
	memset(m_szFileName, 0, 200);
	memset(m_szCurFileName, 0, 200);
	memset(m_szDownLoadFileName, 0, 256);
    memset(&m_struBackupParam, 0, sizeof(m_struBackupParam));
    memset(&m_struDiskList, 0, sizeof(m_struDiskList));
	m_hPlayEnableIcon =			AfxGetApp()->LoadIcon(IDI_PLAY_ENABLE);
	m_hPlayDisableIcon =		AfxGetApp()->LoadIcon(IDI_PLAY_DISABLE);
	m_hPauseEnableIcon =		AfxGetApp()->LoadIcon(IDI_PAUSE_ENABLE);
	m_hPauseDisableIcon =		AfxGetApp()->LoadIcon(IDI_PAUSE_DISABLE);
	m_hStopEnableIcon =			AfxGetApp()->LoadIcon(IDI_STOP_ENABLE);
	m_hStopDisableIcon =		AfxGetApp()->LoadIcon(IDI_STOP_DISABLE);
	m_hSoundStartIcon=			AfxGetApp()->LoadIcon(IDI_SOUND_ENABLE);
	m_hSoundStopIcon=			AfxGetApp()->LoadIcon(IDI_SOUND_DISABLE);
	m_hGotoHeadDisableIcon =	AfxGetApp()->LoadIcon(IDI_GOTOSTART_DISABLE);
	m_hGotoTailDisableIcon =	AfxGetApp()->LoadIcon(IDI_GOTOEND_DISABLE);
	m_hFastDisableIcon =		AfxGetApp()->LoadIcon(IDI_FASTFORWARD_DISABLE);
	m_hSlowDisableIcon=			AfxGetApp()->LoadIcon(IDI_FASTBACKWARD_DISABLE);
	m_hGotoHeadEnableIcon =		AfxGetApp()->LoadIcon(IDI_GOTOSTART_ENABLE);
	m_hGotoTailEnableIcon =		AfxGetApp()->LoadIcon(IDI_GOTOEND_ENABLE);
	m_hFastEnableIcon =			AfxGetApp()->LoadIcon(IDI_FASTFORWARD_ENABLE);
	m_hSlowEnableIcon =			AfxGetApp()->LoadIcon(IDI_FASTBACKWARD_ENABLE);
	m_hStepEnableIcon=			AfxGetApp()->LoadIcon(IDI_STEP_ENABLE);
	m_hStepDisableIcon =		AfxGetApp()->LoadIcon(IDI_STEP_DISABLE);
	m_hStepBackEnableIcon =		AfxGetApp()->LoadIcon(IDI_STEPBACK_ENABLE);
	m_hStepBackDisableIcon =	AfxGetApp()->LoadIcon(IDI_STEPBACK_DISABLE);
	m_hCaptureIcon	=			AfxGetApp()->LoadIcon(IDI_PIC);	
    m_hWriteHandle = NULL;
    //m_pBufferTemp = new char[COMMANDLEN * COMMANDLEN * 2];
    //memset(m_pBufferTemp, 0, sizeof(m_pBufferTemp));
}

/*********************************************************
  Function:	~CDlgPlayRemoteFile
  Desc:		destructor
  Input:	
  Output:	
  Return:	
**********************************************************/
CDlgPlayRemoteFile::~CDlgPlayRemoteFile()
{
}

/*********************************************************
Function:	DoDataExchange
Desc:		the map between control and variable
Input:	
Output:	
Return:	
**********************************************************/
void CDlgPlayRemoteFile::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDlgPlayRemoteFile)
    DDX_Control(pDX, IDC_CMB_USECARD_TYPE, m_comboUseCardType);
    DDX_Control(pDX, IDC_CMB_ATM_TRADETYPE, m_comboATMTradeType);
    DDX_Control(pDX, IDC_COMBO_FIND_TYPE, m_cmbFindType);
    DDX_Control(pDX, IDC_COM_STREAM_TYPE, m_comboStreamtype);
    DDX_Control(pDX, IDC_COMBO_DISK_LIST, m_comboDiskList);
    DDX_Control(pDX, IDC_COMBO_FILE_ATTR, m_comboAttr);
    DDX_Control(pDX, IDC_COMBO_TRANS_TYPE, m_comboTransType);
    DDX_Text(pDX, IDC_EDIT_CARD_NUM, m_csCardNum);
    DDX_Check(pDX, IDC_CHK_CARD_NUM, m_bChkCardNum);
    DDX_Control(pDX, IDC_COMBO_REMOTE_FILE_TYPE, m_comboFileType);
    DDX_DateTimeCtrl(pDX, IDC_COMBO_REMOTE_DATE_START, m_ctDateStart);
    DDX_DateTimeCtrl(pDX, IDC_COMBO_REMOTE_TIME_START, m_ctTimeStart);
    DDX_DateTimeCtrl(pDX, IDC_COMBO_REMOTE_DATE_STOP, m_ctDateStop);
    DDX_DateTimeCtrl(pDX, IDC_COMBO_REMOTE_TIME_STOP, m_ctTimeStop);
    DDX_Control(pDX, IDC_LIST_REMOTE_FILE, m_listRemoteFile);
    DDX_Control(pDX, IDC_STATIC_REMOTE_FILE_WND, m_staticPlayWnd);
    DDX_Control(pDX, IDC_SLIDER_REMOTE_FILE_PROGRESS, m_sliderPlayProgress);
    DDX_Control(pDX, IDC_SLIDER_REMOTE_FILE_VOLUME, m_sliderVolume);
    DDX_Control(pDX, IDC_PROGRESS_REMOTE_FILE_DOWNLOAD, m_progressDownload);
    DDX_Text(pDX, IDC_STATIC_REMOTE_DOWNLOAD_STATE, m_csDownloadStat);
    DDX_Text(pDX, IDC_EDIT_OFFSET, m_iOffset);
    DDX_Text(pDX, IDC_EDIT_SPEED, m_iSpeed);
    DDX_Text(pDX, IDC_EDIT_TOTAL_LEN, m_csFileLen);
    DDX_Text(pDX, IDC_EDIT_NAME, m_csName);
    DDX_DateTimeCtrl(pDX, IDC_COMBO_REMOTE_DATE_SET, m_ctDateSet);
    DDX_DateTimeCtrl(pDX, IDC_COMBO_REMOTE_TIME_SET, m_ctTimeSet);
    DDX_Check(pDX, IDC_CHK_BACKUP_PLAYER, m_bBackupPlayer);
    DDX_Check(pDX, IDC_CHK_SUCCEED, m_bContinue);
    DDX_Text(pDX, IDC_EDIT_ADD_LABEL_NAME, m_csAddLabelName);
    DDX_Check(pDX, IDC_CHK_DRAW_FRAME, m_bDrawFrame);
    DDX_Text(pDX, IDC_EDIT_VOLUME_NUM, m_dwVolumeNum);
    DDX_Check(pDX, IDC_CHECK_QUICK_SEARCH, m_bQuickSearch);
    DDX_Text(pDX, IDC_EDIT_WD_GUID1, m_byGuid1);
    DDX_Text(pDX, IDC_EDIT_WD_GUID2, m_byGuid2);
    DDX_Text(pDX, IDC_EDIT_WD_GUID3, m_byGuid3);
    DDX_Text(pDX, IDC_EDIT_WD_GUID4, m_byGuid4);
    DDX_Text(pDX, IDC_EDIT_WD_GUID5, m_byGuid5);
    DDX_Text(pDX, IDC_EDIT_WD_GUID6, m_byGuid6);
    DDX_Text(pDX, IDC_EDIT_WD_GUID7, m_byGuid7);
    DDX_Text(pDX, IDC_EDIT_WD_GUID8, m_byGuid8);
    DDX_Text(pDX, IDC_EDIT_WD_GUID9, m_byGuid9);
    DDX_Text(pDX, IDC_EDIT_WD_GUID10, m_byGuid10);
    DDX_Text(pDX, IDC_EDIT_WD_GUID11, m_byGuid11);
    DDX_Text(pDX, IDC_EDIT_WD_GUID12, m_byGuid12);
    DDX_Text(pDX, IDC_EDIT_WD_GUID13, m_byGuid13);
    DDX_Text(pDX, IDC_EDIT_WD_GUID14, m_byGuid14);
    DDX_Text(pDX, IDC_EDIT_WD_GUID15, m_byGuid15);
    DDX_Text(pDX, IDC_EDIT_WD_GUID16, m_byGuid16);
    DDX_Text(pDX, IDC_EDIT_WD_GUID, m_csGuid);
    DDX_Text(pDX, IDC_EDT_FIND_TIMEOUT, m_dwFindTimeout);
    DDV_MaxChars(pDX, m_csGuid, 32);
    DDX_Text(pDX, IDC_EDIT_MONEY, m_iMoney);
    DDX_Check(pDX, IDC_CHK_AUDIO_FILE, m_bAudioFile);
    DDX_Check(pDX, IDC_CHK_ESCALLBACK, m_bEsCallBack);
    //}}AFX_DATA_MAP

    DDX_Control(pDX, IDC_COMBO_DRAWTYPE, m_drawtype);
    DDX_DateTimeCtrl(pDX, IDC_COMBO_DRAWDATE, m_drawdate);
    DDX_DateTimeCtrl(pDX, IDC_COMBO_DRAWTIME, m_drawtime);
    DDX_Check(pDX, IDC_CHECK_UTC, m_bEnableUTC);
    DDX_Check(pDX, IDC_CHECK_DOWNLOAD_FILE, m_bDownload);
}


/*********************************************************
Function:	BEGIN_MESSAGE_MAP
Desc:		the map between control and function
Input:	
Output:	
Return:	
**********************************************************/
BEGIN_MESSAGE_MAP(CDlgPlayRemoteFile, CDialog)
	//{{AFX_MSG_MAP(CDlgPlayRemoteFile)	
	ON_BN_CLICKED(IDC_BTN_REMOTE_SEARCH_LIST, OnBnClickedBtnRemoteSearchList)
	ON_BN_CLICKED(IDC_BTN_REMOTE_FILE_DOWNLOAD, OnBnClickedBtnFileDownload)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_REMOTE_FILE, OnNMDblclkListRemoteFile)
	ON_BN_CLICKED(IDC_BTN_REMOTE_FILE_PLAY, OnBnClickedBtnRemoteFilePlay)
	ON_BN_CLICKED(IDC_BTN_REMOTE_FILE_STOP, OnBnClickedBtnRemoteFileStop)
	ON_BN_CLICKED(IDC_BTN_REMOTE_FILE_STOP_SAVE, OnBnClickedBtnRemoteFileStopSave)
	ON_BN_CLICKED(IDC_BTN_REMOTE_FILE_GOTO_BEGINNING, OnBnClickedBtnRemoteFileGotoBeginning)
	ON_BN_CLICKED(IDC_BTN_REMOTE_FILE_SLOW, OnBnClickedBtnRemoteFileSlow)
	ON_BN_CLICKED(IDC_BTN_REMOTE_FILE_SNATCH_PIC, OnBnClickedBtnRemoteFileSnatchPic)
	ON_BN_CLICKED(IDC_BTN_REMOTE_FILE_STEP, OnBnClickedBtnRemoteFileStep)
	ON_BN_CLICKED(IDC_BTN_REMOTE_FILE_SOUND, OnBnClickedBtnRemoteFileSound)
	ON_BN_CLICKED(IDC_BTN_REMOTE_FILE_FAST, OnBnClickedBtnRemoteFileFast)
	ON_BN_CLICKED(IDC_BTN_REMOTE_FILE_SAVE, OnBnClickedBtnRemoteFileSave)
	ON_WM_HSCROLL()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CHK_CARD_NUM, OnChkCardNum)
	ON_NOTIFY(NM_CLICK, IDC_LIST_REMOTE_FILE, OnClickListRemoteFile)
	ON_BN_CLICKED(IDC_BTN_LOCK, OnBtnLock)
	ON_BN_CLICKED(IDC_BTN_TIME_LOCK, OnBtnTimeLock)
	ON_BN_CLICKED(IDC_BTN_TIME_UNLOCK, OnBtnTimeUnlock)
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_REMOTE_FILE_PROGRESS, OnReleasedcaptureSliderRemoteFileProgress)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_REMOTE_FILE_VOLUME, OnReleasedcaptureSliderRemoteFileVolume)
	ON_BN_CLICKED(IDC_BTN_SET_TIME, OnBtnSetTime)
	ON_BN_CLICKED(IDC_BTN_BACKUP, OnBtnBackup)
	ON_CBN_SELCHANGE(IDC_COMBO_DISK_LIST, OnSelchangeComboDiskList)
	ON_BN_CLICKED(IDC_BTN_INSERT_RECOR_LABEL, OnBtnInsertRecordLabel)
	ON_BN_CLICKED(IDC_RADIO_FORWARD, OnRadioForward)
	ON_BN_CLICKED(IDC_RADIO_REVERSE, OnRadioReverse)
	ON_CBN_SELCHANGE(IDC_COMBO_FIND_TYPE, OnSelchangeComboFindType)
	ON_WM_PAINT()
	ON_WM_MOVE()
	ON_WM_CANCELMODE()
	ON_WM_CAPTURECHANGED()
	ON_BN_CLICKED(IDC_BUT_MUTILTHREAD_DOWNLOAD, OnButMutilthreadDownload)
    ON_CBN_SELCHANGE(IDC_CMB_USECARD_TYPE, OnSelchangeCmbUsecardType)
    ON_BN_CLICKED(IDC_BTN_SHOW_OSDINFO, OnBtnShowOsdinfo)
	//}}AFX_MSG_MAP

    ON_BN_CLICKED(IDC_BTN_START_DRAW, &CDlgPlayRemoteFile::OnBnClickedBtnStartDraw)
    ON_BN_CLICKED(IDC_BTN_ENDDRAW, &CDlgPlayRemoteFile::OnBnClickedBtnEnddraw)
    ON_BN_CLICKED(IDC_BTN_MEDICAL, &CDlgPlayRemoteFile::OnBnClickedBtnMedical)
END_MESSAGE_MAP()


// CDlgPlayRemoteFile message handlers
/*********************************************************
Function:	SetParentWnd
Desc:		set the handle of parent window
Input:	    hhWnd,pointer to handle array, parent pointer number
Output:	
Return:	
**********************************************************/
void CDlgPlayRemoteFile::SetParentWnd(HWND *hhWnd, int iParentNum)
{
	VERIFY(iParentNum==2);
	m_hPareDlgWnd=hhWnd[0];
	m_hPareTabWnd=hhWnd[1];
}

/*********************************************************
Function:	OnInitDialog
Desc:		Initialize the dialog
Input:	
Output:	
Return:	
**********************************************************/
BOOL CDlgPlayRemoteFile::OnInitDialog()
{
	CDialog::OnInitDialog();
    m_listRemoteFile.SetExtendedStyle(m_listRemoteFile.GetExtendedStyle()|LVS_EX_FULLROWSELECT);

	CRect rc(0, 0, 0, 0);
	GetParent()->GetClientRect(&rc);
	((CTabCtrl*)GetParent())->AdjustRect(FALSE, &rc);
	MoveWindow(&rc);
	InitUI();
	char szLan[128] = {0};
	g_StringLanType(szLan, "文件名称", "File Name");
	m_listRemoteFile.InsertColumn(0, szLan,LVCFMT_LEFT,120,-1);
	g_StringLanType(szLan, "大小", "Size");
	m_listRemoteFile.InsertColumn(1, szLan,LVCFMT_LEFT,50,-1);
	g_StringLanType(szLan, "开始时间", "Start time");
	m_listRemoteFile.InsertColumn(2, szLan, LVCFMT_LEFT, 120, -1);

	g_StringLanType(szLan, "结束时间", "Stop Time");
	m_listRemoteFile.InsertColumn(3, szLan, LVCFMT_LEFT, 120, -1);
	g_StringLanType(szLan, "属性", "Attribute");
	m_listRemoteFile.InsertColumn(4, szLan,LVCFMT_LEFT,50,-1);

	g_StringLanType(szLan,"文件索引号","Volume Record");
	m_listRemoteFile.InsertColumn(5, szLan,LVCFMT_LEFT,50,-1);

	g_StringLanType(szLan,"录像文件类型","Record Type");
	m_listRemoteFile.InsertColumn(6, szLan,LVCFMT_LEFT,50,-1);

	g_StringLanType(szLan,"码流类型","Stream Type");
	m_listRemoteFile.InsertColumn(7, szLan,LVCFMT_LEFT,50,-1);

    g_StringLanType(szLan, "IP", "IP");
    m_listRemoteFile.InsertColumn(8, szLan, LVCFMT_LEFT, 50, -1);

    g_StringLanType(szLan, "端口号", "Port");
    m_listRemoteFile.InsertColumn(9, szLan, LVCFMT_LEFT, 50, -1);

	OnSelchangeComboFindType();

	CTime timeCur = CTime::GetCurrentTime();
	CTime timeStart(timeCur.GetYear(),timeCur.GetMonth(),timeCur.GetDay(),0,0,0);
	CTime timeStop(timeCur.GetYear(),timeCur.GetMonth(),timeCur.GetDay(),23,59,59);
	m_ctDateStart = timeStart;
	m_ctTimeStart = timeStart;
	m_ctDateStop = timeStop;
	m_ctTimeStop = timeStop;
    m_ctDateSet = timeStart;
    m_ctTimeSet = timeStart;

	m_hPlayWnd = GetDlgItem(IDC_STATIC_REMOTE_FILE_WND)->m_hWnd;
	m_sliderVolume.SetRange(0,100);
	m_sliderPlayProgress.SetRange(0,100);
	m_progressDownload.SetRange(0,100);
	m_sliderPlayProgress.SetPos(0);
	m_progressDownload.SetPos(0);
	m_sliderVolume.SetPos(50);

	GetDlgItem(IDC_STATIC_REMOTE_FILE_WND)->GetWindowRect(m_rcWnd);
	ScreenToClient(m_rcWnd);
	g_pDlgRemoteFile=this;
// 	m_comboFileType.ResetContent();
// 	g_StringLanType(szLan, "全部", "All");
// 	m_comboFileType.AddString(szLan);
// 	g_StringLanType(szLan, "定时录像", "Schedule");
// 	m_comboFileType.AddString(szLan);
// 	g_StringLanType(szLan, "移动侦测", "Motion Detect");
// 	m_comboFileType.AddString(szLan);
// 	g_StringLanType(szLan, "接近报警", "Approach Alarm");
// 	m_comboFileType.AddString(szLan);
// 	g_StringLanType(szLan, "出钞报警", "Cash Out Alarm");
// 	m_comboFileType.AddString(szLan);
// 	g_StringLanType(szLan, "进钞报警", "Cash In Alarm");
// 	m_comboFileType.AddString(szLan);
// 	g_StringLanType(szLan, "命令触发", "Command Record");
// 	m_comboFileType.AddString(szLan);
// 	g_StringLanType(szLan, "手动录像", "Manual Record");
// 	m_comboFileType.AddString(szLan);
// 	g_StringLanType(szLan, "震动报警", "Vibration Alarm");
// 	m_comboFileType.AddString(szLan);
// 	g_StringLanType(szLan, "环境报警", "Environment Alarm");
// 	m_comboFileType.AddString(szLan);
//     g_StringLanType(szLan, "智能报警", "VCA Alarm");
	m_comboFileType.SetCurSel(0);
	m_cmbFindType.SetCurSel(0);
	m_comboAttr.SetCurSel(0);

//	CDateTimeCtrl *pDateStart = (CDateTimeCtrl*)GetDlgItem() 
    m_comboUseCardType.SetCurSel(0);
    GetDlgItem(IDC_STATICCARDNUM)->ShowWindow(FALSE);
    GetDlgItem(IDC_EDIT_CARD_NUM)->ShowWindow(FALSE);
    
    GetDlgItem(IDC_STATIC_ATM_TRADETYPE)->ShowWindow(FALSE);    
    GetDlgItem(IDC_CMB_ATM_TRADETYPE)->ShowWindow(FALSE);
    
    GetDlgItem(IDC_STATIC_MONEY)->ShowWindow(FALSE);    
    GetDlgItem(IDC_EDIT_MONEY)->ShowWindow(FALSE);

	m_comboStreamtype.SetCurSel(0);

	UpdateData(FALSE);
	return TRUE;
}

/*********************************************************
Function:	CheckInitParam
Desc:		check device parameters and initialize dialog parameters
Input:	
Output:	
Return:	    TRUE,check correct, parameters initialized; FALSE,check error, not initialized;
**********************************************************/
BOOL CDlgPlayRemoteFile::CheckInitParam()
{
	((CButton *)GetDlgItem(IDC_RADIO_FORWARD))->SetCheck(TRUE);
	((CButton *)GetDlgItem(IDC_RADIO_REVERSE))->SetCheck(FALSE);

	int iDeviceIndex = g_pMainDlg->GetCurDeviceIndex();
	if (iDeviceIndex == -1)
	{
		UpdateData(FALSE);
		return FALSE;
	}
	if ( m_iDeviceIndex == -1 || m_iDeviceIndex != iDeviceIndex)
	{
		m_iChanIndex = -1;
	}
	int iChanIndex = g_pMainDlg->GetCurChanIndex();
	if (iChanIndex == -1)
	{
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "set the channel index = 0");
		iChanIndex = 0;
		//AfxMessageBox("please select a channel!");
		//UpdateData(FALSE);
		//return FALSE;
	}
	
	UpdateData(TRUE);
	char szLan[128] = {0};

	m_iDeviceIndex = iDeviceIndex;
	m_lLoginID = g_struDeviceInfo[m_iDeviceIndex].lLoginID;
	m_iChanNum = g_struDeviceInfo[m_iDeviceIndex].iDeviceChanNum;
	m_lDeviceType = g_struDeviceInfo[m_iDeviceIndex].iDeviceType;
	m_lStartChan = g_struDeviceInfo[m_iDeviceIndex].iStartChan;

	//initialize parameters again while switching channel
	if (m_iChanIndex != iChanIndex)
	{
		m_iChanIndex = iChanIndex;
		m_iChanShowNum = g_struDeviceInfo[m_iDeviceIndex].pStruChanInfo[m_iChanIndex].iChannelNO;
		if (m_bDownloading)
		{
			OnBnClickedBtnFileDownload();
		}
		if (m_nPlayHandle>=0)
		{
			StopPlay();
		}

		m_bDownloading=FALSE;
		m_bSearching = FALSE;
		m_nPlayHandle = -1;
		m_bSound=TRUE;
		m_bPause=FALSE;
		m_bSetFrame = FALSE;
		m_bGetMaxTime = FALSE;
		m_bFullScreen = FALSE;
		m_bSaveFile = FALSE;
		//m_hPareDlgWnd = NULL;
		//m_hPareTabWnd = NULL;

		m_hFileThread = NULL;
		m_iFileNum = 0;
		m_lDownloadHandle = -1;
		m_nFileTime = 0;
		m_nTotalFrames = 0;
		m_nTotalSecond = 0;
		m_nTotalMinute = 0;
		m_nTotalHour = 0;
		m_listRemoteFile.DeleteAllItems();
		m_bChkCardNum = FALSE;
	}
	if (m_lLoginID < 0)
	{
		UpdateData(FALSE);
		return FALSE;
	}
	BOOL bAlarmIn = TRUE; //false update File Type for old atm
	if (m_lDeviceType == ATMDVR || m_lDeviceType == DS81XX_AH_S || \
		 m_lDeviceType == IDS8104_AHL_S_H ||\
		m_lDeviceType == DS_81XXAH_ST || m_lDeviceType == DS_81XXAHF_ST
		||m_lDeviceType == DS_81XXAHW_ST || m_lDeviceType == DS_81XXAHW_SP
		||m_lDeviceType == DS_81XXAHWL_ST || m_lDeviceType == DS_81XXAHWL_SP) //ATM DVR
    {
// 		GetDlgItem(IDC_CHK_CARD_NUM)->EnableWindow(TRUE);
// 		GetDlgItem(IDC_EDIT_CARD_NUM)->EnableWindow(TRUE);
		if (m_lDeviceType != DS_81XXAH_ST && m_lDeviceType != DS_81XXAHF_ST)
		{
			bAlarmIn = FALSE;
		}
    }
    else
    {
// 		GetDlgItem(IDC_CHK_CARD_NUM)->EnableWindow(FALSE);
// 		GetDlgItem(IDC_EDIT_CARD_NUM)->EnableWindow(FALSE);
    }

	int index = 0;
	m_comboFileType.ResetContent();
    if (!bAlarmIn) //ATM DVR
	{
		g_StringLanType(szLan, "全部", "All");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_CARD_ALL);
        index++;

		g_StringLanType(szLan, "定时录像", "Schedule");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_CARD_TIMING);
        index++;

		g_StringLanType(szLan, "移动侦测", "Motion Detect");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_CARD_MOTION_DETECT);
        index++;

		g_StringLanType(szLan, "接近报警", "Approach Alarm");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_CARD_CLOSETO);
        index++;

		g_StringLanType(szLan, "出钞报警", "Cash Out Alarm");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_CARD_BANKNOTE);
        index++;

		g_StringLanType(szLan, "进钞报警", "Cash In Alarm");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_CARD_FEEDING);
        index++;

		g_StringLanType(szLan, "命令触发", "Command Record");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_CARD_COMMAND);
        index++;

		g_StringLanType(szLan, "手动录像", "Manual Record");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_MANUAL);
        index++;

		g_StringLanType(szLan, "震动报警", "Vibration Alarm");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_CARD_VIBRATION);
        index++;

		g_StringLanType(szLan, "环境报警", "Environment Alarm");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_CARD_ENVIRON);
        index++;

        g_StringLanType(szLan, "智能报警", "VCA Alarm");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_CARD_VCA);
        index++;

// 		GetDlgItem(IDC_CHK_CARD_NUM)->EnableWindow(TRUE);
// 		GetDlgItem(IDC_EDIT_CARD_NUM)->EnableWindow(TRUE);
	}
	else 
	{
		g_StringLanType(szLan, "全部", "All");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_ALL);
        index++;
		
		g_StringLanType(szLan, "定时录像", "Schedule");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_TIMING);
        index++;
		
		g_StringLanType(szLan, "移动侦测", "Motion Detect");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_MOTION_DETECT);
        index++;
		
		g_StringLanType(szLan, "报警触发", "Alarm");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_ALARMIN);
        index++;

		g_StringLanType(szLan, "报警|动测", "Alarm|Motion");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_MOTION_OR_ALARMIN);
        index++;

		g_StringLanType(szLan, "报警&动测", "Alarm&Motion");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_MOTION_AND_ALARMIN);
        index++;

		g_StringLanType(szLan, "命令触发", "Command Record");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_COMMAND);
        index++;

		g_StringLanType(szLan, "手动录像", "Manual Record");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_MANUAL);
        index++;

		g_StringLanType(szLan, "智能报警", "VCA Alarm");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_VCA);
        index++;

		g_StringLanType(szLan, "PIR报警", "PIR Alarm");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_PIR_ALARM);
        index++;
		
		g_StringLanType(szLan, "无线报警", "WIRELESS Alarm");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_WIRELESS_ALARM);
        index++;
		
		g_StringLanType(szLan, "呼救报警", "CALLHELP Alarm");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_CALLHELP_ALARM);
        index++;
		
		g_StringLanType(szLan, "报警录像", "Alarm Record");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_PIR_WIRELESS_CALLHELP);
        index++;

		g_StringLanType(szLan, "智能交通事件报警", "Intelligent Transportation");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_INTELLIGENT_TRANSPORT);
        index++;
// 		GetDlgItem(IDC_CHK_CARD_NUM)->EnableWindow(FALSE);
// 		GetDlgItem(IDC_EDIT_CARD_NUM)->EnableWindow(FALSE);

//#ifdef NET_DVR_GET_TRAVERSE_PLANE_DETECTION
		g_StringLanType(szLan, "越界侦测", "NULL");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_LINE_DETECTION);
        index++;
///#endif //NET_DVR_GET_TRAVERSE_PLANE_DETECTION
		
		g_StringLanType(szLan, "区域入侵", "Field Detection");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_FIELD_DETECTUIN);
        index++;
		
		g_StringLanType(szLan, "声音异常", "Audio Exception");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_AUDIO_EXCEPTION);
        index++;

		//场景变更侦测录像查询2013-07-17
		g_StringLanType(szLan, "场景变更侦测", "Scene Change detection");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_SCENECHANGE_DETECTION);
        index++;
		
		g_StringLanType(szLan, "智能侦测", "Smart detection");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_INTELLIGENT_DETECTION);
        index++;

		g_StringLanType(szLan, "人脸侦测", "face detection");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_FACE_DETECTION);
        index++;
		
		g_StringLanType(szLan, "进入区域侦测", "Enter region detection");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_REGION_ENTRANCE_DETECTION);
        index++;
		
		g_StringLanType(szLan, "离开区域侦测", "Leave region detection");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_REGION_EXITING_DETECTION);
        index++;
		
		g_StringLanType(szLan, "徘徊侦测", "Hovering detection");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_LOITERING_DETECTION);
        index++;
		
		g_StringLanType(szLan, "人员聚集侦测", "Researchers gathered detection");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_PEOPLE_GATHERING_DETECTION);
        index++;
		
		g_StringLanType(szLan, "快速运动侦测", "Fast motion detection");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_FAST_MOVING_DETECTION);
        index++;
		
		g_StringLanType(szLan, "停车侦测", "Parking detection");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_PARKING_DETECTION);
        index++;
		
		g_StringLanType(szLan, "物品遗留侦测", "Goods Legacy Detection");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_UNATTENDED_BAGGAGE_DETECTION);
        index++;
		
		g_StringLanType(szLan, "物品拿取侦测", "Goods Take Detection");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_OBJECT_REMOVAL_DETECTION);
        index++;

		g_StringLanType(szLan, "火点检测", "Fire Detection");
		m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_OBJECT_FIRE_DETECTION);
        index++;

        g_StringLanType(szLan, "防破坏检测", "VandalProof Detection");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_OBJECT_VANDALPROOF_DETECTION);
        index++;

        g_StringLanType(szLan, "POS录像", "POS Record");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_POS_RECORD);
        index++;

        g_StringLanType(szLan, "船只检测", "Ships Detection");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_THERMAL_SHIPS_DETECTION);
        index++;
        g_StringLanType(szLan, "测温预警", "Temperature Warning");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_THERMAL_THERMOMETRY_EARLYWARNING);
        index++;
        g_StringLanType(szLan, "测温报警", "Temperature Alarm");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_THERMAL_THERMOMETRY_ALARM);
        index++;
		g_StringLanType(szLan, "温差报警", "Temperature Diff Alarm");
		m_comboFileType.InsertString(index, szLan);
		m_comboFileType.SetItemData(index, FILE_THERMOMETRY_DIFF_ALARM);
		index++;
        g_StringLanType(szLan, "离线测温报警", "Temperature Offline Alarm");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_THERMOMETRY_OFFLINE_ALARM);
        index++;
        g_StringLanType(szLan, "防区报警", "Zoon AlarmIN Alarm");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_ZONE_ALARMIN_ALARM);
        index++;
        g_StringLanType(szLan, "紧急求助", "Emergency Call Help Alarm");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_EMERGENCY_CALL_HELP_ALARM);
        index++;
        g_StringLanType(szLan, "业务咨询", "Consult");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_CONSULT);
        index++;
        g_StringLanType(szLan, "起身检测", "Getup");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_VCA_EVENT_GET_UP);
        index++;
        g_StringLanType(szLan, "折线攀高", "AdvReachHeight");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_VCA_EVENT_ADV_REACH_HEIGHT);
        index++;
        g_StringLanType(szLan, "如厕超时", "ToiletTarry");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_VCA_EVENT_TOILET_TARRY);
        index++;
        g_StringLanType(szLan, "人脸抓拍", "FaceSnap");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_VCA_EVENT_FACE_SNAP);
        index++;
        g_StringLanType(szLan, "非法摆摊", "unregisteredStreetVendor");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_UNREGISTERED_STREET_VENDOR);
        index++;
        g_StringLanType(szLan, "人体目标识别", "humanRecognition");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_UNREGISTERED_HUMAN_RECOGNITION);
        index++;
        g_StringLanType(szLan, "剧烈运动", "Motion");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_VCA_EVENT_MOTION);
        index++;
        g_StringLanType(szLan, "离岗检测", "LeavePosition");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_VCA_EVENT_LEAVE_POSITION);
        index++;
        g_StringLanType(szLan, "起立检测", "StandUp");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_VCA_EVENT_STAND_UP);
        index++;
        g_StringLanType(szLan, "人数变化", "PeopleNumChange");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_VCA_EVENT_PEOPLE_NUM_CHANGE);
        index++;
        g_StringLanType(szLan, "交通违章", "TrafficViolation");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_TRAFFIC_VIOLATION);
        index++;
        g_StringLanType(szLan, "挖沙船检测", "DredgerDetection");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_THERMOMETRY_DREDGERDETECTION);
        index++;
        g_StringLanType(szLan, "废气排放监测", "WasteGas");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_THERMOMETRY_WASTE);
        index++;

        g_StringLanType(szLan, "灰度报警", "greyScaleAlarms");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_THERMOMETRY_GREY_SCALE_ALARM);
        index++;

        g_StringLanType(szLan, "振动侦测", "VibrationDetection");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_VIBRATION_DETECTION);
        index++;

        g_StringLanType(szLan, "烟雾检测", "smokeDetection");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_SMOKE_DETECTION);
        index++;

        g_StringLanType(szLan, "通道定时录像事件", "ChannelTimingRecording");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_CHANNEL_TIMING_RECORDING);
        index++;

        g_StringLanType(szLan, "AI开放平台事件", "AIOpenPlatform");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_AI_OPEN_PLATFORM);
        index++;

        g_StringLanType(szLan, "传感器事件", "SensorAlarm");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_SENSOR_ALARM);
        index++;

        g_StringLanType(szLan, "紫光电子检测", "UVPhotonDetection");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_UVPHOTONEN_DECTION);
        index++;

        g_StringLanType(szLan, "设备电源状态上报", "DevPowerStatusReport");
        m_comboFileType.InsertString(index, szLan);
        m_comboFileType.SetItemData(index, FILE_DEV_POWER_STATUS_REPORT);
        index++;
	}
	m_iFileType = 0;
	m_comboFileType.SetCurSel(m_iFileType);
	m_iFileAttr = 0;
	m_comboAttr.SetCurSel(m_iFileAttr);
    m_drawtype.SetCurSel(0);

    GetDiskList();
    if (m_struDiskList.dwNodeNum > 0)
    {
        m_comboDiskList.SetCurSel(0);
        OnSelchangeComboDiskList();
    }
	UpdateData(FALSE); 
	return TRUE;
}

/*********************************************************
Function:	InitUI
Desc:		initialize UI
Input:	
Output:	
Return:	
**********************************************************/
void CDlgPlayRemoteFile::InitUI()
{
	// initialize button
	CButton *pButton;

	m_rcScreen=CRect(0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_PLAY);
	pButton->SetIcon(m_hPlayDisableIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_STOP);
	pButton->SetIcon(m_hStopDisableIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_FAST);
	pButton->SetIcon(m_hFastDisableIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_SLOW);
	pButton->SetIcon(m_hSlowDisableIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_STEP);
	pButton->SetIcon(m_hStepDisableIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_GOTO_BEGINNING);
	pButton->SetIcon(m_hGotoHeadDisableIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_SNATCH_PIC);
	pButton->SetIcon(m_hCaptureIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_SOUND);
	pButton->SetIcon(m_hSoundStopIcon);
	pButton->EnableWindow(FALSE);

    pButton = (CButton *)GetDlgItem(IDC_BTN_SHOW_OSDINFO);
	pButton->EnableWindow(FALSE);
}

/*********************************************************
Function:	SetStopState
Desc:		change control state of playback
Input:	
Output:	
Return:	
**********************************************************/
void CDlgPlayRemoteFile::SetStopState()
{
	CButton *pButton;

	m_sliderPlayProgress.SetPos(0);
	m_sliderPlayProgress.EnableWindow(FALSE);
	m_sliderVolume.EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_PLAY);
	pButton->SetIcon(m_hPlayEnableIcon);
	pButton->EnableWindow(TRUE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_STOP);
	pButton->SetIcon(m_hStopDisableIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_FAST);
	pButton->SetIcon(m_hFastDisableIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_SLOW);
	pButton->SetIcon(m_hSlowDisableIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_STEP);
	pButton->SetIcon(m_hStepDisableIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_GOTO_BEGINNING);
	pButton->SetIcon(m_hGotoHeadDisableIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_SNATCH_PIC);
	pButton->SetIcon(m_hCaptureIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_SOUND);
	pButton->SetIcon(m_hSoundStopIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_STEP);
	pButton->SetIcon(m_hStepDisableIcon);
	pButton->EnableWindow(FALSE);	
}

/*********************************************************
  Function:	ExitPlayBack
  Desc:		exit playback and release corresponding resource
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteFile::ExitPlayBack()
{
	if (m_hFileThread)
	{
		TerminateThread(m_hFileThread, 0);
		CloseHandle(m_hFileThread);
		m_hFileThread = NULL;
		NET_DVR_FindClose(m_lFileHandle);
	}	

	if (m_nPlayHandle!=-1)
	{
		NET_DVR_StopPlayBack(m_nPlayHandle);
		m_nPlayHandle = -1;
	}
	Sleep(100);
}

/*********************************************************
Function:	PlayBack
Desc:		start to playback remote video files
Input:	
Output:	
Return:	
**********************************************************/
void CDlgPlayRemoteFile::PlayBack()
{
	UpdateData(TRUE);

	CString csFileName;
	int iFileSelPos = 0;
	HWND hPlayWnd=m_staticPlayWnd.GetSafeHwnd();
	POSITION  posItem = m_listRemoteFile.GetFirstSelectedItemPosition();
	char szLan[128] = {0};
	if (posItem == NULL)
	{
		g_StringLanType(szLan, "请选择要播放的文件!", "Please select the file to play");
		AfxMessageBox(szLan); 
		return;
	}

	iFileSelPos = m_listRemoteFile.GetNextSelectedItem(posItem);
	csFileName.Format("%s",m_listRemoteFile.GetItemText(iFileSelPos,0));
	if (csFileName.IsEmpty())
	{
		return;
	}

	sprintf(m_szFileName,"%s",csFileName);
	//	remoteplay_info.srcfilename=m_szFileName;
	if (m_nPlayHandle>=0)
	{
		if (NET_DVR_StopPlayBack(m_nPlayHandle))
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_StopPlayBack");
		}
		m_nPlayHandle = -1;
		Sleep(400);
	}


    NET_DVR_PLAY_BY_NAME_PARA struPlayByName = { 0 };
    memcpy(struPlayByName.szFileName, m_szFileName, 100);
    struPlayByName.hWnd = hPlayWnd;
    CString szIp;
    CString szPort;
    szIp.Format("%s", m_listRemoteFile.GetItemText(iFileSelPos, 8));
    //ipv4
    if (!g_ValidIPv6((BYTE*)(LPCSTR)szIp))
    {
        sprintf(struPlayByName.struAddr.struIP.sIpV4, "%s", szIp);
    }
    else
    {
        sprintf((char *)struPlayByName.struAddr.struIP.byIPv6, "%s", szIp);
    }
    szPort = m_listRemoteFile.GetItemText(iFileSelPos, 9);
    struPlayByName.struAddr.wPort = _ttoi(szPort);
    struPlayByName.byDownload = m_bDownload;

#ifndef CALLBACK_PLAY
	if (GetCheckedRadioButton(IDC_RADIO_FORWARD, IDC_RADIO_REVERSE) == IDC_RADIO_FORWARD)
	{
		memcpy(m_szCurFileName, m_szFileName, 200);
        m_nPlayHandle = NET_DVR_PlayBackByName_V50(m_lLoginID, &struPlayByName);
	}
	else
	{
		memcpy(m_szCurFileName, m_szFileName, 200);
        m_nPlayHandle = NET_DVR_PlayBackReverseByName_V50(m_lLoginID, &struPlayByName);
	}
#else
	if (GetCheckedRadioButton(IDC_RADIO_FORWARD, IDC_RADIO_REVERSE) == IDC_RADIO_FORWARD)
	{
        struPlayByName.hWnd = NULL;
		memcpy(m_szCurFileName, m_szFileName, 200);
        m_nPlayHandle = NET_DVR_PlayBackByName_V50(m_lLoginID, &struPlayByName);
	}
	else
	{
		memcpy(m_szCurFileName, m_szFileName, 200);
        m_nPlayHandle = NET_DVR_PlayBackReverseByName_V50(m_lLoginID, &struPlayByName);
	}
#endif
	
	if (m_nPlayHandle==-1)
	{
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PlayBackByName[%s]",m_szFileName);
		char szLan[1024] = {0};
		g_StringLanType(szLan, "回放失败!", "Play Back Failed!");
		AfxMessageBox(szLan);
		return;
	}
	else
	{
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PlayBackByName[%s]",m_szFileName);
	}
	
    NET_DVR_LINK_ADDR struParam = { 0 };
    if (NET_DVR_GetLinkAddr(m_nPlayHandle, ENUM_LINK_PLAYBACK, &struParam))
    {
        g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_GetLinkAddr, dev addr[%s:%d], local addr[%s:%d]", \
            &(struParam.uDevIP), struParam.wDevPort[0], &(struParam.uLocalIP), struParam.wLocalPort[0]);
    }
    else
    {
        g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_GetLinkAddr");
    }

    //设置裸码流回调
    if (m_bEsCallBack)
    {
        NET_DVR_SetPlayBackESCallBack(m_nPlayHandle, PlayESCallBack_V40, this);
        g_hESFileHandle =CreateFile("./ESRemoteFile.mp4", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (g_hESFileHandle == INVALID_HANDLE_VALUE)
        {
            AfxMessageBox("Create ESRemoteTime.mp4 fial");
            return;
        }
    }
    NET_DVR_SetPlayDataCallBack_V40(m_nPlayHandle, PlayDataCallBack_V40, this);
	SetPlayState();
	
	//for test
// 	csFileName.Format("C:\\mpeg4record\\test%d-%d.mp4", m_iDeviceIndex, m_iChanIndex);
// 	NET_DVR_PlayBackSaveData(m_nPlayHandle, csFileName.GetBuffer(0));
	
	//设置转码类型
	int iTransType = m_comboTransType.GetCurSel() + 1;
    if (iTransType == 4)
    {
        iTransType = 5;
    }
	if (NET_DVR_PlayBackControl_V40(m_nPlayHandle, NET_DVR_SET_TRANS_TYPE, &iTransType, NULL))
	{
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_SET_TRANS_TYPE TransType[%d]", iTransType);
	}
	else
	{
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_SET_TRANS_TYPE TransType[%d]", iTransType);
	}

	if (NET_DVR_PlayBackControl(m_nPlayHandle, NET_DVR_PLAYSTART, m_iOffset, NULL))//
	{
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAYSTART offset[%d]", m_iOffset);
	}
	else
	{
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAYSTART offset[%d]", m_iOffset);
		char szLan[1024] = {0};
		NET_DVR_StopPlayBack(m_nPlayHandle);
		SetStopState();
		m_nPlayHandle = -1;
		g_StringLanType(szLan, "回放失败!", "NET_DVR_PLAYSTART Failed!");
		AfxMessageBox(szLan);
		return;
	}

	if (NET_DVR_PlayBackControl(m_nPlayHandle, NET_DVR_PLAYSTARTAUDIO, 0, NULL))
	{
		m_bSound=TRUE;
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAYSTARTAUDIO");
		NET_DVR_PlayBackControl(m_nPlayHandle, NET_DVR_PLAYAUDIOVOLUME, (0xffff)/2, NULL);
		m_sliderVolume.SetPos(50);
		((CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_SOUND))->SetIcon(m_hSoundStartIcon);
	}
	else
	{
		m_bSound=FALSE;
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAYSTARTAUDIO");
		((CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_SOUND))->SetIcon(m_hSoundStopIcon);
	}
	m_bGetMaxTime = FALSE;
	SetTimer(REMOTE_PLAY_STATE_TIMER,1000,NULL);
}

/*********************************************************
Function:	SetPlayState
Desc:		update control state while playing back
Input:	
Output:	
Return:	
**********************************************************/
void CDlgPlayRemoteFile::SetPlayState() 
{
	m_sliderPlayProgress.EnableWindow(TRUE);
	m_sliderVolume.EnableWindow(TRUE);

	CButton *pButton;

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_PLAY);
	pButton->SetIcon(m_hPauseEnableIcon);
	pButton->EnableWindow(TRUE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_STOP);
	pButton->SetIcon(m_hStopEnableIcon);
	pButton->EnableWindow(TRUE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_FAST);
	pButton->SetIcon(m_hFastEnableIcon);
	pButton->EnableWindow(TRUE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_SLOW);
	pButton->SetIcon(m_hSlowEnableIcon);
	pButton->EnableWindow(TRUE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_SNATCH_PIC);
	pButton->SetIcon(m_hCaptureIcon);
	pButton->EnableWindow(TRUE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_SOUND);
	if (m_bSound)
	{
		pButton->SetIcon(m_hSoundStartIcon);
	}
	else
	{
		pButton->SetIcon(m_hSoundStopIcon);
	}
	pButton->EnableWindow(TRUE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_STEP);
	pButton->SetIcon(m_hStepEnableIcon);
	pButton->EnableWindow(TRUE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_GOTO_BEGINNING);
	pButton->SetIcon(m_hGotoHeadEnableIcon);
	pButton->EnableWindow(TRUE);

    pButton = (CButton *)GetDlgItem(IDC_BTN_SHOW_OSDINFO);
    pButton->EnableWindow(TRUE);
}

/*********************************************************
  Function:	SetPauseState
  Desc:		set control state
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteFile::SetPauseState()
{
	CButton *pButton;	
	pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_PLAY);
	pButton->SetIcon(m_hPlayEnableIcon);
	pButton->EnableWindow(TRUE);
}

/*********************************************************
  Function:	StopPlay
  Desc:		stop play
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteFile::StopPlay()
{
	//if (m_bFullScreen)
	//{
	//	m_bFullScreen = FALSE;
	//	PlayBackShowNormal();
	//}
	if (m_nPlayHandle >= 0)
	{
		if (m_bSaveFile)
		{
			if (!NET_DVR_StopPlayBackSave(m_nPlayHandle))
			{
				g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_StopPlayBackSave");
			}
			else
			{
				g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_StopPlayBackSave");
				m_bSaveFile = FALSE;
			}
			
		}
		if (!NET_DVR_StopPlayBack(m_nPlayHandle))
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_StopPlayBack");
		}
		else
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_StopPlayBack");
			m_bSaveFile = FALSE;
		}
		if (m_lPort >= 0)
		{
			PlayM4_FreePort(m_lPort);
			m_lPort = -1;
		}
		m_nPlayHandle = -1;
		m_sliderPlayProgress.SetPos(0); 
		CString csText;

		GetDlgItem(IDC_STATIC_REMOTE_FILE_PLAY_STATUS)->SetWindowText(csText);
		KillTimer(REMOTE_PLAY_STATE_TIMER);
		csText.Format("%d/%d  %02d:%02d:%02d/%02d:%02d:%02d",0,0,0,0,0,0,0,0);
		UpdateData(FALSE);
		Invalidate(TRUE);
	}
	m_bPause = FALSE;
	m_bSetFrame = FALSE;
    if (g_hESFileHandle != NULL)
    {
        CloseHandle(g_hESFileHandle);
        g_hESFileHandle = NULL;
    }
	SetStopState();
}

/*********************************************************
Function:	EnableFlashWnd
Desc:		refresh control state
Input:	
Output:	
Return:	
**********************************************************/
void CDlgPlayRemoteFile::EnableFlashWnd(BOOL bEnable)
{
	if (bEnable)
	{
		//THE same to multiplay
		GetDlgItem(IDC_STATIC_REMOTE_FILE_PLAY_STATUS)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_BTN_REMOTE_FILE_PLAY)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_BTN_REMOTE_FILE_STOP)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_BTN_REMOTE_FILE_SLOW)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_BTN_REMOTE_FILE_FAST)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_BTN_REMOTE_FILE_SOUND)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_SLIDER_REMOTE_FILE_VOLUME)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_SLIDER_REMOTE_FILE_PROGRESS)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_COMBO_REMOTE_DATE_START)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_COMBO_REMOTE_DATE_STOP)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_COMBO_REMOTE_TIME_STOP)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_COMBO_REMOTE_TIME_START)->ModifyStyle(0,WS_VISIBLE,0);
		//GetDlgItem(IDC_COMBO_REMOTE_FILE_TYPE)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_BTN_REMOTE_SEARCH_LIST)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_BTN_REMOTE_FILE_SNATCH_PIC)->ModifyStyle(0,WS_VISIBLE,0);	
		//the only for single play
		GetDlgItem(IDC_BTN_REMOTE_FILE_DOWNLOAD)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_LIST_REMOTE_FILE)->ModifyStyle(0,WS_VISIBLE,0);
		if (m_bDownloading)
		{
			GetDlgItem(IDC_PROGRESS_REMOTE_FILE_DOWNLOAD)->ModifyStyle(0,WS_VISIBLE,0);
		}
		GetDlgItem(IDC_BTN_REMOTE_FILE_STEP)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_BTN_REMOTE_FILE_GOTO_BEGINNING)->ModifyStyle(0,WS_VISIBLE,0);
	}
	else
	{
		//THE same to multiplay
		GetDlgItem(IDC_STATIC_REMOTE_FILE_PLAY_STATUS)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_BTN_REMOTE_FILE_PLAY)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_BTN_REMOTE_FILE_STOP)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_BTN_REMOTE_FILE_SLOW)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_BTN_REMOTE_FILE_FAST)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_BTN_REMOTE_FILE_SOUND)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_SLIDER_REMOTE_FILE_VOLUME)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_SLIDER_REMOTE_FILE_PROGRESS)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_COMBO_REMOTE_DATE_START)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_COMBO_REMOTE_DATE_STOP)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_COMBO_REMOTE_TIME_STOP)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_COMBO_REMOTE_TIME_START)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_COMBO_REMOTE_FILE_TYPE)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_BTN_REMOTE_SEARCH_LIST)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_BTN_REMOTE_FILE_SNATCH_PIC)->ModifyStyle(WS_VISIBLE,0,0);
		//the only for single play
		GetDlgItem(IDC_LIST_REMOTE_FILE)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_BTN_REMOTE_FILE_STEP)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_BTN_REMOTE_FILE_GOTO_BEGINNING)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_BTN_REMOTE_FILE_DOWNLOAD)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_PROGRESS_REMOTE_FILE_DOWNLOAD)->ModifyStyle(WS_VISIBLE,0,0);
	}
}

/*********************************************************
Function:	IsInWnd
Desc:		check play window double clicked by mouse
Input:	
Output:	
Return:	
**********************************************************/
BOOL CDlgPlayRemoteFile::IsInWnd(CPoint point)
{
	CRect rc;
	GetDlgItem(IDC_STATIC_REMOTE_FILE_WND)->GetWindowRect(&rc);
	ScreenToClient(&rc);
	if (PtInRect(&rc,point))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*********************************************************
Function:	PlayBackFullScreen
Desc:		playback in full screen
Input:	
Output:	
Return:	
**********************************************************/
void CDlgPlayRemoteFile::PlayBackFullScreen()
{
	EnableFlashWnd(FALSE);

	CRect rc,rcClient;
	::GetWindowRect(m_hPareDlgWnd,m_rcPareDlg);//get playback main window's rect

	::GetWindowRect(m_hPareTabWnd,m_rcPareTab);//get tab box's rect

	::ScreenToClient(m_hPareDlgWnd,(POINT*)(&m_rcPareTab));
	::ScreenToClient(m_hPareDlgWnd,(POINT*)(&m_rcPareTab)+1);

	::GetWindowRect(m_hWnd,m_rcParent);
	::ScreenToClient(m_hPareTabWnd,(POINT*)(&m_rcParent));
	::ScreenToClient(m_hPareTabWnd,(POINT*)(&m_rcParent)+1);


	::SetWindowPos(m_hPareDlgWnd,NULL,m_rcScreen.left-SCREEN_EDGE,m_rcScreen.top-SCREEN_EDGE,m_rcScreen.Width()+2*SCREEN_EDGE,m_rcScreen.Height()+2*SCREEN_EDGE,SWP_SHOWWINDOW);

	::GetClientRect(m_hPareDlgWnd,rc);
	::SetWindowPos(m_hPareTabWnd,NULL,rc.left,rc.top,rc.Width(),rc.Height(),SWP_SHOWWINDOW);

	::GetClientRect(m_hPareTabWnd,rc);
	::SetWindowPos(m_hWnd,NULL,rc.left,rc.top,rc.Width(),rc.Height(),SWP_SHOWWINDOW);

	GetDlgItem(IDC_STATIC_REMOTE_FILE_WND)->GetClientRect(rcClient);
	GetDlgItem(IDC_STATIC_REMOTE_FILE_WND)->GetWindowRect(rc);
	int iEdge=(rc.Width()-rcClient.Width())/2;
	rc=m_rcScreen;
	rc.left-=iEdge;
	rc.right+=iEdge;
	rc.top-=iEdge;
	rc.bottom+=iEdge;
	this->ScreenToClient(rc);
	::SetWindowPos(GetDlgItem(IDC_STATIC_REMOTE_FILE_WND)->m_hWnd,NULL,rc.left,rc.top,rc.Width(),rc.Height(),SWP_SHOWWINDOW);
	Invalidate(FALSE);
	UpdateWindow();
}

/*********************************************************
Function:	PlayBackShowNormal
Desc:		return to original interface
Input:	
Output:	
Return:	
**********************************************************/
void CDlgPlayRemoteFile::PlayBackShowNormal()
{
	GetDlgItem(IDC_STATIC_REMOTE_FILE_WND)->SetWindowPos(NULL,m_rcWnd.left,m_rcWnd.top,m_rcWnd.Width(),m_rcWnd.Height(),SWP_SHOWWINDOW);
	::SetWindowPos(m_hWnd,NULL,m_rcParent.left,m_rcParent.top,m_rcParent.Width(),m_rcParent.Height(),SWP_SHOWWINDOW);
	::SetWindowPos(m_hPareTabWnd,NULL,m_rcPareTab.left,m_rcPareTab.top,m_rcPareTab.Width(),m_rcPareTab.Height(),SWP_SHOWWINDOW);
	::SetWindowPos(m_hPareDlgWnd,NULL,m_rcPareDlg.left,m_rcPareDlg.top,m_rcPareDlg.Width(),m_rcPareDlg.Height(),SWP_SHOWWINDOW);
	EnableFlashWnd(TRUE);
	Invalidate(FALSE);
	UpdateWindow();
}

/*********************************************************
  Function:	OnDestroy
  Desc:		destroy dialog and its corresponding resource while exit
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteFile::OnDestroy() 
{
	KillTimer(DOWNSTATE_TIMER);
	CDialog::OnDestroy();	
}

/*********************************************************
  Function:	OnNMDblclkListRemoteFile
  Desc:		stop playing and play selected video file
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteFile::OnNMDblclkListRemoteFile(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnBnClickedBtnRemoteFileStop();
	OnBnClickedBtnRemoteFilePlay();
	*pResult = 0;
}

/*********************************************************
Function:	OnHScroll
Desc:		response function of draging step scroll
Input:	
Output:	
Return:	
**********************************************************/
void CDlgPlayRemoteFile::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	char chPos = 0;
	WORD temp = (0xffff)/100;
	WORD dwVolume = 0;
	if (GetDlgItem(IDC_SLIDER_REMOTE_FILE_VOLUME) == pScrollBar)
	{
		chPos =(char)m_sliderVolume.GetPos();
		dwVolume =(WORD) (chPos*temp);
		m_sliderVolume.SetPos(chPos);
	}
	else if (GetDlgItem(IDC_SLIDER_REMOTE_FILE_PROGRESS) == pScrollBar)
	{
		chPos = (char)(m_sliderPlayProgress.GetPos());
		m_iOffset = (m_dwCurFileLen/100)*chPos;
		m_sliderPlayProgress.SetPos(chPos);
	}
	UpdateData(FALSE);
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

/*********************************************************
Function:	OnLButtonDblClk
Desc:		double click screen to enlarge image
Input:	
Output:	
Return:	
**********************************************************/
void CDlgPlayRemoteFile::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if ((IsInWnd(point)) && (m_nPlayHandle>=0))
	{
		m_bFullScreen=!m_bFullScreen;
		g_pMainDlg->FullScreen(m_bFullScreen);// main struct deal
		if (m_bFullScreen)
		{
			PlayBackFullScreen();
		}
		else
		{
			PlayBackShowNormal();
		}

	}

	CDialog::OnLButtonDblClk(nFlags, point);
}

/*********************************************************
  Function:	OnTimer
  Desc:		response function on timer, showing state of download and playback
  Input:	
  Output:	
  Return:	
**********************************************************/
#if (_MSC_VER >= 1500)	//vs2008
void CDlgPlayRemoteFile::OnTimer(UINT_PTR nIDEvent)
#else
void CDlgPlayRemoteFile::OnTimer(UINT nIDEvent) 
#endif
{
	// TODO: Add your message handler code here and/or call default
	int nPos;
	DWORD nPos1;
	DWORD nCurrentTime = 0,nHour = 0,nMinute = 0,nSecond = 0,nCurrentFrame = 0;
	CString csText;
	NET_DVR_TIME struOsdTime;
	char szLan[128] = {0};
	switch (nIDEvent)
	{
	case DOWNSTATE_TIMER:
        if (m_bDownloading)
        {
            nPos = m_MutilDowload.CountPos();
            if (nPos < 0)		//failed
            {
                g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_GetDownloadPos %s", g_struDeviceInfo[m_iDeviceIndex].pStruChanInfo[m_iChanIndex].chChanName);
                //NET_DVR_StopGetFile(m_lDownloadHandle);
                m_progressDownload.ShowWindow(SW_HIDE);
                g_pDlgRemoteFile->GetDlgItem(IDC_BTN_REMOTE_FILE_DOWNLOAD)->SetWindowText("下载");
                g_pDlgRemoteFile->m_bDownloading=FALSE;
                g_StringLanType(szLan, "停止下载文件", "Stop downloading files");
                m_csDownloadStat.Format("%s%s", szLan, m_szDownLoadFileName);
                GetDlgItem(IDC_STATIC_REMOTE_DOWNLOAD_STATE)->ShowWindow(SW_HIDE);
                AfxMessageBox("获取下载进度失败!");
            }
            else if (nPos == 100)		//end download
            {
                g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_GetDownloadPos %s", g_struDeviceInfo[m_iDeviceIndex].pStruChanInfo[m_iChanIndex].chChanName);
                //NET_DVR_StopGetFile(m_lDownloadHandle);
                m_progressDownload.ShowWindow(SW_HIDE);
                g_StringLanType(szLan, "下载", "Download");
                g_pDlgRemoteFile->GetDlgItem(IDC_BTN_REMOTE_FILE_DOWNLOAD)->SetWindowText(szLan);
                g_pDlgRemoteFile->m_bDownloading=FALSE;
                g_StringLanType(szLan, "下载文件结束", "Finish downloading files");
                m_csDownloadStat.Format("%s%s", szLan, m_szDownLoadFileName);
                GetDlgItem(IDC_STATIC_REMOTE_DOWNLOAD_STATE)->ShowWindow(SW_HIDE);
                AfxMessageBox("下载完毕!");
                g_StringLanType(szLan, "多路下载", "Download");
                GetDlgItem(IDC_BUT_MUTILTHREAD_DOWNLOAD)->SetWindowText(szLan);
            }
            else if (nPos > 100)		//download exception for network problems or DVR hasten
            {
                //NET_DVR_StopGetFile(m_lDownloadHandle);
                m_progressDownload.ShowWindow(SW_HIDE);
                g_pDlgRemoteFile->GetDlgItem(IDC_BTN_REMOTE_FILE_DOWNLOAD)->SetWindowText("下载");
                g_pDlgRemoteFile->m_bDownloading=FALSE;
                g_StringLanType(szLan, "停止下载文件", "Stop downloading files");
                m_csDownloadStat.Format("%s%s", szLan, m_szDownLoadFileName);
                GetDlgItem(IDC_STATIC_REMOTE_DOWNLOAD_STATE)->ShowWindow(SW_HIDE);
                g_StringLanType(szLan, "由于网络原因或DVR忙,下载异常终止!", " Due to network reasons or DVR is busy, download abnormal termination");
                AfxMessageBox(szLan);
                g_StringLanType(szLan, "多路下载", "Download");
                GetDlgItem(IDC_BUT_MUTILTHREAD_DOWNLOAD)->SetWindowText(szLan);
            }
            else
            {
                
                m_progressDownload.SetPos(nPos);
            }
		}
		break;
	case REMOTE_PLAY_STATE_TIMER:
		//	nTemp++;
		//	if (nTemp == 5)
		//		NET_DVR_PlayBackControl(m_nPlayHandle, NET_DVR_PLAYSETPOS, 80, NULL);

		if (!m_bGetMaxTime)
		{
			NET_DVR_PlayBackControl(m_nPlayHandle, NET_DVR_GETTOTALTIME, 0, &m_nFileTime);
			if (m_nFileTime == 0)
			{
				return;
			}

			if (NET_DVR_PlayBackControl(m_nPlayHandle, NET_DVR_GETTOTALFRAMES, 0, &m_nTotalFrames))
			{
				g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_GETTOTALFRAMES %d", m_nTotalFrames);
				if (m_nTotalFrames == 0)
				{
					return;
				}
			}
			else
			{
				g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_GETTOTALFRAMES");
			}

			m_nTotalHour=m_nFileTime/3600;
			m_nTotalMinute=(m_nFileTime%3600)/60;
            m_nTotalSecond=m_nFileTime%60;
            m_bGetMaxTime = TRUE;
		}

		NET_DVR_PlayBackControl(m_nPlayHandle, NET_DVR_PLAYGETTIME, 0, &nCurrentTime);
		if (nCurrentTime >= m_nFileTime)
		{
			nCurrentTime = m_nFileTime;
		}
		nHour=(nCurrentTime/3600)%24;
		nMinute=(nCurrentTime%3600)/60;
		nSecond=nCurrentTime%60;
		NET_DVR_PlayBackControl(m_nPlayHandle, NET_DVR_PLAYGETFRAME, 0, &nCurrentFrame);
		if (nCurrentFrame>m_nTotalFrames)
		{
			nCurrentFrame = m_nTotalFrames;
		}//%d/%d  ,nCurrentFrame,m_nTotalFrames
		csText.Format("%02d:%02d:%02d/%02d:%02d:%02d  %s",nHour,nMinute,nSecond,m_nTotalHour,m_nTotalMinute,m_nTotalSecond,m_szCurFileName);
		GetDlgItem(IDC_STATIC_REMOTE_FILE_PLAY_STATUS)->SetWindowText(csText);

		NET_DVR_PlayBackControl(m_nPlayHandle, NET_DVR_PLAYGETPOS, 0, &nPos1);
        g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAYGETPOS %d", nPos1);
		if (nPos1 > 100)//200 indicates network exception
		{
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAYGETPOS %d, err = %d", nPos1, NET_DVR_GetLastError());
			StopPlay();
			g_StringLanType(szLan, "由于网络原因或DVR忙,回放异常终止!", " Due to network reasons or DVR is busy, download abnormal termination");
			AfxMessageBox(szLan);
		}
		else
		{
			m_sliderPlayProgress.SetPos(nPos1);
			if (((CButton *)GetDlgItem(IDC_RADIO_FORWARD))->GetCheck()?(nPos1 == 100):(nPos1 == 0))
			{
				g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAYGETPOS %s pos = 100", g_struDeviceInfo[m_iDeviceIndex].pStruChanInfo[m_iChanIndex].chChanName);
				StopPlay();
			}
		}	

		if (NET_DVR_GetPlayBackOsdTime(m_nPlayHandle, &struOsdTime))
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "OSDTime: %04d%02d%02d%02d%02d%02d", \
				struOsdTime.dwYear, struOsdTime.dwMonth,struOsdTime.dwDay, struOsdTime.dwHour, struOsdTime.dwMinute, struOsdTime.dwSecond);
		}
		else
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_GetPlayBackOsdTime");
		}
		break;
	default:
		break;
	}
	CDialog::OnTimer(nIDEvent);
}

/*********************************************************
  Function:	OnBnClickedBtnRemoteSearchList
  Desc:		response function of searching video files corresponding to time or card number
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteFile::OnBnClickedBtnRemoteSearchList()
{
	UpdateData(TRUE);
	char szLan[128] = {0};
    if (m_iDeviceIndex < 0)
    {
        g_StringLanType(szLan,"请先登录设备!", "Please Login Frist!");
        AfxMessageBox(szLan);
        return;
    }
	if (!m_bSearching)
	{
		m_bQuit = TRUE;
        if (1)
        {
            memset(&m_struFileCondV50, 0, sizeof(NET_DVR_FILECOND_V50));
            if (m_iChanIndex > g_struDeviceInfo[m_iDeviceIndex].iDeviceChanNum - 1 && g_struDeviceInfo[m_iDeviceIndex].byMirrorChanNum > 0)
            {
                m_struFileCondV50.struStreamID.dwChannel = g_struDeviceInfo[m_iDeviceIndex].struMirrorChan[m_iChanIndex - MIRROR_CHAN_INDEX].iChannelNO;
            }
            else
            {
                m_struFileCondV50.struStreamID.dwChannel = g_struDeviceInfo[m_iDeviceIndex].pStruChanInfo[m_iChanIndex].iChannelNO;
            }

            m_iFileType = m_comboFileType.GetItemData(m_comboFileType.GetCurSel());
            m_struFileCondV50.dwFileType = m_iFileType;
            m_struFileCondV50.struStartTime.wYear = (WORD)m_ctDateStart.GetYear();
            m_struFileCondV50.struStartTime.byMonth = (char)m_ctDateStart.GetMonth();
            m_struFileCondV50.struStartTime.byDay = (char)m_ctDateStart.GetDay();
            m_struFileCondV50.struStartTime.byHour = (char)m_ctTimeStart.GetHour();
            m_struFileCondV50.struStartTime.byMinute = (char)m_ctTimeStart.GetMinute();
            m_struFileCondV50.struStartTime.bySecond = (char)m_ctTimeStart.GetSecond();
            m_struFileCondV50.struStartTime.byLocalOrUTC = g_bISO8601;
            m_struFileCondV50.struStartTime.cTimeDifferenceH = g_iHourDiffWithUTC;
            m_struFileCondV50.struStartTime.cTimeDifferenceM = g_iMinDiffWithUTC;
            m_struFileCondV50.struStopTime.wYear = (WORD)m_ctDateStop.GetYear();
            m_struFileCondV50.struStopTime.byMonth = (char)m_ctDateStop.GetMonth();
            m_struFileCondV50.struStopTime.byDay = (char)m_ctDateStop.GetDay();
            m_struFileCondV50.struStopTime.byHour = (char)m_ctTimeStop.GetHour();
            m_struFileCondV50.struStopTime.byMinute = (char)m_ctTimeStop.GetMinute();
            m_struFileCondV50.struStopTime.bySecond = (char)m_ctTimeStop.GetSecond();
            m_struFileCondV50.struStopTime.byLocalOrUTC = g_bISO8601;
            m_struFileCondV50.struStopTime.cTimeDifferenceH = g_iHourDiffWithUTC;
            m_struFileCondV50.struStopTime.cTimeDifferenceM = g_iMinDiffWithUTC;
            m_struFileCondV50.byNeedCard = m_bChkCardNum;
            m_struFileCondV50.dwVolumeNum = m_dwVolumeNum;
            m_struFileCondV50.byFindType = m_cmbFindType.GetCurSel();
            m_struFileCondV50.byStreamType = m_comboStreamtype.GetCurSel();
            m_struFileCondV50.byOnlyAudioFile = m_bAudioFile;
            if (4 == m_comboStreamtype.GetCurSel())
            {
                m_struFileCondV50.byStreamType = 0xfe; //双码流搜索(优先返回主码流录像，没有主码流录像时返回子码流录像)，该值不会发给设备，SDK内部处理
            }
            if (m_comboAttr.GetCurSel() == 0)
            {
                m_struFileCondV50.byIsLocked = 0xff;
            }
            else
            {
                m_struFileCondV50.byIsLocked = m_comboAttr.GetCurSel() - 1;
            }
            m_struFileCondV50.szWorkingDeviceGUID[0] = m_byGuid1;
            m_struFileCondV50.szWorkingDeviceGUID[1] = m_byGuid2;
            m_struFileCondV50.szWorkingDeviceGUID[2] = m_byGuid3;
            m_struFileCondV50.szWorkingDeviceGUID[3] = m_byGuid4;
            m_struFileCondV50.szWorkingDeviceGUID[4] = m_byGuid5;
            m_struFileCondV50.szWorkingDeviceGUID[5] = m_byGuid6;
            m_struFileCondV50.szWorkingDeviceGUID[6] = m_byGuid7;
            m_struFileCondV50.szWorkingDeviceGUID[7] = m_byGuid8;
            m_struFileCondV50.szWorkingDeviceGUID[8] = m_byGuid9;
            m_struFileCondV50.szWorkingDeviceGUID[9] = m_byGuid10;
            m_struFileCondV50.szWorkingDeviceGUID[10] = m_byGuid11;
            m_struFileCondV50.szWorkingDeviceGUID[11] = m_byGuid12;
            m_struFileCondV50.szWorkingDeviceGUID[12] = m_byGuid13;
            m_struFileCondV50.szWorkingDeviceGUID[13] = m_byGuid14;
            m_struFileCondV50.szWorkingDeviceGUID[14] = m_byGuid15;
            m_struFileCondV50.szWorkingDeviceGUID[15] = m_byGuid16;
            memcpy(m_struFileCondV50.szWorkingDeviceGUID, m_csGuid, m_csGuid.GetLength());
            m_struFileCondV50.byDrawFrame = m_bDrawFrame;
            m_struFileCondV50.byQuickSearch = m_bQuickSearch;
            m_lDeviceType = g_struDeviceInfo[m_iDeviceIndex].iDeviceType;
            strncpy((char*)m_struFileCondV50.szCardNum, m_csCardNum.GetBuffer(0), CARDNUM_LEN - 1);
            m_struFileCondV50.byNeedCard = m_comboUseCardType.GetCurSel();
            if (m_struFileCondV50.byNeedCard == 5)
            {
                strncpy((char*)m_struFileCondV50.szCardNum, m_csCardNum.GetBuffer(0), CARDNUM_LEN_OUT);
            }
            else
            {
                strncpy((char*)m_struFileCondV50.szCardNum, m_csCardNum.GetBuffer(0), CARDNUM_LEN - 1);
            }
            if (m_comboUseCardType.GetCurSel() > 0)
            {
                m_struFileCondV50.bySpecialFindInfoType = 1;
                m_struFileCondV50.uSpecialFindInfo.struATMFindInfo.byTransactionType = (BYTE)m_comboATMTradeType.GetCurSel();
                m_struFileCondV50.uSpecialFindInfo.struATMFindInfo.dwTransationAmount = m_iMoney;
            }

            m_struFileCondV50.dwTimeout = m_dwFindTimeout;
            m_lFileHandle = NET_DVR_FindFile_V50(m_lLoginID, &m_struFileCondV50);

            if (m_lFileHandle < 0)
            {
                g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_FindFile_V50");
                g_StringLanType(szLan, "获取文件列表失败!", "Fail to get file list");
                AfxMessageBox(szLan);
                return;
            }
        }
        else
        {
            memset(&m_struFileCond, 0, sizeof(NET_DVR_FILECOND_V40));
            if (m_iChanIndex > g_struDeviceInfo[m_iDeviceIndex].iDeviceChanNum - 1 && g_struDeviceInfo[m_iDeviceIndex].byMirrorChanNum > 0)
            {
                m_struFileCond.lChannel = g_struDeviceInfo[m_iDeviceIndex].struMirrorChan[m_iChanIndex - MIRROR_CHAN_INDEX].iChannelNO;
            }
            else
            {
                m_struFileCond.lChannel = g_struDeviceInfo[m_iDeviceIndex].pStruChanInfo[m_iChanIndex].iChannelNO;
            }

            m_iFileType = m_comboFileType.GetItemData(m_comboFileType.GetCurSel());
            // 		if (m_iFileType == 0)
            // 		{
            // 			m_iFileType = 0xFF;
            // 		}
            // 		else
            // 		{
            // 			m_iFileType -= 1;
            // 		}
            m_struFileCond.dwFileType = m_iFileType;
            m_struFileCond.struStartTime.dwYear = (WORD)m_ctDateStart.GetYear();
            m_struFileCond.struStartTime.dwMonth = (WORD)m_ctDateStart.GetMonth();
            m_struFileCond.struStartTime.dwDay = (WORD)m_ctDateStart.GetDay();
            m_struFileCond.struStartTime.dwHour = (char)m_ctTimeStart.GetHour();
            m_struFileCond.struStartTime.dwMinute = (char)m_ctTimeStart.GetMinute();
            m_struFileCond.struStartTime.dwSecond = (char)m_ctTimeStart.GetSecond();
            m_struFileCond.struStopTime.dwYear = (WORD)m_ctDateStop.GetYear();
            m_struFileCond.struStopTime.dwMonth = (WORD)m_ctDateStop.GetMonth();
            m_struFileCond.struStopTime.dwDay = (WORD)m_ctDateStop.GetDay();
            m_struFileCond.struStopTime.dwHour = (char)m_ctTimeStop.GetHour();
            m_struFileCond.struStopTime.dwMinute = (char)m_ctTimeStop.GetMinute();
            m_struFileCond.struStopTime.dwSecond = (char)m_ctTimeStop.GetSecond();
            m_struFileCond.dwUseCardNo = m_bChkCardNum;
            m_struFileCond.dwVolumeNum = m_dwVolumeNum;
            m_struFileCond.byFindType = m_cmbFindType.GetCurSel();
            m_struFileCond.byStreamType = m_comboStreamtype.GetCurSel();
            m_struFileCond.byAudioFile = m_bAudioFile;
            if (4 == m_comboStreamtype.GetCurSel())
            {
                m_struFileCond.byStreamType = 0xfe; //双码流搜索(优先返回主码流录像，没有主码流录像时返回子码流录像)，该值不会发给设备，SDK内部处理
            }
            if (m_comboAttr.GetCurSel() == 0)
            {
                m_struFileCond.dwIsLocked = 0xff;
            }
            else
            {
                m_struFileCond.dwIsLocked = m_comboAttr.GetCurSel() - 1;
            }

            m_struFileCond.byWorkingDeviceGUID[0] = m_byGuid1;
            m_struFileCond.byWorkingDeviceGUID[1] = m_byGuid2;
            m_struFileCond.byWorkingDeviceGUID[2] = m_byGuid3;
            m_struFileCond.byWorkingDeviceGUID[3] = m_byGuid4;
            m_struFileCond.byWorkingDeviceGUID[4] = m_byGuid5;
            m_struFileCond.byWorkingDeviceGUID[5] = m_byGuid6;
            m_struFileCond.byWorkingDeviceGUID[6] = m_byGuid7;
            m_struFileCond.byWorkingDeviceGUID[7] = m_byGuid8;
            m_struFileCond.byWorkingDeviceGUID[8] = m_byGuid9;
            m_struFileCond.byWorkingDeviceGUID[9] = m_byGuid10;
            m_struFileCond.byWorkingDeviceGUID[10] = m_byGuid11;
            m_struFileCond.byWorkingDeviceGUID[11] = m_byGuid12;
            m_struFileCond.byWorkingDeviceGUID[12] = m_byGuid13;
            m_struFileCond.byWorkingDeviceGUID[13] = m_byGuid14;
            m_struFileCond.byWorkingDeviceGUID[14] = m_byGuid15;
            m_struFileCond.byWorkingDeviceGUID[15] = m_byGuid16;

            memcpy(m_struFileCond.byWorkingDeviceGUID, m_csGuid, m_csGuid.GetLength());

            //		strcpy(m_struFileCond.szWorkingDeviceGUID, m_strWDGUID.GetBuffer(0));

            m_struFileCond.byDrawFrame = m_bDrawFrame;
            m_struFileCond.byQuickSearch = m_bQuickSearch;

            m_lDeviceType = g_struDeviceInfo[m_iDeviceIndex].iDeviceType;
            //		if (m_bChkCardNum) //ATM DVR
            // 		{
            strncpy((char*)m_struFileCond.sCardNumber, m_csCardNum.GetBuffer(0), CARDNUM_LEN - 1);
            m_struFileCond.dwUseCardNo = m_comboUseCardType.GetCurSel();
            if (m_struFileCond.dwUseCardNo == 5)
            {
                strncpy((char*)m_struFileCond.sCardNumber, m_csCardNum.GetBuffer(0), CARDNUM_LEN_OUT);
            }
            else
            {
                strncpy((char*)m_struFileCond.sCardNumber, m_csCardNum.GetBuffer(0), CARDNUM_LEN - 1);
            }
            if (m_comboUseCardType.GetCurSel() > 0)
            {
                m_struFileCond.bySpecialFindInfoType = 1;
                m_struFileCond.uSpecialFindInfo.struATMFindInfo.byTransactionType = (BYTE)m_comboATMTradeType.GetCurSel();
                m_struFileCond.uSpecialFindInfo.struATMFindInfo.dwTransationAmount = m_iMoney;
            }
            //		}

            //m_struFileCond.byStreamType = 254;
            m_lFileHandle = NET_DVR_FindFile_V40(m_lLoginID, &m_struFileCond);

            if (m_lFileHandle < 0)
            {
                g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_FindFile_V40");
                g_StringLanType(szLan, "获取文件列表失败!", "Fail to get file list");
                AfxMessageBox(szLan);
                return;
            }
        }
		m_listRemoteFile.DeleteAllItems();
		DWORD dwThreadId;
		if (m_hFileThread == NULL)
		{
			m_hFileThread = CreateThread(NULL,0,LPTHREAD_START_ROUTINE(GetFileThread),this,0,&dwThreadId);		
		}
		if (m_hFileThread == NULL)
		{
			g_StringLanType(szLan, "打开查找线程失败!", "Fail to open finding thread!");
			AfxMessageBox(szLan);
			return;
		}
		g_StringLanType(szLan, "停止查找", "Stop Searching");
		GetDlgItem(IDC_BTN_REMOTE_SEARCH_LIST)->SetWindowText(szLan);
		m_bSearching = TRUE;
		GetDlgItem(IDC_STATIC_REMOTE_SEARCH_STATE)->ShowWindow(SW_SHOW);
	}
	else
	{
		if (m_hFileThread)
		{
			m_bQuit = FALSE;
			//TerminateThread(m_hFileThread, 0);
		}
		CloseHandle(m_hFileThread);
		m_hFileThread = NULL;
		NET_DVR_FindClose(m_lFileHandle);
		g_StringLanType(szLan, "查找", "Search");
		GetDlgItem(IDC_BTN_REMOTE_SEARCH_LIST)->SetWindowText(szLan);
		m_bSearching = FALSE;
		GetDlgItem(IDC_STATIC_REMOTE_SEARCH_STATE)->ShowWindow(SW_HIDE);
		m_iFileNum = 0;
	}
}

/*********************************************************
  Function:	OnBnClickedBtnFileDownload
  Desc:		download corresponding files
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteFile::OnBnClickedBtnFileDownload()
{
    CString csFileName;
    CString csStartTime, csStopTime;
    CString csTemp,csDir;
    char nFileName[256];
    POSITION  posItem;
    int iFileSelPos = 0;
    
    char szLan[128] = {0};
    UpdateData(TRUE);
    g_CreateLocalDir();
    if (!m_bDownloading)
    {
        posItem = m_listRemoteFile.GetFirstSelectedItemPosition();
        if (posItem == NULL)
        {
            g_StringLanType(szLan, "请选择要下载的文件", "Please select the downloaded file");
            AfxMessageBox(szLan);
            return;
        }
        m_iSelListItem = m_listRemoteFile.GetNextSelectedItem(posItem);
        csFileName.Format("%s", m_listRemoteFile.GetItemText(m_iSelListItem, 0));
        if (csFileName.IsEmpty())
            return;
        csStartTime.Format("%s", m_listRemoteFile.GetItemText(m_iSelListItem, 2));
        csStopTime.Format("%s", m_listRemoteFile.GetItemText(m_iSelListItem, 3));
        sprintf(m_szDownLoadFileName,"%s",csFileName);
        
        //if the dvr ip is in ipv6 version, change the ':' to '-' ,
        //because ':' is not allowed in Windows OS's file name 
        char sIPV6Addr[130] = {0};
        for (int i = 0; i < sizeof(g_struDeviceInfo[m_iDeviceIndex].chDeviceIP); i ++)
        {
            if(g_struDeviceInfo[m_iDeviceIndex].chDeviceIP[i] == '\0')
            {
                break;
            }
            else if(g_struDeviceInfo[m_iDeviceIndex].chDeviceIP[i] == ':')
            {
                sIPV6Addr[i] = '-';
            }
            else
            {
                sIPV6Addr[i] = g_struDeviceInfo[m_iDeviceIndex].chDeviceIP[i];
            }
        }
        csDir.Format("%s\\",g_struLocalParam.chDownLoadPath);
        sprintf(nFileName,"%s%s_%02d_D%s_%s.mp4",csDir,sIPV6Addr, m_iChanShowNum,csStartTime,csStopTime);
        
        //m_lDownloadHandle = NET_DVR_GetFileByName(m_lLoginID,m_szDownLoadFileName,nFileName);
        NET_DVR_DOWNLOAD_BY_NAME_COND struDownloadCond = { 0 };
        struDownloadCond.pFileName = nFileName;
        struDownloadCond.pSavedFileName = m_szDownLoadFileName;

        CString szIp;
        CString szPort;
        szIp.Format("%s", m_listRemoteFile.GetItemText(m_iSelListItem, 8));
        //ipv4
        if (!g_ValidIPv6((BYTE*)(LPCSTR)szIp))
        {
            sprintf(struDownloadCond.struAddr.struIP.sIpV4, "%s", szIp);
        }
        else
        {
            sprintf((char *)struDownloadCond.struAddr.struIP.byIPv6, "%s", szIp);
        }
        szPort = m_listRemoteFile.GetItemText(m_iSelListItem, 9);
        struDownloadCond.struAddr.wPort = _ttoi(szPort);

        m_lDownloadHandle = NET_DVR_GetFileByName_V50(m_lLoginID, &struDownloadCond);
        if (m_lDownloadHandle >= 0)
        {
            NET_DVR_LINK_ADDR struParam = { 0 };
            if (NET_DVR_GetLinkAddr(m_lDownloadHandle, ENUM_LINK_PLAYBACK, &struParam))
            {
                g_pMainDlg->AddLog(m_lDownloadHandle, OPERATION_SUCC_T, "NET_DVR_GetLinkAddr, dev addr[%s:%d], local addr[%s:%d]", \
                    &(struParam.uDevIP), struParam.wDevPort[0], &(struParam.uLocalIP), struParam.wLocalPort[0]);
            }
            else
            {
                g_pMainDlg->AddLog(m_lDownloadHandle, OPERATION_FAIL_T, "NET_DVR_GetLinkAddr");
            }

            //设置转码类型
            int iTransType = m_comboTransType.GetCurSel() + 1;
            if (iTransType == 4)
            {
                iTransType = 5;
            }
            if (NET_DVR_PlayBackControl_V40(m_lDownloadHandle, NET_DVR_SET_TRANS_TYPE, &iTransType, NULL))
            {
                g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_SET_TRANS_TYPE TransType[%d]", iTransType);
            }
            else
            {
                g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_SET_TRANS_TYPE TransType[%d]", iTransType);
            }
            
            NET_DVR_PlayBackControl(m_lDownloadHandle, NET_DVR_PLAYSTART, 0, NULL);
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "Download file name: %s", nFileName);
            g_StringLanType(szLan, "停止下载", "Stop Download");
            GetDlgItem(IDC_BTN_REMOTE_FILE_DOWNLOAD)->SetWindowText(szLan);
            m_progressDownload.SetPos(0);
            m_progressDownload.ShowWindow(SW_SHOW);
            m_bDownloading=TRUE;
            g_StringLanType(szLan, "正在下载文件", "Downloading");
            m_csDownloadStat.Format("%s%s", szLan, m_szDownLoadFileName);
            GetDlgItem(IDC_STATIC_REMOTE_DOWNLOAD_STATE)->ShowWindow(SW_SHOW);
            SetTimer(DOWNSTATE_TIMER,200,NULL);
        }
        else
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_GetFileByName");
            g_StringLanType(szLan, "下载文件失败", "Download failed");
            AfxMessageBox(szLan);		
            return;
        }
    }
    else 
    {
        NET_DVR_StopGetFile(m_lDownloadHandle);
        m_bDownloading=FALSE;
        g_StringLanType(szLan, "下载", "Download");
        GetDlgItem(IDC_BTN_REMOTE_FILE_DOWNLOAD)->SetWindowText(szLan);
        KillTimer(DOWNSTATE_TIMER);
        Invalidate(TRUE);
        m_progressDownload.SetPos(0);
        m_progressDownload.ShowWindow(SW_HIDE);
        g_StringLanType(szLan, "停止下载文件", "Stop downloading");
        m_csDownloadStat.Format("%s%s", szLan, m_szDownLoadFileName);
        GetDlgItem(IDC_STATIC_REMOTE_DOWNLOAD_STATE)->ShowWindow(SW_HIDE);
    }
    UpdateData(FALSE);
}


/*********************************************************
  Function:	OnBnClickedBtnRemoteFilePlay
  Desc:		response function of starting playback
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteFile::OnBnClickedBtnRemoteFilePlay()
{
	if (m_nPlayHandle == -1)
	{
		PlayBack();
	}
	else
	{
		if (m_bPause)
		{
			if (NET_DVR_PlayBackControl(m_nPlayHandle, NET_DVR_PLAYRESTART, 0, NULL))
			{
				m_bPause=FALSE;
				SetPlayState();
			}
		}
		else
		{
			if (m_bSetFrame || m_bChangeSpeed)
			{
				if (NET_DVR_PlayBackControl(m_nPlayHandle, NET_DVR_PLAYNORMAL, 0, NULL))
				{
					g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAYNORMAL");
					m_bSetFrame=FALSE;
					m_bChangeSpeed = FALSE;
					SetPlayState();
				}
			}
			else
			{
				if (NET_DVR_PlayBackControl(m_nPlayHandle, NET_DVR_PLAYPAUSE, 0, NULL))
				{
                    g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAYPAUSE");
					m_bPause=TRUE;
					SetPauseState();
				}
                else
                {
                    g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAYPAUSE");
                }
				if (!NET_DVR_RefreshPlay(m_nPlayHandle))
				{
					g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_RefreshPlay");
				}
			}

		}
	}
}

/*********************************************************
  Function:	OnBnClickedBtnRemoteFileStop
  Desc:		response function of stop play
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteFile::OnBnClickedBtnRemoteFileStop()
{
	StopPlay();
}

/*********************************************************
  Function:	OnBnClickedBtnRemoteFileSave
  Desc:		response function of download remote video files to local
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteFile::OnBnClickedBtnRemoteFileSave()
{
	static char szFilter[]="All File(*.mp4)|*.*||";
	char m_sFile[256] = "c:\\Savefile.mp4";
	CString csFile;
	CFileDialog dlg(FALSE,"*.mp4","Savefile.mp4",OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY,
		szFilter);
	if (dlg.DoModal()==IDOK)
	{
		csFile = dlg.GetPathName();
		UpdateData(FALSE);
		sprintf(m_sFile, "%s", csFile);
	}

	if (NET_DVR_PlayBackSaveData(m_nPlayHandle, m_sFile))
	{
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PlayBackSaveData[%s]", m_sFile);
		m_bSaveFile = TRUE;
	}
	else
	{
		char szLan[128] = {0};
		g_StringLanType(szLan, "文件保存失败", "Save the file failed");
		AfxMessageBox(szLan);
	}
}

/*********************************************************
  Function:	OnBnClickedBtnRemoteFileStopSave
  Desc:		stop saving remote files
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteFile::OnBnClickedBtnRemoteFileStopSave()
{

	if (m_bSaveFile)
	{
		NET_DVR_StopPlayBackSave(m_nPlayHandle);
		m_bSaveFile = FALSE;
		char szLan[128] = {0};
		g_StringLanType(szLan, "停止保存成功", "Save the file successfully");
		AfxMessageBox(szLan);
	}
}

/*********************************************************
  Function:	OnBnClickedBtnRemoteFileGotoBeginning
  Desc:		play back to beginning
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteFile::OnBnClickedBtnRemoteFileGotoBeginning()
{
	if (NET_DVR_PlayBackControl(m_nPlayHandle,NET_DVR_PLAYSETPOS, 0, NULL))
	{
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAYSETPOS goto beginning");
	}
	else
	{
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAYSETPOS goto beginning");
	}
}

/*********************************************************
  Function:	OnBnClickedBtnRemoteFileFast
  Desc:		fast play
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteFile::OnBnClickedBtnRemoteFileFast()
{
 	if (NET_DVR_PlayBackControl(m_nPlayHandle, NET_DVR_PLAYFAST, 0, NULL))
 	{	
 		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAYFAST");
 		m_bChangeSpeed = TRUE;
 	}
 	else
 	{
 		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAYFAST");
 	}
	//PlayM4_Fast(g_pDlgRemoteFile->m_lPort);
}

/*********************************************************
  Function:	OnBnClickedBtnRemoteFileSlow
  Desc:		slow play
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteFile::OnBnClickedBtnRemoteFileSlow()
{
	if (NET_DVR_PlayBackControl(m_nPlayHandle, NET_DVR_PLAYSLOW, 0, NULL))
	{	
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAYSLOW");
		m_bChangeSpeed = TRUE;
	}
	else
	{
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAYSLOW");
	}
}

/*********************************************************
  Function:	OnBnClickedBtnRemoteFileSnatchPic
  Desc:		capture current play screen
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteFile::OnBnClickedBtnRemoteFileSnatchPic()
{
	char cFilename[256] = {0};

    sprintf(cFilename, "%s\\%d_%d.bmp", g_struLocalParam.chPictureSavePath, m_iChanShowNum, GetTickCount());

    if (m_bPause)
    {
        if (NET_DVR_CapturePlaybackPictureBlock(m_nPlayHandle, cFilename, 0))
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "%s NET_DVR_CapturePlaybackPictureBlock", cFilename);
            return;
        }
        else
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_CapturePlaybackPictureBlock");
        }
    }
    else
    {
        if (NET_DVR_PlayBackCaptureFile(m_nPlayHandle, cFilename))
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "%s NET_DVR_PlayBackCaptureFile", cFilename);
            return;
        }
        else
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PlayBackCaptureFile");
        }
    }
	
	
}

/*********************************************************
  Function:	OnBnClickedBtnRemoteFileStep
  Desc:		play by single frame
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteFile::OnBnClickedBtnRemoteFileStep()
{
	if (NET_DVR_PlayBackControl(m_nPlayHandle, NET_DVR_PLAYFRAME, 0, NULL))
	{
		CButton *pButton;	
		pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_PLAY);
		pButton->SetIcon(m_hPlayEnableIcon);
		pButton->EnableWindow(TRUE);
		m_bSetFrame = TRUE;
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAYFRAME");
	}
}

/*********************************************************
  Function:	OnBnClickedBtnRemoteFileSound
  Desc:		turn on or off audio while playback
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteFile::OnBnClickedBtnRemoteFileSound()
{
	CButton *pButton;

	if (m_bSound)
	{
		if (NET_DVR_PlayBackControl(m_nPlayHandle, NET_DVR_PLAYSTOPAUDIO, 0, NULL))
		{
			m_bSound=FALSE;
			pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_SOUND);
			pButton->SetIcon(m_hSoundStopIcon);
			GetDlgItem(IDC_SLIDER_REMOTE_FILE_VOLUME)->EnableWindow(FALSE);
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAYSTOPAUDIO %s", g_struDeviceInfo[m_iDeviceIndex].pStruChanInfo[m_iChanIndex].chChanName); 
		}
		else
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAYSTOPAUDIO %s", g_struDeviceInfo[m_iDeviceIndex].pStruChanInfo[m_iChanIndex].chChanName); 
		}
	}
	else
	{		
		if (NET_DVR_PlayBackControl(m_nPlayHandle, NET_DVR_PLAYSTARTAUDIO, 0, NULL))
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAYSTOPAUDIO %s", g_struDeviceInfo[m_iDeviceIndex].pStruChanInfo[m_iChanIndex].chChanName); 
			pButton = (CButton *)GetDlgItem(IDC_BTN_REMOTE_FILE_SOUND);
			GetDlgItem(IDC_SLIDER_REMOTE_FILE_VOLUME)->EnableWindow(TRUE);
			pButton->SetIcon(m_hSoundStartIcon);
			m_bSound=TRUE;
		}
		else
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAYSTOPAUDIO %s", g_struDeviceInfo[m_iDeviceIndex].pStruChanInfo[m_iChanIndex].chChanName); 
		}
	}
}

/*********************************************************
  Function:	OnChkCardNum
  Desc:		the state change
  Input:	none
  Output:	none
  Return:	none
**********************************************************/
void CDlgPlayRemoteFile::OnChkCardNum() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	GetDlgItem(IDC_EDIT_CARD_NUM)->EnableWindow(m_bChkCardNum);
    CheckInitParam();
}

void CDlgPlayRemoteFile::OnClickListRemoteFile(NMHDR* pNMHDR, LRESULT* pResult) 
{
// 	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
 	POSITION  iPos = m_listRemoteFile.GetFirstSelectedItemPosition();
	char szLan[128] = {0};
 	if (iPos == NULL)
 	{
 		return;
 	}
 	m_iItemSel = m_listRemoteFile.GetNextSelectedItem(iPos);
	memset(m_szFileName, 0, 200);
	sprintf(m_szFileName, "%s", m_listRemoteFile.GetItemText(m_iItemSel, 0));
	switch(m_listRemoteFile.GetItemData(m_iItemSel)%10)//single-digit indicates loc state
	{
	case 0:
		g_StringLanType(szLan, "锁定", "Lock");
		GetDlgItem(IDC_BTN_LOCK)->SetWindowText(szLan);
		break;
	case 1:
		g_StringLanType(szLan, "未锁", "Unlock");
		GetDlgItem(IDC_BTN_LOCK)->SetWindowText(szLan);
		break;
	default:
		GetDlgItem(IDC_BTN_LOCK)->SetWindowText("Unlock");
	    break;
	}
	m_iOffset = 0;
	CString csTemp;
	m_dwCurFileLen = m_listRemoteFile.GetItemData(m_iItemSel)/10;
	m_csFileLen.Format("%d", m_dwCurFileLen);
	m_csName.Format("%s", m_szCurFileName);
	GetDlgItem(IDC_BTN_REMOTE_FILE_PLAY)->EnableWindow(TRUE);
	UpdateData(FALSE);
	*pResult = 0;
}

/*********************************************************
  Function:	OnBtnLock
  Desc:		switch the lock status
  Input:	none
  Output:	none
  Return:	none
**********************************************************/
void CDlgPlayRemoteFile::OnBtnLock() 
{
	BOOL bRet = FALSE;
	char szLan[128] = {0};
	if (m_iItemSel < 0)
	{
		g_StringLanType(szLan, "选择一个文件", "Select a file");
		AfxMessageBox(szLan);
		return;
	}

    NET_DVR_LOCK_FILE_BY_NAME_PARA  strLockData = { 0 };
    memcpy(strLockData.byFileName, m_szFileName, 32);
	switch(m_listRemoteFile.GetItemData(m_iItemSel)%10)
	{
	case 0:
        bRet = NET_DVR_LockFileByNameV40(m_lLoginID, TRUE, &strLockData);
		//bRet = NET_DVR_LockFileByName(m_lLoginID, m_szFileName);
		if (bRet)
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_LockFileByNameV40");
			g_StringLanType(szLan, "解锁", "Unlock");
			GetDlgItem(IDC_BTN_LOCK)->SetWindowText(szLan);
			g_StringLanType(szLan, "锁定", "Lock");
			m_listRemoteFile.SetItemText(m_iItemSel, 4, szLan);
			m_listRemoteFile.SetItemData(m_iItemSel, 1);
		}
		else
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_LockFileByNameV40");
			g_StringLanType(szLan, "锁定失败", "Fail to lock");
			AfxMessageBox(szLan);	
		}
		break;
	case 1:
        bRet = NET_DVR_LockFileByNameV40(m_lLoginID, FALSE, &strLockData);
		if (bRet)
		{
			g_StringLanType(szLan, "锁定", "Lock");
			GetDlgItem(IDC_BTN_LOCK)->SetWindowText(szLan);
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_LockFileByNameV40");
			g_StringLanType(szLan, "未锁", "Unlock");
			m_listRemoteFile.SetItemText(m_iItemSel, 4, "未锁");
			m_listRemoteFile.SetItemData(m_iItemSel, 0);
		}
		else
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_LockFileByNameV40");
			g_StringLanType(szLan, "锁定失败", "Fail to lock");
			AfxMessageBox(szLan);
		}
		break;
	case 0xff:
		g_StringLanType(szLan, "不能锁定", "Unlock");
		GetDlgItem(IDC_BTN_LOCK)->SetWindowText(szLan);
	    break;
	default:
	    break;
	}	
}

void CDlgPlayRemoteFile::OnBtnTimeLock() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	NET_DVR_TIME struStartTime, struStopTime;
	struStartTime.dwYear = (WORD)m_ctDateStart.GetYear();
	struStartTime.dwMonth = (WORD)m_ctDateStart.GetMonth();
	struStartTime.dwDay = (WORD)m_ctDateStart.GetDay();
	struStartTime.dwHour = (char)m_ctTimeStart.GetHour();
	struStartTime.dwMinute = (char)m_ctTimeStart.GetMinute();
	struStartTime.dwSecond = (char)m_ctTimeStart.GetSecond();
	struStopTime.dwYear = (WORD)m_ctDateStop.GetYear();
	struStopTime.dwMonth = (WORD)m_ctDateStop.GetMonth();
	struStopTime.dwDay = (WORD)m_ctDateStop.GetDay();
	struStopTime.dwHour = (char)m_ctTimeStop.GetHour();
	struStopTime.dwMinute = (char)m_ctTimeStop.GetMinute();
	struStopTime.dwSecond = (char)m_ctTimeStop.GetSecond();
// 	if (NET_DVR_LockFileByTime(m_lLoginID, m_iChanShowNum, &struStartTime, &struStopTime))
// 	{
// 		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_LockFileByTime");
// 	}
// 	else
// 	{
// 		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_LockFileByTime");
// 	}
}

void CDlgPlayRemoteFile::OnBtnTimeUnlock() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	NET_DVR_TIME struStartTime, struStopTime;
	struStartTime.dwYear = (WORD)m_ctDateStart.GetYear();
	struStartTime.dwMonth = (WORD)m_ctDateStart.GetMonth();
	struStartTime.dwDay = (WORD)m_ctDateStart.GetDay();
	struStartTime.dwHour = (char)m_ctTimeStart.GetHour();
	struStartTime.dwMinute = (char)m_ctTimeStart.GetMinute();
	struStartTime.dwSecond = (char)m_ctTimeStart.GetSecond();
	struStopTime.dwYear = (WORD)m_ctDateStop.GetYear();
	struStopTime.dwMonth = (WORD)m_ctDateStop.GetMonth();
	struStopTime.dwDay = (WORD)m_ctDateStop.GetDay();
	struStopTime.dwHour = (char)m_ctTimeStop.GetHour();
	struStopTime.dwMinute = (char)m_ctTimeStop.GetMinute();
	struStopTime.dwSecond = (char)m_ctTimeStop.GetSecond();
// 	if (NET_DVR_UnlockFileByTime(m_lLoginID, m_iChanShowNum, &struStartTime, &struStopTime))
// 	{
// 		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_UnlockFileByTime");
// 	}
// 	else
// 	{
// 		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_UnlockFileByTime");
// 	}	
}
HBRUSH CDlgPlayRemoteFile::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	if (m_nPlayHandle < 0 || !m_bPause)
	{
		return hbr;
	}
	if (!NET_DVR_RefreshPlay(m_nPlayHandle))
	{
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_RefreshPlay");
	}
	// TODO: Return a different brush if the default is not desired
	return hbr;
}

/*********************************************************
  Function:	OnReleasedcaptureSliderRemoteFileProgress
  Desc:		responding to mouse up message of file progress
  Input:	pNMHDR, point to NMHDR; 
			pResult, point to LRESULT;
  Output:	none
  Return:	none
**********************************************************/
void CDlgPlayRemoteFile::OnReleasedcaptureSliderRemoteFileProgress(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	char chPos = 0;

	chPos = (char)(m_sliderPlayProgress.GetPos());
	m_iOffset = (m_dwCurFileLen/100*chPos);
	m_sliderPlayProgress.SetPos(chPos);
	if (m_nPlayHandle >= 0) 
	{
		if ((chPos >=0) && (chPos <=100))
		{
			if (chPos == 100)
			{			
				StopPlay();
				chPos = 99;
			}
			else
			{
				if(NET_DVR_PlayBackControl(m_nPlayHandle, NET_DVR_PLAYSETPOS, chPos, NULL))
				{
					g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAYSETPOS[%d] %s", chPos, g_struDeviceInfo[m_iDeviceIndex].pStruChanInfo[m_iChanIndex].chChanName); 
				}
				else
				{
					g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAYSETPOS[%d] %s", chPos, g_struDeviceInfo[m_iDeviceIndex].pStruChanInfo[m_iChanIndex].chChanName); 
				}
			}
		}
	}

	UpdateData(FALSE);
	*pResult = 0;
}

/*********************************************************
  Function:	OnReleasedcaptureSliderRemoteFileVolume
  Desc:		responding to mouse up message of volume progress
  Input:	pNMHDR, point to NMHDR; 
			pResult, point to LRESULT;
  Output:	none
  Return:	none
**********************************************************/
void CDlgPlayRemoteFile::OnReleasedcaptureSliderRemoteFileVolume(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UpdateData(TRUE);
	WORD temp = (0xffff)/100;
	WORD dwVolume = 0;
	char chPos = 0;
	chPos =(char)m_sliderVolume.GetPos();
	dwVolume =(WORD) (chPos*temp);
	m_sliderVolume.SetPos(chPos);
	if (m_nPlayHandle >= 0)
	{
		if (NET_DVR_PlayBackControl(m_nPlayHandle, NET_DVR_PLAYAUDIOVOLUME, dwVolume, NULL))
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAYAUDIOVOLUME %s", g_struDeviceInfo[m_iDeviceIndex].pStruChanInfo[m_iChanIndex].chChanName); 
		}
		else
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAYAUDIOVOLUME %s", g_struDeviceInfo[m_iDeviceIndex].pStruChanInfo[m_iChanIndex].chChanName); 
		}		
	}
	*pResult = 0;
}

void CDlgPlayRemoteFile::OnBtnSetTime() 
{
    UpdateData(TRUE);
    NET_DVR_TIME_V50 struTime = {0};
    struTime.wYear = (WORD)m_ctDateSet.GetYear();
    struTime.byMonth = (BYTE)m_ctDateSet.GetMonth();
    struTime.byDay = (BYTE)m_ctDateSet.GetDay();
    struTime.byHour = (BYTE)m_ctTimeSet.GetHour();
    struTime.byMinute = (BYTE)m_ctTimeSet.GetMinute();
    struTime.bySecond = (BYTE)m_ctTimeSet.GetSecond();
    struTime.byISO8601 = g_bISO8601;
    struTime.cTimeDifferenceH = g_iHourDiffWithUTC;
    struTime.cTimeDifferenceM = g_iMinDiffWithUTC;

    if (NET_DVR_PlayBackControl_V40(m_nPlayHandle, NET_DVR_PLAYSETTIME_V50, &struTime, sizeof(struTime), NULL, 0))
    {
        g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAYSETTIME_V50");
    }
    else
    {
        g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAYSETTIME_V50");
        AfxMessageBox("PlayBackContrlo NET_DVR_PLAYSETTIME_V50 fail!");
    }
	
}

DWORD  GetBackupThread(LPVOID pParam)
{
    CDlgPlayRemoteFile *pDlgRemotPlay = (CDlgPlayRemoteFile*)pParam;
    pDlgRemotPlay->GetDlgItem(IDC_STATIC_BACK_STATE)->ShowWindow(SW_SHOW);
    DWORD dwState = 0;
    char szLan[256] = {0};
    while (1)
    {
        NET_DVR_GetBackupProgress(pDlgRemotPlay->m_lBackupHandle, &dwState);
		if (dwState == 100)
        {
            g_StringLanType(szLan, "备份完成", "Succ to backup");
            pDlgRemotPlay->GetDlgItem(IDC_STATIC_BACK_STATE)->SetWindowText(szLan);
            NET_DVR_StopBackup(pDlgRemotPlay->m_lBackupHandle);
            g_StringLanType(szLan, "文件备份", "Backup");
            pDlgRemotPlay->GetDlgItem(IDC_BTN_BACKUP)->SetWindowText(szLan);
            pDlgRemotPlay->m_bBackuping = FALSE;
			Sleep(1000);
            pDlgRemotPlay->GetDlgItem(IDC_STATIC_BACK_STATE)->ShowWindow(SW_HIDE);
            break;
        }
        else if (dwState == 400)
        {
            g_StringLanType(szLan, "备份异常", "backup exception");
            pDlgRemotPlay->GetDlgItem(IDC_STATIC_BACK_STATE)->SetWindowText(szLan);
            NET_DVR_StopBackup(pDlgRemotPlay->m_lBackupHandle);
            g_StringLanType(szLan, "文件备份", "Backup");
            pDlgRemotPlay->GetDlgItem(IDC_BTN_BACKUP)->SetWindowText(szLan);
            pDlgRemotPlay->m_bBackuping = FALSE;
			Sleep(1000);
            pDlgRemotPlay->GetDlgItem(IDC_STATIC_BACK_STATE)->ShowWindow(SW_HIDE);
            break;
        }
        else if (dwState == 500)
        {
            g_StringLanType(szLan, "备份失败", "Failed to backup");
            pDlgRemotPlay->GetDlgItem(IDC_STATIC_BACK_STATE)->SetWindowText(szLan);
            NET_DVR_StopBackup(pDlgRemotPlay->m_lBackupHandle);
            g_StringLanType(szLan, "文件备份", "Backup");
            pDlgRemotPlay->GetDlgItem(IDC_BTN_BACKUP)->SetWindowText(szLan);
            pDlgRemotPlay->m_bBackuping = FALSE;
			Sleep(1000);
            pDlgRemotPlay->GetDlgItem(IDC_STATIC_BACK_STATE)->ShowWindow(SW_HIDE);
            break;
        }
		//进度值
        else if (dwState >= 0 && dwState < 100)
        {
            char szLanCn[128] = {0};
            char szLanEn[128] = {0};
            sprintf(szLanCn, "正在备份[%d]", dwState);
            sprintf(szLanEn, "backuping[%d]", dwState);
            g_StringLanType(szLan, szLanCn, szLanCn);
            pDlgRemotPlay->GetDlgItem(IDC_STATIC_BACK_STATE)->SetWindowText(szLan);
        }
		else if(dwState ==  BACKUP_CHANGE_DEVICE)
		{
			g_StringLanType(szLan, "备份设备已满, 请更换设备继续备份", "Device of backup is full, change another device and continue backuping");
			pDlgRemotPlay->GetDlgItem(IDC_STATIC_BACK_STATE)->SetWindowText(szLan);
			NET_DVR_StopBackup(pDlgRemotPlay->m_lBackupHandle);
			g_StringLanType(szLan, "备份", "Backup");
			pDlgRemotPlay->GetDlgItem(IDC_BTN_BACKUP)->SetWindowText(szLan);
			pDlgRemotPlay->m_bBackuping = FALSE;
			Sleep(1000);
			pDlgRemotPlay->GetDlgItem(IDC_STATIC_BACK_STATE)->ShowWindow(SW_HIDE);
			break;
		}
		//中间过程
		else if(dwState == BACKUP_SEARCH_DEVICE)
		{
            g_StringLanType(szLan, "正在搜索备份设备", "searching backup device");
            pDlgRemotPlay->GetDlgItem(IDC_STATIC_BACK_STATE)->SetWindowText(szLan);
		}
		else if(dwState == BACKUP_SEARCH_FILE)
		{
            g_StringLanType(szLan, "正在搜索录像文件", "searching record files");
            pDlgRemotPlay->GetDlgItem(IDC_STATIC_BACK_STATE)->SetWindowText(szLan);
		}
		//错误值
		else if(dwState >= BACKUP_TIME_SEG_NO_FILE)
		{
            char szLanCn[128] = {0};
            char szLanEn[128] = {0};
            sprintf(szLanCn, "备份失败, 错误值[%d]", dwState);
            sprintf(szLanEn, "Backup failed, ErrorCode[%d]", dwState);
			g_StringLanType(szLan, szLanCn, szLanEn);
            pDlgRemotPlay->GetDlgItem(IDC_STATIC_BACK_STATE)->SetWindowText(szLan);

            NET_DVR_StopBackup(pDlgRemotPlay->m_lBackupHandle);
            g_StringLanType(szLan, "文件备份", "Backup");
            pDlgRemotPlay->GetDlgItem(IDC_BTN_BACKUP)->SetWindowText(szLan);
            pDlgRemotPlay->m_bBackuping = FALSE;
			Sleep(2000);
            pDlgRemotPlay->GetDlgItem(IDC_STATIC_BACK_STATE)->ShowWindow(SW_HIDE);
            break;
		}		

        Sleep(1000);
    }
    CloseHandle(g_pDlgRemoteFile->m_hBackupThread);
    g_pDlgRemoteFile->m_hBackupThread = NULL;

    return 0;
}

void CDlgPlayRemoteFile::OnBtnBackup() 
{
    UpdateData(TRUE);
    char szLan[256] = {0};
    if (!m_bBackuping)
    {
        CString csDiskDesc;
        if (m_comboDiskList.GetCurSel() != CB_ERR)
        {
            m_comboDiskList.GetLBText(m_comboDiskList.GetCurSel(), csDiskDesc);
        }
        else
        {
            g_StringLanType(szLan, "请选择磁盘备份列表", "Please select backup disk");
            AfxMessageBox(szLan);
        }
        strncpy((char*)m_struBackupParam.byDiskDes, (char*)csDiskDesc.GetBuffer(0), sizeof(m_struBackupParam.byDiskDes));
        
        m_struBackupParam.byWithPlayer = m_bBackupPlayer;
        m_struBackupParam.byContinue = m_bContinue;
        
        int iItemCount = 0;
        int iIndex = 0;
        POSITION pos = m_listRemoteFile.GetFirstSelectedItemPosition();
        CString csFileSize;
        while (pos != NULL && iItemCount < (sizeof(m_struBackupParam.struFileList)/sizeof(m_struBackupParam.struFileList[0])))
        {
            iIndex = m_listRemoteFile.GetNextSelectedItem(pos);
            sprintf(m_struBackupParam.struFileList[iItemCount].sFileName, "%s", m_listRemoteFile.GetItemText(iIndex, 0));
            csFileSize = m_listRemoteFile.GetItemText(iIndex, 1);
            m_struBackupParam.struFileList[iItemCount].dwFileSize = ProcessFileSize(csFileSize);
            iItemCount++;
        }
        m_struBackupParam.dwFileNum = iItemCount;
        
        m_lBackupHandle = NET_DVR_BackupByName(m_lLoginID, &m_struBackupParam);
        if (m_lBackupHandle == -1)
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_BackupByName");
            return;
        }
        else
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_BackupByName");
        }
        
        DWORD dwThreadId = 0;
        if (m_hBackupThread == NULL)
        {
            m_hBackupThread = CreateThread(NULL,0,LPTHREAD_START_ROUTINE(GetBackupThread),this,0,&dwThreadId);		
        }
        if (m_hBackupThread  == NULL)
        {
            char szLan[256] = {0};
            g_StringLanType(szLan, "打开备份线程失败!", "Fail to open backup thread!");
            AfxMessageBox(szLan);
            return;
        }
        g_StringLanType(szLan, "停止备份", "Stop Bakcup");
        GetDlgItem(IDC_BTN_BACKUP)->SetWindowText(szLan);
        m_bBackuping = TRUE;
        GetDlgItem(IDC_STATIC_BACK_STATE)->ShowWindow(SW_SHOW);
    }
    else
    {
        if (m_hBackupThread)
        {
            TerminateThread(m_hBackupThread, 0);
        }

        CloseHandle(m_hBackupThread);
        m_hBackupThread = NULL;
        NET_DVR_StopBackup(m_lBackupHandle);
        g_StringLanType(szLan, "文件备份", "Backup");
        GetDlgItem(IDC_BTN_BACKUP)->SetWindowText(szLan);
        m_bBackuping = FALSE;
        GetDlgItem(IDC_STATIC_BACK_STATE)->ShowWindow(SW_HIDE);
    }
}

/******************************************
函数:	ProcessFileSize
描述:	
输入:	
输出:	
返回值: 
******************************************/
DWORD CDlgPlayRemoteFile::ProcessFileSize(CString csFileSize)
{
    DWORD dwFileSize = 0;
    if (csFileSize[csFileSize.GetLength()-1] == 'M')
    {
        dwFileSize = atoi(csFileSize.GetBuffer(csFileSize.GetLength() - 1));
        dwFileSize *= (1024*1024);
    }
    else if (csFileSize[csFileSize.GetLength()-1] == 'K')
    {
        dwFileSize = atoi(csFileSize.GetBuffer(csFileSize.GetLength() - 1));
        dwFileSize *= 1024;
    }
    else
    {
        dwFileSize = atoi(csFileSize.GetBuffer(csFileSize.GetLength()));
    }
    return dwFileSize;
}

void CDlgPlayRemoteFile::GetDiskList()
{
    memset(&m_struDiskList, 0, sizeof(m_struDiskList));
    if (!NET_DVR_GetDiskList(m_lLoginID, &m_struDiskList))
    {
        g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_GetDiskList");
    }
    else
    {
        g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_GetDiskList");
    }
    
    m_comboDiskList.ResetContent();
    for (int i = 0; i < m_struDiskList.dwNodeNum; i++)
    {
        m_comboDiskList.AddString((char*)m_struDiskList.struDescNode[i].byDescribe);
        m_comboDiskList.SetItemData(i, m_struDiskList.struDescNode[i].iValue);
    }
}

void CDlgPlayRemoteFile::OnSelchangeComboDiskList() 
{
	DWORD i = m_comboDiskList.GetCurSel();
    DWORD dwDiskFreeSpace = m_struDiskList.struDescNode[i].dwFreeSpace;
    char szLanCn[256] = {0};
    char szLanEn[256] = {0};
    char szLan[256] = {0};
    sprintf(szLanCn, "剩余磁盘空间%dM", dwDiskFreeSpace);
    sprintf(szLanEn, "Free Disk Spcace%dM", dwDiskFreeSpace);
    g_StringLanType(szLan, szLanCn, szLanEn);
    GetDlgItem(IDC_STATIC_BACKUP_DISK_SIZE)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_STATIC_BACKUP_DISK_SIZE)->SetWindowText(szLan);
}


void CDlgPlayRemoteFile::OnBtnInsertRecordLabel() 
{
	UpdateData(TRUE);
    NET_DVR_RECORD_LABEL struRecordLabel = {0};
    NET_DVR_GetPlayBackOsdTime(m_nPlayHandle, &struRecordLabel.struTimeLabel);
	if(m_csAddLabelName == _T(""))
	{
		struRecordLabel.byQuickAdd = TRUE;
	}
	else
	{
		struRecordLabel.byQuickAdd = FALSE;
		memcpy(struRecordLabel.sLabelName, m_csAddLabelName, LABEL_NAME_LEN);
	}
	
    NET_DVR_LABEL_IDENTIFY struLabelIdentiff = {0};
    InsertRecordLabel(&struRecordLabel, &struLabelIdentiff);
}

BOOL CDlgPlayRemoteFile::InsertRecordLabel(NET_DVR_RECORD_LABEL* lpRecordLabel, NET_DVR_LABEL_IDENTIFY *lpLableIdentify)
{
	char szLan[40] = {0};
	g_StringLanType(szLan, "新增标签成功,标签ID:", "Add Label succeed,Label ID:");
	CString csLabelID = _T("");

    if (NET_DVR_InsertRecordLabel(m_nPlayHandle, lpRecordLabel, lpLableIdentify))
    {
		csLabelID.Format("%s%s", szLan, lpLableIdentify->sLabelIdentify);
        g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_InsertRecordLabel");
		AfxMessageBox(csLabelID);
        return TRUE;
    }
    else
    {
        g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_InsertRecordLabel");
        return FALSE;
    }
}

void CDlgPlayRemoteFile::OnRadioForward() 
{
	UpdateData(TRUE);
	((CButton *)GetDlgItem(IDC_RADIO_FORWARD))->SetCheck(TRUE);
	((CButton *)GetDlgItem(IDC_RADIO_REVERSE))->SetCheck(FALSE);
	UpdateData(FALSE);
	
	if(m_nPlayHandle == -1)
	{
		return;
	}
	else
	{
		if(NET_DVR_PlayBackControl_V40(m_nPlayHandle, NET_DVR_PLAY_FORWARD, NULL, 0, NULL, NULL))
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAY_FORWARD");
		}
		else
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAY_FORWARD");
		}
	}
}

void CDlgPlayRemoteFile::OnRadioReverse() 
{
	UpdateData(TRUE);
	((CButton *)GetDlgItem(IDC_RADIO_FORWARD))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_RADIO_REVERSE))->SetCheck(TRUE);
	UpdateData(FALSE);	
	
	if(m_nPlayHandle == -1)
	{
		return;
	}
	else
	{
		if(NET_DVR_PlayBackControl_V40(m_nPlayHandle, NET_DVR_PLAY_REVERSE, NULL, 0, NULL, NULL))
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAY_REVERSE");
		}
		else
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAY_REVERSE");
		}
	}
}

void CDlgPlayRemoteFile::OnSelchangeComboFindType() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if (m_cmbFindType.GetCurSel() != 2)
	{
		GetDlgItem(IDC_EDIT_WD_GUID)->ShowWindow(FALSE);

		GetDlgItem(IDC_STATIC_GUID)->ShowWindow(FALSE);
		 GetDlgItem(IDC_EDIT_WD_GUID1)->ShowWindow(FALSE);
		GetDlgItem(IDC_EDIT_WD_GUID2)->ShowWindow(FALSE);
		GetDlgItem(IDC_EDIT_WD_GUID3)->ShowWindow(FALSE);
		GetDlgItem(IDC_EDIT_WD_GUID4)->ShowWindow(FALSE);
		GetDlgItem(IDC_EDIT_WD_GUID5)->ShowWindow(FALSE);
		GetDlgItem(IDC_EDIT_WD_GUID6)->ShowWindow(FALSE);
		GetDlgItem(IDC_EDIT_WD_GUID7)->ShowWindow(FALSE);
		GetDlgItem(IDC_EDIT_WD_GUID8)->ShowWindow(FALSE);
		GetDlgItem(IDC_EDIT_WD_GUID9)->ShowWindow(FALSE);
		GetDlgItem(IDC_EDIT_WD_GUID10)->ShowWindow(FALSE);
		GetDlgItem(IDC_EDIT_WD_GUID11)->ShowWindow(FALSE);
		GetDlgItem(IDC_EDIT_WD_GUID12)->ShowWindow(FALSE);
		GetDlgItem(IDC_EDIT_WD_GUID13)->ShowWindow(FALSE);
		GetDlgItem(IDC_EDIT_WD_GUID14)->ShowWindow(FALSE);
		GetDlgItem(IDC_EDIT_WD_GUID15)->ShowWindow(FALSE);
		GetDlgItem(IDC_EDIT_WD_GUID16)->ShowWindow(FALSE);
		 
	}
	else
	{
		GetDlgItem(IDC_STATIC_GUID)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_WD_GUID)->ShowWindow(TRUE);

		 GetDlgItem(IDC_EDIT_WD_GUID1)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_WD_GUID2)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_WD_GUID3)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_WD_GUID4)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_WD_GUID5)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_WD_GUID6)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_WD_GUID7)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_WD_GUID8)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_WD_GUID9)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_WD_GUID10)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_WD_GUID11)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_WD_GUID12)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_WD_GUID13)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_WD_GUID14)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_WD_GUID15)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_WD_GUID16)->ShowWindow(TRUE);
	  
	}
}

//[add] by zja 2013-12-23 10:41:04
UINT CDlgPlayRemoteFile::SingleDownloadThread(LPVOID pParam)
{
    CDlgPlayRemoteFile *pThis = static_cast<CDlgPlayRemoteFile *>(pParam);    
    char sIPV6Addr[130] = {0};
    char szLan[128] = {0}; 
    CString csDir; 
    OutResource struRes; 
    if ( !pThis->m_MutilDowload.GetResource(struRes))
    {
        return 0; 
    }
    //设置转码类型
    int iTransType = pThis->m_comboTransType.GetCurSel() + 1;
    if (iTransType == 4)
    {
        iTransType = 5;
    }
    if (NET_DVR_PlayBackControl_V40(struRes.lDLHandle , NET_DVR_SET_TRANS_TYPE, &iTransType, NULL))
    {
        g_pMainDlg->AddLog(pThis->m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_SET_TRANS_TYPE TransType[%d]", iTransType);
    }
    else
    {
        g_pMainDlg->AddLog(pThis->m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_SET_TRANS_TYPE TransType[%d]", iTransType);
    }
    NET_DVR_PlayBackControl(struRes.lDLHandle, NET_DVR_PLAYSTART, 0, NULL);
    g_pMainDlg->AddLog(pThis->m_iDeviceIndex, OPERATION_SUCC_T, "[Down]%s->%s",  (LPCSTR)struRes.csDLFileN, (LPCSTR)struRes.csSaveFileN );
    pThis->m_progressDownload.SetPos(0);
    pThis->m_progressDownload.ShowWindow(SW_SHOW);
    pThis->m_bDownloading=TRUE;
    g_StringLanType(szLan, "正在下载文件", "Downloading");
    pThis->m_csDownloadStat.Format("%s%s", szLan, (LPCSTR)struRes.csDLFileN);
    //为显示进度条赋值pThis->m_lDownloadHandle 显示进度条为最后一个下载
    pThis->m_lDownloadHandle = struRes.lDLHandle; 
    pThis->GetDlgItem(IDC_STATIC_REMOTE_DOWNLOAD_STATE)->ShowWindow(SW_SHOW);
    pThis->SetTimer(DOWNSTATE_TIMER,200,NULL);
//    pThis->UpdateData(FALSE); 
    return 0; 
}

void CDlgPlayRemoteFile::OnButMutilthreadDownload() 
{
	// TODO: Add your control notification handler code here
    CString csFileName;
    CString csStartTime, csStopTime;
    CString csTemp,csDir;
//    char nFileName[256];
    POSITION  posItem;
    int iFileSelPos = 0;
    char sIPV6Addr[130] = {0};
    char szLan[128] = {0};
  
    UpdateData(TRUE);
    g_CreateLocalDir();
    if (!m_bDownloading)
    {
        m_MutilDowload.Init(); 
        if ((posItem = m_listRemoteFile.GetFirstSelectedItemPosition()) == NULL)
        {
            return; 
        }
        for (int i = 0; i < sizeof(g_struDeviceInfo[m_iDeviceIndex].chDeviceIP); i ++)
        {
            if(g_struDeviceInfo[m_iDeviceIndex].chDeviceIP[i] == '\0')
            {
                break;
            }
            else if(g_struDeviceInfo[m_iDeviceIndex].chDeviceIP[i] == ':')
            {
                sIPV6Addr[i] = '-';
            }
            else
            {
                sIPV6Addr[i] = g_struDeviceInfo[m_iDeviceIndex].chDeviceIP[i];
            }
        }
        csDir.Format("%s\\",g_struLocalParam.chDownLoadPath);
        m_iSelListItem = m_listRemoteFile.GetNextSelectedItem(posItem);
        DWORD dwThreadID; 
        HANDLE hThread; 
        LONG lDownloadHandle; 
        OutResource struRes; 
        while (m_iSelListItem != -1 )
        {
            if (m_MutilDowload.isFull())
            {
                break; 
            }
            
            csFileName.Format("%s", m_listRemoteFile.GetItemText(m_iSelListItem,0));
            if (csFileName.IsEmpty())
            {
                m_iSelListItem = m_listRemoteFile.GetNextSelectedItem(posItem);
                continue; 
            }
            m_csStartTime.Format("%s",m_listRemoteFile.GetItemText(m_iSelListItem,2));
            m_csStartTime.Replace("+", "East");
            m_csStartTime.Replace("-", "West");
            m_csStartTime.Replace(":", "_");
            m_csStopTime.Format("%s", m_listRemoteFile.GetItemText(m_iSelListItem,3));
            m_csStopTime.Replace("+", "East");
            m_csStopTime.Replace("-", "West");
            m_csStopTime.Replace(":", "_");
            struRes.csDLFileN.Format("%s", csFileName); 
            struRes.csSaveFileN.Format("%s%s_%02d_D%s_%s.mp4",csDir,sIPV6Addr, m_iChanShowNum, m_csStartTime, m_csStopTime); 

            //lDownloadHandle = NET_DVR_GetFileByName(m_lLoginID, const_cast<char *>((LPCSTR)struRes.csDLFileN), const_cast<char *>((LPCSTR)struRes.csSaveFileN) );
            NET_DVR_DOWNLOAD_BY_NAME_COND struDownloadCond = { 0 };
            struDownloadCond.pFileName = const_cast<char *>((LPCSTR)struRes.csDLFileN);
            struDownloadCond.pSavedFileName = const_cast<char *>((LPCSTR)struRes.csSaveFileN);

            CString szIp;
            CString szPort;
            szIp.Format("%s", m_listRemoteFile.GetItemText(m_iSelListItem, 8));
            //ipv4
            if (!g_ValidIPv6((BYTE*)(LPCSTR)szIp))
            {
                sprintf(struDownloadCond.struAddr.struIP.sIpV4, "%s", szIp);
            }
            else
            {
                sprintf((char *)struDownloadCond.struAddr.struIP.byIPv6, "%s", szIp);
            }
            szPort = m_listRemoteFile.GetItemText(m_iSelListItem, 9);
            struDownloadCond.struAddr.wPort = _ttoi(szPort);

            lDownloadHandle = NET_DVR_GetFileByName_V50(m_lLoginID, &struDownloadCond);

            if ( lDownloadHandle >= 0 )
            {
                NET_DVR_LINK_ADDR struParam = { 0 };
                if (NET_DVR_GetLinkAddr(lDownloadHandle, ENUM_LINK_PLAYBACK, &struParam))
                {
                    g_pMainDlg->AddLog(lDownloadHandle, OPERATION_SUCC_T, "NET_DVR_GetLinkAddr, dev addr[%s:%d], local addr[%s:%d]", \
                        &(struParam.uDevIP), struParam.wDevPort[0], &(struParam.uLocalIP), struParam.wLocalPort[0]);
                }
                else
                {
                    g_pMainDlg->AddLog(lDownloadHandle, OPERATION_FAIL_T, "NET_DVR_GetLinkAddr");
                }

                hThread = CreateThread(NULL,0,LPTHREAD_START_ROUTINE(SingleDownloadThread),this,0,&dwThreadID); 
                if ( hThread != NULL )
                {
                    struRes.lDLHandle = lDownloadHandle; 
                    struRes.hThread = hThread; 
                    if (! m_MutilDowload.AddResource(struRes) )
                    {
                        break; 
                    }
                }
            }
            else
            {
                g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_GetFileByName");
                g_StringLanType(szLan, "下载文件失败", "Download failed");
                AfxMessageBox(szLan);
                return;
            }
            
            m_iSelListItem = m_listRemoteFile.GetNextSelectedItem(posItem);
       }
       g_StringLanType(szLan, "停止下载", "Download");
       GetDlgItem(IDC_BUT_MUTILTHREAD_DOWNLOAD)->SetWindowText(szLan);
       g_StringLanType(szLan, "正在下载文件", "downloading files");
       m_csDownloadStat.Format("%s", szLan );
    }
    else 
    {
        m_MutilDowload.Del(); 
        m_bDownloading=FALSE;
        g_StringLanType(szLan, "多路下载", "Download");
        GetDlgItem(IDC_BUT_MUTILTHREAD_DOWNLOAD)->SetWindowText(szLan);
        KillTimer(DOWNSTATE_TIMER);
        Invalidate(TRUE);
        m_progressDownload.SetPos(0);
        m_progressDownload.ShowWindow(SW_HIDE);
        g_StringLanType(szLan, "停止下载文件", "Stop downloading");
        m_csDownloadStat.Format("%s%s", szLan, m_szDownLoadFileName);
        GetDlgItem(IDC_STATIC_REMOTE_DOWNLOAD_STATE)->ShowWindow(SW_HIDE);
    }
    UpdateData(FALSE);
}


DownloadThread::DownloadThread():m_iWaitTime(10),m_iTryCount(3)
{
    m_byResCount = 0; 
    m_startDlIndex = 0; 
}

BOOL   DownloadThread::Del(HANDLE hThread)
{
    m_ResourceOpLock.Lock(); 
    DWORD QueSize = m_ResourceQue.size(); 
    if ( hThread == NULL)
    {
        //删除所有       
        for (int i=0; i< QueSize; i++)
        {
            _Del(i); 
        }
        m_ResourceQue.clear(); 
        m_byResCount = 0; 
        m_startDlIndex = 0; 
    } 
    else
    {
        for (int i=0; i<QueSize; i++)
        {
            if ( hThread == m_ResourceQue[i].hThread)
            {
                _Del(i); 
                break; 
            }
        }
        m_byResCount --;  
    }
    m_ResourceOpLock.Unlock(); 
    return TRUE; 
}


BOOL    DownloadThread::_Del(BYTE i)
{
    //具体实现代码， 关闭句柄和线程 置0
    g_pMainDlg->AddLog(1, OPERATION_SUCC_T, "(DownloadThread::_Del) lDLHandle %x, hThread %x  Download file %s Save file %s",\
        m_ResourceQue[i].lDLHandle, m_ResourceQue[i].hThread, m_ResourceQue[i].csDLFileN, m_ResourceQue[i].csSaveFileN);

    if ( m_ResourceQue[i].lDLHandle != -1 )
    {
        NET_DVR_StopGetFile(m_ResourceQue[i].lDLHandle); 
        m_ResourceQue[i].lDLHandle = -1; 
    }
    if ( m_ResourceQue[i].hThread != NULL ) 
    {
        TerminateThread(m_ResourceQue[i].hThread, 0); 
        CloseHandle(m_ResourceQue[i].hThread); 
        m_ResourceQue[i].hThread = NULL; 
    }
    return TRUE; 
}

//计算总的进度 获取100或异常的都会删除线程句柄
BYTE   DownloadThread::CountPos()
{
    WORD wAvgPos = 0; 
    short sPos = 0; 
    BYTE byInvaildCount = 0; 
    DWORD QueSize = m_ResourceQue.size(); 
    if ( QueSize == 0)
    {
        return 100; 
    }
    m_ResourceOpLock.Lock(); 
    for (int i=0 ; i<QueSize; i++)                   
    {
        if ( m_ResourceQue[i].hThread == NULL )
        {
            //byInvaildCount ++; 
            wAvgPos += 100; 
            continue;                                  //已经完成的线程
        }
        sPos  = NET_DVR_GetDownloadPos(m_ResourceQue[i].lDLHandle);
        
        if ( sPos < 0 || sPos >= 100)
        {        
            byInvaildCount ++;       
            _Del(i);                                           //状态异常结束线程
            //sPos = 100;                                        //错误后置100，防止进度条后退
        }
        else if ( sPos == 100 )
        {
            _CloseHandle(i);                                   //状态为100的关闭线程句柄， 线程自然结束
        }
        wAvgPos += sPos; 
    }
    wAvgPos = (WORD) (wAvgPos / QueSize); 
    m_ResourceOpLock.Unlock(); 
    return  wAvgPos; 
}

BOOL    DownloadThread::_CloseHandle(BYTE i)
{
//具体实现代码， 关闭句柄和线程 置0
   
    if ( m_ResourceQue[i].lDLHandle != NULL )
    {
        NET_DVR_StopGetFile(m_ResourceQue[i].lDLHandle); 
        m_ResourceQue[i].lDLHandle = NULL; 
    }
    if ( m_ResourceQue[i].hThread != NULL ) 
    {
        CloseHandle(m_ResourceQue[i].hThread); 
        m_ResourceQue[i].hThread = NULL; 
    }
    return TRUE; 
}

BOOL    DownloadThread::Init()
{
    Del(); 
    m_byResCount = 0; 
    m_startDlIndex = 0; 
    return TRUE; 
}
//添加外部资源 多于5个线程返回错误
BOOL   DownloadThread::AddResource(const OutResource &struRes)
{
    if ( m_byResCount >= MAX_DOWNLOAD_THREAN_NUM )
    {
        g_pMainDlg->AddLog(1, OPERATION_FAIL_T, "(DownloadThread::AddResource) m_byRescount %d, Download file %s Save file %s", m_byResCount, struRes.csDLFileN, struRes.csSaveFileN);
        return FALSE; 
    }
    m_ResourceOpLock.Lock(); 
    m_ResourceQue.push_back(struRes); 
    m_byResCount ++; 
    m_ResourceOpLock.Unlock(); 
    g_pMainDlg->AddLog(1, OPERATION_SUCC_T, "(DownloadThread::AddResource) m_byRescount %d, Download file %s Save file %s", m_byResCount, struRes.csDLFileN, struRes.csSaveFileN);
    return TRUE; 
}           

 //获取第一个资源  
BOOL    DownloadThread::GetResource(OutResource &struRes)
{
    BOOL BRet; 
    CString csDebug; 
    for (int i=0; i<m_iTryCount; i++)
    {
        m_ResourceOpLock.Lock(); 
        if ( m_ResourceQue.size() != 0 && m_startDlIndex != m_ResourceQue.size() )
        {
            struRes = m_ResourceQue[m_startDlIndex++]; 
            BRet = TRUE; 
            g_pMainDlg->AddLog(1, OPERATION_SUCC_T, "(DownloadThread::GetResource) RetValue %d, deque size %d, m_startDlIndex %d",BRet, m_ResourceQue.size(), m_startDlIndex);   
            m_ResourceOpLock.Unlock();
            break; 
        }
        BRet = FALSE;   
        g_pMainDlg->AddLog(1, OPERATION_FAIL_T, "(DownloadThread::GetResource) RetValue %d, deque size %d, m_startDlIndex %d",BRet, m_ResourceQue.size(), m_startDlIndex);   
        m_ResourceOpLock.Unlock();   
        Sleep(m_iWaitTime); 
    }
    return BRet; 
}            


void CDlgPlayRemoteFile::OnSelchangeCmbUsecardType() 
{
    // TODO: Add your control notification handler code here
    
    DWORD dwSelectIndex = m_comboUseCardType.GetCurSel();
    switch(dwSelectIndex)
    {
    case 0:  //不带卡号
        GetDlgItem(IDC_STATICCARDNUM)->ShowWindow(FALSE);
        GetDlgItem(IDC_EDIT_CARD_NUM)->ShowWindow(FALSE);
        
        GetDlgItem(IDC_STATIC_ATM_TRADETYPE)->ShowWindow(FALSE);    
        GetDlgItem(IDC_CMB_ATM_TRADETYPE)->ShowWindow(FALSE);
        
        GetDlgItem(IDC_STATIC_MONEY)->ShowWindow(FALSE);    
        GetDlgItem(IDC_EDIT_MONEY)->ShowWindow(FALSE);
        break;
    case 1: //带卡号
        GetDlgItem(IDC_STATICCARDNUM)->ShowWindow(TRUE);
		SetDlgItemText(IDC_STATICCARDNUM,"卡号");
        GetDlgItem(IDC_EDIT_CARD_NUM)->ShowWindow(TRUE);
        
        GetDlgItem(IDC_STATIC_ATM_TRADETYPE)->ShowWindow(FALSE);    
        GetDlgItem(IDC_CMB_ATM_TRADETYPE)->ShowWindow(FALSE);
        
        GetDlgItem(IDC_STATIC_MONEY)->ShowWindow(FALSE);    
        GetDlgItem(IDC_EDIT_MONEY)->ShowWindow(FALSE);
        break;
    case 2:  //交易类型
        GetDlgItem(IDC_STATICCARDNUM)->ShowWindow(FALSE);
        GetDlgItem(IDC_EDIT_CARD_NUM)->ShowWindow(FALSE);
        
        GetDlgItem(IDC_STATIC_ATM_TRADETYPE)->ShowWindow(TRUE);    
        GetDlgItem(IDC_CMB_ATM_TRADETYPE)->ShowWindow(TRUE);
        
        GetDlgItem(IDC_STATIC_MONEY)->ShowWindow(FALSE);    
        GetDlgItem(IDC_EDIT_MONEY)->ShowWindow(FALSE);
        break;
    case 3:   //交易金额
        GetDlgItem(IDC_STATICCARDNUM)->ShowWindow(FALSE);
        GetDlgItem(IDC_EDIT_CARD_NUM)->ShowWindow(FALSE);
        
        GetDlgItem(IDC_STATIC_ATM_TRADETYPE)->ShowWindow(FALSE);    
        GetDlgItem(IDC_CMB_ATM_TRADETYPE)->ShowWindow(FALSE);
        
        GetDlgItem(IDC_STATIC_MONEY)->ShowWindow(TRUE);    
        GetDlgItem(IDC_EDIT_MONEY)->ShowWindow(TRUE);
        break;
    case 4:
        GetDlgItem(IDC_STATICCARDNUM)->ShowWindow(TRUE);
		SetDlgItemText(IDC_STATICCARDNUM,"卡号");
        GetDlgItem(IDC_EDIT_CARD_NUM)->ShowWindow(TRUE);
        
        GetDlgItem(IDC_STATIC_ATM_TRADETYPE)->ShowWindow(TRUE);    
        GetDlgItem(IDC_CMB_ATM_TRADETYPE)->ShowWindow(TRUE);
        
        GetDlgItem(IDC_STATIC_MONEY)->ShowWindow(TRUE);    
        GetDlgItem(IDC_EDIT_MONEY)->ShowWindow(TRUE);
        break;
	case 5:
        GetDlgItem(IDC_STATICCARDNUM)->ShowWindow(TRUE);
		SetDlgItemText(IDC_STATICCARDNUM,"课程名");
        GetDlgItem(IDC_EDIT_CARD_NUM)->ShowWindow(TRUE);
        
        GetDlgItem(IDC_STATIC_ATM_TRADETYPE)->ShowWindow(FALSE);    
        GetDlgItem(IDC_CMB_ATM_TRADETYPE)->ShowWindow(FALSE);
        
        GetDlgItem(IDC_STATIC_MONEY)->ShowWindow(FALSE);    
        GetDlgItem(IDC_EDIT_MONEY)->ShowWindow(FALSE);
        break;
    default:
        
        GetDlgItem(IDC_STATICCARDNUM)->ShowWindow(FALSE);
        GetDlgItem(IDC_EDIT_CARD_NUM)->ShowWindow(FALSE);
        
        GetDlgItem(IDC_STATIC_ATM_TRADETYPE)->ShowWindow(FALSE);    
        GetDlgItem(IDC_CMB_ATM_TRADETYPE)->ShowWindow(FALSE);
        
        GetDlgItem(IDC_STATIC_MONEY)->ShowWindow(FALSE);    
        GetDlgItem(IDC_EDIT_MONEY)->ShowWindow(FALSE);
        
        break;
    }
    
    UpdateData(FALSE);
}          

void CDlgPlayRemoteFile::OnBtnShowOsdinfo() 
{
    // TODO: Add your control notification handler code here
    char szLan[128] = {0};
    LONG lPlayerPort = NET_DVR_GetPlayBackPlayerIndex(m_nPlayHandle);
    BOOL bRet = FALSE;
    
    
    CString struAlarmOutText = "";
    DWORD dwID = 0;
    GetDlgItem(IDC_BTN_SHOW_OSDINFO)->GetWindowText(struAlarmOutText);
    DWORD dwPRIDATA_RENDER = PLAYM4_RENDER_ANA_INTEL_DATA | PLAYM4_RENDER_MD | PLAYM4_RENDER_ADD_POS | PLAYM4_RENDER_ADD_PIC | PLAYM4_RENDER_FIRE_DETCET;
    if (struAlarmOutText.Find("显示OSD") != -1)
    {
        if (lPlayerPort > -1)
        {
            bRet = PlayM4_RenderPrivateData(lPlayerPort, dwPRIDATA_RENDER,TRUE );
            if (!bRet)
            {
                g_StringLanType(szLan, "调用PlayM4_EnablePOS接口设置 打开OSD 失败", "Load PlayM4_EnablePOS show OSD show  failed");
                AfxMessageBox(szLan);
                return;
            }
            
            GetDlgItem(IDC_BTN_SHOW_OSDINFO)->SetWindowText("关闭OSD");
        }
        else
        {
            g_StringLanType(szLan, "获取回放时用来解码显示的播放库句柄失败", "Get PlayBack Player Index Failed");
            AfxMessageBox(szLan);
            return;
        }
    } 
    else
    {
        if (lPlayerPort > -1)
        {
            bRet = PlayM4_RenderPrivateData(lPlayerPort, dwPRIDATA_RENDER,FALSE);
            if (!bRet)
            {
                g_StringLanType(szLan, "调用PlayM4_EnablePOS接口 关闭OSD 失败", "Load PlayM4_EnablePOS Close OSD show  failed");
                AfxMessageBox(szLan);
                return;
            }
            GetDlgItem(IDC_BTN_SHOW_OSDINFO)->SetWindowText("显示OSD");
        }
        else
        {
            g_StringLanType(szLan, "获取回放时用来解码显示的播放库句柄失败", "Get PlayBack Player Index Failed");
            AfxMessageBox(szLan);
            return;
        }
    }
}   


void CDlgPlayRemoteFile::OnBnClickedBtnStartDraw()
{
    // TODO:  在此添加控件通知处理程序代码
    UpdateData(TRUE);
    NET_DVR_VOD_DRAWFRAME_PARA struVODDrawframePara = { 0 };
    struVODDrawframePara.dwDrawType = m_drawtype.GetCurSel();

    struVODDrawframePara.struTime.wYear = m_drawdate.GetYear();
    struVODDrawframePara.struTime.byMonth = m_drawdate.GetMonth();
    struVODDrawframePara.struTime.byDay = m_drawdate.GetDay();
    struVODDrawframePara.struTime.byHour = m_drawtime.GetHour();
    struVODDrawframePara.struTime.byMinute = m_drawtime.GetMinute();
    struVODDrawframePara.struTime.bySecond = m_drawtime.GetSecond();

    if (m_nPlayHandle >= 0)
    {
        if (NET_DVR_PlayBackControl_V40(m_nPlayHandle, NET_DVR_START_DRAWFRAME, &struVODDrawframePara, sizeof(struVODDrawframePara), NULL, NULL))
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_START_DRAWFRAME");
        }
        else
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_START_DRAWFRAME");
        }
    }
    else
    {
        if (NET_DVR_PlayBackControl_V40(m_lDownloadHandle, NET_DVR_START_DRAWFRAME, &struVODDrawframePara, sizeof(struVODDrawframePara), NULL, NULL))
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_START_DRAWFRAME");
        }
        else
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_START_DRAWFRAME");
        }
    }
}


void CDlgPlayRemoteFile::OnBnClickedBtnEnddraw()
{
    // TODO:  在此添加控件通知处理程序代码
    UpdateData(TRUE);
    NET_DVR_TIME_EX strudvrtimeex = { 0 };

    strudvrtimeex.wYear = m_drawdate.GetYear();
    strudvrtimeex.byMonth = m_drawdate.GetMonth();
    strudvrtimeex.byDay = m_drawdate.GetDay();
    strudvrtimeex.byHour = m_drawtime.GetHour();
    strudvrtimeex.byMinute = m_drawtime.GetMinute();
    strudvrtimeex.bySecond = m_drawtime.GetSecond();

    if (m_nPlayHandle >= 0 )
    {
        if (NET_DVR_PlayBackControl_V40(m_nPlayHandle, NET_DVR_STOP_DRAWFRAME, &strudvrtimeex, sizeof(strudvrtimeex), NULL, NULL))
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_STOP_DRAWFRAME");
        }
        else
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_STOP_DRAWFRAME");
        }
    }
    else if (m_lDownloadHandle >= 0)
    {
        if (NET_DVR_PlayBackControl_V40(m_lDownloadHandle, NET_DVR_STOP_DRAWFRAME, &strudvrtimeex, sizeof(strudvrtimeex), NULL, NULL))
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_STOP_DRAWFRAME");
        }
        else
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_STOP_DRAWFRAME");
        }
    }
}


void CDlgPlayRemoteFile::OnBnClickedBtnMedical()
{
    // TODO:  在此添加控件通知处理程序代码
    CDlgPlayRemoteFileMedical dlg;
    dlg.DoModal();
}
