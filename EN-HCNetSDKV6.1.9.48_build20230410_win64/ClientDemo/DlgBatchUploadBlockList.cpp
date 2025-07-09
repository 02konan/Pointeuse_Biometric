// DlgBatchUploadBlockList.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ClientDemo.h"
#include "DlgBatchUploadBlockList.h"
#include "afxdialogex.h"

#define WM_MSG_UPLOAD_FINISH 1002

// CDlgBatchUploadBlockList �Ի���

IMPLEMENT_DYNAMIC(CDlgBatchUploadBlockList, CDialog)

CDlgBatchUploadBlockList::CDlgBatchUploadBlockList(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgBatchUploadBlockList::IDD, pParent)
    , m_csName(_T(""))
    , m_bIsLongTermValid(FALSE)
    , m_csAddress(_T(""))
    , m_csIDNumber(_T(""))
    , m_csIssuingAuthority(_T(""))
    , m_dwNumOfBlockID(0)
{
    m_bValid = TRUE;
    m_dwSendIndex = 0;
}

CDlgBatchUploadBlockList::~CDlgBatchUploadBlockList()
{
}

void CDlgBatchUploadBlockList::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_BALCK_LIST_PICTURE_CFG, m_listViewOfBlockList);
    DDX_Text(pDX, IDC_EDIT_NAME, m_csName);
    DDX_Control(pDX, IDC_COMBO_SEX, m_comboxSex);
    DDX_Control(pDX, IDC_COMBO_NATION, m_comboxNation);
    DDX_Control(pDX, IDC_DATETIMEPICKER_BIRTH, m_dtcBirthday);
    DDX_Check(pDX, IDC_CHECK_ISVALID_LONGTERM, m_bIsLongTermValid);
    DDX_Control(pDX, IDC_DATETIMEPICKER_VALIDDATE_START, m_dtcValidPeriodStart);
    DDX_Control(pDX, IDC_DATETIMEPICKER__VALIDDATE_END, m_dtcValidPeriodEnd);
    DDX_Text(pDX, IDC_EDIT_ADDRESS, m_csAddress);
    DDX_Text(pDX, IDC_EDIT_IDNUMBER, m_csIDNumber);
    DDX_Text(pDX, IDC_EDIT_ISSUING_AUTHORITY, m_csIssuingAuthority);
    DDX_Text(pDX, IDC_EDIT_NUM_OF_BLOCK_ID, m_dwNumOfBlockID);
}


BEGIN_MESSAGE_MAP(CDlgBatchUploadBlockList, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_UPLOAD_LIST, &CDlgBatchUploadBlockList::OnBnClickedButtonUploadList)
    ON_BN_CLICKED(IDC_BUTTON_DELETE_BYID, &CDlgBatchUploadBlockList::OnBnClickedButtonDeleteByid)
    ON_BN_CLICKED(IDCANCEL, &CDlgBatchUploadBlockList::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_BUTTON_ADD, &CDlgBatchUploadBlockList::OnBnClickedButtonAdd)
    ON_BN_CLICKED(IDC_BUTTON_DELETE, &CDlgBatchUploadBlockList::OnBnClickedButtonDelete)
    ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CDlgBatchUploadBlockList::OnBnClickedButtonClear)
    ON_WM_CLOSE()
    ON_MESSAGE(WM_MSG_UPLOAD_FINISH, &CDlgBatchUploadBlockList::OnMsgUploadFinish)
END_MESSAGE_MAP()


// CDlgBatchUploadBlockList ��Ϣ�������


afx_msg LRESULT CDlgBatchUploadBlockList::OnMsgUploadFinish(WPARAM wParam, LPARAM lParam)
{
    if (!NET_DVR_StopRemoteConfig(m_lUploadCfgHandle))
    {
        g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_StopRemoteConfig failed");
        return 0;
    }
    m_lUploadCfgHandle = -1;
    g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_BULK_UPLOAD_ID_BLOCKLIST Upload finished");
    return 0;
}

void CALLBACK g_fUploadCallback(DWORD dwType, void* lpBuffer, DWORD dwBufLen, void* pUserData)
{
    CDlgBatchUploadBlockList* pDlg = (CDlgBatchUploadBlockList*)pUserData;
    if (pDlg == NULL)
    {
        return;
    }
    pDlg->ProcessUploadCfgCallbackData(dwType, lpBuffer, dwBufLen);
}

void CDlgBatchUploadBlockList::ProcessUploadCfgCallbackData(DWORD dwType, void* lpBuffer, DWORD dwBufLen)
{
    if (dwType == NET_SDK_CALLBACK_TYPE_STATUS)
    {
        DWORD dwStatus = *(DWORD*)lpBuffer;
        if (dwStatus == NET_SDK_CALLBACK_STATUS_PROCESSING)
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "Upload PROCESSING %d", m_dwSendIndex);

            SendNext();
        }
        else if (dwStatus == NET_SDK_CALLBACK_STATUS_FAILED)
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "Upload Err:NET_SDK_CALLBACK_STATUS_FAILED %d", m_dwSendIndex);

            SendNext();
        }
        //���������رճ�����
        else if (dwStatus == NET_SDK_CALLBACK_STATUS_SUCCESS)
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "Upload SUCCESS");
            PostMessage(WM_MSG_UPLOAD_FINISH, 0, 0);
        }
        else if (dwStatus == NET_SDK_CALLBACK_STATUS_EXCEPTION)
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "Upload Exception");
            PostMessage(WM_MSG_UPLOAD_FINISH, 0, 0);
        }
        else
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "Upload SUCCESS");
            PostMessage(WM_MSG_UPLOAD_FINISH, 0, 0);
        }
    }
    else if (dwType == NET_SDK_CALLBACK_TYPE_DATA)
    {
        NET_DVR_UPLOAD_ID_BLOCKLIST_STATUS struCfg = { 0 };
        memcpy(&struCfg, lpBuffer, sizeof(struCfg));


        BOOL bSendOk = FALSE;

        if (struCfg.byStatus == 3 || struCfg.byStatus == 1)
        {
            bSendOk = TRUE;
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "Upload PROCESSING,IDNum:%s", struCfg.byIDNum);
        }

        if (!bSendOk)
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "Upload Failed,CardNo:%s", struCfg.byIDNum);
        }

        SendNext();
    }
}

void CDlgBatchUploadBlockList::OnBnClickedButtonUploadList()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������

    //UTF-8����
    //std::string UTF82ASCII(const char* cont);
    //std::string ASCII2UTF8(const char* cont);

    if (m_lUploadCfgHandle != -1)
    {
        NET_DVR_StopRemoteConfig(m_lUploadCfgHandle);
    }

    UpdateData(TRUE);

    NET_DVR_UPLOAD_ID_BLOCKLIST_COND struCond = { 0 };
    struCond.dwSize = sizeof(struCond);
    struCond.dwBlockListNum = m_dwNumOfBlockID;

    m_lUploadCfgHandle = NET_DVR_StartRemoteConfig(m_lServerID, NET_DVR_BULK_UPLOAD_ID_BLOCKLIST, &struCond, sizeof(struCond), g_fUploadCallback, this);
    if (m_lUploadCfgHandle == -1)
    {
        g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_BULK_UPLOAD_ID_BLOCKLIST failed");
        return;
    }
    else
    {
        g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_BULK_UPLOAD_ID_BLOCKLIST success");
    }

    //�ϴ���һ��ͼƬ, ʧ��ʱ�ر�����
    m_dwSendIndex = 0;
    if (!SendFirst())
    {
        NET_DVR_StopRemoteConfig(m_lUploadCfgHandle);
        m_lUploadCfgHandle = -1;
    }

}

/** @fn BOOL SendFirst()
*  @brief ���͵�һ����ֹ������Ϣ
*  @return  HPR_TRUE--success, HPR_FALSE--fail.
*/
BOOL CDlgBatchUploadBlockList::SendFirst()
{
    int nItemCount = m_listViewOfBlockList.GetItemCount();

    if (nItemCount <= 0)
    {
        AfxMessageBox("Please add info to list at first");
        return FALSE;
    }

    m_dwSendIndex = 0;

    //��ȡ��һ����ֹ��������
    LPNET_DVR_UPLOAD_ID_BLOCKLIST_CFG pPictureInfo = (LPNET_DVR_UPLOAD_ID_BLOCKLIST_CFG)m_listViewOfBlockList.GetItemData(m_dwSendIndex);

    pPictureInfo->byBlockListValid = m_bValid;

    if (pPictureInfo == NULL)
    {
        return FALSE;
    }
    if (!NET_DVR_SendRemoteConfig(m_lUploadCfgHandle, ENUM_ACS_SEND_DATA, (char *)pPictureInfo, sizeof(*pPictureInfo)))
    {
        g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "Send Fail,IDNum: %s", pPictureInfo->struIDCardCfg.byIDNum);
        return FALSE;
    }

    return TRUE;
}

/** @fn BOOL SendNext()
*  @brief ������һ�Ž�ֹ������Ϣ
*  @return  HPR_TRUE--success, HPR_FALSE--fail.
*/
BOOL CDlgBatchUploadBlockList::SendNext()
{
    int nItemCount = m_listViewOfBlockList.GetItemCount();

    if (nItemCount < 0)
    {
        AfxMessageBox("Please add info to list at first");
    }

    m_dwSendIndex++;

    //�Ѿ�������
    if (m_dwSendIndex >= m_dwNumOfBlockID || m_dwSendIndex >= nItemCount)
    {
        return TRUE;
    }

    LPNET_DVR_UPLOAD_ID_BLOCKLIST_CFG pPictureInfo = (LPNET_DVR_UPLOAD_ID_BLOCKLIST_CFG)m_listViewOfBlockList.GetItemData(m_dwSendIndex);

    pPictureInfo->byBlockListValid = m_bValid;

    if (pPictureInfo == NULL)
    {
        return FALSE;
    }

    if (!NET_DVR_SendRemoteConfig(m_lUploadCfgHandle, ENUM_ACS_SEND_DATA, (char *)pPictureInfo, sizeof(*pPictureInfo)))
    {
        g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "Send Fail,IDNum: %s", pPictureInfo->struIDCardCfg.byIDNum);
        PostMessage(WM_MSG_UPLOAD_FINISH, 0, 0);
        return FALSE;
    }

    return TRUE;
}

void CDlgBatchUploadBlockList::OnBnClickedButtonDeleteByid()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    m_bValid = FALSE;
    CDlgBatchUploadBlockList::OnBnClickedButtonUploadList();
    m_bValid = TRUE;
}


void CDlgBatchUploadBlockList::OnBnClickedCancel()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    Clear();
    CDialog::OnCancel();
}

void CDlgBatchUploadBlockList::OnBnClickedButtonAdd()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    UpdateData(TRUE);

    //1.get data from control
    NET_DVR_UPLOAD_ID_BLOCKLIST_CFG struBatchIDCfg = { 0 };
    struBatchIDCfg.byBlockListValid = m_bValid;
    struBatchIDCfg.dwSize = sizeof(NET_DVR_UPLOAD_ID_BLOCKLIST_CFG);

    strncpy((char *)struBatchIDCfg.struIDCardCfg.byName, ASCII2UTF8((LPCTSTR)m_csName).c_str(), MAX_ID_NAME_LEN);
    strncpy((char *)struBatchIDCfg.struIDCardCfg.byIDNum, ASCII2UTF8((LPCTSTR)m_csIDNumber).c_str(), MAX_ID_NUM_LEN);
    strncpy((char *)struBatchIDCfg.struIDCardCfg.byAddr, ASCII2UTF8((LPCTSTR)m_csAddress).c_str(), MAX_ID_ADDR_LEN);
    strncpy((char *)struBatchIDCfg.struIDCardCfg.byIssuingAuthority, ASCII2UTF8((LPCTSTR)m_csIssuingAuthority).c_str(), MAX_ID_ISSUING_AUTHORITY_LEN);
    struBatchIDCfg.struIDCardCfg.byNation = m_comboxNation.GetCurSel();
    struBatchIDCfg.struIDCardCfg.bySex = m_comboxSex.GetCurSel();

    COleDateTime  currOleDate;
    m_dtcBirthday.GetTime(currOleDate);

    struBatchIDCfg.struIDCardCfg.struBirth.wYear = currOleDate.GetYear();
    struBatchIDCfg.struIDCardCfg.struBirth.byMonth = currOleDate.GetMonth();
    struBatchIDCfg.struIDCardCfg.struBirth.byDay = currOleDate.GetDay();

    struBatchIDCfg.struIDCardCfg.byTermOfValidity = m_bIsLongTermValid;

    m_dtcValidPeriodStart.GetTime(currOleDate);
    struBatchIDCfg.struIDCardCfg.struStartDate.wYear = currOleDate.GetYear();
    struBatchIDCfg.struIDCardCfg.struStartDate.byMonth = currOleDate.GetMonth();
    struBatchIDCfg.struIDCardCfg.struStartDate.byDay = currOleDate.GetDay();

    m_dtcValidPeriodEnd.GetTime(currOleDate);
    struBatchIDCfg.struIDCardCfg.struEndDate.wYear = currOleDate.GetYear();
    struBatchIDCfg.struIDCardCfg.struEndDate.byMonth = currOleDate.GetMonth();
    struBatchIDCfg.struIDCardCfg.struEndDate.byDay = currOleDate.GetDay();

    //2.is duplicated?
    LPNET_DVR_UPLOAD_ID_BLOCKLIST_CFG pBatchIDCfg = NULL;
    int iItemIndex = GetExistItem(&struBatchIDCfg);
    if (iItemIndex == -1)
    {
        pBatchIDCfg = new NET_DVR_UPLOAD_ID_BLOCKLIST_CFG();
        if (pBatchIDCfg == NULL)
        {
            return;
        }
        int iSize = sizeof(NET_DVR_UPLOAD_ID_BLOCKLIST_CFG);
        int nItemCount = m_listViewOfBlockList.GetItemCount();

        CString strItem = "";
        strItem.Format("%d", nItemCount + 1);
        iItemIndex = m_listViewOfBlockList.InsertItem(nItemCount, strItem);
        m_listViewOfBlockList.SetItemData(nItemCount, (DWORD)pBatchIDCfg);
    }
    else
    {
        pBatchIDCfg = (LPNET_DVR_UPLOAD_ID_BLOCKLIST_CFG)m_listViewOfBlockList.GetItemData(iItemIndex);
    }
    memcpy(pBatchIDCfg, &struBatchIDCfg, sizeof(struBatchIDCfg));

    //3.set to list
    char szLan[256] = { 0 };
    CString csTmp;
    char szName[129] = { 0 };
    char szAddress[281] = { 0 };
    char szIDNum[33] = { 0 };
    char szIssuingAuthority[129] = { 0 };

    memset(szName, 0, 129);
    memcpy(szName, (LPCTSTR)m_csName, MAX_ID_NAME_LEN);
    csTmp.Format("%s", szName);
    m_listViewOfBlockList.SetItemText(iItemIndex, 1, csTmp);

    csTmp.Format("%04d-%02d-%02d", pBatchIDCfg->struIDCardCfg.struBirth.wYear, pBatchIDCfg->struIDCardCfg.struBirth.byMonth, pBatchIDCfg->struIDCardCfg.struBirth.byDay);
    m_listViewOfBlockList.SetItemText(iItemIndex, 2, csTmp);

    memset(szAddress, 0, 281);
    memcpy(szAddress, (LPCTSTR)m_csAddress, MAX_ID_ADDR_LEN);
    csTmp.Format("%s", szAddress);
    m_listViewOfBlockList.SetItemText(iItemIndex, 3, csTmp);

    memset(szIDNum, 0, 33);
    memcpy(szIDNum, (LPCTSTR)m_csIDNumber, MAX_ID_NUM_LEN);
    csTmp.Format("%s", szIDNum);
    m_listViewOfBlockList.SetItemText(iItemIndex, 4, csTmp);

    memset(szIssuingAuthority, 0, 129);
    memcpy(szIssuingAuthority, (LPCTSTR)m_csIssuingAuthority, MAX_ID_ISSUING_AUTHORITY_LEN);
    csTmp.Format("%s", szIssuingAuthority);
    m_listViewOfBlockList.SetItemText(iItemIndex, 5, csTmp);

    csTmp.Format("%04d-%02d-%02d", pBatchIDCfg->struIDCardCfg.struStartDate.wYear, pBatchIDCfg->struIDCardCfg.struStartDate.byMonth, pBatchIDCfg->struIDCardCfg.struStartDate.byDay);
    m_listViewOfBlockList.SetItemText(iItemIndex, 6, csTmp);

    csTmp.Format("%04d-%02d-%02d", pBatchIDCfg->struIDCardCfg.struEndDate.wYear, pBatchIDCfg->struIDCardCfg.struEndDate.byMonth, pBatchIDCfg->struIDCardCfg.struEndDate.byDay);
    m_listViewOfBlockList.SetItemText(iItemIndex, 7, csTmp);

    switch (pBatchIDCfg->struIDCardCfg.byTermOfValidity)
    {
    case 0:
        g_StringLanType(szLan, "��", "No");
        csTmp.Format("%s", szLan);
        break;
    case 1:
        g_StringLanType(szLan, "��", "Yes");
        csTmp.Format("%s", szLan);
        break;
    default:
        g_StringLanType(szLan, "/", "/");
        csTmp.Format("%s", szLan);
        break;
    }
    m_listViewOfBlockList.SetItemText(iItemIndex, 8, csTmp);

    switch (pBatchIDCfg->struIDCardCfg.bySex)
    {
    case 1:
        g_StringLanType(szLan, "��", "Male");
        csTmp.Format("%s", szLan);
        break;
    case 2:
        g_StringLanType(szLan, "Ů", "Female");
        csTmp.Format("%s", szLan);
        break;
    default:
        g_StringLanType(szLan, "/", "/");
        csTmp.Format("%s", szLan);
        break;
    }
    m_listViewOfBlockList.SetItemText(iItemIndex, 9, csTmp);

    switch (pBatchIDCfg->struIDCardCfg.byNation)
    {
    case 1:
        g_StringLanType(szLan, "��", "Han");
        break;
    case 2:
        g_StringLanType(szLan, "�ɹ�", "Mongolia");
        break;
    case 3:
        g_StringLanType(szLan, "��", "Hui");
        break;
    case 4:
        g_StringLanType(szLan, "��", "Zang");
        break;
    case 5:
        g_StringLanType(szLan, "unknown", "unknown");
        break;
    case 6:
        g_StringLanType(szLan, "��", "Miao");
        break;
    case 7:
        g_StringLanType(szLan, "��", "Yi");
        break;
    case 8:
        g_StringLanType(szLan, "׳", "Zhuang");
        break;
    case 9:
        g_StringLanType(szLan, "����", "Buyei");
        break;
    case 10:
        g_StringLanType(szLan, "����", "Korean");
        break;
    case 11:
        g_StringLanType(szLan, "��", "Manchu");
        break;
    case 12:
        g_StringLanType(szLan, "��", "Dong");
        break;
    case 13:
        g_StringLanType(szLan, "��", "Yao");
        break;
    case 14:
        g_StringLanType(szLan, "��", "Bai");
        break;
    case 15:
        g_StringLanType(szLan, "����", "Tujia");
        break;
    case 16:
        g_StringLanType(szLan, "����", "Hani");
        break;
    case 17:
        g_StringLanType(szLan, "������", "Kazak");
        break;
    case 18:
        g_StringLanType(szLan, "��", "Dai");
        break;
    case 19:
        g_StringLanType(szLan, "��", "Li");
        break;
    case 20:
        g_StringLanType(szLan, "����", "Lisu");
        break;
    case 21:
        g_StringLanType(szLan, "��", "Va");
        break;
    case 22:
        g_StringLanType(szLan, "�", "She");
        break;
    case 23:
        g_StringLanType(szLan, "��ɽ", "Gaoshan");
        break;
    case 24:
        g_StringLanType(szLan, "����", "Lahu");
        break;
    case 25:
        g_StringLanType(szLan, "ˮ", "Sui");
        break;
    case 26:
        g_StringLanType(szLan, "����", "Dongxiang");
        break;
    case 27:
        g_StringLanType(szLan, "����", "Naxi");
        break;
    case 28:
        g_StringLanType(szLan, "����", "Jingpo");
        break;
    case 29:
        g_StringLanType(szLan, "�¶�����", "Kirgiz");
        break;
    case 30:
        g_StringLanType(szLan, "��", "Tu");
        break;
    case 31:
        g_StringLanType(szLan, "���Ӷ�", "Daur");
        break;
    case 32:
        g_StringLanType(szLan, "����", "Mulao");
        break;
    case 33:
        g_StringLanType(szLan, "Ǽ", "Qiang");
        break;
    case 34:
        g_StringLanType(szLan, "����", "Blang");
        break;
    case 35:
        g_StringLanType(szLan, "����", "Salar");
        break;
    case 36:
        g_StringLanType(szLan, "ë��", "Maonan");
        break;
    case 37:
        g_StringLanType(szLan, "����", "Gelao");
        break;
    case 38:
        g_StringLanType(szLan, "����", "Xibe");
        break;
    case 39:
        g_StringLanType(szLan, "����", "Achang");
        break;
    case 40:
        g_StringLanType(szLan, "����", "Pumi");
        break;
    case 41:
        g_StringLanType(szLan, "������", "Tajik");
        break;
    case 42:
        g_StringLanType(szLan, "ŭ", "Nu");
        break;
    case 43:
        g_StringLanType(szLan, "���α��", "Uzbek");
        break;
    case 44:
        g_StringLanType(szLan, "����˹", "Russians");
        break;
    case 45:
        g_StringLanType(szLan, "���¿�", "Ewenki");
        break;
    case 46:
        g_StringLanType(szLan, "�°�", "De'ang");
        break;
    case 47:
        g_StringLanType(szLan, "����", "Baoan");
        break;
    case 48:
        g_StringLanType(szLan, "ԣ��", "Yugur");
        break;
    case 49:
        g_StringLanType(szLan, "��", "Gin");
        break;
    case 50:
        g_StringLanType(szLan, "������", "Tatar");
        break;
    case 51:
        g_StringLanType(szLan, "����", "Derung");
        break;
    case 52:
        g_StringLanType(szLan, "���״�", "Oroqen");
        break;
    case 53:
        g_StringLanType(szLan, "����", "Hezhen");
        break;
    case 54:
        g_StringLanType(szLan, "�Ű�", "Monba");
        break;
    case 55:
        g_StringLanType(szLan, "���", "Lhoba");
        break;
    case 56:
        g_StringLanType(szLan, "��ŵ", "Jino");
        break;
    default:
        g_StringLanType(szLan, "δ֪", "Unknown");
        break;
    }
    csTmp.Format("%s", szLan);
    m_listViewOfBlockList.SetItemText(iItemIndex, 10, csTmp);

    UpdateData(FALSE);
}


void CDlgBatchUploadBlockList::OnBnClickedButtonDelete()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    POSITION pos = m_listViewOfBlockList.GetFirstSelectedItemPosition();

    int nItem = 0;
    int itemCount = m_listViewOfBlockList.GetItemCount();

    if (pos == NULL)
    {
        TRACE(_T("No items were selected!\n"));
    }
    else
    {
        while (pos)
        {
            nItem = m_listViewOfBlockList.GetNextSelectedItem(pos);
            TRACE(_T("Item %d was selected!\n"), nItem);
            // do own processing on nItem here

            LPNET_DVR_UPLOAD_ID_BLOCKLIST_CFG pPictureInfo = (LPNET_DVR_UPLOAD_ID_BLOCKLIST_CFG)m_listViewOfBlockList.GetItemData(nItem);
            if (pPictureInfo != NULL)
            {
                delete pPictureInfo;
                pPictureInfo = NULL;
                m_listViewOfBlockList.SetItemData(nItem, 0);
            }

            //���еĺ���item����ǰŲ��һ����λ
            for (int i = nItem + 1; i < itemCount; i++)
            {
                CString strTmp = m_listViewOfBlockList.GetItemText(i, 0);
                //CString strNow;
                strTmp.Format("%d", atoi((LPSTR)(LPCTSTR)strTmp) - 1);
                m_listViewOfBlockList.SetItemText(i, 0, strTmp);
            }

            m_listViewOfBlockList.DeleteItem(nItem);
        }
    }

}


void CDlgBatchUploadBlockList::OnBnClickedButtonClear()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    UpdateData(TRUE);
    m_listViewOfBlockList.DeleteAllItems();
    Clear();
    UpdateData(FALSE);
}


/** @fn  CDlgBatchUploadBlockList::GetExistItem(const NET_DVR_UPLOAD_ID_BLOCK_LIST_CFG *lpCardCfg)
*  @brief ���б��в����Ƿ������ͬ��ʵ��
*  @param(IN) NET_DVR_UPLOAD_ID_BLOCK_LIST_CFG *lpCardCfg
*  @return  index--the index, -1--no duplicate.
*/
int CDlgBatchUploadBlockList::GetExistItem(const NET_DVR_UPLOAD_ID_BLOCKLIST_CFG *lpCardCfg)
{
    int nItemCount = m_listViewOfBlockList.GetItemCount();
    int i = 0;
    LPNET_DVR_UPLOAD_ID_BLOCKLIST_CFG lpTemp = NULL;
    for (i = 0; i < nItemCount; i++)
    {
        lpTemp = (LPNET_DVR_UPLOAD_ID_BLOCKLIST_CFG)m_listViewOfBlockList.GetItemData(i);
        if (!lpCardCfg)
        {
            continue;
        }
        //���֤������Ψһ��ʶ�������ظ��ж�
        if (strcmp((char *)lpCardCfg->struIDCardCfg.byIDNum, (char *)lpTemp->struIDCardCfg.byIDNum) == 0)
        {
            return i;
        }
    }
    return -1;
}

/** @fn  void CDlgUploadBlockListPicture::Clear()
*  @brief ��Դ�ͷ�
*  @return  void
*/
void CDlgBatchUploadBlockList::Clear()
{
    int nItemCount = m_listViewOfBlockList.GetItemCount();
    for (int i = 0; i < nItemCount; i++)
    {
        LPNET_DVR_UPLOAD_ID_BLOCKLIST_CFG pPictureInfo = (LPNET_DVR_UPLOAD_ID_BLOCKLIST_CFG)m_listViewOfBlockList.GetItemData(i);
        if (pPictureInfo != NULL)
        {
            delete pPictureInfo;
            pPictureInfo = NULL;
            m_listViewOfBlockList.SetItemData(i, 0);
        }
    }
    return;
}

BOOL CDlgBatchUploadBlockList::OnInitDialog()
{
    CDialog::OnInitDialog();

    // TODO:  �ڴ���Ӷ���ĳ�ʼ��
    UpdateData(TRUE);
    //list
    char szLan[128] = { 0 };
    m_listViewOfBlockList.SetExtendedStyle(m_listViewOfBlockList.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES);
    g_StringLanType(szLan, "���", "No");
    m_listViewOfBlockList.InsertColumn(0, szLan, LVCFMT_CENTER, 20, -1);
    g_StringLanType(szLan, "����", "Name");
    m_listViewOfBlockList.InsertColumn(1, szLan, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLan, "��������", "Birthday");
    m_listViewOfBlockList.InsertColumn(2, szLan, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLan, "��ַ", "Address");
    m_listViewOfBlockList.InsertColumn(3, szLan, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLan, "���֤����", "Id card number");
    m_listViewOfBlockList.InsertColumn(4, szLan, LVCFMT_CENTER, 160, -1);
    g_StringLanType(szLan, "ǩ������", "Issuing authority");
    m_listViewOfBlockList.InsertColumn(5, szLan, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLan, "��Ч��ʼ����", "Effective start date");
    m_listViewOfBlockList.InsertColumn(6, szLan, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLan, "��Ч��ֹ����", "Effective deadline");
    m_listViewOfBlockList.InsertColumn(7, szLan, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLan, "�Ƿ�����Ч", "Whether long-term effective");
    m_listViewOfBlockList.InsertColumn(8, szLan, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLan, "�Ա�", "Sex");
    m_listViewOfBlockList.InsertColumn(9, szLan, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLan, "_", "National");
    m_listViewOfBlockList.InsertColumn(10, szLan, LVCFMT_CENTER, 80, -1);

    //combox
    m_comboxSex.ResetContent();
    g_StringLanType(szLan, "��Ч", "Invalid");
    m_comboxSex.AddString(szLan);
    g_StringLanType(szLan, "��", "male");
    m_comboxSex.AddString(szLan);
    g_StringLanType(szLan, "Ů", "female");
    m_comboxSex.AddString(szLan);
    m_comboxSex.SetCurSel(1);

    m_comboxNation.ResetContent();
    g_StringLanType(szLan, "δ֪", "Unknown");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "��", "Han");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "�ɹ�", "Mongolia");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "��", "Hui");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "��", "Zang");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "δ֪", "unknown");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "��", "Miao");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "��", "Yi");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "׳", "Zhuang");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "����", "Buyei");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "����", "Korean");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "��", "Manchu");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "��", "Dong");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "��", "Yao");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "��", "Bai");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "����", "Tujia");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "����", "Hani");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "������", "Kazak");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "��", "Dai");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "��", "Li");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "����", "Lisu");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "��", "Va");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "�", "She");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "��ɽ", "Gaoshan");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "����", "Lahu");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "ˮ", "Sui");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "����", "Dongxiang");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "����", "Naxi");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "����", "Jingpo");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "�¶�����", "Kirgiz");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "��", "Tu");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "���Ӷ�", "Daur");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "����", "Mulao");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "Ǽ", "Qiang");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "����", "Blang");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "����", "Salar");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "ë��", "Maonan");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "����", "Gelao");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "����", "Xibe");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "����", "Achang");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "����", "Pumi");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "������", "Tajik");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "ŭ", "Nu");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "���α��", "Uzbek");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "����˹", "Russians");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "���¿�", "Ewenki");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "�°�", "De'ang");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "����", "Baoan");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "ԣ��", "Yugur");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "��", "Gin");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "������", "Tatar");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "����", "Derung");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "���״�", "Oroqen");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "����", "Hezhen");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "�Ű�", "Monba");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "���", "Lhoba");
    m_comboxNation.AddString(szLan);
    g_StringLanType(szLan, "��ŵ", "Jino");
    m_comboxNation.AddString(szLan);

    m_comboxNation.SetCurSel(1);

    g_StringLanType(szLan, "����", "San Zhang");
    m_csName = szLan;

    g_StringLanType(szLan, "�㽭ʡ�����б��������˽ֵ�XXX��", "Hangzhou City, Zhejiang Province, Binjiang District West Hing Street No. XXX ");
    m_csAddress = szLan;

    m_csIDNumber = _T("330108199001012454");

    g_StringLanType(szLan, "�����б�����������", "Binjiang District Public Security Bureau of Hangzhou ");
    m_csIssuingAuthority = szLan;

    m_bIsLongTermValid = FALSE;

    COleDateTime oleDate;

    oleDate.SetDate(1990, 1, 1);
    m_dtcBirthday.SetTime(oleDate);

    oleDate.SetDate(2008, 6, 1);
    m_dtcValidPeriodStart.SetTime(oleDate);

    oleDate.SetDate(2018, 6, 1);
    m_dtcValidPeriodEnd.SetTime(oleDate);
    UpdateData(FALSE);
    return TRUE;  // return TRUE unless you set the focus to a control
    // �쳣:  OCX ����ҳӦ���� FALSE
}


void CDlgBatchUploadBlockList::OnClose()
{
    // TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
    Clear();
    CDialog::OnClose();
}

