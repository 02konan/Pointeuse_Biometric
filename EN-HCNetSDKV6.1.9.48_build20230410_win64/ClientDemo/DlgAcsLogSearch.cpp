// DlgAcsLogSearch.cpp : 实现文件
//

#include "stdafx.h"
#include "ClientDemo.h"
#include "DlgAcsLogSearch.h"
#include "afxdialogex.h"

void CALLBACK g_fGetAcsEventCallback(DWORD dwType, void* lpBuffer, DWORD dwBufLen, void* pUserData);
#define WM_MSG_ADD_ACS_EVENT_TOLIST 1002
#define WM_MSG_GET_ACS_EVENT_FINISH 1003

const char* szAcs[] = { "invalid", "authenticated", "authenticationFailed", "openingDoor", "closingDoor", "doorException",
"remoteOperation", "timeSynchronization", "deviceException", "deviceRecovered", "alarmTriggered",
"alarmRecovered", "callCenter" };


// CDlgAcsLogSearch 对话框
CDlgAcsLogSearch * pLogSearch = NULL;

IMPLEMENT_DYNAMIC(CDlgAcsLogSearch, CDialog)

CDlgAcsLogSearch::CDlgAcsLogSearch(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgAcsLogSearch::IDD, pParent)
    , m_iChannel(0)
    , m_strMonitorId(_T(""))
{
    memset(&m_struAcsEventCond, 0, sizeof(m_struAcsEventCond));
    m_bPictureEnable = FALSE;
    m_csCardNo = _T("");
    m_csEmployeeNo = _T("");
    m_csName = _T("");

    m_startDate = COleDateTime::GetCurrentTime();
    m_startTime = COleDateTime::GetCurrentTime();
    m_stopDate = COleDateTime::GetCurrentTime();
    m_stopTime = COleDateTime::GetCurrentTime();

    m_lGetAcsEvent = -1;
    m_lLogNum = 0;

    m_dwBeginSerialNo = 0;
    m_dwEndSerialNo = 0;
}

CDlgAcsLogSearch::~CDlgAcsLogSearch()
{
}

void CDlgAcsLogSearch::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_MAJOR_TYPE, m_cmbMajorType);
    DDX_Control(pDX, IDC_COMBO_MINOR_TYPE, m_cmbMinorType);
    DDX_Control(pDX, IDC_COMBO_TIME_TYPE, m_cmbTimeType);
    DDX_DateTimeCtrl(pDX, IDC_DATE_LOG_START, m_startDate);
    DDX_DateTimeCtrl(pDX, IDC_TIME_LOG_START, m_startTime);
    DDX_DateTimeCtrl(pDX, IDC_DATE_LOG_STOP, m_stopDate);
    DDX_DateTimeCtrl(pDX, IDC_TIME_LOG_STOP, m_stopTime);
    DDX_Control(pDX, IDC_LIST_DEVICE_LOG, m_listDeviceLog);
    DDX_Text(pDX, IDC_EDIT_ACS_CARD_NO, m_csCardNo);
    DDX_Text(pDX, IDC_EDIT_ACS_EMPLOYEE_NO, m_csEmployeeNo);
    DDX_Text(pDX, IDC_EDIT_ACS_NAME, m_csName);
    DDX_Check(pDX, IDC_CHK_ACS_PICTURE, m_bPictureEnable);
    DDX_Text(pDX, IDC_EDIT_BEGIN_SERIAL_NO, m_dwBeginSerialNo);
    DDX_Text(pDX, IDC_EDIT_END_SERIAL_NO, m_dwEndSerialNo);
    DDX_Control(pDX, IDC_COMBO_INDUCTIVE_EVENT_TYPE, m_comInductiveEventType);
    DDX_Control(pDX, IDC_COMBO_SEARCH_TYPE, m_comSearchType);
    DDX_Text(pDX, IDC_EDIT_CHANNEL, m_iChannel);
    DDX_Text(pDX, IDC_EDIT_MONITOR_ID, m_strMonitorId);
    DDX_Control(pDX, IDC_COM_EVENT_ATTRIBUTE, m_comEventAttribute);
}

BOOL CDlgAcsLogSearch::OnInitDialog()
{
    CDialog::OnInitDialog();

    pLogSearch = this;

    m_cmbTimeType.SetCurSel(0);

    m_cmbMajorType.ResetContent();
    m_cmbMajorType.AddString("All");
    m_cmbMajorType.SetItemData(0, 0);
    m_cmbMajorType.AddString("Alarm");
    m_cmbMajorType.SetItemData(1, 1);
    m_cmbMajorType.AddString("Exception");
    m_cmbMajorType.SetItemData(2, 2);
    m_cmbMajorType.AddString("Operation");
    m_cmbMajorType.SetItemData(3, 3);
    m_cmbMajorType.AddString("Event");
    m_cmbMajorType.SetItemData(4, 5);

    m_cmbMinorType.ResetContent();
    int iSel = 0;
    iSel = AddMinorAllString(iSel);
    iSel = AddMinorAlarmString(iSel);
    iSel = AddMinorExceptionString(iSel);
    iSel = AddMinorOperationString(iSel);
    iSel = AddMinorEventString(iSel);

    m_cmbMajorType.SetCurSel(0);
    m_cmbMinorType.SetCurSel(0);

    char szLanTemp[128] = { 0 };
    m_listDeviceLog.SetExtendedStyle(m_listDeviceLog.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES);
    g_StringLanType(szLanTemp, "序号", "No");
    m_listDeviceLog.InsertColumn(0, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "日志时间", "Log Time");
    m_listDeviceLog.InsertColumn(1, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "主类型", "Major Type");
    m_listDeviceLog.InsertColumn(2, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "次类型", "Minor Type");
    m_listDeviceLog.InsertColumn(3, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "卡号", "Card No");
    m_listDeviceLog.InsertColumn(4, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "卡类型", "Card Type");
    m_listDeviceLog.InsertColumn(5, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "允许名单单号", "Allow List No");
    m_listDeviceLog.InsertColumn(6, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "报告上传通道", "Report Channel");
    m_listDeviceLog.InsertColumn(7, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "读卡器种类", "Card Reader Kind");
    m_listDeviceLog.InsertColumn(8, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "读卡器编号", "Card Reader No");
    m_listDeviceLog.InsertColumn(9, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "门编号", "Door No");
    m_listDeviceLog.InsertColumn(10, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "多重卡认证序号", "Verify No");
    m_listDeviceLog.InsertColumn(11, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "报警输入号", "Alarm In No");
    m_listDeviceLog.InsertColumn(12, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "报警输出号", "Alarm Out No");
    m_listDeviceLog.InsertColumn(13, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "事件触发器编号", "Case Sensor No");
    m_listDeviceLog.InsertColumn(14, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "RS485通道号", "RS485 No");
    m_listDeviceLog.InsertColumn(15, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "群组编号", "Multi Card Group No");
    m_listDeviceLog.InsertColumn(16, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "人员通道号", "Access Channel");
    m_listDeviceLog.InsertColumn(17, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "设备编号", "Device No");
    m_listDeviceLog.InsertColumn(18, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "工号", "Employee No");
    m_listDeviceLog.InsertColumn(19, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "分控器编号", "Distract Control No");
    m_listDeviceLog.InsertColumn(20, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "就地控制器编号", "Local Controller ID");
    m_listDeviceLog.InsertColumn(21, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "网口ID", "InterNet Access");
    m_listDeviceLog.InsertColumn(22, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "防区类型", "Type");
    m_listDeviceLog.InsertColumn(23, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "物理地址", "Mac Addr");
    m_listDeviceLog.InsertColumn(24, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "刷卡类型", "Swipe Card Type");
    m_listDeviceLog.InsertColumn(25, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "事件流水号", "Serial No");
    m_listDeviceLog.InsertColumn(26, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "通道控制器ID", "Channel Controller ID");
    m_listDeviceLog.InsertColumn(27, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "通道控制器灯板ID", "Channel Controller Lamp ID");
    m_listDeviceLog.InsertColumn(28, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "通道控制器红外转接板ID", "Channel Controller IR Adaptor ID");
    m_listDeviceLog.InsertColumn(29, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "通道控制器红外对射ID", "Channel Controller IR Emitter ID");
    m_listDeviceLog.InsertColumn(30, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "归纳事件类型", "Inductive Event Type");
    m_listDeviceLog.InsertColumn(31, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "录像通道号", "Record Channel");
    m_listDeviceLog.InsertColumn(32, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "人员类型", "User Type");
    m_listDeviceLog.InsertColumn(33, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "读卡器当前验证方式", "Current Verify Mode");
    m_listDeviceLog.InsertColumn(34, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "考勤状态", "Attendance Status");
    m_listDeviceLog.InsertColumn(35, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "考勤状态值", "StatusValue");
    m_listDeviceLog.InsertColumn(36, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "工号（人员ID）", "Employee No String");
    m_listDeviceLog.InsertColumn(37, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "远程主机地址", "Remote Host Addr");
    m_listDeviceLog.InsertColumn(38, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "网络操作的用户名", "Net User");
    m_listDeviceLog.InsertColumn(39, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "事件属性", "Event Attribute");
    m_listDeviceLog.InsertColumn(40, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "是否带口罩", "Mask");
    m_listDeviceLog.InsertColumn(41, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "测温单位", "ThermometryUnit");
    m_listDeviceLog.InsertColumn(42, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "是否温度异常", "IsAbnomalTemperature");
    m_listDeviceLog.InsertColumn(43, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "人脸温度", "CurrTemperature");
    m_listDeviceLog.InsertColumn(44, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "人脸温度坐标", "RegionCoordinates");
    m_listDeviceLog.InsertColumn(45, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "人脸框信息", "Face Coordinates");
    m_listDeviceLog.InsertColumn(46, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "健康码信息", "Health Code");
    m_listDeviceLog.InsertColumn(47, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "核酸检测信息", "NAD Info");
    m_listDeviceLog.InsertColumn(48, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "行程信息", "Travel Info");
    m_listDeviceLog.InsertColumn(49, szLanTemp, LVCFMT_CENTER, 80, -1);
    g_StringLanType(szLanTemp, "打疫苗状态", "Vaccine Status");
    m_listDeviceLog.InsertColumn(50, szLanTemp, LVCFMT_CENTER, 80, -1);

    m_comInductiveEventType.ResetContent();

    int iIndex = 0;
    for (iIndex = 0; iIndex < sizeof(szAcs) / sizeof(char*); iIndex++)
    {
        m_comInductiveEventType.InsertString(iIndex, szAcs[iIndex]);
        m_comInductiveEventType.SetItemData(iIndex, iIndex);
    }
    m_comInductiveEventType.InsertString(iIndex, "all");
    m_comInductiveEventType.SetItemData(iIndex, 0xffff);

    m_comSearchType.ResetContent();
    g_StringLanType(szLanTemp, "无效", "Invalid");
    m_comSearchType.InsertString(0, szLanTemp);
    g_StringLanType(szLanTemp, "按事件源搜索", "Event source");
    m_comSearchType.InsertString(1, szLanTemp);
    g_StringLanType(szLanTemp, "按布防点ID搜索", "Monitor ID");
    m_comSearchType.InsertString(2, szLanTemp);

    m_comEventAttribute.ResetContent();
    g_StringLanType(szLanTemp, "无效", "Invalid");
    m_comEventAttribute.InsertString(0, szLanTemp);
    g_StringLanType(szLanTemp, "合法事件", "attendance");
    m_comEventAttribute.InsertString(1, szLanTemp);
    g_StringLanType(szLanTemp, "其它", "other");
    m_comEventAttribute.InsertString(2, szLanTemp);
    m_comEventAttribute.SetCurSel(0);

    return TRUE;
}

int CDlgAcsLogSearch::AddMinorAllString(int iStartSel)
{
    int iSel = iStartSel;

    m_cmbMinorType.AddString("All");
    m_cmbMinorType.SetItemData(iSel, 0);
    iSel++;

    return iSel;
}

int CDlgAcsLogSearch::AddMinorAlarmString(int iStartSel)
{
    CString szTemp;
    int iSel = iStartSel;

    szTemp.Format("ALARMIN_SHORT_CIRCUIT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_ALARMIN_SHORT_CIRCUIT);
    iSel++;

    szTemp.Format("ALARMIN_BROKEN_CIRCUIT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_ALARMIN_BROKEN_CIRCUIT);
    iSel++;

    szTemp.Format("ALARMIN_EXCEPTION");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_ALARMIN_EXCEPTION);
    iSel++;

    szTemp.Format("ALARMIN_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_ALARMIN_RESUME);
    iSel++;

    szTemp.Format("HOST_DESMANTLE_ALARM");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_HOST_DESMANTLE_ALARM);
    iSel++;

    szTemp.Format("HOST_DESMANTLE_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_HOST_DESMANTLE_RESUME);
    iSel++;

    szTemp.Format("CARD_READER_DESMANTLE_ALARM");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CARD_READER_DESMANTLE_ALARM);
    iSel++;

    szTemp.Format("CARD_READER_DESMANTLE_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CARD_READER_DESMANTLE_RESUME);
    iSel++;

    szTemp.Format("CASE_SENSOR_ALARM");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CASE_SENSOR_ALARM);
    iSel++;

    szTemp.Format("CASE_SENSOR_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CASE_SENSOR_RESUME);
    iSel++;

    szTemp.Format("STRESS_ALARM");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_STRESS_ALARM);
    iSel++;

    szTemp.Format("OFFLINE_ECENT_NEARLY_FULL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_OFFLINE_ECENT_NEARLY_FULL);
    iSel++;

    szTemp.Format("CARD_MAX_AUTHENTICATE_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CARD_MAX_AUTHENTICATE_FAIL);
    iSel++;

    szTemp.Format("SD_CARD_FULL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_SD_CARD_FULL);
    iSel++;

    szTemp.Format("LINKAGE_CAPTURE_PIC");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LINKAGE_CAPTURE_PIC);
    iSel++;

    szTemp.Format("SECURITY_MODULE_DESMANTLE_ALARM");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_SECURITY_MODULE_DESMANTLE_ALARM);
    iSel++;

    szTemp.Format("SECURITY_MODULE_DESMANTLE_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_SECURITY_MODULE_DESMANTLE_RESUME);
    iSel++;

    szTemp.Format("POS_START_ALARM");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_POS_START_ALARM);
    iSel++;

    szTemp.Format("POS_END_ALARM");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_POS_END_ALARM);
    iSel++;

    szTemp.Format("FACE_IMAGE_QUALITY_LOW");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_FACE_IMAGE_QUALITY_LOW);
    iSel++;

    szTemp.Format("FINGE_RPRINT_QUALITY_LOW");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_FINGE_RPRINT_QUALITY_LOW);
    iSel++;

    szTemp.Format("FIRE_IMPORT_BROKEN_CIRCUIT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_FIRE_IMPORT_BROKEN_CIRCUIT);
    iSel++;

    szTemp.Format("FIRE_IMPORT_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_FIRE_IMPORT_RESUME);
    iSel++;

    szTemp.Format("FIRE_BUTTON_TRIGGER");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_FIRE_BUTTON_TRIGGER);
    iSel++;

    szTemp.Format("FIRE_BUTTON_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_FIRE_BUTTON_RESUME);
    iSel++;

    szTemp.Format("MAINTENANCE_BUTTON_TRIGGER");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_MAINTENANCE_BUTTON_TRIGGER);
    iSel++;

    szTemp.Format("MAINTENANCE_BUTTON_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_MAINTENANCE_BUTTON_RESUME);
    iSel++;

    szTemp.Format("EMERGENCY_BUTTON_TRIGGER");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_EMERGENCY_BUTTON_TRIGGER);
    iSel++;

    szTemp.Format("EMERGENCY_BUTTON_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_EMERGENCY_BUTTON_RESUME);
    iSel++;

    szTemp.Format("DISTRACT_CONTROLLER_ALARM");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_DISTRACT_CONTROLLER_ALARM);
    iSel++;

    szTemp.Format("DISTRACT_CONTROLLER_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_DISTRACT_CONTROLLER_RESUME);
    iSel++;

    szTemp.Format("CHANNEL_CONTROLLER_DESMANTLE_ALARM");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CHANNEL_CONTROLLER_DESMANTLE_ALARM);
    iSel++;

    szTemp.Format("CHANNEL_CONTROLLER_DESMANTLE_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CHANNEL_CONTROLLER_DESMANTLE_RESUME);
    iSel++;

    szTemp.Format("CHANNEL_CONTROLLER_FIRE_IMPORT_ALARM");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CHANNEL_CONTROLLER_FIRE_IMPORT_ALARM);
    iSel++;

    szTemp.Format("CHANNEL_CONTROLLER_FIRE_IMPORT_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CHANNEL_CONTROLLER_FIRE_IMPORT_RESUME);
    iSel++;

    szTemp.Format("PRINTER_OUT_OF_PAPER");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_PRINTER_OUT_OF_PAPER);
    iSel++;

    szTemp.Format("LEGAL_EVENT_NEARLY_FULL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LEGAL_EVENT_NEARLY_FULL);
    iSel++;

    szTemp.Format("TRANSACTION_RECORD_NEARLY_FULL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_TRANSACTION_RECORD_NEARLY_FULL);
    iSel++;
    return iSel;
}

int CDlgAcsLogSearch::AddMinorExceptionString(int iStartSel)
{
    CString szTemp;
    int iSel = iStartSel;

    szTemp.Format("NET_BROKEN");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_NET_BROKEN);
    iSel++;

    szTemp.Format("RS485_DEVICE_ABNORMAL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_RS485_DEVICE_ABNORMAL);
    iSel++;

    szTemp.Format("RS485_DEVICE_REVERT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_RS485_DEVICE_REVERT);
    iSel++;

    szTemp.Format("DEV_POWER_ON");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_DEV_POWER_ON);
    iSel++;

    szTemp.Format("DEV_POWER_OFF");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_DEV_POWER_OFF);
    iSel++;

    szTemp.Format("WATCH_DOG_RESET");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_WATCH_DOG_RESET);
    iSel++;

    szTemp.Format("LOW_BATTERY");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LOW_BATTERY);
    iSel++;

    szTemp.Format("BATTERY_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_BATTERY_RESUME);
    iSel++;

    szTemp.Format("AC_OFF");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_AC_OFF);
    iSel++;

    szTemp.Format("AC_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_AC_RESUME);
    iSel++;

    szTemp.Format("NET_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_NET_RESUME);
    iSel++;

    szTemp.Format("FLASH_ABNORMAL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_FLASH_ABNORMAL);
    iSel++;

    szTemp.Format("CARD_READER_OFFLINE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CARD_READER_OFFLINE);
    iSel++;

    szTemp.Format("CARD_READER_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CARD_READER_RESUME);
    iSel++;

    szTemp.Format("INDICATOR_LIGHT_OFF");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_INDICATOR_LIGHT_OFF);
    iSel++;

    szTemp.Format("INDICATOR_LIGHT_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_INDICATOR_LIGHT_RESUME);
    iSel++;

    szTemp.Format("CHANNEL_CONTROLLER_OFF");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CHANNEL_CONTROLLER_OFF);
    iSel++;

    szTemp.Format("CHANNEL_CONTROLLER_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CHANNEL_CONTROLLER_RESUME);
    iSel++;

    szTemp.Format("SECURITY_MODULE_OFF");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_SECURITY_MODULE_OFF);
    iSel++;

    szTemp.Format("SECURITY_MODULE_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_SECURITY_MODULE_RESUME);
    iSel++;

    szTemp.Format("BATTERY_ELECTRIC_LOW");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_BATTERY_ELECTRIC_LOW);
    iSel++;

    szTemp.Format("BATTERY_ELECTRIC_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_BATTERY_ELECTRIC_RESUME);
    iSel++;

    szTemp.Format("LOCAL_CONTROL_NET_BROKEN");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LOCAL_CONTROL_NET_BROKEN);
    iSel++;

    szTemp.Format("LOCAL_CONTROL_NET_RSUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LOCAL_CONTROL_NET_RSUME);
    iSel++;

    szTemp.Format("MASTER_RS485_LOOPNODE_BROKEN");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_MASTER_RS485_LOOPNODE_BROKEN);
    iSel++;

    szTemp.Format("MASTER_RS485_LOOPNODE_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_MASTER_RS485_LOOPNODE_RESUME);
    iSel++;

    szTemp.Format("LOCAL_CONTROL_OFFLINE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LOCAL_CONTROL_OFFLINE);
    iSel++;

    szTemp.Format("LOCAL_CONTROL_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LOCAL_CONTROL_RESUME);
    iSel++;

    szTemp.Format("LOCAL_DOWNSIDE_RS485_LOOPNODE_BROKEN");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LOCAL_DOWNSIDE_RS485_LOOPNODE_BROKEN);
    iSel++;

    szTemp.Format("LOCAL_DOWNSIDE_RS485_LOOPNODE_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LOCAL_DOWNSIDE_RS485_LOOPNODE_RESUME);
    iSel++;

    szTemp.Format("DISTRACT_CONTROLLER_ONLINE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_DISTRACT_CONTROLLER_ONLINE);
    iSel++;

    szTemp.Format("DISTRACT_CONTROLLER_OFFLINE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_DISTRACT_CONTROLLER_OFFLINE);
    iSel++;

    szTemp.Format("ID_CARD_READER_NOT_CONNECT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_ID_CARD_READER_NOT_CONNECT);
    iSel++;

    szTemp.Format("ID_CARD_READER_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_ID_CARD_READER_RESUME);
    iSel++;

    szTemp.Format("FINGER_PRINT_MODULE_NOT_CONNECT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_FINGER_PRINT_MODULE_NOT_CONNECT);
    iSel++;

    szTemp.Format("FINGER_PRINT_MODULE_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_FINGER_PRINT_MODULE_RESUME);
    iSel++;

    szTemp.Format("CAMERA_NOT_CONNECT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CAMERA_NOT_CONNECT);
    iSel++;

    szTemp.Format("CAMERA_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CAMERA_RESUME);
    iSel++;

    szTemp.Format("COM_NOT_CONNECT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_COM_NOT_CONNECT);
    iSel++;

    szTemp.Format("COM_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_COM_RESUME);
    iSel++;

    szTemp.Format("DEVICE_NOT_AUTHORIZE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_DEVICE_NOT_AUTHORIZE);
    iSel++;

    szTemp.Format("PEOPLE_AND_ID_CARD_DEVICE_ONLINE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_PEOPLE_AND_ID_CARD_DEVICE_ONLINE);
    iSel++;

    szTemp.Format("PEOPLE_AND_ID_CARD_DEVICE_OFFLINE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_PEOPLE_AND_ID_CARD_DEVICE_OFFLINE);
    iSel++;

    szTemp.Format("LOCAL_LOGIN_LOCK");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LOCAL_LOGIN_LOCK);
    iSel++;

    szTemp.Format("LOCAL_LOGIN_UNLOCK");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LOCAL_LOGIN_UNLOCK);
    iSel++;

    szTemp.Format("SUBMARINEBACK_COMM_BREAK");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_SUBMARINEBACK_COMM_BREAK);
    iSel++;

    szTemp.Format("SUBMARINEBACK_COMM_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_SUBMARINEBACK_COMM_RESUME);
    iSel++;

    szTemp.Format("MOTOR_SENSOR_EXCEPTION");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_MOTOR_SENSOR_EXCEPTION);
    iSel++;

    szTemp.Format("CAN_BUS_EXCEPTION");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CAN_BUS_EXCEPTION);
    iSel++;

    szTemp.Format("CAN_BUS_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CAN_BUS_RESUME);
    iSel++;

    szTemp.Format("GATE_TEMPERATURE_OVERRUN");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_GATE_TEMPERATURE_OVERRUN);
    iSel++;

    szTemp.Format("IR_EMITTER_EXCEPTION");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_IR_EMITTER_EXCEPTION);
    iSel++;

    szTemp.Format("IR_EMITTER_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_IR_EMITTER_RESUME);
    iSel++;

    szTemp.Format("LAMP_BOARD_COMM_EXCEPTION");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LAMP_BOARD_COMM_EXCEPTION);
    iSel++;

    szTemp.Format("LAMP_BOARD_COMM_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LAMP_BOARD_COMM_RESUME);
    iSel++;

    szTemp.Format("IR_ADAPTOR_COMM_EXCEPTION");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_IR_ADAPTOR_COMM_EXCEPTION);
    iSel++;

    szTemp.Format("IR_ADAPTOR_COMM_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_IR_ADAPTOR_COMM_RESUME);
    iSel++;

    szTemp.Format("PRINTER_ONLINE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_PRINTER_ONLINE);
    iSel++;

    szTemp.Format("PRINTER_OFFLINE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_PRINTER_OFFLINE);
    iSel++;

    szTemp.Format("4G_MOUDLE_ONLINE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_4G_MOUDLE_ONLINE);
    iSel++;

    szTemp.Format("4G_MOUDLE_OFFLINE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_4G_MOUDLE_OFFLINE);
    iSel++;

    return iSel;
}

int CDlgAcsLogSearch::AddMinorOperationString(int iStartSel)
{
    CString szTemp;
    int iSel = iStartSel;

    szTemp.Format("LOCAL_UPGRADE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LOCAL_UPGRADE);
    iSel++;

    szTemp.Format("REMOTE_LOGIN");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_LOGIN);
    iSel++;

    szTemp.Format("REMOTE_LOGOUT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_LOGOUT);
    iSel++;

    szTemp.Format("MINOR_REMOTE_ARM");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_ARM);
    iSel++;

    szTemp.Format("REMOTE_DISARM");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_DISARM);
    iSel++;

    szTemp.Format("REMOTE_REBOOT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_REBOOT);
    iSel++;

    szTemp.Format("REMOTE_UPGRADE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_UPGRADE);
    iSel++;

    szTemp.Format("REMOTE_CFGFILE_OUTPUT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_CFGFILE_OUTPUT);
    iSel++;

    szTemp.Format("REMOTE_CFGFILE_INTPUT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_CFGFILE_INTPUT);
    iSel++;

    szTemp.Format("REMOTE_ALARMOUT_OPEN_MAN");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_ALARMOUT_OPEN_MAN);
    iSel++;

    szTemp.Format("REMOTE_ALARMOUT_CLOSE_MAN");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_ALARMOUT_CLOSE_MAN);
    iSel++;

    szTemp.Format("REMOTE_OPEN_DOOR");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_OPEN_DOOR);
    iSel++;

    szTemp.Format("REMOTE_CLOSE_DOOR");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_CLOSE_DOOR);
    iSel++;

    szTemp.Format("REMOTE_ALWAYS_OPEN");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_ALWAYS_OPEN);
    iSel++;

    szTemp.Format("REMOTE_ALWAYS_CLOSE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_ALWAYS_CLOSE);
    iSel++;

    szTemp.Format("REMOTE_CHECK_TIME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_CHECK_TIME);
    iSel++;

    szTemp.Format("NTP_CHECK_TIME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_NTP_CHECK_TIME);
    iSel++;

    szTemp.Format("REMOTE_CLEAR_CARD");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_CLEAR_CARD);
    iSel++;

    szTemp.Format("REMOTE_RESTORE_CFG");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_RESTORE_CFG);
    iSel++;

    szTemp.Format("ALARMIN_ARM");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_ALARMIN_ARM);
    iSel++;

    szTemp.Format("ALARMIN_DISARM");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_ALARMIN_DISARM);
    iSel++;

    szTemp.Format("LOCAL_RESTORE_CFG");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LOCAL_RESTORE_CFG);
    iSel++;

    szTemp.Format("REMOTE_CAPTURE_PIC");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_CAPTURE_PIC);
    iSel++;

    szTemp.Format("MOD_NET_REPORT_CFG");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_MOD_NET_REPORT_CFG);
    iSel++;

    szTemp.Format("MOD_GPRS_REPORT_PARAM");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_MOD_GPRS_REPORT_PARAM);
    iSel++;

    szTemp.Format("MOD_REPORT_GROUP_PARAM");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_MOD_REPORT_GROUP_PARAM);
    iSel++;

    szTemp.Format("UNLOCK_PASSWORD_OPEN_DOOR");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_UNLOCK_PASSWORD_OPEN_DOOR);
    iSel++;

    szTemp.Format("AUTO_RENUMBER");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_AUTO_RENUMBER);
    iSel++;

    szTemp.Format("AUTO_COMPLEMENT_NUMBER");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_AUTO_COMPLEMENT_NUMBER);
    iSel++;

    szTemp.Format("NORMAL_CFGFILE_INPUT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_NORMAL_CFGFILE_INPUT);
    iSel++;

    szTemp.Format("NORMAL_CFGFILE_OUTTPUT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_NORMAL_CFGFILE_OUTTPUT);
    iSel++;

    szTemp.Format("CARD_RIGHT_INPUT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CARD_RIGHT_INPUT);
    iSel++;

    szTemp.Format("CARD_RIGHT_OUTTPUT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CARD_RIGHT_OUTTPUT);
    iSel++;

    szTemp.Format("LOCAL_USB_UPGRADE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LOCAL_USB_UPGRADE);
    iSel++;

    szTemp.Format("REMOTE_VISITOR_CALL_LADDER");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_VISITOR_CALL_LADDER);
    iSel++;

    szTemp.Format("REMOTE_HOUSEHOLD_CALL_LADDER");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_HOUSEHOLD_CALL_LADDER);
    iSel++;

    szTemp.Format("REMOTE_ACTUAL_GUARD");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_ACTUAL_GUARD);
    iSel++;

    szTemp.Format("REMOTE_ACTUAL_UNGUARD");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_ACTUAL_UNGUARD);
    iSel++;

    szTemp.Format("REMOTE_CONTROL_NOT_CODE_OPER_FAILED");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_CONTROL_NOT_CODE_OPER_FAILED);
    iSel++;

    szTemp.Format("REMOTE_CONTROL_CLOSE_DOOR");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_CONTROL_CLOSE_DOOR);
    iSel++;

    szTemp.Format("REMOTE_CONTROL_OPEN_DOOR");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_CONTROL_OPEN_DOOR);
    iSel++;

    szTemp.Format("REMOTE_CONTROL_ALWAYS_OPEN_DOOR");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_CONTROL_ALWAYS_OPEN_DOOR);
    iSel++;

    szTemp.Format("M1_CARD_ENCRYPT_VERIFY_OPEN");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_M1_CARD_ENCRYPT_VERIFY_OPEN);
    iSel++;

    szTemp.Format("M1_CARD_ENCRYPT_VERIFY_CLOSE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_M1_CARD_ENCRYPT_VERIFY_CLOSE);
    iSel++;

    szTemp.Format("NFC_FUNCTION_OPEN");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_NFC_FUNCTION_OPEN);
    iSel++;

    szTemp.Format("NFC_FUNCTION_CLOSE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_NFC_FUNCTION_CLOSE);
    iSel++;

    szTemp.Format("LOCAL_PARA_FACTORY_DEFAULT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LOCAL_PARA_FACTORY_DEFAULT);
    iSel++;

    szTemp.Format("REMOTE_PARA_FACTORY_DEFAULT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_PARA_FACTORY_DEFAULT);
    iSel++;

    szTemp.Format("MIMOR_REMOTE_DELETE_ALL_VERIFYORCAP_PICS");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MIMOR_REMOTE_DELETE_ALL_VERIFYORCAP_PICS);
    iSel++;

    szTemp.Format("MIMOR_LOCAL_DELETE_ALL_VERIFYORCAP_PICS");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MIMOR_LOCAL_DELETE_ALL_VERIFYORCAP_PICS);
    iSel++;

    szTemp.Format("MIMOR_REMOTE_DELETE_EVENTS_AT_SPECTIME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MIMOR_REMOTE_DELETE_EVENTS_AT_SPECTIME);
    iSel++;

    szTemp.Format("MIMOR_LOCAL_DELETE_EVENTS_AT_SPECTIME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MIMOR_LOCAL_DELETE_EVENTS_AT_SPECTIME);
    iSel++;

    szTemp.Format("MIMOR_REMOTE_OPEN_SUMMER_TIME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MIMOR_REMOTE_OPEN_SUMMER_TIME);
    iSel++;

    szTemp.Format("MIMOR_LOCAL_OPEN_SUMMER_TIME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MIMOR_LOCAL_OPEN_SUMMER_TIME);
    iSel++;

    szTemp.Format("MIMOR_REMOTE_CLOSE_SUMMER_TIME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MIMOR_REMOTE_CLOSE_SUMMER_TIME);
    iSel++;

    szTemp.Format("MIMOR_LOCAL_CLOSE_SUMMER_TIME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MIMOR_LOCAL_CLOSE_SUMMER_TIME);
    iSel++;

    szTemp.Format("MIMOR_REMOTE_EZVIZ_UNBIND");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MIMOR_REMOTE_EZVIZ_UNBIND);
    iSel++;

    szTemp.Format("MIMOR_LOCAL_EZVIZ_UNBIND");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MIMOR_LOCAL_EZVIZ_UNBIND);
    iSel++;

    szTemp.Format("MIMOR_ENTER_LOCALUI_BACKGROUND");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MIMOR_LOCAL_CLOSE_SUMMER_TIME);
    iSel++;

    szTemp.Format("MIMOR_REMOTE_DELETE_FACEBASEMAP");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MIMOR_REMOTE_EZVIZ_UNBIND);
    iSel++;

    szTemp.Format("MIMOR_LOCAL_DELETE_FACEBASEMAP");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MIMOR_LOCAL_EZVIZ_UNBIND);
    iSel++;

    return iSel;
}

int CDlgAcsLogSearch::AddMinorEventString(int iStartSel)
{
    CString szTemp;
    int iSel = iStartSel;

    szTemp.Format("LEGAL_CARD_PASS");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LEGAL_CARD_PASS);
    iSel++;

    szTemp.Format("CARD_AND_PSW_PASS");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CARD_AND_PSW_PASS);
    iSel++;

    szTemp.Format("CARD_AND_PSW_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CARD_AND_PSW_FAIL);
    iSel++;

    szTemp.Format("CARD_AND_PSW_TIMEOUT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CARD_AND_PSW_TIMEOUT);
    iSel++;

    szTemp.Format("CARD_AND_PSW_OVER_TIME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CARD_AND_PSW_OVER_TIME);
    iSel++;

    szTemp.Format("CARD_NO_RIGHT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CARD_NO_RIGHT);
    iSel++;

    szTemp.Format("CARD_INVALID_PERIOD");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CARD_INVALID_PERIOD);
    iSel++;

    szTemp.Format("CARD_OUT_OF_DATE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CARD_OUT_OF_DATE);
    iSel++;

    szTemp.Format("INVALID_CARD");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_INVALID_CARD);
    iSel++;

    szTemp.Format("ANTI_SNEAK_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_ANTI_SNEAK_FAIL);
    iSel++;

    szTemp.Format("INTERLOCK_DOOR_NOT_CLOSE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_INTERLOCK_DOOR_NOT_CLOSE);
    iSel++;

    szTemp.Format("NOT_BELONG_MULTI_GROUP");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_NOT_BELONG_MULTI_GROUP);
    iSel++;

    szTemp.Format("INVALID_MULTI_VERIFY_PERIOD");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_INVALID_MULTI_VERIFY_PERIOD);
    iSel++;

    szTemp.Format("MULTI_VERIFY_SUPER_RIGHT_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_MULTI_VERIFY_SUPER_RIGHT_FAIL);
    iSel++;

    szTemp.Format("MULTI_VERIFY_REMOTE_RIGHT_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_MULTI_VERIFY_REMOTE_RIGHT_FAIL);
    iSel++;

    szTemp.Format("MULTI_VERIFY_SUCCESS");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_MULTI_VERIFY_SUCCESS);
    iSel++;

    szTemp.Format("LEADER_CARD_OPEN_BEGIN");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LEADER_CARD_OPEN_BEGIN);
    iSel++;

    szTemp.Format("LEADER_CARD_OPEN_END");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LEADER_CARD_OPEN_END);
    iSel++;

    szTemp.Format("ALWAYS_OPEN_BEGIN");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_ALWAYS_OPEN_BEGIN);
    iSel++;

    szTemp.Format("ALWAYS_OPEN_END");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_ALWAYS_OPEN_END);
    iSel++;

    szTemp.Format("LOCK_OPEN");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LOCK_OPEN);
    iSel++;

    szTemp.Format("LOCK_CLOSE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LOCK_CLOSE);
    iSel++;

    szTemp.Format("DOOR_BUTTON_PRESS");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_DOOR_BUTTON_PRESS);
    iSel++;

    szTemp.Format("DOOR_BUTTON_RELEASE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_DOOR_BUTTON_RELEASE);
    iSel++;

    szTemp.Format("DOOR_OPEN_NORMAL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_DOOR_OPEN_NORMAL);
    iSel++;

    szTemp.Format("DOOR_CLOSE_NORMAL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_DOOR_CLOSE_NORMAL);
    iSel++;

    szTemp.Format("DOOR_OPEN_ABNORMAL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_DOOR_OPEN_ABNORMAL);
    iSel++;

    szTemp.Format("DOOR_OPEN_TIMEOUT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_DOOR_OPEN_TIMEOUT);
    iSel++;

    szTemp.Format("ALARMOUT_ON");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_ALARMOUT_ON);
    iSel++;

    szTemp.Format("ALARMOUT_OFF");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_ALARMOUT_OFF);
    iSel++;

    szTemp.Format("ALWAYS_CLOSE_BEGIN");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_ALWAYS_CLOSE_BEGIN);
    iSel++;

    szTemp.Format("ALWAYS_CLOSE_END");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_ALWAYS_CLOSE_END);
    iSel++;

    szTemp.Format("MULTI_VERIFY_NEED_REMOTE_OPEN");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_MULTI_VERIFY_NEED_REMOTE_OPEN);
    iSel++;

    szTemp.Format("MULTI_VERIFY_SUPERPASSWD_VERIFY_SUCCESS");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_MULTI_VERIFY_SUPERPASSWD_VERIFY_SUCCESS);
    iSel++;

    szTemp.Format("MULTI_VERIFY_REPEAT_VERIFY");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_MULTI_VERIFY_REPEAT_VERIFY);
    iSel++;

    szTemp.Format("MULTI_VERIFY_TIMEOUT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_MULTI_VERIFY_TIMEOUT);
    iSel++;

    szTemp.Format("DOORBELL_RINGING");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_DOORBELL_RINGING);
    iSel++;

    szTemp.Format("FINGERPRINT_COMPARE_PASS");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_FINGERPRINT_COMPARE_PASS);
    iSel++;

    szTemp.Format("FINGERPRINT_COMPARE_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_FINGERPRINT_COMPARE_FAIL);
    iSel++;

    szTemp.Format("CARD_FINGERPRINT_VERIFY_PASS");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CARD_FINGERPRINT_VERIFY_PASS);
    iSel++;

    szTemp.Format("CARD_FINGERPRINT_VERIFY_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CARD_FINGERPRINT_VERIFY_FAIL);
    iSel++;

    szTemp.Format("CARD_FINGERPRINT_VERIFY_TIMEOUT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CARD_FINGERPRINT_VERIFY_TIMEOUT);
    iSel++;

    szTemp.Format("CARD_FINGERPRINT_PASSWD_VERIFY_PASS");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CARD_FINGERPRINT_PASSWD_VERIFY_PASS);
    iSel++;

    szTemp.Format("CARD_FINGERPRINT_PASSWD_VERIFY_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CARD_FINGERPRINT_PASSWD_VERIFY_FAIL);
    iSel++;

    szTemp.Format("CARD_FINGERPRINT_PASSWD_VERIFY_TIMEOUT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CARD_FINGERPRINT_PASSWD_VERIFY_TIMEOUT);
    iSel++;

    szTemp.Format("FINGERPRINT_PASSWD_VERIFY_PASS");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_FINGERPRINT_PASSWD_VERIFY_PASS);
    iSel++;

    szTemp.Format("FINGERPRINT_PASSWD_VERIFY_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_FINGERPRINT_PASSWD_VERIFY_FAIL);
    iSel++;

    szTemp.Format("FINGERPRINT_PASSWD_VERIFY_TIMEOUT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_FINGERPRINT_PASSWD_VERIFY_TIMEOUT);
    iSel++;

    szTemp.Format("FINGERPRINT_INEXISTENCE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_FINGERPRINT_INEXISTENCE);
    iSel++;

    szTemp.Format("CARD_PLATFORM_VERIFY");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CARD_PLATFORM_VERIFY);
    iSel++;

    szTemp.Format("LEGAL_MESSAGE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LEGAL_MESSAGE);
    iSel++;

    szTemp.Format("ILLEGAL_MESSAGE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_ILLEGAL_MESSAGE);
    iSel++;

    szTemp.Format("DOOR_OPEN_OR_DORMANT_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_DOOR_OPEN_OR_DORMANT_FAIL);
    iSel++;

    szTemp.Format("AUTH_PLAN_DORMANT_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_AUTH_PLAN_DORMANT_FAIL);
    iSel++;

    szTemp.Format("CARD_ENCRYPT_VERIFY_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CARD_ENCRYPT_VERIFY_FAIL);
    iSel++;

    szTemp.Format("SUBMARINEBACK_REPLY_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_SUBMARINEBACK_REPLY_FAIL);
    iSel++;

    szTemp.Format("DOOR_OPEN_OR_DORMANT_OPEN_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_DOOR_OPEN_OR_DORMANT_OPEN_FAIL);
    iSel++;

    szTemp.Format("DOOR_OPEN_OR_DORMANT_LINKAGE_OPEN_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_DOOR_OPEN_OR_DORMANT_LINKAGE_OPEN_FAIL);
    iSel++;

    szTemp.Format("TRAILING");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_TRAILING);
    iSel++;

    szTemp.Format("REVERSE_ACCESS");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REVERSE_ACCESS);
    iSel++;

    szTemp.Format("FORCE_ACCESS");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_FORCE_ACCESS);
    iSel++;

    szTemp.Format("CLIMBING_OVER_GATE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CLIMBING_OVER_GATE);
    iSel++;

    szTemp.Format("PASSING_TIMEOUT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_PASSING_TIMEOUT);
    iSel++;

    szTemp.Format("INTRUSION_ALARM");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_INTRUSION_ALARM);
    iSel++;

    szTemp.Format("FREE_GATE_PASS_NOT_AUTH");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_FREE_GATE_PASS_NOT_AUTH);
    iSel++;

    szTemp.Format("DROP_ARM_BLOCK");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_DROP_ARM_BLOCK);
    iSel++;

    szTemp.Format("DROP_ARM_BLOCK_RESUME");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_DROP_ARM_BLOCK_RESUME);
    iSel++;

    szTemp.Format("LOCAL_FACE_MODELING_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LOCAL_FACE_MODELING_FAIL);
    iSel++;

    szTemp.Format("STAY_EVENT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_STAY_EVENT);
    iSel++;

    szTemp.Format("CERTIFICATE_ALLOWLIST_PASS");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CERTIFICATE_ALLOWLIST_PASS);
    iSel++;

    szTemp.Format("CERTIFICATE_ALLOWLIST_FALL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CERTIFICATE_ALLOWLIST_FALL);
    iSel++;

    szTemp.Format("FACE_COLLECTION");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_FACE_COLLECTION);
    iSel++;

    szTemp.Format("CARD_COLLECTION");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CARD_COLLECTION);
    iSel++;

    szTemp.Format("PSW_ERROR_OVER_TIMES");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_PSW_ERROR_OVER_TIMES);
    iSel++;

    szTemp.Format("ID_CARD_COLLECTION");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_ID_CARD_COLLECTION);
    iSel++;

    szTemp.Format("PSW_VERIFY_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_PSW_VERIFY_FAIL);
    iSel++;

    szTemp.Format("PASSWORD_MISMATCH");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_PASSWORD_MISMATCH);
    iSel++;

    szTemp.Format("EMPLOYEE_NO_NOT_EXIST");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_EMPLOYEE_NO_NOT_EXIST);
    iSel++;

    szTemp.Format("COMBINED_VERIFY_PASS");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_COMBINED_VERIFY_PASS);
    iSel++;

    szTemp.Format("COMBINED_VERIFY_TIMEOUT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_COMBINED_VERIFY_TIMEOUT);
    iSel++;

    szTemp.Format("VERIFY_MODE_MISMATCH");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_VERIFY_MODE_MISMATCH);
    iSel++;

    szTemp.Format("ORCODE_VERIFY_PASS");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_ORCODE_VERIFY_PASS);
    iSel++;

    szTemp.Format("ORCODE_VERIFY_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_ORCODE_VERIFY_FAIL);
    iSel++;

    szTemp.Format("HOUSEHOLDER_AUTHORIZE_PASS");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_HOUSEHOLDER_AUTHORIZE_PASS);
    iSel++;

    szTemp.Format("BLUETOOTH_VERIFY_PASS");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_BLUETOOTH_VERIFY_PASS);
    iSel++;

    szTemp.Format("BLUETOOTH_VERIFY_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_BLUETOOTH_VERIFY_FAIL);
    iSel++;

    szTemp.Format("PASSPORT_VERIFY_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_PASSPORT_VERIFY_FAIL);
    iSel++;

    szTemp.Format("INFORMAL_M1_CARD_VERIFY_FAIL ");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_INFORMAL_M1_CARD_VERIFY_FAIL);
    iSel++;

    szTemp.Format("CPU_CARD_ENCRYPT_VERIFY_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CPU_CARD_ENCRYPT_VERIFY_FAIL);
    iSel++;

    szTemp.Format("NFC_DISABLE_VERIFY_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_NFC_DISABLE_VERIFY_FAIL);
    iSel++;

    szTemp.Format("EM_CARD_RECOGNIZE_NOT_ENABLED");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_EM_CARD_RECOGNIZE_NOT_ENABLED);
    iSel++;

    szTemp.Format("M1_CARD_RECOGNIZE_NOT_ENABLED");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_M1_CARD_RECOGNIZE_NOT_ENABLED);
    iSel++;

    szTemp.Format("CPU_CARD_RECOGNIZE_NOT_ENABLED");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CPU_CARD_RECOGNIZE_NOT_ENABLED);
    iSel++;

    szTemp.Format("ID_CARD_RECOGNIZE_NOT_ENABLED");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_ID_CARD_RECOGNIZE_NOT_ENABLED);
    iSel++;

    szTemp.Format("CARD_SET_SECRET_KEY_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_CARD_SET_SECRET_KEY_FAIL);
    iSel++;

    szTemp.Format("LOCAL_UPGRADE_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_LOCAL_UPGRADE_FAIL);
    iSel++;

    szTemp.Format("REMOTE_UPGRADE_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_UPGRADE_FAIL);
    iSel++;

    szTemp.Format("REMOTE_EXTEND_MODULE_UPGRADE_SUCC");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_EXTEND_MODULE_UPGRADE_SUCC);
    iSel++;

    szTemp.Format("REMOTE_EXTEND_MODULE_UPGRADE_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_EXTEND_MODULE_UPGRADE_FAIL);
    iSel++;

    szTemp.Format("REMOTE_FINGER_PRINT_MODULE_UPGRADE_SUCC");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_FINGER_PRINT_MODULE_UPGRADE_SUCC);
    iSel++;

    szTemp.Format("REMOTE_FINGER_PRINT_MODULE_UPGRADE_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REMOTE_FINGER_PRINT_MODULE_UPGRADE_FAIL);
    iSel++;

    szTemp.Format("PASSWD_VERIFY_PASS");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_PASSWD_VERIFY_PASS);
    iSel++;

    szTemp.Format("MINOR_COMSUME_TIMEOUT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_COMSUME_TIMEOUT);
    iSel++;

    szTemp.Format("MINOR_REFUND_TIMEOUT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_REFUND_TIMEOUT);
    iSel++;

    szTemp.Format("MINOR_COMSUME_AMOUNT_OVERLIMIT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_COMSUME_AMOUNT_OVERLIMIT);
    iSel++;

    szTemp.Format("MINOR_COMSUME_TIMES_OVERLIMIT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_COMSUME_TIMES_OVERLIMIT);
    iSel++;

    szTemp.Format("MINOR_USER_COMSUME_ENSURE_TIMEOUT");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_USER_COMSUME_ENSURE_TIMEOUT);
    iSel++;

    szTemp.Format("PEOPLE_AND_ID_CARD_COMPARE_PASS");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_PEOPLE_AND_ID_CARD_COMPARE_PASS);
    iSel++;

    szTemp.Format("PEOPLE_AND_ID_CARD_COMPARE_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_PEOPLE_AND_ID_CARD_COMPARE_FAIL);
    iSel++;

    szTemp.Format("MINOR_BLOCKLIST_REACH_THRESHOLD");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_BLOCKLIST_REACH_THRESHOLD);
    iSel++;

    szTemp.Format("MINOR_IRIS_VERIFY_PASS");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_IRIS_VERIFY_PASS);
    iSel++;

    szTemp.Format("MINOR_IRIS_VERIFY_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_IRIS_VERIFY_FAIL);
    iSel++;

    szTemp.Format("MINOR_IRIS_BIOASSAY_FAIL");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_IRIS_BIOASSAY_FAIL);
    iSel++;

    szTemp.Format("MINOR_FULL_STAFF");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_FULL_STAFF);
    iSel++;

    szTemp.Format("MINOR_ATTENDANCE_RECORD_EXCEED_THRESHOLD");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_ATTENDANCE_RECORD_EXCEED_THRESHOLD);
    iSel++;

    szTemp.Format("MINOR_DYNAMICCODE_VERIFY_INVALID");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_DYNAMICCODE_VERIFY_INVALID);
    iSel++;

    szTemp.Format("MINOR_MEETING_NO_SIGNIN");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_MEETING_NO_SIGNIN);
    iSel++;

    szTemp.Format("MINOR_MEETING_SIGNIN");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_MEETING_SIGNIN);
    iSel++;

    szTemp.Format("MINOR_MEETING_LATE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_MEETING_LATE);
    iSel++;


    szTemp.Format("MINOR_INTERLOCK_SERVER_FAILED_RESPONSE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_INTERLOCK_SERVER_FAILED_RESPONSE);
    iSel++;


    szTemp.Format("MINOR_INTERLOCK_SERVER_FAILED_RESPONSE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_FINGERPRINT_DISENABELD_VERIFY_FAIL);
    iSel++;

    szTemp.Format("MINOR_INTERLOCK_SERVER_FAILED_RESPONSE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_FACE_DISENABELD_VERIFY_FAIL);
    iSel++;

    szTemp.Format("MINOR_INTERLOCK_SERVER_FAILED_RESPONSE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_MONITOR_OPEN_DOOR);
    iSel++;

    szTemp.Format("MINOR_INTERLOCK_SERVER_FAILED_RESPONSE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_READER_FACTORY_RESET);
    iSel++;

    szTemp.Format("MINOR_INTERLOCK_SERVER_FAILED_RESPONSE");
    m_cmbMinorType.AddString(szTemp);
    m_cmbMinorType.SetItemData(iSel, MINOR_READER_ID_CONFLICT);
    iSel++;
    return iSel;
}


BEGIN_MESSAGE_MAP(CDlgAcsLogSearch, CDialog)
    ON_BN_CLICKED(IDC_BTN_SEARCH, &CDlgAcsLogSearch::OnBnClickedBtnSearch)
    ON_BN_CLICKED(IDC_BTN_EXIT, &CDlgAcsLogSearch::OnBnClickedBtnExit)
    ON_CBN_SELCHANGE(IDC_COMBO_MAJOR_TYPE, &CDlgAcsLogSearch::OnCbnSelchangeComboMajorType)
    ON_MESSAGE(WM_MSG_ADD_ACS_EVENT_TOLIST, OnMsgAddAcsEventToList)
    ON_MESSAGE(WM_MSG_GET_ACS_EVENT_FINISH, OnMsgGetAcsEventFinish)
END_MESSAGE_MAP()


// CDlgAcsLogSearch 消息处理程序


void CDlgAcsLogSearch::OnBnClickedBtnSearch()
{
    // TODO:  在此添加控件通知处理程序代码
    m_listDeviceLog.DeleteAllItems();
    UpdateData(TRUE);

    memset(&m_struAcsEventCond, 0, sizeof(m_struAcsEventCond));
    m_struAcsEventCond.dwSize = sizeof(m_struAcsEventCond);
    m_struAcsEventCond.dwMajor = m_cmbMajorType.GetItemData(m_cmbMajorType.GetCurSel());
    m_struAcsEventCond.dwMinor = m_cmbMinorType.GetItemData(m_cmbMinorType.GetCurSel());
    m_struAcsEventCond.struStartTime.dwYear = m_startDate.GetYear();
    m_struAcsEventCond.struStartTime.dwMonth = m_startDate.GetMonth();
    m_struAcsEventCond.struStartTime.dwDay = m_startDate.GetDay();
    m_struAcsEventCond.struStartTime.dwHour = m_startTime.GetHour();
    m_struAcsEventCond.struStartTime.dwMinute = m_startTime.GetMinute();
    m_struAcsEventCond.struStartTime.dwSecond = m_startTime.GetSecond();
    m_struAcsEventCond.struEndTime.dwYear = m_stopDate.GetYear();
    m_struAcsEventCond.struEndTime.dwMonth = m_stopDate.GetMonth();
    m_struAcsEventCond.struEndTime.dwDay = m_stopDate.GetDay();
    m_struAcsEventCond.struEndTime.dwHour = m_stopTime.GetHour();
    m_struAcsEventCond.struEndTime.dwMinute = m_stopTime.GetMinute();
    m_struAcsEventCond.struEndTime.dwSecond = m_stopTime.GetSecond();
    memcpy(m_struAcsEventCond.byCardNo, m_csCardNo, m_csCardNo.GetLength());
    memcpy(m_struAcsEventCond.byName, m_csName, m_csName.GetLength());
    if (g_struDeviceInfo[m_iDeviceIndex].byCharaterEncodeType == 6)
    {
        string str_utf8 = GB2UTF((const char *)(m_struAcsEventCond.byName));
        memset(m_struAcsEventCond.byName, 0, sizeof(m_struAcsEventCond.byName));
        strncpy((char *)m_struAcsEventCond.byName, str_utf8.c_str(), NAME_LEN);
    }
    m_struAcsEventCond.byPicEnable = m_bPictureEnable;
    m_struAcsEventCond.dwBeginSerialNo = m_dwBeginSerialNo;
    m_struAcsEventCond.dwEndSerialNo = m_dwEndSerialNo;
    m_struAcsEventCond.dwIOTChannelNo = m_iChannel;
    m_struAcsEventCond.wInductiveEventType = m_comInductiveEventType.GetItemData(m_comInductiveEventType.GetCurSel());
    m_struAcsEventCond.bySearchType = m_comSearchType.GetCurSel();
    memcpy(m_struAcsEventCond.szMonitorID, m_strMonitorId, m_strMonitorId.GetLength());
    m_struAcsEventCond.byTimeType = m_cmbTimeType.GetCurSel();
    memcpy(m_struAcsEventCond.byEmployeeNo, m_csEmployeeNo, m_csEmployeeNo.GetLength());
    m_struAcsEventCond.byEventAttribute = m_comEventAttribute.GetCurSel();

    m_lLogNum = 0;
    m_lGetAcsEvent = NET_DVR_StartRemoteConfig(m_lUserID, NET_DVR_GET_ACS_EVENT, &m_struAcsEventCond, sizeof(m_struAcsEventCond), g_fGetAcsEventCallback, this);
    if (m_lGetAcsEvent == -1)
    {
        g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_GET_ACS_EVENT");
        return;
    }
    else
    {
        g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_GET_ACS_EVENT");
    }

    UpdateData(FALSE);
}


void CDlgAcsLogSearch::OnBnClickedBtnExit()
{
    // TODO:  在此添加控件通知处理程序代码
    CDialog::OnCancel();
}


void CDlgAcsLogSearch::OnCbnSelchangeComboMajorType()
{
    // TODO:  在此添加控件通知处理程序代码
    int iSel = 0;
    switch (m_cmbMajorType.GetCurSel())
    {
    case 0:
        m_cmbMinorType.ResetContent();
        iSel = 0;
        iSel = AddMinorAllString(iSel);
        iSel = AddMinorAlarmString(iSel);
        iSel = AddMinorExceptionString(iSel);
        iSel = AddMinorOperationString(iSel);
        iSel = AddMinorEventString(iSel);
        m_cmbMinorType.SetCurSel(0);
    	break;
    case 1:
        m_cmbMinorType.ResetContent();
        iSel = 0;
        iSel = AddMinorAllString(iSel);
        iSel = AddMinorAlarmString(iSel);
        m_cmbMinorType.SetCurSel(0);
        break;
    case 2:
        m_cmbMinorType.ResetContent();
        iSel = 0;
        iSel = AddMinorAllString(iSel);
        iSel = AddMinorExceptionString(iSel);
        m_cmbMinorType.SetCurSel(0);
        break;
    case 3:
        m_cmbMinorType.ResetContent();
        iSel = 0;
        iSel = AddMinorAllString(iSel);
        iSel = AddMinorOperationString(iSel);
        m_cmbMinorType.SetCurSel(0);
        break;
    case 4:
        m_cmbMinorType.ResetContent();
        iSel = 0;
        iSel = AddMinorAllString(iSel);
        iSel = AddMinorEventString(iSel);
        m_cmbMinorType.SetCurSel(0);
        break;
    default:
        break;
    }
}

/*********************************************************
Function:	GetLogFileThread
Desc:		get log file thread
Input:		pParam, parameter pointer
Output:
Return:
**********************************************************/
//DWORD WINAPI CDlgAcsLogSearch::GetLogFileThread(LPVOID pParam)
//{
    /*LONG bRet = -1;

    NET_DVR_LOG_V30 struFileInfo = {0};
    NET_DVR_ACS_EVENT_LOG struAcsEventLog = { 0 };

    CString csTmp;
    char szLan[128] = { 0 };
    char szCardNo[33] = { 0 };

    while (1)
    {
        bRet = NET_DVR_FindNextLog_V30(pLogSearch->m_lLogHandle, &struFileInfo);
        memcpy(&struAcsEventLog, struFileInfo.sInfo, sizeof(struAcsEventLog));

        if (bRet == NET_DVR_FILE_SUCCESS)
        {
            csTmp.Format("%d", pLogSearch->m_lLogNum + 1);
            pLogSearch->m_listDeviceLog.InsertItem(pLogSearch->m_lLogNum, csTmp, 0);

            csTmp.Format("%04d-%02d-%02d %02d:%02d:%02d", struFileInfo.strLogTime.dwYear, struFileInfo.strLogTime.dwMonth, struFileInfo.strLogTime.dwDay, \
                struFileInfo.strLogTime.dwHour, struFileInfo.strLogTime.dwMinute, struFileInfo.strLogTime.dwSecond);
            pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 1, csTmp);

            switch (struFileInfo.dwMajorType)
            {
            case 1:
                g_StringLanType(szLan, "报警", "Alarm");
                csTmp.Format("%s", szLan);
                break;
            case 2:
                g_StringLanType(szLan, "异常", "Exception");
                csTmp.Format("%s", szLan);
                break;
            case 3:
                g_StringLanType(szLan, "操作", "Operation");
                csTmp.Format("%s", szLan);
                break;
            case 5:
                g_StringLanType(szLan, "事件", "Event");
                csTmp.Format("%s", szLan);
                break;
            default:
                g_StringLanType(szLan, "未知类型", "Unknown");
                csTmp.Format("%s", szLan);
                break;
            }
            pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 2, csTmp);

            if (MAJOR_ALARM == struFileInfo.dwMajorType)
            {
                AlarmMinorTypeMap(struFileInfo, csTmp);
            }
            else if (MAJOR_EXCEPTION == struFileInfo.dwMajorType)
            {
                OperationMinorTypeMap(struFileInfo, csTmp);
            }
            else if (MAJOR_OPERATION == struFileInfo.dwMajorType)
            {
                AlarmMinorTypeMap(struFileInfo, csTmp);
            }
            else if (MAJOR_EVENT == struFileInfo.dwMajorType)
            {
                ExceptionMinorTypeMap(struFileInfo, csTmp);
            }
            else
            {
                g_StringLanType(szLan, "未知类型", "Unknown");
                csTmp.Format("%s", szLan);
            }
            pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 3, csTmp);

            memset(szCardNo, 0, 33);
            memcpy(szCardNo, struAcsEventLog.byCardNo, ACS_CARD_NO_LEN);
            csTmp.Format("%s", szCardNo);
            pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 4, csTmp);

            if (struAcsEventLog.byCardType == 1)
            {
                g_StringLanType(szLan, "普通卡", "Ordinary Card");
                csTmp.Format("%s", szLan);
            } 
            else if (struAcsEventLog.byCardType == 2)
            {
                g_StringLanType(szLan, "特殊群体卡", "Accessible Card");
                csTmp.Format("%s", szLan);
            }
            else if (struAcsEventLog.byCardType == 3)
            {
                g_StringLanType(szLan, "禁止名单卡", "Block List Card");
                csTmp.Format("%s", szLan);
            }
            else if (struAcsEventLog.byCardType == 4)
            {
                g_StringLanType(szLan, "巡更卡", "Patrol Card");
                csTmp.Format("%s", szLan);
            }
            else if (struAcsEventLog.byCardType == 5)
            {
                g_StringLanType(szLan, "胁迫卡", "Stress Card");
                csTmp.Format("%s", szLan);
            }
            else if (struAcsEventLog.byCardType == 6)
            {
                g_StringLanType(szLan, "超级卡", "Super Card");
                csTmp.Format("%s", szLan);
            }
            else if (struAcsEventLog.byCardType == 7)
            {
                g_StringLanType(szLan, "来宾卡", "Guest Card");
                csTmp.Format("%s", szLan);
            }
            else
            {
                g_StringLanType(szLan, "无效", "No effect");
                csTmp.Format("%s", szLan);
            }
            pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 5, csTmp);

            csTmp.Format("%d", struAcsEventLog.byAllowListNo);
            pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 6, csTmp);

            if (struAcsEventLog.byReportChannel == 1)
            {
                g_StringLanType(szLan, "布防上传", "Upload");
                csTmp.Format("%s", szLan);
            }
            else if (struAcsEventLog.byReportChannel == 2)
            {
                g_StringLanType(szLan, "中心组1上传", "Center 1 Upload");
                csTmp.Format("%s", szLan);
            }
            else if (struAcsEventLog.byReportChannel == 3)
            {
                g_StringLanType(szLan, "中心组2上传", "Center 2 Upload");
                csTmp.Format("%s", szLan);
            }
            else
            {
                g_StringLanType(szLan, "无效", "No effect");
                csTmp.Format("%s", szLan);
            }
            pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 7, csTmp);

            if (struAcsEventLog.byCardReaderKind == 1)
            {
                g_StringLanType(szLan, "IC读卡器", "IC Reader");
                csTmp.Format("%s", szLan);
            }
            else if (struAcsEventLog.byCardReaderKind == 2)
            {
                g_StringLanType(szLan, "身份证读卡器", "Certificate Reader");
                csTmp.Format("%s", szLan);
            }
            else if (struAcsEventLog.byCardReaderKind == 3)
            {
                g_StringLanType(szLan, "二维码读卡器", "Two-dimension Reader");
                csTmp.Format("%s", szLan);
            }
            else if (struAcsEventLog.byCardReaderKind == 4)
            {
                g_StringLanType(szLan, "指纹头", "Finger Print Head");
                csTmp.Format("%s", szLan);
            }
            else
            {
                g_StringLanType(szLan, "无效", "No effect");
                csTmp.Format("%s", szLan);
            }
            pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 8, csTmp);

            csTmp.Format("%d", struAcsEventLog.dwCardReaderNo);
            pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 9, csTmp);

            csTmp.Format("%d", struAcsEventLog.dwDoorNo);
            pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 10, csTmp);

            csTmp.Format("%d", struAcsEventLog.dwVerifyNo);
            pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 11, csTmp);

            csTmp.Format("%d", struAcsEventLog.dwAlarmInNo);
            pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 12, csTmp);

            csTmp.Format("%d", struAcsEventLog.dwAlarmOutNo);
            pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 13, csTmp);

            csTmp.Format("%d", struAcsEventLog.dwCaseSensorNo);
            pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 14, csTmp);

            csTmp.Format("%d", struAcsEventLog.dwRs485No);
            pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 15, csTmp);

            csTmp.Format("%d", struAcsEventLog.dwMultiCardGroupNo);
            pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 16, csTmp);

            csTmp.Format("%d", struAcsEventLog.wAccessChannel);
            pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 17, csTmp);

            csTmp.Format("%d", struAcsEventLog.byDeviceNo);
            pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 18, csTmp);

            csTmp.Format("%d", struAcsEventLog.dwEmployeeNo);
            pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 19, csTmp);

            pLogSearch->m_lLogNum++;
        }
        else
        {
            if (bRet == NET_DVR_ISFINDING)
            {
                pLogSearch->GetDlgItem(IDC_STATIC_SEARCH)->ShowWindow(SW_SHOW);
                pLogSearch->GetDlgItem(IDC_STATIC_SEARCH)->SetWindowText("日志搜索中......");
                Sleep(5);
                continue;
            }
            if ((bRet == NET_DVR_NOMOREFILE) || (bRet == NET_DVR_FILE_NOFIND))
            {
                g_StringLanType(szLan, "搜索日志", "Search Log");
                pLogSearch->GetDlgItem(IDC_BTN_SEARCH)->SetWindowText(szLan);
                pLogSearch->m_bSearch = FALSE;
                pLogSearch->GetDlgItem(IDC_STATIC_SEARCH)->ShowWindow(SW_HIDE);
                g_StringLanType(szLan, "搜索日志结束!", "Search log Ending");
                AfxMessageBox(szLan);
                break;
            }
            else
            {
                g_StringLanType(szLan, "搜索日志", "Search Log");
                pLogSearch->GetDlgItem(IDC_BTN_SEARCH)->SetWindowText(szLan);
                pLogSearch->m_bSearch = FALSE;
                g_StringLanType(szLan, "由于服务器忙,或网络故障,搜索日志异常终止!", \
                    "Since the server is busy, or network failure, search the log abnormal termination");
                AfxMessageBox(szLan);
                break;
            }
        }
    }
    CloseHandle(pLogSearch->m_hFileThread);
    pLogSearch->m_hFileThread = NULL;
    NET_DVR_FindLogClose_V30(pLogSearch->m_lLogHandle);*/

    //return 0;
//}

void CDlgAcsLogSearch::AlarmMinorTypeMap(LPNET_DVR_ACS_EVENT_CFG lpAcsEventCfg, CString &szTemp)
{
    CString szParaType;
    switch (lpAcsEventCfg->dwMinor)
    {
    case MINOR_ALARMIN_SHORT_CIRCUIT:
        szTemp.Format("%s", "ALARMIN_SHORT_CIRCUIT");
        break;
    case MINOR_ALARMIN_BROKEN_CIRCUIT:
        szTemp.Format("%s", "ALARMIN_BROKEN_CIRCUIT");
        break;
    case MINOR_ALARMIN_EXCEPTION:
        szTemp.Format("%s", "ALARMIN_EXCEPTION");
        break;
    case MINOR_ALARMIN_RESUME:
        szTemp.Format("%s", "ALARMIN_RESUME");
        break;
    case MINOR_HOST_DESMANTLE_ALARM:
        szTemp.Format("%s", "HOST_DESMANTLE_ALARM");
        break;
    case MINOR_HOST_DESMANTLE_RESUME:
        szTemp.Format("%s", "HOST_DESMANTLE_RESUME");
        break;
    case MINOR_CARD_READER_DESMANTLE_ALARM:
        szTemp.Format("%s", "CARD_READER_DESMANTLE_ALARM");
        break;
    case MINOR_CARD_READER_DESMANTLE_RESUME:
        szTemp.Format("%s", "CARD_READER_DESMANTLE_RESUME");
        break;
    case MINOR_CASE_SENSOR_ALARM:
        szTemp.Format("%s", "CASE_SENSOR_ALARM");
        break;
    case MINOR_CASE_SENSOR_RESUME:
        szTemp.Format("%s", "CASE_SENSOR_RESUME");
        break;
    case MINOR_STRESS_ALARM:
        szTemp.Format("%s", "STRESS_ALARM");
        break;
    case MINOR_OFFLINE_ECENT_NEARLY_FULL:
        szTemp.Format("%s", "OFFLINE_ECENT_NEARLY_FULL");
        break;
    case MINOR_CARD_MAX_AUTHENTICATE_FAIL:
        szTemp.Format("%s", "CARD_MAX_AUTHENTICATE_FAIL");
        break;
    case MINOR_SD_CARD_FULL:
        szTemp.Format("%s", "MINOR_SD_CARD_FULL");
        break;
    case MINOR_LINKAGE_CAPTURE_PIC:
        szTemp.Format("%s", "MINOR_LINKAGE_CAPTURE_PIC");
        break;
    case MINOR_SECURITY_MODULE_DESMANTLE_ALARM:
        szTemp.Format("%s", "MINOR_SECURITY_MODULE_DESMANTLE_ALARM");
        break;
    case MINOR_SECURITY_MODULE_DESMANTLE_RESUME:
        szTemp.Format("%s", "MINOR_SECURITY_MODULE_DESMANTLE_RESUME");
        break;
    case MINOR_POS_START_ALARM:
        szTemp.Format("%s", "MINOR_POS_START_ALARM");
        break;
    case MINOR_POS_END_ALARM:
        szTemp.Format("%s", "MINOR_POS_END_ALARM");
        break;
    case MINOR_FACE_IMAGE_QUALITY_LOW:
        szTemp.Format("%s", "MINOR_FACE_IMAGE_QUALITY_LOW");
        break;
    case MINOR_FINGE_RPRINT_QUALITY_LOW:
        szTemp.Format("%s", "MINOR_FINGE_RPRINT_QUALITY_LOW");
        break;
    case MINOR_FIRE_IMPORT_SHORT_CIRCUIT:
        szTemp.Format("%s", "MINOR_FIRE_IMPORT_SHORT_CIRCUIT");
        break;
    case MINOR_FIRE_IMPORT_BROKEN_CIRCUIT:
        szTemp.Format("%s", "MINOR_FIRE_IMPORT_BROKEN_CIRCUIT");
        break;
    case MINOR_FIRE_IMPORT_RESUME:
        szTemp.Format("%s", "MINOR_FIRE_IMPORT_RESUME");
        break;
    case MINOR_FIRE_BUTTON_TRIGGER:
        szTemp.Format("%s", "FIRE_BUTTON_TRIGGER");
        break;
    case MINOR_FIRE_BUTTON_RESUME:
        szTemp.Format("%s", "FIRE_BUTTON_RESUME");
        break;
    case MINOR_MAINTENANCE_BUTTON_TRIGGER:
        szTemp.Format("%s", "MAINTENANCE_BUTTON_TRIGGER");
        break;
    case MINOR_MAINTENANCE_BUTTON_RESUME:
        szTemp.Format("%s", "MAINTENANCE_BUTTON_RESUME");
        break;
    case MINOR_EMERGENCY_BUTTON_TRIGGER:
        szTemp.Format("%s", "EMERGENCY_BUTTON_TRIGGER");
        break;
    case MINOR_EMERGENCY_BUTTON_RESUME:
        szTemp.Format("%s", "EMERGENCY_BUTTON_RESUME");
        break;
    case MINOR_DISTRACT_CONTROLLER_ALARM:
        szTemp.Format("%s", "DISTRACT_CONTROLLER_ALARM");
        break;
    case MINOR_DISTRACT_CONTROLLER_RESUME:
        szTemp.Format("%s", "DISTRACT_CONTROLLER_RESUME");
        break;
    case MINOR_CHANNEL_CONTROLLER_DESMANTLE_ALARM:
        szTemp.Format("%s", "MINOR_CHANNEL_CONTROLLER_DESMANTLE_ALARM");
        break;
    case MINOR_CHANNEL_CONTROLLER_DESMANTLE_RESUME:
        szTemp.Format("%s", "MINOR_CHANNEL_CONTROLLER_DESMANTLE_RESUME");
        break;
    case MINOR_CHANNEL_CONTROLLER_FIRE_IMPORT_ALARM:
        szTemp.Format("%s", "MINOR_CHANNEL_CONTROLLER_FIRE_IMPORT_ALARM");
        break;
    case MINOR_CHANNEL_CONTROLLER_FIRE_IMPORT_RESUME:
        szTemp.Format("%s", "MINOR_CHANNEL_CONTROLLER_FIRE_IMPORT_RESUME");
        break;
    case MINOR_LEGAL_EVENT_NEARLY_FULL:
        szTemp.Format("%s", "MINOR_LEGAL_EVENT_NEARLY_FULL");
        break;
    case MINOR_TRANSACTION_RECORD_NEARLY_FULL:
        szTemp.Format("%s", "MINOR_TRANSACTION_RECORD_NEARLY_FULL");
        break;
    default:
        szTemp.Format("0x%x", lpAcsEventCfg->dwMinor);
        break;
    }
}

void CDlgAcsLogSearch::ExceptionMinorTypeMap(LPNET_DVR_ACS_EVENT_CFG lpAcsEventCfg, CString &szTemp)
{
    CString szParaType;
    switch (lpAcsEventCfg->dwMinor)
    {
    case MINOR_NET_BROKEN:
        szTemp.Format("%s", "NET_BROKEN");
        break;
    case MINOR_RS485_DEVICE_ABNORMAL:
        szTemp.Format("%s", "RS485_DEVICE_ABNORMAL");
        break;
    case MINOR_RS485_DEVICE_REVERT:
        szTemp.Format("%s", "RS485_DEVICE_REVERT");
        break;
    case MINOR_DEV_POWER_ON:
        szTemp.Format("%s", "DEV_POWER_ON");
        break;
    case MINOR_DEV_POWER_OFF:
        szTemp.Format("%s", "DEV_POWER_OFF");
        break;
    case MINOR_WATCH_DOG_RESET:
        szTemp.Format("%s", "WATCH_DOG_RESET");
        break;
    case MINOR_LOW_BATTERY:
        szTemp.Format("%s", "LOW_BATTERY");
        break;
    case MINOR_BATTERY_RESUME:
        szTemp.Format("%s", "BATTERY_RESUME");
        break;
    case MINOR_AC_OFF:
        szTemp.Format("%s", "AC_OFF");
        break;
    case MINOR_AC_RESUME:
        szTemp.Format("%s", "AC_RESUME");
        break;
    case MINOR_NET_RESUME:
        szTemp.Format("%s", "NET_RESUME");
        break;
    case MINOR_FLASH_ABNORMAL:
        szTemp.Format("%s", "FLASH_ABNORMAL");
        break;
    case MINOR_CARD_READER_OFFLINE:
        szTemp.Format("%s", "CARD_READER_OFFLINE");
        break;
    case MINOR_CARD_READER_RESUME:
        szTemp.Format("%s", "CARD_READER_RESUME");
        break;
    case MINOR_INDICATOR_LIGHT_OFF:
        szTemp.Format("%s", "INDICATOR_LIGHT_OFF");
        break;
    case MINOR_INDICATOR_LIGHT_RESUME:
        szTemp.Format("%s", "INDICATOR_LIGHT_RESUME");
        break;
    case MINOR_CHANNEL_CONTROLLER_OFF:
        szTemp.Format("%s", "CHANNEL_CONTROLLER_OFF");
        break;
    case MINOR_CHANNEL_CONTROLLER_RESUME:
        szTemp.Format("%s", "CHANNEL_CONTROLLER_RESUME");
        break;
    case MINOR_SECURITY_MODULE_OFF:
        szTemp.Format("%s", "SECURITY_MODULE_OFF");
        break;
    case MINOR_SECURITY_MODULE_RESUME:
        szTemp.Format("%s", "SECURITY_MODULE_RESUME");
        break;
    case MINOR_BATTERY_ELECTRIC_LOW:
        szTemp.Format("%s", "BATTERY_ELECTRIC_LOW");
        break;
    case MINOR_BATTERY_ELECTRIC_RESUME:
        szTemp.Format("%s", "BATTERY_ELECTRIC_RESUME");
        break;
    case MINOR_LOCAL_CONTROL_NET_BROKEN:
        szTemp.Format("%s", "LOCAL_CONTROL_NET_BROKEN");
        break;
    case MINOR_LOCAL_CONTROL_NET_RSUME:
        szTemp.Format("%s", "LOCAL_CONTROL_NET_RSUME");
        break;
    case MINOR_MASTER_RS485_LOOPNODE_BROKEN:
        szTemp.Format("%s", "MASTER_RS485_LOOPNODE_BROKEN");
        break;
    case MINOR_MASTER_RS485_LOOPNODE_RESUME:
        szTemp.Format("%s", "MASTER_RS485_LOOPNODE_RESUME");
        break;
    case MINOR_LOCAL_CONTROL_OFFLINE:
        szTemp.Format("%s", "LOCAL_CONTROL_OFFLINE");
        break;
    case MINOR_LOCAL_CONTROL_RESUME:
        szTemp.Format("%s", "LOCAL_CONTROL_RESUME");
        break;
    case MINOR_LOCAL_DOWNSIDE_RS485_LOOPNODE_BROKEN:
        szTemp.Format("%s", "LOCAL_DOWNSIDE_RS485_LOOPNODE_BROKEN");
        break;
    case MINOR_LOCAL_DOWNSIDE_RS485_LOOPNODE_RESUME:
        szTemp.Format("%s", "LOCAL_DOWNSIDE_RS485_LOOPNODE_RESUME");
        break;
    case MINOR_DISTRACT_CONTROLLER_ONLINE:
        szTemp.Format("%s", "DISTRACT_CONTROLLER_ONLINE");
        break;
    case MINOR_DISTRACT_CONTROLLER_OFFLINE:
        szTemp.Format("%s", "DISTRACT_CONTROLLER_OFFLINE");
        break;
    case MINOR_ID_CARD_READER_NOT_CONNECT:
        szTemp.Format("%s", "ID_CARD_READER_NOT_CONNECT");
        break;
    case MINOR_ID_CARD_READER_RESUME:
        szTemp.Format("%s", "ID_CARD_READER_RESUME");
        break;
    case MINOR_FINGER_PRINT_MODULE_NOT_CONNECT:
        szTemp.Format("%s", "FINGER_PRINT_MODULE_NOT_CONNECT");
        break;
    case MINOR_FINGER_PRINT_MODULE_RESUME:
        szTemp.Format("%s", "FINGER_PRINT_MODULE_RESUME");
        break;
    case MINOR_CAMERA_NOT_CONNECT:
        szTemp.Format("%s", "CAMERA_NOT_CONNECT");
        break;
    case MINOR_CAMERA_RESUME:
        szTemp.Format("%s", "CAMERA_RESUME");
        break;
    case MINOR_COM_NOT_CONNECT:
        szTemp.Format("%s", "COM_NOT_CONNECT");
        break;
    case MINOR_COM_RESUME:
        szTemp.Format("%s", "COM_RESUME");
        break;
    case MINOR_DEVICE_NOT_AUTHORIZE:
        szTemp.Format("%s", "DEVICE_NOT_AUTHORIZE");
        break;
    case MINOR_PEOPLE_AND_ID_CARD_DEVICE_ONLINE:
        szTemp.Format("%s", "PEOPLE_AND_ID_CARD_DEVICE_ONLINE");
        break;
    case MINOR_PEOPLE_AND_ID_CARD_DEVICE_OFFLINE:
        szTemp.Format("%s", "PEOPLE_AND_ID_CARD_DEVICE_OFFLINE");
        break;
    case MINOR_LOCAL_LOGIN_LOCK:
        szTemp.Format("%s", "LOCAL_LOGIN_LOCK");
        break;
    case MINOR_LOCAL_LOGIN_UNLOCK:
        szTemp.Format("%s", "LOCAL_LOGIN_UNLOCK");
        break;
    case MINOR_SUBMARINEBACK_COMM_BREAK:
        szTemp.Format("%s", "SUBMARINEBACK_COMM_BREAK");
        break;
    case MINOR_SUBMARINEBACK_COMM_RESUME:
        szTemp.Format("%s", "SUBMARINEBACK_COMM_RESUME");
        break;
    case MINOR_MOTOR_SENSOR_EXCEPTION:
        szTemp.Format("%s", "MOTOR_SENSOR_EXCEPTION");
        break;
    case MINOR_CAN_BUS_EXCEPTION:
        szTemp.Format("%s", "CAN_BUS_EXCEPTION");
        break;
    case MINOR_CAN_BUS_RESUME:
        szTemp.Format("%s", "CAN_BUS_RESUME");
        break;
    case MINOR_GATE_TEMPERATURE_OVERRUN:
        szTemp.Format("%s", "GATE_TEMPERATURE_OVERRUN");
        break;
    case MINOR_IR_EMITTER_EXCEPTION:
        szTemp.Format("%s", "IR_EMITTER_EXCEPTION");
        break;
    case MINOR_IR_EMITTER_RESUME:
        szTemp.Format("%s", "IR_EMITTER_RESUME");
        break;
    case MINOR_LAMP_BOARD_COMM_EXCEPTION:
        szTemp.Format("%s", "LAMP_BOARD_COMM_EXCEPTION");
        break;
    case MINOR_LAMP_BOARD_COMM_RESUME:
        szTemp.Format("%s", "LAMP_BOARD_COMM_RESUME");
        break;
    case MINOR_IR_ADAPTOR_COMM_EXCEPTION:
        szTemp.Format("%s", "IR_ADAPTOR_COMM_EXCEPTION");
        break;
    case MINOR_IR_ADAPTOR_COMM_RESUME:
        szTemp.Format("%s", "IR_ADAPTOR_COMM_RESUME");
        break;
    default:
        szTemp.Format("0x%x", lpAcsEventCfg->dwMinor);
        break;
    }
}

void CDlgAcsLogSearch::OperationMinorTypeMap(LPNET_DVR_ACS_EVENT_CFG lpAcsEventCfg, CString &szTemp)
{
    CString szParaType;
    switch (lpAcsEventCfg->dwMinor)
    {
    case MINOR_LOCAL_UPGRADE:
        szTemp.Format("%s", "LOCAL_UPGRADE");
        break;
    case MINOR_REMOTE_LOGIN:
        szTemp.Format("%s", "REMOTE_LOGIN");
        break;
    case MINOR_REMOTE_LOGOUT:
        szTemp.Format("%s", "REMOTE_LOGOUT");
        break;
    case MINOR_REMOTE_ARM:
        szTemp.Format("%s", "REMOTE_ARM");
        break;
    case MINOR_REMOTE_DISARM:
        szTemp.Format("%s", "REMOTE_DISARM");
        break;
    case MINOR_REMOTE_REBOOT:
        szTemp.Format("%s", "REMOTE_REBOOT");
        break;
    case MINOR_REMOTE_UPGRADE:
        szTemp.Format("%s", "REMOTE_UPGRADE");
        break;
    case MINOR_REMOTE_CFGFILE_OUTPUT:
        szTemp.Format("%s", "REMOTE_CFGFILE_OUTPUT");
        break;
    case MINOR_REMOTE_CFGFILE_INTPUT:
        szTemp.Format("%s", "REMOTE_CFGFILE_INTPUT");
        break;
    case MINOR_REMOTE_ALARMOUT_OPEN_MAN:
        szTemp.Format("%s", "REMOTE_ALARMOUT_OPEN_MAN");
        break;
    case MINOR_REMOTE_ALARMOUT_CLOSE_MAN:
        szTemp.Format("%s", "REMOTE_ALARMOUT_CLOSE_MAN");
        break;
    case MINOR_REMOTE_OPEN_DOOR:
        szTemp.Format("%s", "REMOTE_OPEN_DOOR");
        break;
    case MINOR_REMOTE_CLOSE_DOOR:
        szTemp.Format("%s", "REMOTE_CLOSE_DOOR");
        break;
    case MINOR_REMOTE_ALWAYS_OPEN:
        szTemp.Format("%s", "REMOTE_ALWAYS_OPEN");
        break;
    case MINOR_REMOTE_ALWAYS_CLOSE:
        szTemp.Format("%s", "REMOTE_ALWAYS_CLOSE");
        break;
    case MINOR_REMOTE_CHECK_TIME:
        szTemp.Format("%s", "REMOTE_CHECK_TIME");
        break;
    case MINOR_NTP_CHECK_TIME:
        szTemp.Format("%s", "NTP_CHECK_TIME");
        break;
    case MINOR_REMOTE_CLEAR_CARD:
        szTemp.Format("%s", "REMOTE_CLEAR_CARD");
        break;
    case MINOR_REMOTE_RESTORE_CFG:
        szTemp.Format("%s", "REMOTE_RESTORE_CFG");
        break;
    case MINOR_ALARMIN_ARM:
        szTemp.Format("%s", "ALARMIN_ARM");
        break;
    case MINOR_ALARMIN_DISARM:
        szTemp.Format("%s", "ALARMIN_DISARM");
        break;
    case MINOR_LOCAL_RESTORE_CFG:
        szTemp.Format("%s", "LOCAL_RESTORE_CFG");
        break;
    case MINOR_REMOTE_CAPTURE_PIC:
        szTemp.Format("%s", "REMOTE_CAPTURE_PIC");
        break;
    case MINOR_MOD_NET_REPORT_CFG:
        szTemp.Format("%s", "MOD_NET_REPORT_CFG");
        break;
    case MINOR_MOD_GPRS_REPORT_PARAM:
        szTemp.Format("%s", "MOD_GPRS_REPORT_PARAM");
        break;
    case MINOR_MOD_REPORT_GROUP_PARAM:
        szTemp.Format("%s", "MOD_REPORT_GROUP_PARAM");
        break;
    case MINOR_UNLOCK_PASSWORD_OPEN_DOOR:
        szTemp.Format("%s", "UNLOCK_PASSWORD_OPEN_DOOR");
        break;
    case MINOR_AUTO_RENUMBER:
        szTemp.Format("%s", "AUTO_RENUMBER");
        break;
    case MINOR_AUTO_COMPLEMENT_NUMBER:
        szTemp.Format("%s", "AUTO_COMPLEMENT_NUMBER");
        break;
    case MINOR_NORMAL_CFGFILE_INPUT:
        szTemp.Format("%s", "NORMAL_CFGFILE_INPUT");
        break;
    case MINOR_NORMAL_CFGFILE_OUTTPUT:
        szTemp.Format("%s", "NORMAL_CFGFILE_OUTTPUT");
        break;
    case MINOR_CARD_RIGHT_INPUT:
        szTemp.Format("%s", "CARD_RIGHT_INPUT");
        break;
    case MINOR_CARD_RIGHT_OUTTPUT:
        szTemp.Format("%s", "CARD_RIGHT_OUTTPUT");
        break;
    case MINOR_LOCAL_USB_UPGRADE:
        szTemp.Format("%s", "LOCAL_USB_UPGRADE");
        break;
    case MINOR_REMOTE_VISITOR_CALL_LADDER:
        szTemp.Format("%s", "REMOTE_VISITOR_CALL_LADDER");
        break;
    case MINOR_REMOTE_HOUSEHOLD_CALL_LADDER:
        szTemp.Format("%s", "REMOTE_HOUSEHOLD_CALL_LADDER");
        break;
    case MINOR_REMOTE_ACTUAL_GUARD:
        szTemp.Format("%s", "REMOTE_ACTUAL_GUARD");
        break;
    case MINOR_REMOTE_ACTUAL_UNGUARD:
        szTemp.Format("%s", "REMOTE_ACTUAL_UNGUARD");
        break;
    case MINOR_REMOTE_CONTROL_NOT_CODE_OPER_FAILED:
        szTemp.Format("%s", "REMOTE_CONTROL_NOT_CODE_OPER_FAILED");
        break;
    case MINOR_REMOTE_CONTROL_CLOSE_DOOR:
        szTemp.Format("%s", "REMOTE_CONTROL_CLOSE_DOOR");
        break;
    case MINOR_REMOTE_CONTROL_OPEN_DOOR:
        szTemp.Format("%s", "REMOTE_CONTROL_OPEN_DOOR");
        break;
    case MINOR_REMOTE_CONTROL_ALWAYS_OPEN_DOOR:
        szTemp.Format("%s", "REMOTE_CONTROL_ALWAYS_OPEN_DOOR");
        break;
    case MINOR_M1_CARD_ENCRYPT_VERIFY_OPEN:
        szTemp.Format("%s", "M1_CARD_ENCRYPT_VERIFY_OPEN");
        break;
    case MINOR_M1_CARD_ENCRYPT_VERIFY_CLOSE:
        szTemp.Format("%s", "M1_CARD_ENCRYPT_VERIFY_CLOSE");
        break;
    case MINOR_NFC_FUNCTION_OPEN:
        szTemp.Format("%s", "NFC_FUNCTION_OPEN");
        break;
    case MINOR_NFC_FUNCTION_CLOSE:
        szTemp.Format("%s", "NFC_FUNCTION_CLOSE");
        break;
    case MINOR_LOCAL_PARA_FACTORY_DEFAULT:
        szTemp.Format("%s", "LOCAL_PARA_FACTORY_DEFAULT");
        break;
    case MINOR_REMOTE_PARA_FACTORY_DEFAULT:
        szTemp.Format("%s", "REMOTE_PARA_FACTORY_DEFAULT");
        break;
    default:
        szTemp.Format("0x%x", lpAcsEventCfg->dwMinor);
        break;
    }
}

void CDlgAcsLogSearch::EventMinorTypeMap(LPNET_DVR_ACS_EVENT_CFG lpAcsEventCfg, CString &szTemp)
{
    CString szParaType;
    switch (lpAcsEventCfg->dwMinor)
    {
    case MINOR_LEGAL_CARD_PASS:
        szTemp.Format("%s", "LEGAL_CARD_PASS");
        break;
    case MINOR_CARD_AND_PSW_PASS:
        szTemp.Format("%s", "CARD_AND_PSW_PASS");
        break;
    case MINOR_CARD_AND_PSW_FAIL:
        szTemp.Format("%s", "CARD_AND_PSW_FAIL");
        break;
    case MINOR_CARD_AND_PSW_TIMEOUT:
        szTemp.Format("%s", "CARD_AND_PSW_TIMEOUT");
        break;
    case MINOR_CARD_AND_PSW_OVER_TIME:
        szTemp.Format("%s", "CARD_AND_PSW_OVER_TIME");
        break;
    case MINOR_CARD_NO_RIGHT:
        szTemp.Format("%s", "CARD_NO_RIGHT");
        break;
    case MINOR_CARD_INVALID_PERIOD:
        szTemp.Format("%s", "CARD_INVALID_PERIOD");
        break;
    case MINOR_CARD_OUT_OF_DATE:
        szTemp.Format("%s", "CARD_OUT_OF_DATE");
        break;
    case MINOR_INVALID_CARD:
        szTemp.Format("%s", "INVALID_CARD");
        break;
    case MINOR_ANTI_SNEAK_FAIL:
        szTemp.Format("%s", "ANTI_SNEAK_FAIL");
        break;
    case MINOR_INTERLOCK_DOOR_NOT_CLOSE:
        szTemp.Format("%s", "INTERLOCK_DOOR_NOT_CLOSE");
        break;
    case MINOR_NOT_BELONG_MULTI_GROUP:
        szTemp.Format("%s", "NOT_BELONG_MULTI_GROUP");
        break;
    case MINOR_INVALID_MULTI_VERIFY_PERIOD:
        szTemp.Format("%s", "INVALID_MULTI_VERIFY_PERIOD");
        break;
    case MINOR_MULTI_VERIFY_SUPER_RIGHT_FAIL:
        szTemp.Format("%s", "MULTI_VERIFY_SUPER_RIGHT_FAIL");
        break;
    case MINOR_MULTI_VERIFY_REMOTE_RIGHT_FAIL:
        szTemp.Format("%s", "MULTI_VERIFY_REMOTE_RIGHT_FAIL");
        break;
    case MINOR_MULTI_VERIFY_SUCCESS:
        szTemp.Format("%s", "MULTI_VERIFY_SUCCESS");
        break;
    case MINOR_LEADER_CARD_OPEN_BEGIN:
        szTemp.Format("%s", "LEADER_CARD_OPEN_BEGIN");
        break;
    case MINOR_LEADER_CARD_OPEN_END:
        szTemp.Format("%s", "LEADER_CARD_OPEN_END");
        break;
    case MINOR_ALWAYS_OPEN_BEGIN:
        szTemp.Format("%s", "ALWAYS_OPEN_BEGIN");
        break;
    case MINOR_ALWAYS_OPEN_END:
        szTemp.Format("%s", "ALWAYS_OPEN_END");
        break;
    case MINOR_LOCK_OPEN:
        szTemp.Format("%s", "LOCK_OPEN");
        break;
    case MINOR_LOCK_CLOSE:
        szTemp.Format("%s", "LOCK_CLOSE");
        break;
    case MINOR_DOOR_BUTTON_PRESS:
        szTemp.Format("%s", "DOOR_BUTTON_PRESS");
        break;
    case MINOR_DOOR_BUTTON_RELEASE:
        szTemp.Format("%s", "DOOR_BUTTON_RELEASE");
        break;
    case MINOR_DOOR_OPEN_NORMAL:
        szTemp.Format("%s", "DOOR_OPEN_NORMAL");
        break;
    case MINOR_DOOR_CLOSE_NORMAL:
        szTemp.Format("%s", "DOOR_CLOSE_NORMAL");
        break;
    case MINOR_DOOR_OPEN_ABNORMAL:
        szTemp.Format("%s", "DOOR_OPEN_ABNORMAL");
        break;
    case MINOR_DOOR_OPEN_TIMEOUT:
        szTemp.Format("%s", "DOOR_OPEN_TIMEOUT");
        break;
    case MINOR_ALARMOUT_ON:
        szTemp.Format("%s", "ALARMOUT_ON");
        break;
    case MINOR_ALARMOUT_OFF:
        szTemp.Format("%s", "ALARMOUT_OFF");
        break;
    case MINOR_ALWAYS_CLOSE_BEGIN:
        szTemp.Format("%s", "ALWAYS_CLOSE_BEGIN");
        break;
    case MINOR_ALWAYS_CLOSE_END:
        szTemp.Format("%s", "ALWAYS_CLOSE_END");
        break;
    case MINOR_MULTI_VERIFY_NEED_REMOTE_OPEN:
        szTemp.Format("%s", "MULTI_VERIFY_NEED_REMOTE_OPEN");
        break;
    case MINOR_MULTI_VERIFY_SUPERPASSWD_VERIFY_SUCCESS:
        szTemp.Format("%s", "MULTI_VERIFY_SUPERPASSWD_VERIFY_SUCCESS");
        break;
    case MINOR_MULTI_VERIFY_REPEAT_VERIFY:
        szTemp.Format("%s", "MULTI_VERIFY_REPEAT_VERIFY");
        break;
    case MINOR_MULTI_VERIFY_TIMEOUT:
        szTemp.Format("%s", "MULTI_VERIFY_TIMEOUT");
        break;
    case MINOR_DOORBELL_RINGING:
        szTemp.Format("%s", "DOORBELL_RINGING");
        break;
    case MINOR_FINGERPRINT_COMPARE_PASS:
        szTemp.Format("%s", "FINGERPRINT_COMPARE_PASS");
        break;
    case MINOR_FINGERPRINT_COMPARE_FAIL:
        szTemp.Format("%s", "FINGERPRINT_COMPARE_FAIL");
        break;
    case MINOR_CARD_FINGERPRINT_VERIFY_PASS:
        szTemp.Format("%s", "CARD_FINGERPRINT_VERIFY_PASS");
        break;
    case MINOR_CARD_FINGERPRINT_VERIFY_FAIL:
        szTemp.Format("%s", "CARD_FINGERPRINT_VERIFY_FAIL");
        break;
    case MINOR_CARD_FINGERPRINT_VERIFY_TIMEOUT:
        szTemp.Format("%s", "CARD_FINGERPRINT_VERIFY_TIMEOUT");
        break;
    case MINOR_CARD_FINGERPRINT_PASSWD_VERIFY_PASS:
        szTemp.Format("%s", "CARD_FINGERPRINT_PASSWD_VERIFY_PASS");
        break;
    case MINOR_CARD_FINGERPRINT_PASSWD_VERIFY_FAIL:
        szTemp.Format("%s", "CARD_FINGERPRINT_PASSWD_VERIFY_FAIL");
        break;
    case MINOR_CARD_FINGERPRINT_PASSWD_VERIFY_TIMEOUT:
        szTemp.Format("%s", "CARD_FINGERPRINT_PASSWD_VERIFY_TIMEOUT");
        break;
    case MINOR_FINGERPRINT_PASSWD_VERIFY_PASS:
        szTemp.Format("%s", "FINGERPRINT_PASSWD_VERIFY_PASS");
        break;
    case MINOR_FINGERPRINT_PASSWD_VERIFY_FAIL:
        szTemp.Format("%s", "FINGERPRINT_PASSWD_VERIFY_FAIL");
        break;
    case MINOR_FINGERPRINT_PASSWD_VERIFY_TIMEOUT:
        szTemp.Format("%s", "FINGERPRINT_PASSWD_VERIFY_TIMEOUT");
        break;
    case MINOR_FINGERPRINT_INEXISTENCE:
        szTemp.Format("%s", "FINGERPRINT_INEXISTENCE");
        break;
    case MINOR_CARD_PLATFORM_VERIFY:
        szTemp.Format("%s", "CARD_PLATFORM_VERIFY");
        break;
    case MINOR_LEGAL_MESSAGE:
        szTemp.Format("%s", "MINOR_LEGAL_MESSAGE");
        break;
    case MINOR_ILLEGAL_MESSAGE:
        szTemp.Format("%s", "MINOR_ILLEGAL_MESSAGE");
        break;
    case MINOR_DOOR_OPEN_OR_DORMANT_FAIL:
        szTemp.Format("%s", "DOOR_OPEN_OR_DORMANT_FAIL");
        break;
    case MINOR_AUTH_PLAN_DORMANT_FAIL:
        szTemp.Format("%s", "AUTH_PLAN_DORMANT_FAIL");
        break;
    case MINOR_CARD_ENCRYPT_VERIFY_FAIL:
        szTemp.Format("%s", "CARD_ENCRYPT_VERIFY_FAIL");
        break;
    case MINOR_SUBMARINEBACK_REPLY_FAIL:
        szTemp.Format("%s", "SUBMARINEBACK_REPLY_FAIL");
        break;
    case MINOR_DOOR_OPEN_OR_DORMANT_OPEN_FAIL:
        szTemp.Format("%s", "DOOR_OPEN_OR_DORMANT_OPEN_FAIL");
        break;
    case MINOR_DOOR_OPEN_OR_DORMANT_LINKAGE_OPEN_FAIL:
        szTemp.Format("%s", "DOOR_OPEN_OR_DORMANT_LINKAGE_OPEN_FAIL");
        break;
    case MINOR_TRAILING:
        szTemp.Format("%s", "TRAILING");
        break;
    case MINOR_REVERSE_ACCESS:
        szTemp.Format("%s", "REVERSE_ACCESS");
        break;
    case MINOR_FORCE_ACCESS:
        szTemp.Format("%s", "FORCE_ACCESS");
        break;
    case MINOR_CLIMBING_OVER_GATE:
        szTemp.Format("%s", "CLIMBING_OVER_GATE");
        break;
    case MINOR_PASSING_TIMEOUT:
        szTemp.Format("%s", "PASSING_TIMEOUT");
        break;
    case MINOR_INTRUSION_ALARM:
        szTemp.Format("%s", "INTRUSION_ALARM");
        break;
    case MINOR_FREE_GATE_PASS_NOT_AUTH:
        szTemp.Format("%s", "FREE_GATE_PASS_NOT_AUTH");
        break;
    case MINOR_DROP_ARM_BLOCK:
        szTemp.Format("%s", "DROP_ARM_BLOCK");
        break;
    case MINOR_DROP_ARM_BLOCK_RESUME:
        szTemp.Format("%s", "DROP_ARM_BLOCK_RESUME");
        break;
    case MINOR_LOCAL_FACE_MODELING_FAIL:
        szTemp.Format("%s", "LOCAL_FACE_MODELING_FAIL");
        break;
    case MINOR_STAY_EVENT:
        szTemp.Format("%s", "STAY_EVENT");
        break;
    case MINOR_CERTIFICATE_ALLOWLIST_PASS:
        szTemp.Format("%s", "CERTIFICATE_ALLOWLIST_PASS");
        break;
    case MINOR_CERTIFICATE_ALLOWLIST_FALL:
        szTemp.Format("%s", "CERTIFICATE_ALLOWLIST_FALL");
        break;
    case MINOR_FACE_COLLECTION:
        szTemp.Format("%s", "FACE_COLLECTION");
        break;
    case MINOR_CARD_COLLECTION:
        szTemp.Format("%s", "CARD_COLLECTION");
        break;
    case MINOR_PSW_ERROR_OVER_TIMES:
        szTemp.Format("%s", "PSW_ERROR_OVER_TIMES");
        break;
    case MINOR_ID_CARD_COLLECTION:
        szTemp.Format("%s", "ID_CARD_COLLECTION");
        break;
    case MINOR_PSW_VERIFY_FAIL:
        szTemp.Format("%s", "PSW_VERIFY_FAIL");
        break;
    case MINOR_PASSWORD_MISMATCH:
        szTemp.Format("%s", "PASSWORD_MISMATCH");
        break;
    case MINOR_EMPLOYEE_NO_NOT_EXIST:
        szTemp.Format("%s", "EMPLOYEE_NO_NOT_EXIST");
        break;
    case MINOR_COMBINED_VERIFY_PASS:
        szTemp.Format("%s", "COMBINED_VERIFY_PASS");
        break;
    case MINOR_COMBINED_VERIFY_TIMEOUT:
        szTemp.Format("%s", "COMBINED_VERIFY_TIMEOUT");
        break;
    case MINOR_VERIFY_MODE_MISMATCH:
        szTemp.Format("%s", "VERIFY_MODE_MISMATCH");
        break;
    case MINOR_ORCODE_VERIFY_PASS:
        szTemp.Format("%s", "ORCODE_VERIFY_PASS");
        break;
    case MINOR_ORCODE_VERIFY_FAIL:
        szTemp.Format("%s", "ORCODE_VERIFY_FAIL");
        break;
    case MINOR_HOUSEHOLDER_AUTHORIZE_PASS:
        szTemp.Format("%s", "HOUSEHOLDER_AUTHORIZE_PASS");
        break;
    case MINOR_BLUETOOTH_VERIFY_PASS:
        szTemp.Format("%s", "BLUETOOTH_VERIFY_PASS");
        break;
    case MINOR_BLUETOOTH_VERIFY_FAIL:
        szTemp.Format("%s", "BLUETOOTH_VERIFY_FAIL");
        break;
    case MINOR_PASSPORT_VERIFY_FAIL:
        szTemp.Format("%s", "PASSPORT_VERIFY_FAIL");
        break;
    case MINOR_INFORMAL_M1_CARD_VERIFY_FAIL:
        szTemp.Format("%s", "INFORMAL_M1_CARD_VERIFY_FAIL");
        break;
    case MINOR_CPU_CARD_ENCRYPT_VERIFY_FAIL:
        szTemp.Format("%s", "CPU_CARD_ENCRYPT_VERIFY_FAIL");
        break;
    case MINOR_NFC_DISABLE_VERIFY_FAIL:
        szTemp.Format("%s", "NFC_DISABLE_VERIFY_FAIL");
        break;
    case MINOR_EM_CARD_RECOGNIZE_NOT_ENABLED:
        szTemp.Format("%s", "EM_CARD_RECOGNIZE_NOT_ENABLED");
        break;
    case MINOR_M1_CARD_RECOGNIZE_NOT_ENABLED:
        szTemp.Format("%s", "M1_CARD_RECOGNIZE_NOT_ENABLED");
        break;
    case MINOR_CPU_CARD_RECOGNIZE_NOT_ENABLED:
        szTemp.Format("%s", "CPU_CARD_RECOGNIZE_NOT_ENABLED");
        break;
    case MINOR_ID_CARD_RECOGNIZE_NOT_ENABLED:
        szTemp.Format("%s", "ID_CARD_RECOGNIZE_NOT_ENABLED");
        break;
    case MINOR_CARD_SET_SECRET_KEY_FAIL:
        szTemp.Format("%s", "CARD_SET_SECRET_KEY_FAIL");
        break;
    case MINOR_LOCAL_UPGRADE_FAIL:
        szTemp.Format("%s", "LOCAL_UPGRADE_FAIL");
        break;
    case MINOR_REMOTE_UPGRADE_FAIL:
        szTemp.Format("%s", "REMOTE_UPGRADE_FAIL");
        break;
    case MINOR_REMOTE_EXTEND_MODULE_UPGRADE_SUCC:
        szTemp.Format("%s", "REMOTE_EXTEND_MODULE_UPGRADE_SUCC");
        break;
    case MINOR_REMOTE_EXTEND_MODULE_UPGRADE_FAIL:
        szTemp.Format("%s", "REMOTE_EXTEND_MODULE_UPGRADE_FAIL");
        break;
    case MINOR_REMOTE_FINGER_PRINT_MODULE_UPGRADE_SUCC:
        szTemp.Format("%s", "REMOTE_FINGER_PRINT_MODULE_UPGRADE_SUCC");
        break;
    case MINOR_REMOTE_FINGER_PRINT_MODULE_UPGRADE_FAIL:
        szTemp.Format("%s", "REMOTE_FINGER_PRINT_MODULE_UPGRADE_FAIL");
        break;
    case MINOR_PASSWD_VERIFY_PASS:
        szTemp.Format("%s", "PASSWD_VERIFY_PASS");
        break;
    case MINOR_PEOPLE_AND_ID_CARD_COMPARE_PASS:
        szTemp.Format("%s", "PEOPLE_AND_ID_CARD_COMPARE_PASS");
        break;
    case MINOR_PEOPLE_AND_ID_CARD_COMPARE_FAIL:
        szTemp.Format("%s", "PEOPLE_AND_ID_CARD_COMPARE_FAIL");
        break;
    default:
        szTemp.Format("0x%x", lpAcsEventCfg->dwMinor);
        break;
    }
}

void CALLBACK g_fGetAcsEventCallback(DWORD dwType, void* lpBuffer, DWORD dwBufLen, void* pUserData)
{
    CDlgAcsLogSearch* pDlg = (CDlgAcsLogSearch*)pUserData;
    if (pDlg == NULL)
    {
        return;
    }
    pDlg->ProcessGetAcsEventCallbackData(dwType, lpBuffer, dwBufLen);
}

void CDlgAcsLogSearch::ProcessGetAcsEventCallbackData(DWORD dwType, void* lpBuffer, DWORD dwBufLen)
{
    CString strItem = "";
    if (dwType == NET_SDK_CALLBACK_TYPE_DATA)
    {
        LPNET_DVR_ACS_EVENT_CFG lpAcsEventCfg = new NET_DVR_ACS_EVENT_CFG;
        memcpy(lpAcsEventCfg, lpBuffer, sizeof(*lpAcsEventCfg));
        if (lpAcsEventCfg->dwPicDataLen != 0)
        {
            char* pPicData = new char[lpAcsEventCfg->dwPicDataLen];
            if (lpAcsEventCfg->pPicData == NULL)
            {
                return;
            }
            memset(pPicData, 0, lpAcsEventCfg->dwPicDataLen);
            memcpy(pPicData, lpAcsEventCfg->pPicData, lpAcsEventCfg->dwPicDataLen);
            lpAcsEventCfg->pPicData = pPicData;
        }
        if (lpAcsEventCfg->struAcsEventInfo.dwRecordChannelNum != 0)
        {
            char* pRecordChannelData = new char[lpAcsEventCfg->struAcsEventInfo.dwRecordChannelNum * sizeof(int)];
            if (lpAcsEventCfg->struAcsEventInfo.pRecordChannelData == NULL)
            {
                return;
            }
            memset(pRecordChannelData, 0, lpAcsEventCfg->struAcsEventInfo.dwRecordChannelNum * sizeof(int));
            memcpy(pRecordChannelData, lpAcsEventCfg->struAcsEventInfo.pRecordChannelData, lpAcsEventCfg->struAcsEventInfo.dwRecordChannelNum * sizeof(int));
            lpAcsEventCfg->struAcsEventInfo.pRecordChannelData = pRecordChannelData;
        }
        PostMessage(WM_MSG_ADD_ACS_EVENT_TOLIST, (WPARAM)lpAcsEventCfg, 0);
    }
    else if (dwType == NET_SDK_CALLBACK_TYPE_STATUS)
    {
        DWORD dwStatus = *(DWORD*)lpBuffer;
        if (dwStatus == NET_SDK_CALLBACK_STATUS_SUCCESS)
        {
            PostMessage(WM_MSG_GET_ACS_EVENT_FINISH, 0, 0);
        }
        else if (dwStatus == NET_SDK_CALLBACK_STATUS_FAILED)
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_GET_ACS_EVENT failed");
        }
    }
}

LRESULT CDlgAcsLogSearch::OnMsgAddAcsEventToList(WPARAM wParam, LPARAM lParam)
{
    LPNET_DVR_ACS_EVENT_CFG lpAcsEventCfg = (LPNET_DVR_ACS_EVENT_CFG)wParam;
    if (lpAcsEventCfg->dwSize == 0)
    {
        if (lpAcsEventCfg->pPicData != NULL)
        {
            delete[] lpAcsEventCfg->pPicData;
        }
        lpAcsEventCfg->pPicData = NULL;

        if (lpAcsEventCfg->struAcsEventInfo.pRecordChannelData != NULL)
        {
            delete[] lpAcsEventCfg->struAcsEventInfo.pRecordChannelData;
        }
        lpAcsEventCfg->struAcsEventInfo.pRecordChannelData = NULL;

        if (lpAcsEventCfg != NULL)
        {
            delete lpAcsEventCfg;
        }
        lpAcsEventCfg = NULL;
        return 0;
    }

    LONG bRet = -1;

    CString csTmp;
    char szLan[128] = { 0 };
    char szCardNo[33] = { 0 };
    char szEmployeeNo[33] = { 0 };

    csTmp.Format("%d", pLogSearch->m_lLogNum + 1);
    pLogSearch->m_listDeviceLog.InsertItem(pLogSearch->m_lLogNum, csTmp, 0);

    csTmp.Format("%04d-%02d-%02d %02d:%02d:%02d", lpAcsEventCfg->struTime.dwYear, lpAcsEventCfg->struTime.dwMonth, lpAcsEventCfg->struTime.dwDay, \
        lpAcsEventCfg->struTime.dwHour, lpAcsEventCfg->struTime.dwMinute, lpAcsEventCfg->struTime.dwSecond);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 1, csTmp);

    switch (lpAcsEventCfg->dwMajor)
    {
    case 1:
        g_StringLanType(szLan, "报警", "Alarm");
        csTmp.Format("%s", szLan);
        break;
    case 2:
        g_StringLanType(szLan, "异常", "Exception");
        csTmp.Format("%s", szLan);
        break;
    case 3:
        g_StringLanType(szLan, "操作", "Operation");
        csTmp.Format("%s", szLan);
        break;
    case 5:
        g_StringLanType(szLan, "事件", "Event");
        csTmp.Format("%s", szLan);
        break;
    default:
        g_StringLanType(szLan, "未知类型", "Unknown");
        csTmp.Format("%s", szLan);
        break;
    }
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 2, csTmp);

    if (MAJOR_ALARM == lpAcsEventCfg->dwMajor)
    {
        AlarmMinorTypeMap(lpAcsEventCfg, csTmp);
    }
    else if (MAJOR_EXCEPTION == lpAcsEventCfg->dwMajor)
    {
        ExceptionMinorTypeMap(lpAcsEventCfg, csTmp);
    }
    else if (MAJOR_OPERATION == lpAcsEventCfg->dwMajor)
    {
        OperationMinorTypeMap(lpAcsEventCfg, csTmp);
    }
    else if (MAJOR_EVENT == lpAcsEventCfg->dwMajor)
    {
        EventMinorTypeMap(lpAcsEventCfg, csTmp);
    }
    else
    {
        g_StringLanType(szLan, "未知类型", "Unknown");
        csTmp.Format("%s", szLan);
    }
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 3, csTmp);

    memset(szCardNo, 0, 33);
    memcpy(szCardNo, lpAcsEventCfg->struAcsEventInfo.byCardNo, ACS_CARD_NO_LEN);
    csTmp.Format("%s", szCardNo);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 4, csTmp);

    if (lpAcsEventCfg->struAcsEventInfo.byCardType == 1)
    {
        g_StringLanType(szLan, "普通卡", "Ordinary Card");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCardType == 2)
    {
        g_StringLanType(szLan, "特殊群体卡", "Accessible Card");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCardType == 3)
    {
        g_StringLanType(szLan, "禁止名单卡", "Block List Card");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCardType == 4)
    {
        g_StringLanType(szLan, "巡更卡", "Patrol Card");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCardType == 5)
    {
        g_StringLanType(szLan, "胁迫卡", "Stress Card");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCardType == 6)
    {
        g_StringLanType(szLan, "超级卡", "Super Card");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCardType == 7)
    {
        g_StringLanType(szLan, "来宾卡", "Guest Card");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCardType == 8)
    {
        g_StringLanType(szLan, "解除卡", "Release Card");
        csTmp.Format("%s", szLan);
    }
    else
    {
        g_StringLanType(szLan, "无效", "No effect");
        csTmp.Format("%s", szLan);
    }
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 5, csTmp);

    csTmp.Format("%d", lpAcsEventCfg->struAcsEventInfo.byAllowListNo);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 6, csTmp);

    if (lpAcsEventCfg->struAcsEventInfo.byReportChannel == 1)
    {
        g_StringLanType(szLan, "布防上传", "Upload");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byReportChannel == 2)
    {
        g_StringLanType(szLan, "中心组1上传", "Center 1 Upload");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byReportChannel == 3)
    {
        g_StringLanType(szLan, "中心组2上传", "Center 2 Upload");
        csTmp.Format("%s", szLan);
    }
    else
    {
        g_StringLanType(szLan, "无效", "No effect");
        csTmp.Format("%s", szLan);
    }
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 7, csTmp);

    if (lpAcsEventCfg->struAcsEventInfo.byCardReaderKind == 1)
    {
        g_StringLanType(szLan, "IC读卡器", "IC Reader");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCardReaderKind == 2)
    {
        g_StringLanType(szLan, "身份证读卡器", "Certificate Reader");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCardReaderKind == 3)
    {
        g_StringLanType(szLan, "二维码读卡器", "Two-dimension Reader");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCardReaderKind == 4)
    {
        g_StringLanType(szLan, "指纹头", "Finger Print Head");
        csTmp.Format("%s", szLan);
    }
    else
    {
        g_StringLanType(szLan, "无效", "No effect");
        csTmp.Format("%s", szLan);
    }
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 8, csTmp);

    csTmp.Format("%d", lpAcsEventCfg->struAcsEventInfo.dwCardReaderNo);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 9, csTmp);

    csTmp.Format("%d", lpAcsEventCfg->struAcsEventInfo.dwDoorNo);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 10, csTmp);

    csTmp.Format("%d", lpAcsEventCfg->struAcsEventInfo.dwVerifyNo);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 11, csTmp);

    csTmp.Format("%d", lpAcsEventCfg->struAcsEventInfo.dwAlarmInNo);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 12, csTmp);

    csTmp.Format("%d", lpAcsEventCfg->struAcsEventInfo.dwAlarmOutNo);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 13, csTmp);

    csTmp.Format("%d", lpAcsEventCfg->struAcsEventInfo.dwCaseSensorNo);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 14, csTmp);

    csTmp.Format("%d", lpAcsEventCfg->struAcsEventInfo.dwRs485No);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 15, csTmp);

    csTmp.Format("%d", lpAcsEventCfg->struAcsEventInfo.dwMultiCardGroupNo);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 16, csTmp);

    csTmp.Format("%d", lpAcsEventCfg->struAcsEventInfo.wAccessChannel);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 17, csTmp);

    csTmp.Format("%d", lpAcsEventCfg->struAcsEventInfo.byDeviceNo);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 18, csTmp);

    csTmp.Format("%d", lpAcsEventCfg->struAcsEventInfo.dwEmployeeNo);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 19, csTmp);

    csTmp.Format("%d", lpAcsEventCfg->struAcsEventInfo.byDistractControlNo);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 20, csTmp);

    csTmp.Format("%d", lpAcsEventCfg->struAcsEventInfo.wLocalControllerID);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 21, csTmp);

    if (lpAcsEventCfg->struAcsEventInfo.byInternetAccess == 1)
    {
        g_StringLanType(szLan, "上行网口1", "Up Network Port 1");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byInternetAccess == 2)
    {
        g_StringLanType(szLan, "上行网口2", "Up Network Port 2");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byInternetAccess == 3)
    {
        g_StringLanType(szLan, "下行网口1", "Down Network Port 1");
        csTmp.Format("%s", szLan);
    }
    else
    {
        g_StringLanType(szLan, "无效", "No effect");
        csTmp.Format("%s", szLan);
    }
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 22, csTmp);

    if (lpAcsEventCfg->struAcsEventInfo.byType == 0)
    {
        g_StringLanType(szLan, "即时防区", "Instant Zone");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byType == 1)
    {
        g_StringLanType(szLan, "24小时防区", "24 Hour Zone");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byType == 2)
    {
        g_StringLanType(szLan, "延时防区", "Delay Zone");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byType == 3)
    {
        g_StringLanType(szLan, "内部防区", "Internal Zone");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byType == 4)
    {
        g_StringLanType(szLan, "钥匙防区", "Key Zone");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byType == 5)
    {
        g_StringLanType(szLan, "火警防区", "Fire Zone");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byType == 6)
    {
        g_StringLanType(szLan, "周界防区", "Perimeter Zone");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byType == 7)
    {
        g_StringLanType(szLan, "24小时无声防区", "24 Hour Silent Zone");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byType == 8)
    {
        g_StringLanType(szLan, "24小时辅助防区", "24 Hour Auxiliary Zone");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byType == 9)
    {
        g_StringLanType(szLan, "24小时震动防区", "24 Hour Vibration Zone");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byType == 10)
    {
        g_StringLanType(szLan, "门禁紧急开门防区", "Acs Emergency Open Zone");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byType == 11)
    {
        g_StringLanType(szLan, "门禁紧急关门防区", "Acs Emergency Close Zone");
        csTmp.Format("%s", szLan);
    }
    else
    {
        g_StringLanType(szLan, "无效", "No effect");
        csTmp.Format("%s", szLan);
    }
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 23, csTmp);

    if (lpAcsEventCfg->struAcsEventInfo.byMACAddr[0] != 0)
    {
        csTmp.Format("%02x:%02x:%02x:%02x:%02x:%02x", lpAcsEventCfg->struAcsEventInfo.byMACAddr[0], \
            lpAcsEventCfg->struAcsEventInfo.byMACAddr[1], lpAcsEventCfg->struAcsEventInfo.byMACAddr[2], lpAcsEventCfg->struAcsEventInfo.byMACAddr[3], \
            lpAcsEventCfg->struAcsEventInfo.byMACAddr[4], lpAcsEventCfg->struAcsEventInfo.byMACAddr[5]);
    }
    else
    {
        g_StringLanType(szLan, "无效", "No effect");
        csTmp.Format("%s", szLan);
    }
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 24, csTmp);

    if (lpAcsEventCfg->struAcsEventInfo.bySwipeCardType == 1)
    {
        g_StringLanType(szLan, "二维码", "QR Code");
        csTmp.Format("%s", szLan);
    }
    else
    {
        g_StringLanType(szLan, "无效", "No effect");
        csTmp.Format("%s", szLan);
    }
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 25, csTmp);

    csTmp.Format("%u", lpAcsEventCfg->struAcsEventInfo.dwSerialNo);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 26, csTmp);

    csTmp.Format("%d", lpAcsEventCfg->struAcsEventInfo.byChannelControllerID);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 27, csTmp);

    csTmp.Format("%d", lpAcsEventCfg->struAcsEventInfo.byChannelControllerLampID);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 28, csTmp);

    csTmp.Format("%d", lpAcsEventCfg->struAcsEventInfo.byChannelControllerIRAdaptorID);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 29, csTmp);

    csTmp.Format("%d", lpAcsEventCfg->struAcsEventInfo.byChannelControllerIREmitterID);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 30, csTmp);

    if (lpAcsEventCfg->wInductiveEventType < sizeof(szAcs) / sizeof(char*))
    {
        pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 31, szAcs[lpAcsEventCfg->wInductiveEventType]);
    }
    else
    {
        pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 31, "Invalid");
    }

    for (int iIndex = 0; iIndex < lpAcsEventCfg->struAcsEventInfo.dwRecordChannelNum; iIndex++)
    {
        csTmp.Format("%s %d", csTmp, (int*)(lpAcsEventCfg->struAcsEventInfo.pRecordChannelData)++);
    }
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 32, csTmp);

    if (lpAcsEventCfg->struAcsEventInfo.byUserType == 1)
    {
        g_StringLanType(szLan, "普通人", "host");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byUserType == 2)
    {
        g_StringLanType(szLan, "来宾", "visitor");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byUserType == 3)
    {
        g_StringLanType(szLan, "禁止名单人", "block list");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byUserType == 4)
    {
        g_StringLanType(szLan, "管理员", "administrator");
        csTmp.Format("%s", szLan);
    }
    else
    {
        g_StringLanType(szLan, "无效", "No effect");
        csTmp.Format("%s", szLan);
    }
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 33, csTmp);

    if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 1)
    {
        g_StringLanType(szLan, "休眠", "dormancy");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 2)
    {
        g_StringLanType(szLan, "刷卡+密码", "cardAndPw");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 3)
    {
        g_StringLanType(szLan, "刷卡", "card");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 4)
    {
        g_StringLanType(szLan, "刷卡或密码", "cardOrPw");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 5)
    {
        g_StringLanType(szLan, "指纹", "fp");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 6)
    {
        g_StringLanType(szLan, "指纹+密码", "fpAndPw");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 7)
    {
        g_StringLanType(szLan, "指纹或刷卡", "fpOrCard");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 8)
    {
        g_StringLanType(szLan, "指纹+刷卡", "fpAndCard");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 9)
    {
        g_StringLanType(szLan, "指纹+刷卡+密码", "fpAndCardAndPw");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 10)
    {
        g_StringLanType(szLan, "人脸或指纹或刷卡或密码", "faceOrFpOrCardOrPw");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 11)
    {
        g_StringLanType(szLan, "人脸+指纹", "faceAndFp");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 12)
    {
        g_StringLanType(szLan, "人脸+密码", "faceAndPw");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 13)
    {
        g_StringLanType(szLan, "人脸+刷卡", "faceAndCard");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 14)
    {
        g_StringLanType(szLan, "人脸", "face");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 15)
    {
        g_StringLanType(szLan, "工号+密码", "employeeNoAndPw");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 16)
    {
        g_StringLanType(szLan, "指纹或密码", "fpOrPw");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 17)
    {
        g_StringLanType(szLan, "工号+指纹", "employeeNoAndFp");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 18)
    {
        g_StringLanType(szLan, "工号+指纹+密码", "employeeNoAndFpAndPw");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 19)
    {
        g_StringLanType(szLan, "人脸+指纹+刷卡", "faceAndFpAndCard");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 20)
    {
        g_StringLanType(szLan, "人脸+密码+指纹", "faceAndPwAndFp");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 21)
    {
        g_StringLanType(szLan, "工号+人脸", "employeeNoAndFace");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 22)
    {
        g_StringLanType(szLan, "人脸或人脸+刷卡", "faceOrfaceAndCard");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 23)
    {
        g_StringLanType(szLan, "指纹或人脸", "fpOrface");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 24)
    {
        g_StringLanType(szLan, "刷卡或人脸或密码", "cardOrfaceOrPw");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 25)
    {
        g_StringLanType(szLan, "刷卡或人脸", "cardOrFace");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 26)
    {
        g_StringLanType(szLan, "刷卡或人脸或指纹", "cardOrFaceOrFp");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 27)
    {
        g_StringLanType(szLan, "刷卡或指纹或密码", "cardOrFpOrPw");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 28)
    {
        g_StringLanType(szLan, "人脸或密码", "faceOrPw");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 29)
    {
        g_StringLanType(szLan, "工号+人脸+密码", "employeeNoAndFaceAndPw");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 30)
    {
        g_StringLanType(szLan, "刷卡或人脸或人脸+刷卡", "cardOrFaceOrFaceAndCard");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 31)
    {
        g_StringLanType(szLan, "人脸或指纹或密码", "faceOrFpOrPw");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 32)
    {
        g_StringLanType(szLan, "虹膜", "iris");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 33)
    {
        g_StringLanType(szLan, "人脸或指纹或刷卡或密码或虹膜", "faceOrFpOrPwOrIris");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byCurrentVerifyMode == 34)
    {
        g_StringLanType(szLan, "人脸或刷卡或密码或虹膜", "faceOrCardOrPwOrIris");
        csTmp.Format("%s", szLan);
    }
    else
    {
        g_StringLanType(szLan, "无效", "No effect");
        csTmp.Format("%s", szLan);
    }
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 34, csTmp);

    if (lpAcsEventCfg->struAcsEventInfo.byAttendanceStatus == 1)
    {
        g_StringLanType(szLan, "上班", "checkIn");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byAttendanceStatus == 2)
    {
        g_StringLanType(szLan, "下班", "checkOut");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byAttendanceStatus == 3)
    {
        g_StringLanType(szLan, "开始休息", "breakOut");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byAttendanceStatus == 4)
    {
        g_StringLanType(szLan, "结束休息", "breakIn");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byAttendanceStatus == 5)
    {
        g_StringLanType(szLan, "开始加班", "overtimeIn");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byAttendanceStatus == 6)
    {
        g_StringLanType(szLan, "结束加班", "overtimeOut");
        csTmp.Format("%s", szLan);
    }
    else
    {
        g_StringLanType(szLan, "未定义", "undefined");
        csTmp.Format("%s", szLan);
    }
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 35, csTmp);

    csTmp.Format("%d", lpAcsEventCfg->struAcsEventInfo.byStatusValue);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 36, csTmp);

    memset(szEmployeeNo, 0, 33);
    memcpy(szEmployeeNo, lpAcsEventCfg->struAcsEventInfo.byEmployeeNo, NET_SDK_EMPLOYEE_NO_LEN);
    csTmp.Format("%s", szEmployeeNo);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 37, csTmp);

    char szRemoteHostAddr[17] = { 0 };
    memcpy(szRemoteHostAddr, lpAcsEventCfg->struRemoteHostAddr.sIpV4, 16);
    csTmp.Format("%s", szRemoteHostAddr);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 38, csTmp);

    if (g_struDeviceInfo[m_iDeviceIndex].byCharaterEncodeType == 6)
    {
        BYTE tempNetUser[MAX_NAMELEN + 1] = {0};
        memcpy(tempNetUser, lpAcsEventCfg->sNetUser, sizeof(lpAcsEventCfg->sNetUser));
        string str_gb2312 = UTF2GB((const char *)tempNetUser);
        memset(lpAcsEventCfg->sNetUser, 0, sizeof(lpAcsEventCfg->sNetUser));
        strncpy((char *)lpAcsEventCfg->sNetUser, str_gb2312.c_str(), str_gb2312.length());
    }
    char szNetUser[MAX_NAMELEN + 1] = { 0 };
    memcpy(szNetUser, lpAcsEventCfg->sNetUser, MAX_NAMELEN);
    csTmp.Format("%s", szNetUser);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 39, csTmp);

    if (lpAcsEventCfg->struAcsEventInfo.byEventAttribute == 1)
    {
        g_StringLanType(szLan, "合法事件", "attendance");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byEventAttribute == 2)
    {
        g_StringLanType(szLan, "其它", "other");
        csTmp.Format("%s", szLan);
    }
    else
    {
        g_StringLanType(szLan, "无效", "No effect");
        csTmp.Format("%s", szLan);
    }
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 40, csTmp);

    if (lpAcsEventCfg->struAcsEventInfo.byMask == 1)
    {
        g_StringLanType(szLan, "未知", "unknown");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byMask == 2)
    {
        g_StringLanType(szLan, "不戴口罩", "no");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byMask == 3)
    {
        g_StringLanType(szLan, "戴口罩", "yes");
        csTmp.Format("%s", szLan);
    }
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 41, csTmp);

    if (lpAcsEventCfg->struAcsEventInfo.byThermometryUnit == 0)
    {
        g_StringLanType(szLan, "摄氏度", "C");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byThermometryUnit == 1)
    {
        g_StringLanType(szLan, "华氏度", "F");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byThermometryUnit == 2)
    {
        g_StringLanType(szLan, "开尔文", "K");
        csTmp.Format("%s", szLan);
    }
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 42, csTmp);

    if (lpAcsEventCfg->struAcsEventInfo.byIsAbnomalTemperature == 0)
    {
        g_StringLanType(szLan, "否", "no");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byIsAbnomalTemperature == 1)
    {
        g_StringLanType(szLan, "是", "yes");
        csTmp.Format("%s", szLan);
    }
    
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 43, csTmp);

    csTmp.Format("%f", lpAcsEventCfg->struAcsEventInfo.fCurrTemperature);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 44, csTmp);

    csTmp.Format("x:%f y:%f", lpAcsEventCfg->struAcsEventInfo.struRegionCoordinates.fX,lpAcsEventCfg->struAcsEventInfo.struRegionCoordinates.fY);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 45, csTmp);

    csTmp.Format("XCoordinate:%d YCoordinate:%d Width:%d Height:%d", lpAcsEventCfg->struAcsEventInfo.byHealthCode, lpAcsEventCfg->struAcsEventInfo.wYCoordinate,
        lpAcsEventCfg->struAcsEventInfo.wWidth, lpAcsEventCfg->struAcsEventInfo.wHeight);
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 46, csTmp);

    if (lpAcsEventCfg->struAcsEventInfo.byHealthCode == 0)
    {
        g_StringLanType(szLan, "保留", "res");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byHealthCode == 1)
    {
        g_StringLanType(szLan, "未请求", "no request");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byHealthCode == 2)
    {
        g_StringLanType(szLan, "未申领", "no apply");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byHealthCode == 3)
    {
        g_StringLanType(szLan, "绿码", "green code");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byHealthCode == 4)
    {
        g_StringLanType(szLan, "黄码", "yellow code");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byHealthCode == 5)
    {
        g_StringLanType(szLan, "红码", "red code");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byHealthCode == 6)
    {
        g_StringLanType(szLan, "无此人员", "no such person");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byHealthCode == 7)
    {
        g_StringLanType(szLan, "其他错误信息", "other error info");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byHealthCode == 8)
    {
        g_StringLanType(szLan, "查询健康码超时", "health code query timeout");
        csTmp.Format("%s", szLan);
    }
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 47, csTmp);

    if (lpAcsEventCfg->struAcsEventInfo.byNADCode == 0)
    {
        g_StringLanType(szLan, "保留", "res");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byNADCode == 1)
    {
        g_StringLanType(szLan, "未查询到核酸检测结果", "no nucleic acid test results");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byNADCode == 2)
    {
        g_StringLanType(szLan, "核酸检测阴性（代表正常）", "nucleic acid test negative");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byNADCode == 3)
    {
        g_StringLanType(szLan, "核酸检测阳性（代表确诊）", "nucleic acid test positive");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byNADCode == 4)
    {
        g_StringLanType(szLan, "核酸检测有效期已过", "validity period of nucleic acid detection has expired");
        csTmp.Format("%s", szLan);
    }
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 48, csTmp);

    if (lpAcsEventCfg->struAcsEventInfo.byTravelCode == 0)
    {
        g_StringLanType(szLan, "保留", "res");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byTravelCode == 1)
    {
        g_StringLanType(szLan, "14天内一直在当地", "been the local place for 14 days");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byTravelCode == 2)
    {
        g_StringLanType(szLan, "14天内离开过当地", "have left the place within 14 days");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byTravelCode == 3)
    {
        g_StringLanType(szLan, "14天内到过疫区", "have been to the epidemic area within 14 days");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byTravelCode == 4)
    {
        g_StringLanType(szLan, "其他", "other");
        csTmp.Format("%s", szLan);
    }
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 49, csTmp);

    if (lpAcsEventCfg->struAcsEventInfo.byVaccineStatus == 0)
    {
        g_StringLanType(szLan, "保留", "res");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byVaccineStatus == 1)
    {
        g_StringLanType(szLan, "未打疫苗", "not vaccinated");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byVaccineStatus == 2)
    {
        g_StringLanType(szLan, "部分注射疫苗", "partial vaccinated");
        csTmp.Format("%s", szLan);
    }
    else if (lpAcsEventCfg->struAcsEventInfo.byVaccineStatus == 3)
    {
        g_StringLanType(szLan, "打过疫苗", "vaccinated");
        csTmp.Format("%s", szLan);
    }
    pLogSearch->m_listDeviceLog.SetItemText(pLogSearch->m_lLogNum, 50, csTmp);

    
    //if (lpAcsEventCfg->dwMinor == 0x4c)
    //{
        SavePictureToLocal(lpAcsEventCfg, "AcsEventPicture");
        SavePictureToLocal(lpAcsEventCfg, "QRCodeInfo");
        SavePictureToLocal(lpAcsEventCfg, "VisibileLightData");
        SavePictureToLocal(lpAcsEventCfg, "ThermalData");
    //}
    

    pLogSearch->m_lLogNum++;
    if (lpAcsEventCfg->pPicData != NULL)
    {
        delete[] lpAcsEventCfg->pPicData;
    }
    lpAcsEventCfg->pPicData = NULL;
    if (lpAcsEventCfg != NULL)
    {
        delete lpAcsEventCfg;
    }
    lpAcsEventCfg = NULL;
    return 0;
}


int CDlgAcsLogSearch::SavePictureToLocal(LPNET_DVR_ACS_EVENT_CFG lpAcsEventCfg, char* pPictureType)
{
    char* pPictureData = NULL;
    int iPictureLength = 0;
    
    if (strcmp(pPictureType, "AcsEventPicture") == 0)
    {
        pPictureData = lpAcsEventCfg->pPicData;
        iPictureLength = lpAcsEventCfg->dwPicDataLen;
    }
    else if (strcmp(pPictureType, "QRCodeInfo") == 0)
    {
        pPictureData = lpAcsEventCfg->pQRCodeInfo;
        iPictureLength = lpAcsEventCfg->dwQRCodeInfoLen;
    }
    else if (strcmp(pPictureType, "VisibileLightData") == 0)
    {
        pPictureData = lpAcsEventCfg->pVisibleLightData;
        iPictureLength = lpAcsEventCfg->dwVisibleLightDataLen;
    }
    else if (strcmp(pPictureType, "ThermalData") == 0)
    {
        pPictureData = lpAcsEventCfg->pThermalData;
        iPictureLength = lpAcsEventCfg->dwThermalDataLen;
    }

    if (iPictureLength > 0 && pPictureData != NULL)
    {
        SYSTEMTIME t;
        GetLocalTime(&t);
        char chTime[128];
        char cPicName[256] = { 0 };
        DWORD dwWrittenBytes = 0;
        sprintf(chTime, "%4.4d-%2.2d-%2.2d_%2.2d-%2.2d-%2.2d", lpAcsEventCfg->struTime.dwYear, lpAcsEventCfg->struTime.dwMonth, lpAcsEventCfg->struTime.dwDay, \
            lpAcsEventCfg->struTime.dwHour, lpAcsEventCfg->struTime.dwMinute, lpAcsEventCfg->struTime.dwSecond);

        sprintf(cPicName, "%s\\%s", g_struLocalParam.chPictureSavePath, g_struDeviceInfo[m_iDeviceIndex].chDeviceIP);

        if (GetFileAttributes(cPicName) != FILE_ATTRIBUTE_DIRECTORY)
        {
            CreateDirectory(cPicName, NULL);
        }

        sprintf(cPicName, "%s\\LocalTime[%s]Major[0x%x]Minor[0x%x]_%s_%d.jpg", cPicName, chTime, lpAcsEventCfg->dwMajor, lpAcsEventCfg->dwMinor, pPictureType, pLogSearch->m_lLogNum);

        HANDLE hFile = CreateFile(cPicName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            if (pPictureData != NULL)
            {
                delete[] pPictureData;
            }
            pPictureData = NULL;
            if (lpAcsEventCfg != NULL)
            {
                delete lpAcsEventCfg;
            }
            lpAcsEventCfg = NULL;
            return 0;
        }
        DWORD dwRet = WriteFile(hFile, pPictureData, iPictureLength, &dwWrittenBytes, NULL);
        if (dwRet == 0 || dwWrittenBytes < iPictureLength)
        {
            DWORD dwError = GetLastError();
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "Acs Event Picture Write to File Fail.System Error:%d, dwPicLen:%d, pBuffer:%d ", dwError, lpAcsEventCfg->dwPicDataLen, dwWrittenBytes);
            return 0;
        }
        CloseHandle(hFile);
        hFile = NULL;
        return 1;
    }
    return 0;
}

LRESULT CDlgAcsLogSearch::OnMsgGetAcsEventFinish(WPARAM wParam, LPARAM lParam)
{
    NET_DVR_StopRemoteConfig(m_lGetAcsEvent);
    m_lGetAcsEvent = -1;
    m_lLogNum = 0;
    g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_GET_ACS_EVENT finish");
    return 0;
}
