/**********************************************************
FileName:    DlgTalkMR.cpp
Description: voice media redirect      
Date:        2008/12/18
Note: 		the global Macro definition and structure is in "GeneralDef.h", global variable and function is in "ClientDemo.cpp"
Modification History:      
    <version> <time>         <desc>
    <1.0    > <2008/12/18>       <created>
***********************************************************/

#include "stdafx.h"
#include "DlgTalkMREx.h"
#include "HCNetSDK.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//#define  RECORD

CDlgTalkMREx* g_pDlgTalkMR = NULL;


/*********************************************************
  Function:	fVoiceDataCallBack
  Desc:		call back the audio data from device/从回调中得到设备的音频数据
  Input:	lVoiceComHandle,handle of voice communication;pRecvDataBuffer, buffer for voice data; dwBufSize, voice data length; pUser, User data;
  Output:	none
  Return:	none
**********************************************************/
void CALLBACK fVoiceDataCallBackEx(LONG lVoiceComHandle, char *pRecvDataBuffer, DWORD dwBufSize, BYTE byAudioFlag, void* pUser)
{
	LPTALK_MR lpTalkMr = (LPTALK_MR)pUser;	
	switch(byAudioFlag)
	{
	case 0:
		TRACE("local audio data\n");
		break;
	case 1:
		g_pDlgTalkMR->PutIntoBuf(pRecvDataBuffer, (int)dwBufSize, lpTalkMr);
	//	TRACE("Input data size[%d]\n", dwBufSize);
		break;
	case 2:
		TRACE("audio sending and recving thread exit\n");
		break;
	default:
		break;
	}	
}

/*********************************************************
  Function:	DataFromSoundIn
  Desc:		get local audio data and send to device
  Input:	buffer, local voice data buffer; dwSize, data length; dwOwner, owner data;
  Output:	none
  Return:	none
**********************************************************/
void CALLBACK DataFromSoundInEx(char* buffer, DWORD dwSize, void *pOwner)
{
    //buffer为原始PCM音频数据
    ASSERT(pOwner);
    CDlgTalkMREx* p = (CDlgTalkMREx*)pOwner;
    p->SendDataToDVR(buffer, dwSize);
}


/////////////////////////////////////////////////////////////////////////////
/*********************************************************
  Function:	CDlgTalkMR
  Desc:		Constructor
  Input:	pParent, parent window pointer
  Output:	none
  Return:	none
**********************************************************/
CDlgTalkMREx::CDlgTalkMREx(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgTalkMREx::IDD, pParent)
{
	m_bWaveDeal = FALSE;
	m_iDeviceIndex = -1;
	//{{AFX_DATA_INIT(CDlgTalkMR)
	//}}AFX_DATA_INIT
	
	m_hFile = NULL;
    m_hExitEvent = INVALID_HANDLE_VALUE;
	memset(&m_hHaveRDataRV, 0, sizeof(HANDLE) * MAX_SOUND_OUT);
	memset(&m_hPlayHandle, -1, sizeof(HANDLE) * MAX_SOUND_OUT);
	m_pEncoder = NULL;

	m_pG726Enc = NULL;
	m_pG726EncM = NULL;
    m_pG711Enc = NULL;
	m_bReset = TRUE;
    m_byAudioEncType = -1;

	memset(m_bOpenWavOut, 0 , sizeof(BOOL) * MAX_SOUND_OUT);
	m_bOpenWavIn = FALSE;

	m_dwBufSize = AUDENCSIZE;
	m_dwBufNum = 6;
	//init WAVEFORMATEX
	m_struWaveFormat.cbSize =			sizeof(WAVEFORMATEX);
	m_struWaveFormat.nBlockAlign =		CHANNEL * BITS_PER_SAMPLE / 8;
	m_struWaveFormat.nChannels =		CHANNEL;
	m_struWaveFormat.nSamplesPerSec =	SAMPLES_PER_SECOND;
	m_struWaveFormat.wBitsPerSample =	BITS_PER_SAMPLE;
	m_struWaveFormat.nAvgBytesPerSec =	SAMPLES_PER_SECOND * m_struWaveFormat.nBlockAlign;
	m_struWaveFormat.wFormatTag =		WAVE_FORMAT_PCM;

	memset(m_pRenderBuf, 0, sizeof(LPBYTE) * MAX_SOUND_OUT);
	memset(m_nBufNo, 0, sizeof(int) * MAX_SOUND_OUT);
	memset(m_rIndexRV, 0 , sizeof(int) * MAX_SOUND_OUT);
	memset(m_ReceiveIndexRV, 0 , sizeof(int) * MAX_SOUND_OUT);
	memset(m_bOpenPlayThread, 0, sizeof(BOOL) * MAX_SOUND_OUT);

    memset(m_byAudioTemp, 0, AUDENCSIZE);
    memset(m_talkMr, 0, sizeof(TALK_MR) * MAX_SOUND_OUT);
	memset(m_talkIndex, 0 , sizeof(int) * MAX_DEVICES * MAX_AUDIO_V40);
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDlgTalkMREx::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgTalkMR)
	DDX_Control(pDX, IDC_COMBO_AUDIO_ENC_TYPE, m_comAudioEncType);
	DDX_Control(pDX, IDC_TREE_DEV, m_treeBroadCast);
	DDX_Control(pDX, IDC_BUTTALK, m_btTalk);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgTalkMREx, CDialog)
	//{{AFX_MSG_MAP(CDlgTalkMR)
	ON_BN_CLICKED(IDC_BUTTALK, OnButtalk)
	ON_BN_CLICKED(IDC_BTN_EXIT, OnBtnExit)
	ON_NOTIFY(NM_CLICK, IDC_TREE_DEV, OnClickTreeDev)
	ON_BN_CLICKED(IDC_BTN_REFRESH, OnBtnRefresh)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
/*********************************************************
  Function:	OnInitDialog
  Desc:		Initialize the dialog
  Input:	none
  Output:	none
  Return:	none
**********************************************************/
BOOL CDlgTalkMREx::OnInitDialog()
{
	CDialog::OnInitDialog();
	// TODO: Add extra initialization here
	
#ifdef RECORD 
	CreateWaveFile("record.wav");
#endif
	g_pDlgTalkMR = this;
	GetDlgItem(IDC_BUTTALK)->EnableWindow(TRUE);

	m_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	int i = 0;
	for (i = 0; i < MAX_SOUND_OUT; i++)
	{
		m_hHaveRDataRV[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
	}	

#if (!defined _WIN64)
	//encoder 722
	m_pEncoder = NET_DVR_InitG722Encoder();

#elif (defined _WIN64)
    //722
    NET_DVR_AUDIOENC_INFO struEncInfo = { 0 };
    m_pEncoder = NET_DVR_InitG722Encoder(&struEncInfo);
    
    //711
    NET_DVR_AUDIOENC_INFO struEncInfo711 = { 0 };
    m_pG711Enc = NET_DVR_InitG711Encoder(&struEncInfo711);
#endif

    //g726
    m_pG726Enc = NET_DVR_InitG726Encoder(&m_pG726EncM);
	
	LoadTreeImage();
	CreateTree();
	GetDlgItem(IDC_TREE_DEV)->EnableWindow(TRUE);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

/*********************************************************
  Function:	CreateWaveFile
  Desc:		create the file to save wave data
  Input:	sFileName, point to file name;
  Output:	none
  Return:	none
**********************************************************/
BOOL CDlgTalkMREx::CreateWaveFile(char* sFileName)
{
	// check if file is already open
	if (m_hFile) 
	{
		return FALSE;	
	}
	
	// open file
	m_hFile = ::mmioOpen(sFileName,NULL, MMIO_CREATE|MMIO_WRITE|MMIO_EXCLUSIVE | MMIO_ALLOCBUF);
	if(m_hFile == NULL) 
	{
		return FALSE;
	}
	
	ZeroMemory(&m_MMCKInfoParent, sizeof(MMCKINFO));
	m_MMCKInfoParent.fccType = mmioFOURCC('W','A','V','E');
	
	MMRESULT mmResult = ::mmioCreateChunk( m_hFile,&m_MMCKInfoParent, MMIO_CREATERIFF);
	
	ZeroMemory(&m_MMCKInfoChild, sizeof(MMCKINFO));
	m_MMCKInfoChild.ckid = mmioFOURCC('f','m','t',' ');
	m_MMCKInfoChild.cksize = sizeof(WAVEFORMATEX) + m_struWaveFormat.cbSize;
	mmResult = ::mmioCreateChunk(m_hFile, &m_MMCKInfoChild, 0);
	mmResult = ::mmioWrite(m_hFile, (char*)&m_struWaveFormat, sizeof(WAVEFORMATEX) + m_struWaveFormat.cbSize); 
	mmResult = ::mmioAscend(m_hFile, &m_MMCKInfoChild, 0);
	m_MMCKInfoChild.ckid = mmioFOURCC('d', 'a', 't', 'a');
	mmResult = ::mmioCreateChunk(m_hFile, &m_MMCKInfoChild, 0);
	
	return TRUE;
}

/*********************************************************
  Function:	OnButtalk
  Desc:		begin get client audio data and send to device/获取本地数据并发送给设备
  Input:	none
  Output:	none
  Return:	none
**********************************************************/
void CDlgTalkMREx::OnButtalk()
{
    char szLan[32] = { 0 };
    if (!m_bWaveDeal)
    {
        m_bReset = TRUE;

        WAVEFORMATEX struWaveFormat = { 0 };

        //open wavout
        DWORD dwBufSize = AUDENCSIZE;	//G722
        memcpy(&struWaveFormat, &m_struWaveFormat, sizeof(WAVEFORMATEX));

        if (m_comAudioEncType.GetCurSel() != AUDIOTALKTYPE_G722)
        {
            //struWaveFormat.nSamplesPerSec = SAMPLES_PER_SECOND / 2;
            //struWaveFormat.nAvgBytesPerSec = SAMPLES_PER_SECOND * m_struWaveFormat.nBlockAlign / 2;
            if (m_comAudioEncType.GetCurSel() == 4)
            {
                dwBufSize = G726_AUDENCSIZE; //G726
            }
            else if (m_comAudioEncType.GetCurSel() == 1 || m_comAudioEncType.GetCurSel() == 2)
            {
                dwBufSize = G711_AUDENCSIZE; //G711
            }
            else if (m_comAudioEncType.GetCurSel() == 6)//PCM
            {
                dwBufSize = PCM_AUDENCSIZE;
            }
            else if (m_comAudioEncType.GetCurSel() == 3) //MP2L2
            {
                dwBufSize = MP2L2_AUDDECSIZE;
            }
        }

        //目前支持一路转发听声音
        if (m_SoundOut[0].OpenSound(struWaveFormat, m_dwBufNum, dwBufSize, CALLBACK_FUNCTION, 0))
        {
            g_pMainDlg->AddLog(-1, OPERATION_SUCC_T, "open wavout");
            if (m_SoundOut[0].PlaySound())
            {
                g_pMainDlg->AddLog(-1, OPERATION_SUCC_T, "play wavout");
                m_bOpenWavOut[0] = TRUE;
                //m_SoundOut.SetVolume(0x7fff7fff);
            }
        }
        else
        {
            g_pMainDlg->AddLog(-1, OPERATION_FAIL_T, "play wavout failed");
        }

        //open wavin
        m_SoundIn.SetSoundInDataCB(DataFromSoundInEx, this);
        if (m_SoundIn.Start(&struWaveFormat, m_dwBufNum, dwBufSize))
        {
            g_pMainDlg->AddLog(-1, OPERATION_SUCC_T, "open wave in");
            m_bOpenWavIn = TRUE;
        }
        else
        {
            m_SoundIn.Stop();
            g_pMainDlg->AddLog(-1, OPERATION_FAIL_T, "open wave in failed");
        }
        GetDlgItem(IDC_TREE_DEV)->EnableWindow(TRUE);
        g_StringLanType(szLan, "停止转发", "Stop Talk MR");
        m_btTalk.SetWindowText(szLan);
    }
    else
    {
        StopSoundInAndSoundOut();
        g_StringLanType(szLan, "开始转发", "Start Talk MR");
        m_btTalk.SetWindowText(szLan);
    }
    m_bWaveDeal = !m_bWaveDeal;
}

/*********************************************************
  Function:	SendDataToDVR
  Desc:		send client audio data to device
  Input:	buf, the audio data buffer;
			dwSize, data length;
  Output:	none
  Return:	TRUE/FALSE
**********************************************************/
BOOL CDlgTalkMREx::SendDataToDVR(char *buf, DWORD dwSize)
{
    if (!m_bWaveDeal)
    {
        return FALSE;
    }
    //G722规定输入数据的大小为1280字节,编码后的输出数据大小为80字节

    //G726规定输入数据的大小为640字节,编码后的输出数据大小为80字节
    BYTE G726EncOutBuf[G726_AUDDECSIZE] = { 0 };

    //G711规定输入数据的大小为320字节, 编码后的输出数据大小为160字节
    BYTE G711EncBuf[G711_AUDDECSIZE * 2] = { 0 };
    BYTE G711EncBufA[G711_AUDDECSIZE * 2] = { 0 };

    if (m_byAudioEncType == AUDIOTALKTYPE_G711_A)
    {
#if (!defined _WIN64)
        NET_DVR_EncodeG711Frame(1, (BYTE*)buf, G711EncBufA);
#elif(defined _WIN64)
        NET_DVR_AUDIOENC_PROCESS_PARAM struEncProcParam = { 0 };
        struEncProcParam.g711_type = 1;
        struEncProcParam.in_buf = (BYTE*)buf;
        struEncProcParam.out_buf = G711EncBufA;
        NET_DVR_EncodeG711Frame(m_pG711Enc, &struEncProcParam);
#endif
    }
    else if (m_byAudioEncType == AUDIOTALKTYPE_G711_MU)
    {
#if (!defined _WIN64)
        NET_DVR_EncodeG711Frame(0, (BYTE*)buf, G711EncBuf);
#elif(defined _WIN64)
        NET_DVR_AUDIOENC_PROCESS_PARAM struEncProcParam = { 0 };
        struEncProcParam.g711_type = 0;
        struEncProcParam.in_buf = (BYTE*)buf;
        struEncProcParam.out_buf = G711EncBuf;
        NET_DVR_EncodeG711Frame(m_pG711Enc, &struEncProcParam);
#endif
    }
    else if (m_byAudioEncType == 4)
    {
        if (NULL != m_pG726EncM)
        {
            NET_DVR_EncodeG726Frame(m_pG726EncM, (BYTE*)buf, G726EncOutBuf, m_bReset);
            m_bReset = 0;
        }
    }
    else if (m_byAudioEncType == AUDIOTALKTYPE_G722)
    {
        if (NULL != m_pEncoder)
        {
#if (!defined _WIN64)
            NET_DVR_EncodeG722Frame(m_pEncoder, (BYTE*)buf, m_byEncBuf);
            //TRACE("NET_DVR_EncodeG722Frame failed\n");
            //return FALSE;
#elif (defined _WIN64)
            NET_DVR_AUDIOENC_PROCESS_PARAM struEncProcParam = { 0 };
            struEncProcParam.in_buf = (unsigned char *)buf;
            struEncProcParam.out_buf = (unsigned char *)&m_byEncBuf;
            NET_DVR_EncodeG722Frame(m_pEncoder, &struEncProcParam);
#endif

        }
    }

    int i = 0, j=0;

#ifdef RECORD 
    if (mmioWrite(m_hFile, buf, dwSize) != dwSize)
    {
        TRACE("mmioWrite failed\n");
    }
#endif

    //send to dvr
    for (i = 0; i < MAX_DEVICES; i++)
    {
        if (g_struDeviceInfo[i].lLoginID >= 0)
        {
            for (j = 0; j < g_struDeviceInfo[i].iAudioNum; j++)
            {
                if (g_struDeviceInfo[i].lVoiceCom[j] >= 0)
                {
                    if (g_struDeviceInfo[i].iAudioEncType == AUDIOTALKTYPE_G722)
                    {
                        if (!NET_DVR_VoiceComSendData(g_struDeviceInfo[i].lVoiceCom[j], (char*)m_byEncBuf, AUDDECSIZE))
                        {
                            g_pMainDlg->AddLog(i, OPERATION_FAIL_T, "NET_DVR_VoiceComSendData audioindex[%d]", j);
                        }
                    }
                    else if (g_struDeviceInfo[i].iAudioEncType == AUDIOTALKTYPE_G711_MU)
                    {
                        if (!NET_DVR_VoiceComSendData(g_struDeviceInfo[i].lVoiceCom[j], (char*)G711EncBuf, G711_AUDDECSIZE))
                        {
                            g_pMainDlg->AddLog(i, OPERATION_FAIL_T, "NET_DVR_VoiceComSendData audioindex[%d]", j);
                        }
                    }
                    else if (g_struDeviceInfo[i].iAudioEncType == AUDIOTALKTYPE_G711_A)
                    {
                        if (!NET_DVR_VoiceComSendData(g_struDeviceInfo[i].lVoiceCom[j], (char*)G711EncBufA, G711_AUDDECSIZE))
                        {
                            g_pMainDlg->AddLog(i, OPERATION_FAIL_T, "NET_DVR_VoiceComSendData audioindex[%d]", j);
                        }
                    }
                    else if (g_struDeviceInfo[i].iAudioEncType == AUDIOTALKTYPE_G726)
                    {
                        if (!NET_DVR_VoiceComSendData(g_struDeviceInfo[i].lVoiceCom[j], (char*)G726EncOutBuf, G726_AUDDECSIZE))
                        {
                            g_pMainDlg->AddLog(i, OPERATION_FAIL_T, "NET_DVR_VoiceComSendData audioindex[%d]", j);
                        }
                    }
                    else if (g_struDeviceInfo[i].iAudioEncType == AUDIOTALKTYPE_PCM)
                    {
                        if (!NET_DVR_VoiceComSendData(g_struDeviceInfo[i].lVoiceCom[j], buf, dwSize))
                        {
                            g_pMainDlg->AddLog(i, OPERATION_FAIL_T, "NET_DVR_VoiceComSendData audioindex[%d]", j);
                        }
                    }
                    //	else
                    //	{
                    //	g_struDeviceInfo[i].lVoiceCom[j] = -1;
                    //	g_pMainDlg->AddLog(i, OPERATION_SUCC_T, "NET_DVR_VoiceComSendData audioindex[%d]", j);
                    //	}			
                }
            }
        }
    }

    return TRUE; 
}


/*********************************************************
  Function:	PutIntoBuf
  Desc:		input the device audio data to buffer for decoding better/对收到的音频数据做缓冲处理
  Input:	lpTemp, voice data buffer;
			Bytes, voice data lenght;
  Output:	none
  Return:	none
**********************************************************/
BOOL CDlgTalkMREx::PutIntoBuf(char *lpTemp, int Bytes, LPTALK_MR lpTalkMR)
{
	int nTemp = 0;
	int nPacketStart = 0;
	DWORD dwAudDecSize = 0;
	if (lpTalkMR->byAudioType == AUDIOTALKTYPE_G722)
	{
		dwAudDecSize = AUDDECSIZE;
	}
	else if (lpTalkMR->byAudioType == AUDIOTALKTYPE_G726)
	{
		dwAudDecSize = G726_AUDDECSIZE;
	}
    else if (lpTalkMR->byAudioType == AUDIOTALKTYPE_PCM)
    {
        dwAudDecSize = PCM_AUDENCSIZE;
    }
    else
    {
        dwAudDecSize = G711_AUDDECSIZE;
    }

	try{
		if ((m_ReceiveIndexRV[lpTalkMR->byIndex] + Bytes) <= AUDIOBUF)
		{
			if (!m_bOpenPlayThread[lpTalkMR->byIndex])
			{
				memcpy(m_pRenderBuf[lpTalkMR->byIndex]+m_ReceiveIndexRV[lpTalkMR->byIndex], lpTemp, Bytes);
				m_ReceiveIndexRV[lpTalkMR->byIndex] += Bytes;
				//m_ReceiveIndexRV[lpTalkMR->byIndex] = m_ReceiveIndexRV[lpTalkMR->byIndex] % AUDIOBUF;
			}
			else
			{
				if (((m_ReceiveIndexRV[lpTalkMR->byIndex] + Bytes) >= m_rIndexRV[lpTalkMR->byIndex])  
					&& (m_ReceiveIndexRV[lpTalkMR->byIndex] < m_rIndexRV[lpTalkMR->byIndex]))
				{   //buffer1 overflow
					TRACE("buffer overflow.");
					//::SetEvent(m_hHaveRDataRV[lpTalkMR->byIndex]);
					nPacketStart = (m_rIndexRV[lpTalkMR->byIndex] - dwAudDecSize + m_ReceiveIndexRV[lpTalkMR->byIndex] % dwAudDecSize);
					if ((nPacketStart + Bytes) <= (DWORD)AUDIOBUF)
					{
						memcpy(m_pRenderBuf[lpTalkMR->byIndex] + nPacketStart, lpTemp, Bytes);
						m_ReceiveIndexRV[lpTalkMR->byIndex] = nPacketStart + Bytes;
					}
					else
					{
						nTemp = AUDIOBUF - nPacketStart;
						memcpy(m_pRenderBuf[lpTalkMR->byIndex] + nPacketStart, lpTemp, nTemp);
						memcpy(m_pRenderBuf[lpTalkMR->byIndex], lpTemp + nTemp, Bytes - nTemp);
						m_ReceiveIndexRV[lpTalkMR->byIndex] = Bytes - nTemp;
					}
				}
				else	
				{
					memcpy(m_pRenderBuf[lpTalkMR->byIndex] + m_ReceiveIndexRV[lpTalkMR->byIndex], lpTemp, Bytes);
					m_ReceiveIndexRV[lpTalkMR->byIndex] += Bytes;
					//m_ReceiveIndexRV[lpTalkMR->byIndex] = m_ReceiveIndexRV[lpTalkMR->byIndex] % AUDIOBUF;
				}
			}
		}
		else
		{
			if (m_bOpenPlayThread[lpTalkMR->byIndex])
			{
                if (m_ReceiveIndexRV[lpTalkMR->byIndex] + Bytes >= m_rIndexRV[lpTalkMR->byIndex] + AUDIOBUF)
                {
                    TRACE("buffer overflow.");
                    //SetEvent(m_hHaveRDataRV[lpTalkMR->byIndex]);

					if (m_rIndexRV[lpTalkMR->byIndex] != 0)
					{
						nPacketStart = (m_rIndexRV[lpTalkMR->byIndex] - dwAudDecSize + m_ReceiveIndexRV[lpTalkMR->byIndex] % dwAudDecSize);
					}
					else
					{
						nPacketStart = (m_rIndexRV[lpTalkMR->byIndex] + AUDIOBUF - dwAudDecSize + m_ReceiveIndexRV[lpTalkMR->byIndex] % dwAudDecSize);
					}
					if ((nPacketStart + Bytes) <= (DWORD)AUDIOBUF)
					{
						memcpy(m_pRenderBuf[lpTalkMR->byIndex] + nPacketStart, lpTemp, Bytes);
						m_ReceiveIndexRV[lpTalkMR->byIndex] = nPacketStart + Bytes;
					}
					else
					{
						nTemp = AUDIOBUF - nPacketStart;
                        if(nTemp < 0)
                        {
                            nTemp = 0;
                        }
						memcpy(m_pRenderBuf[lpTalkMR->byIndex] + nPacketStart, lpTemp, nTemp);
						memcpy(m_pRenderBuf[lpTalkMR->byIndex], lpTemp + nTemp, Bytes - nTemp);
						m_ReceiveIndexRV[lpTalkMR->byIndex] = Bytes - nTemp;
					}
				}
				else
				{
                    nTemp = AUDIOBUF - m_ReceiveIndexRV[lpTalkMR->byIndex];
                    if(nTemp < 0)
                    {
                        nTemp = 0;
                    }
                    memcpy(m_pRenderBuf[lpTalkMR->byIndex] + m_ReceiveIndexRV[lpTalkMR->byIndex], lpTemp, nTemp);
					memcpy(m_pRenderBuf[lpTalkMR->byIndex], lpTemp + nTemp, Bytes - nTemp);
					m_ReceiveIndexRV[lpTalkMR->byIndex] = Bytes - nTemp;
				}
			}
		}
	}
	catch(...)
	{
		return FALSE;
	}
    //	TRACE("number:%d, m_nBufNo:%d\n", (m_ReceiveIndexRV[lpTalkMR->byIndex] + AUDIOBUF - m_rIndexRV[lpTalkMR->byIndex]) % (AUDIOBUF), m_nBufNo[lpTalkMR->byIndex]*dwAudDecSize);
    // 	if ((((m_ReceiveIndexRV[lpTalkMR->byIndex] + AUDIOBUF - m_rIndexRV[lpTalkMR->byIndex]) % (AUDIOBUF)) >= (m_nBufNo[lpTalkMR->byIndex] * dwAudDecSize)))
    // 	{
    // 		::SetEvent(m_hHaveRDataRV[lpTalkMR->byIndex]);
    // 		m_nBufNo[lpTalkMR->byIndex] = 1;
    //         if (!m_bOpenPlayThread[lpTalkMR->byIndex] && m_bWaveDeal) //开启播放时再开启线程回调
    // 		{
    //             m_hPlayHandle[0] = CreateThread(NULL, 0, PlayAudioThread, lpTalkMR, 0, NULL);
    // 			m_bOpenPlayThread[lpTalkMR->byIndex] = TRUE;
    // 		}
    // 	}
    //m_nBufNo[lpTalkMR->byIndex] = 1;
    if (!m_bOpenPlayThread[lpTalkMR->byIndex] && m_bWaveDeal) //开启播放时再开启线程回调
    {
        m_hPlayHandle[lpTalkMR->byIndex] = CreateThread(NULL, 0, PlayAudioThread, lpTalkMR, 0, NULL);
        m_bOpenPlayThread[lpTalkMR->byIndex] = TRUE;
    }
    SetEvent(m_hHaveRDataRV[lpTalkMR->byIndex]);

	return TRUE;
}

/*********************************************************
  Function:	CopyAudioData
  Desc:		deal the audio data from device/从接收缓冲区中拷贝数据出来解码
  Input:	lpTemp, data buffer; dwReadLength, data length;
  Output:	none
  Return:	TRUE/FALSE
**********************************************************/
BOOL CDlgTalkMREx::CopyAudioData(PBYTE lpTemp, DWORD dwReadLength, int nIndex)
{
	DWORD dwTemp;
	int  dwPacketNumber = 0;
	int itemp = 0;
	__try
	{
		if (m_rIndexRV[nIndex] == m_ReceiveIndexRV[nIndex])
		{
			dwPacketNumber = 0;
			m_nBufNo[nIndex] = 6;
			::ResetEvent(m_hHaveRDataRV[nIndex]);
			itemp = 0;
			return FALSE;
		}
		else if (m_rIndexRV[nIndex] < m_ReceiveIndexRV[nIndex])    //read < write
		{
			if (dwReadLength > (DWORD)(m_ReceiveIndexRV[nIndex]-m_rIndexRV[nIndex]))
			{
				dwPacketNumber = 0;
				m_nBufNo[nIndex] = 6;
				::ResetEvent(m_hHaveRDataRV[nIndex]);
				itemp = 0;
				return FALSE;
			}
			else
			{
				CopyMemory(lpTemp, m_pRenderBuf[nIndex] + m_rIndexRV[nIndex], dwReadLength);
				m_rIndexRV[nIndex] += dwReadLength;
				dwPacketNumber = (m_ReceiveIndexRV[nIndex] - m_rIndexRV[nIndex]) / dwReadLength;
				itemp = 1;
			}
		}
		else	//read > write
		{
			if (dwReadLength > (DWORD)(AUDIOBUF - m_rIndexRV[nIndex]))
			{
				dwTemp = AUDIOBUF - m_rIndexRV[nIndex];
				if ((dwReadLength-dwTemp) < (DWORD)(m_ReceiveIndexRV[nIndex] + 1))
				{
					CopyMemory(lpTemp, m_pRenderBuf[nIndex] + m_rIndexRV[nIndex], dwTemp);
					CopyMemory(lpTemp + dwTemp, m_pRenderBuf[nIndex], dwReadLength - dwTemp);
					m_rIndexRV[nIndex] = dwReadLength - dwTemp;
					dwPacketNumber = (m_ReceiveIndexRV[nIndex] - m_rIndexRV[nIndex]) / dwReadLength;
					itemp = 2;
				}
				else
				{
					dwPacketNumber = 0;
					::ResetEvent(m_hHaveRDataRV[nIndex]);
					m_nBufNo[nIndex] = 6;
					itemp = 3;
					return FALSE;	
				}
			}
			else
			{
				itemp = 4;
				CopyMemory(lpTemp, m_pRenderBuf[nIndex] + m_rIndexRV[nIndex], dwReadLength);
				m_rIndexRV[nIndex] += dwReadLength;
				dwPacketNumber = (AUDIOBUF + m_ReceiveIndexRV[nIndex] - m_rIndexRV[nIndex]) / dwReadLength;
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return FALSE;
	}
	return TRUE;
}

/*********************************************************
  Function:	InputAudioData
  Desc:		input the device audio data to decoder and decode/取出音频数据并解码
  Input:	none
  Output:	none
  Return:	none
**********************************************************/
void CDlgTalkMREx::InputAudioData(LPTALK_MR lpTalkMR)
{
	HANDLE hWaitEvents[2];
	hWaitEvents[0] = m_hExitEvent;
	hWaitEvents[1] = m_hHaveRDataRV[lpTalkMR->byIndex];
	
	void*	pDecoder = NULL;//g722 decoder
	void*	pG726Dec = NULL;//g726解码句柄
    void*	pG711Dec = NULL;//g711解码句柄 win64
	void*	pG726DecM = NULL;//g726解码模块句柄

	DWORD dwReadLength = 0;
	DWORD dwPlayLength = 0;
	if (lpTalkMR->byAudioType == AUDIOTALKTYPE_G722)
	{
		dwReadLength = AUDDECSIZE;
		dwPlayLength = AUDENCSIZE;
		//decoder
#if (!defined _WIN64)
	    pDecoder = NET_DVR_InitG722Decoder(BIT_RATE_16000);
#elif (defined _WIN64)
        pDecoder = NET_DVR_InitG722Decoder();
#endif
	}
	else if (lpTalkMR->byAudioType == AUDIOTALKTYPE_G726)
	{
		dwReadLength = G726_AUDDECSIZE;
		dwPlayLength = G726_AUDENCSIZE;
		pG726Dec = NET_DVR_InitG726Decoder(&pG726DecM);
	}
    else if (lpTalkMR->byAudioType == AUDIOTALKTYPE_PCM)
    {
        dwReadLength = PCM_AUDENCSIZE;
        dwPlayLength = PCM_AUDENCSIZE;
    }
    else if (lpTalkMR->byAudioType == AUDIOTALKTYPE_MP2L2)
    {
        dwReadLength = MP2L2_AUDDECSIZE;
        dwPlayLength = MP2L2_AUDENCSIZE;
    }
	else
	{
		dwReadLength = G711_AUDDECSIZE;
		dwPlayLength = G711_AUDENCSIZE;
#if (defined _WIN64)
        pG711Dec = NET_DVR_InitG711Decoder();
#endif
	}

	BYTE  *lpTemp = NULL;
	lpTemp = new BYTE[dwReadLength];
	BYTE  *lpPlayBuf = NULL;
	lpPlayBuf = new BYTE[dwPlayLength];
	DWORD dwWaitResult = 0;
	int i = 0;
	BYTE byReset = 1;
	try
	{
        while (m_bOpenWavOut[lpTalkMR->byIndex])
        {
            dwWaitResult = WaitForMultipleObjects(2, hWaitEvents, FALSE, 500);
            if (WAIT_OBJECT_0 == dwWaitResult || WAIT_FAILED == dwWaitResult)
            {
                break;
            }
            else if (WAIT_TIMEOUT == dwWaitResult)
            {
                continue;
            }

			if (CopyAudioData(lpTemp, dwReadLength, lpTalkMR->byIndex))
			{
				if (lpTalkMR->byAudioType == AUDIOTALKTYPE_G722)
				{
					if (NULL != pDecoder)
					{
#if (!defined _WIN64)
						NET_DVR_DecodeG722Frame(pDecoder, (BYTE*)lpTemp, lpPlayBuf);
#elif (defined _WIN64)
                        NET_DVR_AUDIODEC_PROCESS_PARAM struEncProcParam = { 0 };
                        struEncProcParam.in_buf = (BYTE*)lpTemp;
                        struEncProcParam.out_buf = lpPlayBuf;
                        struEncProcParam.in_data_size = dwReadLength;
                        NET_DVR_DecodeG722Frame(pDecoder, &struEncProcParam);
#endif
					}					
				}				
				else if (lpTalkMR->byAudioType == AUDIOTALKTYPE_G711_MU)
				{
#if (!defined _WIN64)
					NET_DVR_DecodeG711Frame(0,(BYTE*)lpTemp,lpPlayBuf);
#elif (defined _WIN64)
                    NET_DVR_AUDIODEC_PROCESS_PARAM struEncProcParam = { 0 };
                    struEncProcParam.in_buf = (BYTE*)lpTemp;
                    struEncProcParam.out_buf = lpPlayBuf;
                    struEncProcParam.in_data_size = dwReadLength;
                    if (!NET_DVR_DecodeG711Frame(pG711Dec, &struEncProcParam))
                    {
                    }
#endif
				}
				else if (lpTalkMR->byAudioType == AUDIOTALKTYPE_G711_A)
				{
#if (!defined _WIN64)
					NET_DVR_DecodeG711Frame(1,(BYTE*)lpTemp,lpPlayBuf);
#elif (defined _WIN64)
                    NET_DVR_AUDIODEC_PROCESS_PARAM struEncProcParam = { 0 };
                    struEncProcParam.in_buf = (BYTE*)lpTemp;
                    struEncProcParam.out_buf = lpPlayBuf;
                    struEncProcParam.in_data_size = dwReadLength;
                    struEncProcParam.g711_type = 1;
                    NET_DVR_DecodeG711Frame(pG711Dec, &struEncProcParam);
#endif
				}
				else if (lpTalkMR->byAudioType == AUDIOTALKTYPE_G726)
				{
					if (NULL != pG726DecM)
					{
						NET_DVR_DecodeG726Frame(pG726DecM, (BYTE*)lpTemp, lpPlayBuf, byReset);
						byReset = 0;
					}
				}				
                else if (lpTalkMR->byAudioType == AUDIOTALKTYPE_PCM)
                {
                    memcpy(lpPlayBuf, lpTemp, PCM_AUDENCSIZE);
                }

				for (i=0; i<35; i++)
				{				
					if (m_SoundOut[lpTalkMR->byIndex].InputData((BYTE*)lpPlayBuf, 0))
					{
						break;
					}
					Sleep(1);
				}				
			}
			else if (m_pRenderBuf[lpTalkMR->byIndex] == NULL)
			{
                break;
			}
		}	
	}
	catch (...)
	{
		TRACE("InputAudioData exception\n");
        ExitPlayAudio(lpTemp, lpPlayBuf, pDecoder, pG726Dec, pG726DecM, pG711Dec, lpTalkMR->byIndex);
		return ;
	}

    ResetEvent(m_hExitEvent);
    ExitPlayAudio(lpTemp, lpPlayBuf, pDecoder, pG726Dec, pG726DecM, pG711Dec, lpTalkMR->byIndex);
	return ;
}

/*********************************************************
  Function:	PlayAudioThread
  Desc:		play the audio data from device
  Input:	pParam, point to current dialog
  Output:	none
  Return:	0
**********************************************************/
DWORD WINAPI CDlgTalkMREx::PlayAudioThread(LPVOID pParam)
{
	ASSERT(pParam);
//	CDlgTalkMREx * pClient = (CDlgTalkMREx *)pParam;
	try
	{
	//	pClient->InputAudioData();
		g_pDlgTalkMR->InputAudioData((LPTALK_MR)pParam);
	}
	catch (...)
	{
		TRACE("Input data exception\n");
		return 1;
	}
	
	return 0;
}

/*********************************************************
  Function:	OnBtnExit
  Desc:		exit the dialog
  Input:	none
  Output:	none
  Return:	none
**********************************************************/
void CDlgTalkMREx::OnBtnExit() 
{
    if (m_bWaveDeal)
    {
        OnButtalk();
    }

	int i=0;
	int j=0;

		
	if (m_pEncoder)
	{
		NET_DVR_ReleaseG722Encoder(m_pEncoder);
		m_pEncoder = NULL;
	}
	
	if (m_pG726Enc)
	{
		NET_DVR_ReleaseG726Encoder(m_pG726Enc);
		m_pG726Enc = NULL;
		m_pG726EncM = NULL;
	}

    if (m_pG711Enc)
    {
#if (defined _WIN64)
        NET_DVR_ReleaseG711Encoder(m_pG711Enc);
#endif
        m_pG711Enc = NULL;
        m_pG711Enc = NULL;
    }
    	
    if (INVALID_HANDLE_VALUE != m_hExitEvent)
    {
        CloseHandle(m_hExitEvent);
        m_hExitEvent = INVALID_HANDLE_VALUE;
    }

	for (i=0; i<MAX_DEVICES; i++)
	{
		if (g_struDeviceInfo[i].lLoginID >= 0)
		{
			for (j=0; j<g_struDeviceInfo[i].iAudioNum; j++)
			{
				if (g_struDeviceInfo[i].lVoiceCom[j]>=0)
				{
					if (!NET_DVR_StopVoiceCom(g_struDeviceInfo[i].lVoiceCom[j]))
					{
						g_pMainDlg->AddLog(i, OPERATION_FAIL_T, "NET_DVR_StopVoiceCom audioindex[%d]", j);
					}
					else
					{
						g_struDeviceInfo[i].lVoiceCom[j] = -1;
						g_pMainDlg->AddLog(i, OPERATION_SUCC_T, "NET_DVR_StopVoiceCom audioindex[%d]", j);
					}
					
				}
			}
		}
	}
	for (i = 0; i < MAX_SOUND_OUT; i++)
	{
		if (m_hHaveRDataRV[i])
		{
			CloseHandle(m_hHaveRDataRV[i]);	
			m_hHaveRDataRV[i] = NULL;
		}
		
		if (m_pRenderBuf[i])
		{
			//	::VirtualFree(m_pRenderBuf[i], 0, MEM_RELEASE);
			delete []m_pRenderBuf[i];
			m_pRenderBuf[i] = NULL;
		//	memset(&m_talkMr[i], 0, sizeof(TALK_MR));
			m_talkMr[i].byAudioType = 0;
			m_talkMr[i].byFlag = 0;
		}
	}

	CDialog::OnCancel();
}

/*********************************************************
  Function:	CreateTree
  Desc:		create the voice channels of all devices/创建语音通道树
  Input:	none
  Output:	none
  Return:	none
**********************************************************/
void CDlgTalkMREx::CreateTree(void)
{
	char szLan[1024] = {0};
	g_StringLanType(szLan, "所有设备", "All Devices");
	m_treeBroadCast.DeleteAllItems();
	HTREEITEM hRoot = m_treeBroadCast.InsertItem(szLan, TREE_ALL, TREE_ALL,TVI_ROOT);
	CString strTemp =_T("");
	CString strChanTmp = _T("");
	int i = 0;
	int iData =0;
	int j = 0;
	HTREEITEM hDevice = NULL;
	HTREEITEM hChanItem = NULL;
	NET_DVR_WORKSTATE_V30 struWorkState;
	for (i = 0; i < MAX_DEVICES; i++)
	{
		if (g_struDeviceInfo[i].iDeviceChanNum != -1)
		{
			strTemp.Format("%s", g_struDeviceInfo[i].chLocalNodeName);
			if (g_struDeviceInfo[i].lLoginID >= 0)
			{
				hDevice =  m_treeBroadCast.InsertItem(strTemp, DEVICE_LOGIN, DEVICE_LOGIN, hRoot);
				m_treeBroadCast.SetItemData(hDevice, DEVICE_LOGIN*1000 + g_struDeviceInfo[i].iDeviceIndex);

				memset(&struWorkState, 0, sizeof(NET_DVR_WORKSTATE_V30));
				if (!NET_DVR_GetDVRWorkState_V30(g_struDeviceInfo[i].lLoginID, &struWorkState))
				{
					g_pMainDlg->AddLog(i, OPERATION_FAIL_T, "NET_DVR_GetDVRWorkState_V30 before Talk MR");
				}
							
				for (j=0; j<g_struDeviceInfo[i].iAudioNum; j++)
				{
					if (struWorkState.byAudioChanStatus[j] == 1)
					{
						strTemp.Format("audio%d-used", j+1);
						iData = 3*1000 + struWorkState.byAudioChanStatus[j]*100 + j;
					}
					else if (struWorkState.byAudioChanStatus[j] == 0)
					{
						strTemp.Format("audio%d-not used", j+1);
						iData = 3*1000 + struWorkState.byAudioChanStatus[j]*100 + j;
					}
					else if (struWorkState.byAudioChanStatus[j] == 0xff)
					{
						strTemp.Format("audio%d", j+1);
						iData = 3*1000 + j;
					}
					 			
					hChanItem =  m_treeBroadCast.InsertItem(strTemp, 3, 3, hDevice);
					m_treeBroadCast.SetItemData(hChanItem, iData);
					//is talking, set check box
					if (g_struDeviceInfo[i].lVoiceCom[j] >= 0)
					{
						m_treeBroadCast.SetCheck(hChanItem, TRUE);
					}				
				}
				m_treeBroadCast.Expand(hDevice,TVE_EXPAND);
			}
			else
			{
				hDevice =  m_treeBroadCast.InsertItem(strTemp, DEVICE_LOGOUT, DEVICE_LOGOUT,hRoot);
				m_treeBroadCast.SetItemData(hDevice, DEVICE_LOGOUT*1000 + g_struDeviceInfo[i].iDeviceIndex);
			}	
		}
	}
	m_treeBroadCast.Expand(hRoot,TVE_EXPAND);
	m_treeBroadCast.Expand(m_treeBroadCast.GetRootItem(),TVE_EXPAND);
}

/*********************************************************
  Function:	OnClickTreeDev
  Desc:		create the device tree
  Input:	pNMHDR, point the NMHEADER which contains the click information;
			pResult, point to the result;
  Output:	none
  Return:	none
**********************************************************/
void CDlgTalkMREx::OnClickTreeDev(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CPoint pt(0,0);
	CRect rc(0,0,0,0);
	GetCursorPos(&pt);
	GetDlgItem(IDC_TREE_DEV)->GetWindowRect(&rc);
	ScreenToClient(&rc);
	ScreenToClient(&pt);
	pt.x = pt.x - rc.left;
	pt.y = pt.y - rc.top;

	UINT uFlag = 0;
	HTREEITEM hSelect= m_treeBroadCast.HitTest(pt, &uFlag);

	if (NULL == hSelect) 
	{
		return;
	}
	
	m_treeBroadCast.SelectItem(hSelect);

	BOOL bCheck = m_treeBroadCast.GetCheck(hSelect);
	bCheck = !bCheck;
	int iDevIndex = 0;
	int iType = m_treeBroadCast.GetItemData(hSelect)/1000;
	if (TREE_ALL_T == iType)
	{
		CheckAllDevState(hSelect, bCheck, uFlag);
	}
	else if (DEVICE_LOGIN == iType)
	{
		CheckAllAudioState(hSelect, bCheck, uFlag);
	}
	else if (3 == iType)
	{
		HTREEITEM hDevItem = m_treeBroadCast.GetParentItem(hSelect);
        int iAudioIndex = m_treeBroadCast.GetItemData(hSelect) % 100; //刷新状态时，有status*100，故此处应对100取余！！！
        iDevIndex = m_treeBroadCast.GetItemData(hDevItem)%1000;

		if ((m_treeBroadCast.GetItemData(hDevItem)%1000)/100 ==1)
		{
			AfxMessageBox("the audio port of device is using!");
		}
		if (uFlag != LVHT_TOLEFT )
		{
			m_treeBroadCast.SetCheck(hSelect, bCheck);
		}
		else
		{
			m_treeBroadCast.SetCheck(hSelect, !bCheck);
		}
		StartMR(iDevIndex, iAudioIndex, bCheck);
	}

	*pResult = 0;
}

/*********************************************************
  Function:	CheckAllDevState
  Desc:		check the status of all devices
  Input:	hRoot, root node;bCheck, status of the check box; uFlag, the click item flag data
  Output:	none
  Return:	none
**********************************************************/
void CDlgTalkMREx::CheckAllDevState(HTREEITEM hRoot, BOOL bCheck, int uFlag)
{
	if (uFlag == LVHT_TOLEFT)
	{
		m_treeBroadCast.SetCheck(hRoot, !bCheck);
	}
	else
	{
		m_treeBroadCast.SetCheck(hRoot, bCheck);
	}	

	HTREEITEM hChild = m_treeBroadCast.GetChildItem(hRoot);
	int iDevIndex = 0;
	while (hChild)
	{
		iDevIndex = m_treeBroadCast.GetItemData(hChild)%1000;
		
		if (g_struDeviceInfo[iDevIndex].lLoginID >= 0)
		{
			m_treeBroadCast.SetCheck(hChild, bCheck);
			CheckAllAudioState(hChild, bCheck, 0);
		}

		hChild = m_treeBroadCast.GetNextSiblingItem(hChild);
	}
}

/*********************************************************
  Function:	LoadTreeImage
  Desc:		load the tree image
  Input:	none
  Output:	none
  Return:	none
**********************************************************/
void CDlgTalkMREx::LoadTreeImage(void)
{
	CBitmap bmp[4];

	m_TreeImage.Create(16,16,ILC_COLOR32 | ILC_MASK,1,1);
	bmp[TREE_ALL].LoadBitmap(IDB_BITMAP_TREE);
	m_TreeImage.Add(&bmp[TREE_ALL],RGB(1,1,1));
	bmp[DEVICE_LOGOUT].LoadBitmap(IDB_BITMAP_LOGOUT);
	m_TreeImage.Add(&bmp[DEVICE_LOGOUT],RGB(1,1,1));
	bmp[DEVICE_LOGIN].LoadBitmap(IDB_BITMAP_LOGIN);
	m_TreeImage.Add(&bmp[DEVICE_LOGIN],RGB(1,1,1));
	bmp[3].LoadBitmap(IDB_BITMAP_AUDIO);
	m_TreeImage.Add(&bmp[3],RGB(1,1,1));
	
	m_treeBroadCast.SetImageList(&m_TreeImage, LVSIL_NORMAL);
}
 
/*********************************************************
  Function:	StartMR
  Desc:		start to create redirect transmission/开始创建语音转发通道
  Input:	iDeviceIndex,device index;
			iAudiIndex, audio index;
			bCheck, begin transmiting or not;
  Output:	
  Return:	
**********************************************************/
BOOL CDlgTalkMREx::StartMR(int iDeviceIndex, int iAudioIndex, BOOL bCheck)
{
	BOOL bRet = FALSE;
	DWORD dwUserData = iDeviceIndex*100+iAudioIndex;
	if (bCheck)
	{
		if (g_struDeviceInfo[iDeviceIndex].lVoiceCom[iAudioIndex] < 0)  
		{
			NET_DVR_COMPRESSION_AUDIO struCodeType;
			memset(&struCodeType,0,sizeof(NET_DVR_COMPRESSION_AUDIO));
			if (!NET_DVR_GetCurrentAudioCompress(g_struDeviceInfo[iDeviceIndex].lLoginID, &struCodeType))
			{
				g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_GetCurrentAudioCompress");
			}
			else
			{
                m_comAudioEncType.SetCurSel(struCodeType.byAudioEncType);
                m_byAudioEncType = struCodeType.byAudioEncType;
                if (5 == struCodeType.byAudioEncType)
                {
                    m_comAudioEncType.SetCurSel(3);
                    m_byAudioEncType = 3;
                } 
                else if (6 == struCodeType.byAudioEncType)
                {
                    m_comAudioEncType.SetCurSel(4);
                    m_byAudioEncType = 4;
                }
                else if (7 == struCodeType.byAudioEncType)
                {
                    m_comAudioEncType.SetCurSel(5);
                    m_byAudioEncType = 5;
                }
                else if (8 == struCodeType.byAudioEncType)
                {
                    m_comAudioEncType.SetCurSel(6);
                    m_byAudioEncType = 6;
                }
                SetWaveFormat(struCodeType);
				g_struDeviceInfo[iDeviceIndex].iAudioEncType = struCodeType.byAudioEncType;
				g_pMainDlg->AddLog(iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_GetCurrentAudioCompress");
			}

			int i = 0;
			for (i = 0; i < MAX_SOUND_OUT; i++)
			{
				if (m_talkMr[i].byFlag == 0)
				{
					break;
				}
			}
			if (i == MAX_SOUND_OUT)
			{
				AfxMessageBox("转发数量超过限制!");
				return FALSE;
			}
			
			//audio buffer
		//	m_pRenderBuf[i] = (PBYTE)::VirtualAlloc(NULL, 160*40L, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            m_pRenderBuf[i] = new BYTE[AUDIOBUF];
            if (m_pRenderBuf[i] == NULL)
            {
                return FALSE;
            }
			m_talkMr[i].byFlag = 1;
			m_talkMr[i].byAudioType = g_struDeviceInfo[iDeviceIndex].iAudioEncType;
			m_talkMr[i].byIndex = i;
			m_talkIndex[iDeviceIndex][iAudioIndex] = i;
			g_struDeviceInfo[iDeviceIndex].lVoiceCom[iAudioIndex] = NET_DVR_StartVoiceCom_MR_V30(g_struDeviceInfo[iDeviceIndex].lLoginID, iAudioIndex+1, fVoiceDataCallBackEx, &m_talkMr[i]);
			if (g_struDeviceInfo[iDeviceIndex].lVoiceCom[iAudioIndex] >=0 )
			{
				g_pMainDlg->AddLog(iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_StartVoiceCom_MR_V30 audio index[%d]", iAudioIndex);
                NET_DVR_LINK_ADDR struParam = { 0 };
                if (NET_DVR_GetLinkAddr(g_struDeviceInfo[iDeviceIndex].lVoiceCom[iAudioIndex], ENUM_LINK_VOICEPLAY, &struParam))
                {
                    g_pMainDlg->AddLog(iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_GetLinkAddr, dev addr[%s:%d], local addr[%s:%d]", \
                        &(struParam.uDevIP), struParam.wDevPort[0], &(struParam.uLocalIP), struParam.wLocalPort[0]);
                }
                else
                {
                    g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_GetLinkAddr");
                }
				bRet = TRUE;			
			}
			else
			{
				g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_StartVoiceCom_MR_V30 audio index[%d]", iAudioIndex);
			//	memset(&m_talkMr[i], 0, sizeof(TALK_MR));
				m_talkMr[i].byAudioType = 0;
				m_talkMr[i].byFlag = 0;
			}
		}
	}
	else
	{
		//delete device from broadcast list
		
		if (g_struDeviceInfo[iDeviceIndex].lVoiceCom[iAudioIndex] >= 0)
		{
			
			bRet = NET_DVR_StopVoiceCom(g_struDeviceInfo[iDeviceIndex].lVoiceCom[iAudioIndex]);	
			
			if (bRet)
			{
				g_pMainDlg->AddLog(iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_StopVoiceCom");
				g_struDeviceInfo[iDeviceIndex].lVoiceCom[iAudioIndex] = -1;				
			}
			else
			{
				g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_StopVoiceCom");
			}			
		}
		if (m_pRenderBuf[m_talkIndex[iDeviceIndex][iAudioIndex]])
        {
			delete []m_pRenderBuf[m_talkIndex[iDeviceIndex][iAudioIndex]];				
			//	::VirtualFree(m_pRenderBuf[m_talkIndex[iAudioIndex * iDeviceIndex]], 0, MEM_RELEASE);
			m_pRenderBuf[m_talkIndex[iDeviceIndex][iAudioIndex]] = NULL;				
		}
	//	memset(&m_talkMr[m_talkIndex[iDeviceIndex][iAudioIndex]], 0, sizeof(TALK_MR));
		m_talkMr[m_talkIndex[iDeviceIndex][iAudioIndex]].byAudioType = 0;
		m_talkMr[m_talkIndex[iDeviceIndex][iAudioIndex]].byFlag = 0;
	}

	return bRet;
}

void CDlgTalkMREx::SetWaveFormat(NET_DVR_COMPRESSION_AUDIO &struCodeType)
{
    //音频采样率
    switch (struCodeType.byAudioSamplingRate)
    {
    case 1:m_struWaveFormat.nSamplesPerSec = 16000; break;
    case 2:m_struWaveFormat.nSamplesPerSec = 32000; break;
    case 3:m_struWaveFormat.nSamplesPerSec = 48000; break;
    case 4:m_struWaveFormat.nSamplesPerSec = 44100; break;
    case 5:m_struWaveFormat.nSamplesPerSec = 8000; break;
    default:break;
    }
    ////音频码率
    //switch (struCodeType.byAudioBitRate)
    //{
    //case 1:m_struWaveFormat.wBitsPerSample = 8; break;
    //case 2:m_struWaveFormat.wBitsPerSample = 16; break;
    //case 3:m_struWaveFormat.wBitsPerSample = 32; break;
    //case 4:m_struWaveFormat.wBitsPerSample = 64; break;
    //case 5:m_struWaveFormat.wBitsPerSample = 128; break;
    //default:break; //还有其他用到再补充，参考BITRATE_ENCODE_INDEX
    //}
    m_struWaveFormat.nAvgBytesPerSec = m_struWaveFormat.nSamplesPerSec * m_struWaveFormat.nBlockAlign;
}

/*********************************************************
  Function:	CheckAllDevState
  Desc:		begin to redirect audio data 
  Input:	hDev, device node item; bCheck, add or delete
  Output:	none
  Return:	none
**********************************************************/
void CDlgTalkMREx::CheckAllAudioState(HTREEITEM hDev, BOOL bCheck, int uFlag)
{
	if (uFlag == LVHT_TOLEFT)
	{
		m_treeBroadCast.SetCheck(hDev, !bCheck);
	}
	else
	{
		m_treeBroadCast.SetCheck(hDev, bCheck);
	}	
	int iAudioIndex = 0;
	int iDevIndex = m_treeBroadCast.GetItemData(hDev)%1000;
	if (iDevIndex < 0)
	{
		return;
	}
	HTREEITEM hChild = m_treeBroadCast.GetChildItem(hDev);
	while (hChild)
	{
		iAudioIndex = m_treeBroadCast.GetItemData(hChild)%1000;
		m_treeBroadCast.SetCheck(hChild, bCheck);
		g_struDeviceInfo[iDevIndex].bCheckBroadcast[iAudioIndex] = bCheck;
		StartMR(iDevIndex, iAudioIndex, bCheck);
		hChild = m_treeBroadCast.GetNextSiblingItem(hChild);
	}
}

/*********************************************************
  Function:	OnBtnRefresh
  Desc:		refresh the state of the device audio channels
  Input:	none
  Output:	none
  Return:	none
**********************************************************/
void CDlgTalkMREx::OnBtnRefresh() 
{
	// TODO: Add your control notification handler code here
	CreateTree();
}

/***************************************************************
Function:     AudioBufDownScale
Description:  音频数据减少一倍
Input:        in:       数据输入
inputLen: 输入数据长度
Output:    out:      数据输出
Returns:      void
****************************************************************/
void CDlgTalkMREx::AudioBufDownScale(char const*in, char* out, int inbufLen) const
{
    if (m_comAudioEncType.GetCurSel() == AUDIOTALKTYPE_G711_A || m_comAudioEncType.GetCurSel() == AUDIOTALKTYPE_G711_MU)
    {
        int i = 0;
        for (i = 0; i < inbufLen; i++)
        {
            out[i] = in[i * 4];
        }
    }
    else if (m_comAudioEncType.GetCurSel() == 4)
    {
        int i = 0;
        for (i = 0; i < inbufLen; i++)
        {
            out[i] = in[i * 2];
        }
    }
    else
    {

    }
}

/***************************************************************
Function:     StopSoundInAndSoundOut
Description:  停止录音和本地解码播放
Returns:      void
****************************************************************/
void CDlgTalkMREx::StopSoundInAndSoundOut()
{
    //close wavin
    if (m_bOpenWavIn)
    {
        m_SoundIn.Stop();
        m_bOpenWavIn = FALSE;
        g_pMainDlg->AddLog(-1, OPERATION_SUCC_T, "close wave in");

#ifdef RECORD
        if (m_hFile)
        {
            ::mmioAscend(m_hFile, &m_MMCKInfoChild, 0);
            ::mmioAscend(m_hFile, &m_MMCKInfoParent, 0);
            ::mmioClose(m_hFile, 0);
            m_hFile = NULL;
        }
#endif
    }

    //close wavout
    SetEvent(m_hExitEvent);
    int i = 0;
    for (i = 0; i < MAX_SOUND_OUT; i++)
    {
        BOOL bOpenWaveOutTemp = m_bOpenWavOut[i];
        m_bOpenWavOut[i] = FALSE;
        if (m_hPlayHandle[i] != INVALID_HANDLE_VALUE)
        {
            WaitForSingleObject(m_hPlayHandle[i], INFINITE);
            m_hPlayHandle[i] = INVALID_HANDLE_VALUE;
        }

        if (bOpenWaveOutTemp)
        {
            m_SoundOut[i].CloseSound();
            g_pMainDlg->AddLog(-1, OPERATION_SUCC_T, "close wave out");
        }
    }
}

/***************************************************************
Function:     ExitPlayAudio
Description:  退出播放线程时，释放资源
Input:        
	lpTemp: 待解码数据
	lpPlayBuf: 解码后数据
	pDecoder: g722解码句柄
	pG726Dec: g726解码句柄
	pG726DecM: g726解码模块句柄
	byIndex:	线程对应的序号
Output:    
Returns:      void
****************************************************************/
void CDlgTalkMREx::ExitPlayAudio(BYTE *lpTemp, BYTE *lpPlayBuf, void *pDecoder, void *pG726Dec, void *pG726DecM, void *pG711Dec, BYTE byIndex)
{
	m_bOpenPlayThread[byIndex] = FALSE;
	if (lpTemp != NULL)
	{
		delete []lpTemp;
		lpTemp = NULL;
	}
	if (lpPlayBuf != NULL)
	{
		delete []lpPlayBuf;
		lpPlayBuf = NULL;
	}
	if (pDecoder)
	{
		NET_DVR_ReleaseG722Decoder(pDecoder);
		pDecoder = NULL;
	}
	if (pG726Dec)
	{
		NET_DVR_ReleaseG726Decoder(pG726Dec);
		pG726Dec = NULL;
		pG726DecM = NULL;
	}
    if (pG711Dec)
    {
#if (defined _WIN64)
        NET_DVR_ReleaseG711Encoder(pG711Dec);
        pG711Dec = NULL;
        pG711Dec = NULL;
#endif
    }
    
}
