// DlgUploadHd.cpp : 实现文件
//

#include "stdafx.h"
#include "ClientDemo.h"
#include "DlgUploadHd.h"
#include "afxdialogex.h"
#include "DlgUploadHdFile.h"


// CDlgUploadHd 对话框

IMPLEMENT_DYNAMIC(CDlgUploadHd, CDialogEx)

CDlgUploadHd::CDlgUploadHd(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgUploadHd::IDD, pParent)
    , m_csFilePath(_T(""))
    , m_csUploadFilePath(_T(""))
    , m_bUploadCalib(FALSE)
    , m_csDowmloadFilePath(_T(""))
{

}

CDlgUploadHd::~CDlgUploadHd()
{
    StopRealPlay();
}

void CDlgUploadHd::DoDataExchange(CDataExchange* pDX)
 {
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT, m_csFilePath);
    DDX_Text(pDX, IDC_EDIT_UPLOAD_PATH, m_csUploadFilePath);
    DDX_Text(pDX, IDC_EDIT_DOWNLOAD_PATH, m_csDowmloadFilePath);
    DDX_Control(pDX, IDC_COMBO_UPLOAD_TYPE, m_combUploadType);
    DDX_Control(pDX, IDC_COMBO_DOWNLOAD_TYPE, m_combDownloadType);
}


BEGIN_MESSAGE_MAP(CDlgUploadHd, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_FILE_PATH, &CDlgUploadHd::OnBnClickedBtnFilePath)
    ON_BN_CLICKED(IDC_BTN_FILE_UPLOAD, &CDlgUploadHd::OnBnClickedBtnFileUpload)
    ON_BN_CLICKED(IDC_BTN_SNAP, &CDlgUploadHd::OnBnClickedBtnSnap)
    ON_BN_CLICKED(IDC_BTN_FILE_PATH_UPLOAD, &CDlgUploadHd::OnBnClickedBtnFilePathUpload)
    ON_BN_CLICKED(IDC_BTN_FILE_UPLOAD_3200W, &CDlgUploadHd::OnBnClickedBtnFileUpload3200w)
    ON_BN_CLICKED(IDC_BTN_FILE_PATH_DOWNLOAD, &CDlgUploadHd::OnBnClickedBtnFilePathDownload)
    ON_BN_CLICKED(IDC_BTN_FILE_DOWNLOAD_3200W, &CDlgUploadHd::OnBnClickedBtnFileDownload3200w)
    ON_BN_CLICKED(IDC_BTN_FILE_UPLOAD_DOWNLOAD, &CDlgUploadHd::OnBnClickedBtnFileUploadDownload)
END_MESSAGE_MAP()


// CDlgUploadHd 

BOOL CDlgUploadHd::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    // TODO:  在此添加额外的初始化
    m_bUpLoading = false;
    GetDlgItem(IDC_STATIC_PLAY)->GetWindowRect(&m_rcPlayWnd);
    ScreenToClient(&m_rcPlayWnd);
    StartRealPlay(GetDlgItem(IDC_STATIC_PLAY)->GetSafeHwnd(), NULL, this);
    m_combDownloadType.SetCurSel(0);
    m_combUploadType.SetCurSel(0);
    return TRUE;  // return TRUE unless you set the focus to a control
    // 异常:  OCX 属性页应返回 FALSE
}




DWORD  GetUpLoadHDFileThread(LPVOID pParam)
{
    CDlgUploadHd *pThis = (CDlgUploadHd*)pParam;

    DWORD dwState = 0;
    DWORD dwProgress = 0;
    char szLan[256] = { 0 };


    while (TRUE)
    {
        dwState = NET_DVR_GetUploadState(pThis->m_lUploadHandle, &dwProgress);
        if (dwState == 1)
        {
            g_StringLanType(szLan, "上传成功", "Upload successfully");
            g_StringLanType(szLan, "上传", "Upload");
            pThis->GetDlgItem(IDC_BTN_FILE_UPLOAD)->SetWindowText(szLan);
            pThis->m_bUpLoading = FALSE;
            break;
        }
        else if (dwState == 2)
        {
            g_StringLanType(szLan, "正在上传,已上传:", "Is uploading,progress:");
            sprintf(szLan, "%s%d", szLan, dwProgress);
        }
        else if (dwState == 3)
        {
            g_StringLanType(szLan, "上传失败", "Upload failed");
            break;
        }
        else if (dwState == 4)
        {
            if (dwProgress == 100)
            {
                g_StringLanType(szLan, "上传成功", "Upload successfully");
                g_StringLanType(szLan, "上传", "UpLoad");
                pThis->GetDlgItem(IDC_BTN_FILE_UPLOAD)->SetWindowText(szLan);
                pThis->m_bUpLoading = FALSE;
                break;
            }
            else
            {
                g_StringLanType(szLan, "网络断开，状态未知", "Network disconnect, status unknown");
                break;
            }
        }
        if (dwState != 2 && dwState != 5)
        {
            NET_DVR_UploadClose(pThis->m_lUploadHandle);
            pThis->m_bUpLoading = FALSE;
            g_StringLanType(szLan, "上传", "UpLoad");
            pThis->GetDlgItem(IDC_BTN_FILE_UPLOAD)->SetWindowText(szLan);
            break;
        }
    }
    return FALSE;
}



void CDlgUploadHd::OnBnClickedBtnFilePath()
{
    // TODO: Add your control notification handler code here
    static char szFilter[] = "All File(*.*)|*.*||";
    CFileDialog dlg(TRUE, "*.*", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
    if (dlg.DoModal() == IDOK)
    {
        m_csFilePath = dlg.GetPathName();
        UpdateData(FALSE);
    }
}


void CDlgUploadHd::OnBnClickedBtnFileUpload()
{
    // TODO: Add your control notification handler code here
    char szLan[128] = { 0 };
    if (m_bUpLoading == FALSE)
    {
        UpdateData(TRUE);
        char szFileName[MAX_PATH];
        strcpy(szFileName, m_csFilePath);
        CFile cFile;
        if (!cFile.Open(szFileName, NULL))
        {
            g_StringLanType(szLan, "打开文件失败或无此文件", "Open file failed or no this file");
            AfxMessageBox(szLan);
            return;
        }
        DWORD dwFileSize = (DWORD)cFile.GetLength();
        if (dwFileSize == 0)
        {
            g_StringLanType(szLan, "配置文件为空", "Configure file is empty");
            AfxMessageBox(szLan);
        }
        cFile.Close();


        m_lUploadHandle = NET_DVR_UploadFile(m_lServerID, UPLOAD_HD_CAMERA_CORRECT_TABLE, NULL, NULL, szFileName);
        if (m_lUploadHandle < 0)
        {
            g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "UPLOAD_HD_CAMERA_CORRECT_TABLE");
            AfxMessageBox("UPLOAD_HD_CAMERA_CORRECT_TABLE Upload Failed");
            m_lUploadHandle = -1;
            return;
        }
        else
        {
            g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, "UPLOAD_HD_CAMERA_CORRECT_TABLE");
        }

        DWORD dwThreadId = 0;
        m_hUpLoadThread = CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(GetUpLoadHDFileThread), this, 0, &dwThreadId);
        if (m_hUpLoadThread == NULL)
        {
            char szLan[256] = { 0 };
            g_StringLanType(szLan, "打开长传文件线程失败!", "open UpLoad thread Fail!");
            AfxMessageBox(szLan);
            return;
        }
        g_StringLanType(szLan, "停止上传", "Stop UpLoad");
        GetDlgItem(IDC_BTN_FILE_UPLOAD)->SetWindowText(szLan);
        m_bUpLoading = TRUE;
    }
    else
    {
        NET_DVR_UploadClose(m_lUploadHandle);
        m_bUpLoading = FALSE;
        g_StringLanType(szLan, "上传", "UpLoad");
        GetDlgItem(IDC_BTN_FILE_UPLOAD)->SetWindowText(szLan);
    }
    UpdateData(FALSE);
}

//采用播放库PlayM4_GetJPEG接口抓图，demo预览界面中封装的播放库接口对于大分辨率的设备抓图无法实现
void CDlgUploadHd::OnBnClickedBtnSnap()
{
    LONG lWidth = 0;
    LONG lHeight = 0;
    DWORD lPrealSize = 0;
    char cFilename[256];
    CString sTemp;
    CTime time = CTime::GetCurrentTime();
    sTemp.Format("%s\\", "C:\\Picture");
    DWORD dwRet = GetFileAttributes(sTemp);
    if ((dwRet == -1) || !(dwRet & FILE_ATTRIBUTE_DIRECTORY))
    {
        CreateDirectory(sTemp, NULL);
    }
    sprintf(cFilename, "%s\\Preview.bmp", sTemp);
    int iPort = NET_DVR_GetRealPlayerIndex(m_lPlayHandle);
    if (PlayM4_GetPictureSize(iPort, &lWidth, &lHeight))
    {
        if (lWidth == 704 && (lHeight == 288 || lHeight == 240))
        {
            lHeight <<= 1;
        }
        //实际的BMP头为54字节，这里附加100字节
        LONG lSize = 4 * lWidth * lHeight + 100;
        lSize = 3 * lSize;
        byte *pPicture = new byte[lSize];
        if (pPicture == NULL)
        {
            return;
        }
        if (!PlayM4_GetJPEG(iPort, pPicture, lSize, &lPrealSize))
        {
            g_pMainDlg->AddLog(m_iDevIndex, PLAY_FAIL_T, "PlayM4_GetJPEG err[%d]", PlayM4_GetLastError(m_lPlayHandle));
            return;
        }
        DWORD dwWrittenBytes = 0;
        HANDLE hFile = CreateFile(cFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            return;
        }
        DWORD dwRet = WriteFile(hFile, pPicture, lPrealSize, &dwWrittenBytes, NULL);
        if (dwRet == 0 || dwWrittenBytes < lPrealSize)
        {

            DWORD dwError = GetLastError();
            sTemp.Format("JPEG capture fail dwWrittenBytes < lPrealSize, dwError= %d!", dwError);
            AfxMessageBox(sTemp);
        }
        CloseHandle(hFile);
        if (pPicture != NULL)
        {
            delete[]pPicture;
            pPicture = NULL;
        }
        CString sTempMsg;
        sTempMsg.Format("JPEG capture succ %s!", cFilename);
        g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, "PlayM4_GetJPEG file[%s]", cFilename);
        AfxMessageBox(sTempMsg);
    }
    else
    {
        g_pMainDlg->AddLog(m_iDevIndex, PLAY_FAIL_T, "PlayM4_GetPictureSize err[%d]", PlayM4_GetLastError(m_lPlayHandle));
        return;
    }
}


void CDlgUploadHd::OnBnClickedBtnFilePathUpload()
{
    // TODO:  在此添加控件通知处理程序代码
    static char szFilter[] = "All File(*.*)|*.*||";
    CFileDialog dlg(TRUE, "*.*", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
    if (dlg.DoModal() == IDOK)
    {
        m_csUploadFilePath = dlg.GetPathName();
        UpdateData(FALSE);
    }
}


void CDlgUploadHd::OnBnClickedBtnFileUpload3200w()
{
    // TODO:  在此添加控件通知处理程序代码
    char szLan[128] = { 0 };
    if (m_bUpLoading == FALSE)
    {
        UpdateData(TRUE);
        char szFileName[MAX_PATH];
        strcpy(szFileName, m_csUploadFilePath);
        CFile cFile;
        if (!cFile.Open(szFileName, NULL))
        {
            g_StringLanType(szLan, "打开文件失败或无此文件", "Open file failed or no this file");
            AfxMessageBox(szLan);
            return;
        }
        DWORD dwFileSize = (DWORD)cFile.GetLength();
        if (dwFileSize == 0)
        {
            g_StringLanType(szLan, "配置文件为空", "Configure file is empty");
            AfxMessageBox(szLan);
        }
        cFile.Close();

        NET_SDK_UPLOAD_TYPE fileType;
        CString csUploadType;
        switch (m_combUploadType.GetCurSel())
        {
        case 0:
            fileType = UPLOAD_HD_CAMERA_CORRECT_TABLE_3200W;
            csUploadType = "UPLOAD_HD_CAMERA_CORRECT_TABLE_3200W";
            break;
        case 1:
            fileType = UPLOAD_CLIENT_CALIBFILE;
            csUploadType = "UPLOAD_CLIENT_CALIBFILE";
            break;
        case 2:
            fileType = UPLOAD_APP_FILE;
            csUploadType = "UPLOAD_APP_FILE";
            break;
        case 3:
            fileType = UPLOAD_LENS_PARAM_FILE;
            csUploadType = "UPLOAD_LENS_PARAM_FILE";
            break;
        default:
            fileType = UPLOAD_HD_CAMERA_CORRECT_TABLE_3200W;
            csUploadType = "UPLOAD_HD_CAMERA_CORRECT_TABLE_3200W";
            break;
        }

        m_lUploadHandle = NET_DVR_UploadFile(m_lServerID, fileType, NULL, NULL, szFileName);
        if (m_lUploadHandle < 0)
        {
            g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, csUploadType);
            AfxMessageBox("UPLOAD_HD_CAMERA_CORRECT_TABLE Upload Failed");
            m_lUploadHandle = -1;
            return;
        }
        else
        {
            g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, csUploadType);
        }

        DWORD dwThreadId = 0;
        m_hUpLoadThread = CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(GetUpLoadHDFileThread), this, 0, &dwThreadId);
        if (m_hUpLoadThread == NULL)
        {
            char szLan[256] = { 0 };
            g_StringLanType(szLan, "打开长传文件线程失败!", "open UpLoad thread Fail!");
            AfxMessageBox(szLan);
            return;
        }
        g_StringLanType(szLan, "停止上传", "Stop UpLoad");
        m_bUpLoading = TRUE;
    }
    else
    {
        NET_DVR_UploadClose(m_lUploadHandle);
        m_bUpLoading = FALSE;
        g_StringLanType(szLan, "上传", "UpLoad");
    }
    UpdateData(FALSE);
}


void CDlgUploadHd::OnBnClickedBtnFilePathDownload()
{
    // TODO:  在此添加控件通知处理程序代码
    CString strFilter = _T("All Files(*.*)|*.*||");
    CFileDialog fileChose(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter);
    if (fileChose.DoModal() != IDOK)
    {
        return;
    }
    m_csDowmloadFilePath = fileChose.GetPathName();
    GetDlgItem(IDC_EDIT_DOWNLOAD_PATH)->SetWindowText(m_csDowmloadFilePath);
}


void CDlgUploadHd::OnBnClickedBtnFileDownload3200w()
{
    // TODO:  在此添加控件通知处理程序代码
    UpdateData(TRUE);

    char szLan[128] = { 0 };
    NET_SDK_DOWNLOAD_TYPE fileType;
    CString csDownloadType;
    switch (m_combDownloadType.GetCurSel())
    {
    case 0:
        fileType = NET_SDK_DOWNLOAD_DEVTYPE_CALIBFILE;
        csDownloadType = "NET_SDK_DOWNLOAD_DEVTYPE_CALIBFILE";
        break;
    case 1:
        fileType = NET_SDK_DOWNLOAD_HD_CAMERA_CORRECT_TABLE;
        csDownloadType = "NET_SDK_DOWNLOAD_HD_CAMERA_CORRECT_TABLE";
        break;
    case 2:
        fileType = NET_SDK_DOWNLOAD_CLIENT_CALIBFILE;
        csDownloadType = "NET_SDK_DOWNLOAD_CLIENT_CALIBFILE";
        break;
    case 3:
        fileType = NET_SDK_DOWNLOAD_FOUE_CAMERAS_PICTURES;
        csDownloadType = "NET_SDK_DOWNLOAD_FOUE_CAMERAS_PICTURES";
        break;
    case 4:
        fileType = NET_SDK_DOWNLOAD_LENS_PARAM_FILE;
        csDownloadType = "NET_SDK_DOWNLOAD_LENS_PARAM_FILE";
        break;
    default:
        fileType = NET_SDK_DOWNLOAD_DEVTYPE_CALIBFILE;
        csDownloadType = "NET_SDK_DOWNLOAD_DEVTYPE_CALIBFILE";
        break;
    }
 
    m_lDownloadHandle = NET_DVR_StartDownload(m_lServerID, fileType, NULL, 0, m_csDowmloadFilePath.GetBuffer(0));
    if (m_lDownloadHandle == -1)
    {
        g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, csDownloadType);
        AfxMessageBox("NET_DVR_StartDownload failed");
    }
    else
    {
        g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, csDownloadType);
    }
    UpdateData(FALSE);
}


void CDlgUploadHd::OnBnClickedBtnFileUploadDownload()
{
    // TODO:  在此添加控件通知处理程序代码
    CDlgUploadHdFile dlg;
    dlg.m_lServerID = m_lServerID;
    dlg.m_iDevIndex = m_iDevIndex;
    dlg.m_lChannel = m_lChannel;
    dlg.DoModal();
}
