﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.IO;

namespace AlarmCSharpDemo
{
    public partial class AlarmDemo : Form
    {
        private Int32 m_lUserID = -1;
        private Int32[] m_lAlarmHandle = new Int32[200];
        private Int32 iListenHandle = -1; 
        private int iDeviceNumber = 0; //添加设备个数
        private uint iLastErr = 0;
        private string strErr;
        private CHCNetSDK.MSGCallBack_V31 m_falarmData_V31 = null;
        private CHCNetSDK.MSGCallBack m_falarmData = null;
        public delegate void UpdateListBoxCallback(string strAlarmTime, string strDevIP, string strAlarmMsg);

        CHCNetSDK.NET_VCA_TRAVERSE_PLANE m_struTraversePlane = new CHCNetSDK.NET_VCA_TRAVERSE_PLANE();
        CHCNetSDK.NET_VCA_AREA m_struVcaArea = new CHCNetSDK.NET_VCA_AREA();
        CHCNetSDK.NET_VCA_INTRUSION m_struIntrusion = new CHCNetSDK.NET_VCA_INTRUSION();
        CHCNetSDK.UNION_STATFRAME m_struStatFrame = new CHCNetSDK.UNION_STATFRAME();
        CHCNetSDK.UNION_STATTIME m_struStatTime = new CHCNetSDK.UNION_STATTIME();
        public AlarmDemo()
        {
            InitializeComponent();
            bool m_bInitSDK = CHCNetSDK.NET_DVR_Init();
            if (m_bInitSDK == false)
            {
                MessageBox.Show("NET_DVR_Init error!");
                return;
            }
            else
            {
                byte[] strIP = new byte[16 * 16];
                uint dwValidNum=0;
                Boolean bEnableBind=false;

                //获取本地PC网卡IP信息
                if (CHCNetSDK.NET_DVR_GetLocalIP(strIP, ref dwValidNum, ref bEnableBind))
                {
                    if (dwValidNum > 0)
                    {
                        //取第一张网卡的IP地址为默认监听端口
                        textBoxListenIP.Text = System.Text.Encoding.UTF8.GetString(strIP, 0, 16);
                        CHCNetSDK.NET_DVR_SetValidIP(0,true); //绑定第一张网卡
                    }
                
                }

                //保存SDK日志 To save the SDK log
                CHCNetSDK.NET_DVR_SetLogToFile(3, "C:\\SdkLog\\", true);
                for (int i = 0; i < 200; i++)
                {
                    m_lAlarmHandle[i] = -1;
                }

                //设置报警回调函数
                if (m_falarmData_V31 == null)
                {
                    m_falarmData_V31 = new CHCNetSDK.MSGCallBack_V31(MsgCallback_V31);
                }
                CHCNetSDK.NET_DVR_SetDVRMessageCallBack_V31(m_falarmData_V31, IntPtr.Zero);
            }
        }

        public bool MsgCallback_V31(int lCommand, ref CHCNetSDK.NET_DVR_ALARMER pAlarmer, IntPtr pAlarmInfo, uint dwBufLen, IntPtr pUser)
        {
            //通过lCommand来判断接收到的报警信息类型，不同的lCommand对应不同的pAlarmInfo内容
            AlarmMessageHandle(lCommand, ref pAlarmer, pAlarmInfo, dwBufLen, pUser);
            
            return true; //回调函数需要有返回，表示正常接收到数据
        }

        public void MsgCallback(int lCommand, ref CHCNetSDK.NET_DVR_ALARMER pAlarmer, IntPtr pAlarmInfo, uint dwBufLen, IntPtr pUser)
        {
            //通过lCommand来判断接收到的报警信息类型，不同的lCommand对应不同的pAlarmInfo内容
            AlarmMessageHandle(lCommand, ref pAlarmer, pAlarmInfo, dwBufLen, pUser);
        }

        public void AlarmMessageHandle(int lCommand, ref CHCNetSDK.NET_DVR_ALARMER pAlarmer, IntPtr pAlarmInfo, uint dwBufLen, IntPtr pUser)
        {
            //通过lCommand来判断接收到的报警信息类型，不同的lCommand对应不同的pAlarmInfo内容
            switch (lCommand)
            {
                case CHCNetSDK.COMM_ALARM: //(DS-8000老设备)移动侦测、视频丢失、遮挡、IO信号量等报警信息
                    ProcessCommAlarm(ref pAlarmer, pAlarmInfo, dwBufLen, pUser);
                    break;
                case CHCNetSDK.COMM_ALARM_V30://移动侦测、视频丢失、遮挡、IO信号量等报警信息
                    ProcessCommAlarm_V30(ref pAlarmer, pAlarmInfo, dwBufLen, pUser);
                    break;
                case CHCNetSDK.COMM_ALARM_RULE://进出区域、入侵、徘徊、人员聚集等异常行为检测报警信息
                    ProcessCommAlarm_RULE(ref pAlarmer, pAlarmInfo, dwBufLen, pUser);
                    break;
                case CHCNetSDK.COMM_UPLOAD_PLATE_RESULT://交通抓拍结果上传(老报警信息类型)
                    ProcessCommAlarm_Plate(ref pAlarmer, pAlarmInfo, dwBufLen, pUser);
                    break;
                case CHCNetSDK.COMM_ITS_PLATE_RESULT://交通抓拍结果上传(新报警信息类型)
                    ProcessCommAlarm_ITSPlate(ref pAlarmer, pAlarmInfo, dwBufLen, pUser);
                    break;
                case CHCNetSDK.COMM_ALARM_PDC://客流量统计报警信息
                    ProcessCommAlarm_PDC(ref pAlarmer, pAlarmInfo, dwBufLen, pUser);
                    break;
                case CHCNetSDK.COMM_ITS_PARK_VEHICLE://客流量统计报警信息
                    ProcessCommAlarm_PARK(ref pAlarmer, pAlarmInfo, dwBufLen, pUser);
                    break;
                case CHCNetSDK.COMM_DIAGNOSIS_UPLOAD://VQD报警信息
                    ProcessCommAlarm_VQD(ref pAlarmer, pAlarmInfo, dwBufLen, pUser);
                    break;
                case CHCNetSDK.COMM_UPLOAD_FACESNAP_RESULT://人脸抓拍结果信息
                    ProcessCommAlarm_FaceSnap(ref pAlarmer, pAlarmInfo, dwBufLen, pUser);
                    break;
                case CHCNetSDK.COMM_SNAP_MATCH_ALARM://人脸比对结果信息
                    ProcessCommAlarm_FaceMatch(ref pAlarmer, pAlarmInfo, dwBufLen, pUser);
                    break;
                case CHCNetSDK.COMM_ALARM_FACE_DETECTION://人脸侦测报警信息
                    ProcessCommAlarm_FaceDetect(ref pAlarmer, pAlarmInfo, dwBufLen, pUser);
                    break;
                case CHCNetSDK.COMM_ALARMHOST_CID_ALARM://报警主机CID报警上传
                    ProcessCommAlarm_CIDAlarm(ref pAlarmer, pAlarmInfo, dwBufLen, pUser);
                    break;
                case CHCNetSDK.COMM_ALARM_ACS://门禁主机报警上传
                    ProcessCommAlarm_AcsAlarm(ref pAlarmer, pAlarmInfo, dwBufLen, pUser);
                    break;
                case CHCNetSDK.COMM_ID_INFO_ALARM://身份证刷卡信息上传
                    ProcessCommAlarm_IDInfoAlarm(ref pAlarmer, pAlarmInfo, dwBufLen, pUser);
                    break;
                default:
                    {
                        //报警设备IP地址
                        string strIP = pAlarmer.sDeviceIP;

                        //报警信息类型
                        string stringAlarm = "upload alarm，alarm message type：" + lCommand;

                        if (InvokeRequired)
                        {
                            object[] paras = new object[3];
                            paras[0] = DateTime.Now.ToString(); //当前PC系统时间
                            paras[1] = strIP;
                            paras[2] = stringAlarm;
                            listViewAlarmInfo.BeginInvoke(new UpdateListBoxCallback(UpdateClientList), paras);
                        }
                        else
                        {
                            //创建该控件的主线程直接更新信息列表 
                            UpdateClientList(DateTime.Now.ToString(), strIP, stringAlarm);
                        }
                    }
                    break;
            }
        }

        public void ProcessCommAlarm(ref CHCNetSDK.NET_DVR_ALARMER pAlarmer, IntPtr pAlarmInfo, uint dwBufLen, IntPtr pUser)
        {
            CHCNetSDK.NET_DVR_ALARMINFO struAlarmInfo = new CHCNetSDK.NET_DVR_ALARMINFO();

            struAlarmInfo = (CHCNetSDK.NET_DVR_ALARMINFO)Marshal.PtrToStructure(pAlarmInfo, typeof(CHCNetSDK.NET_DVR_ALARMINFO));

            string strIP = pAlarmer.sDeviceIP;
            string stringAlarm = "";
            int i = 0;

            switch (struAlarmInfo.dwAlarmType)
            {
                case 0:
                    stringAlarm = "IO alarm, alarm input number：" + struAlarmInfo.dwAlarmInputNumber + "，triggered recording channel：";
                    for (i = 0; i < CHCNetSDK.MAX_CHANNUM; i++)
                    {
                        if (struAlarmInfo.dwAlarmRelateChannel[i] == 1)
                        {
                            stringAlarm += (i + 1) + " \\ ";
                        }
                    }
                    break;
                case 1:
                    stringAlarm = "HDD full，alarm disk number：";
                    for (i = 0; i < CHCNetSDK.MAX_DISKNUM; i++)
                    {
                        if (struAlarmInfo.dwDiskNumber[i] == 1)
                        {
                            stringAlarm += (i + 1) + " \\ ";
                        }
                    }
                    break;
                case 2:
                    stringAlarm = "video loss，alarm channel number：";
                    for (i = 0; i < CHCNetSDK.MAX_CHANNUM; i++)
                    {
                        if (struAlarmInfo.dwChannel[i] == 1)
                        {
                            stringAlarm += (i + 1) + " \\ ";
                        }
                    }
                    break;
                case 3:
                    stringAlarm = "motion detection，alarm channel number：";
                    for (i = 0; i < CHCNetSDK.MAX_CHANNUM; i++)
                    {
                        if (struAlarmInfo.dwChannel[i] == 1)
                        {
                            stringAlarm += (i + 1) + " \\ ";
                        }
                    }
                    break;
                case 4:
                    stringAlarm = "HDD unformatted, alarm disk number：";
                    for (i = 0; i < CHCNetSDK.MAX_DISKNUM; i++)
                    {
                        if (struAlarmInfo.dwDiskNumber[i] == 1)
                        {
                            stringAlarm += (i + 1) + " \\ ";
                        }
                    }
                    break;
                case 5:
                    stringAlarm = "Read or Write HDD error，alarm disk number：";
                    for (i = 0; i < CHCNetSDK.MAX_DISKNUM; i++)
                    {
                        if (struAlarmInfo.dwDiskNumber[i] == 1)
                        {
                            stringAlarm += (i + 1) + " \\ ";
                        }
                    }
                    break;
                case 6:
                    stringAlarm = "Tampering alarm，alarm channel：";
                    for (i = 0; i < CHCNetSDK.MAX_CHANNUM; i++)
                    {
                        if (struAlarmInfo.dwChannel[i] == 1)
                        {
                            stringAlarm += (i + 1) + " \\ ";
                        }
                    }
                    break;
                case 7:
                    stringAlarm = "Input or Output video standard mismatch,alarm channel:";
                    for (i = 0; i < CHCNetSDK.MAX_CHANNUM; i++)
                    {
                        if (struAlarmInfo.dwChannel[i] == 1)
                        {
                            stringAlarm += (i + 1) + " \\ ";
                        }
                    }
                    break;
                case 8:
                    stringAlarm = "illegal access";
                    break;
                default:
                    stringAlarm = "other unknown alarm info";
                    break;
            }

            if (InvokeRequired)
            {
                object[] paras = new object[3];
                paras[0] = DateTime.Now.ToString();
                paras[1] = strIP;
                paras[2] = stringAlarm;                
                listViewAlarmInfo.BeginInvoke(new UpdateListBoxCallback(UpdateClientList), paras);
            }
            else
            {
                //创建该控件的主线程直接更新信息列表 
                UpdateClientList(DateTime.Now.ToString(),strIP, stringAlarm);
            }
        }

        private void ProcessCommAlarm_V30(ref CHCNetSDK.NET_DVR_ALARMER pAlarmer, IntPtr pAlarmInfo, uint dwBufLen, IntPtr pUser)
        {

            CHCNetSDK.NET_DVR_ALARMINFO_V30 struAlarmInfoV30 = new CHCNetSDK.NET_DVR_ALARMINFO_V30();

            struAlarmInfoV30 = (CHCNetSDK.NET_DVR_ALARMINFO_V30)Marshal.PtrToStructure(pAlarmInfo, typeof(CHCNetSDK.NET_DVR_ALARMINFO_V30));

            string strIP = pAlarmer.sDeviceIP;
            string stringAlarm = "";
            int i;

            switch (struAlarmInfoV30.dwAlarmType)
            {
                case 0:
                    stringAlarm = "IO alarm，alarm input number：" + struAlarmInfoV30.dwAlarmInputNumber + "，triggered recording channel：";
                    for (i = 0; i < CHCNetSDK.MAX_CHANNUM_V30; i++)
                    {
                        if (struAlarmInfoV30.byAlarmRelateChannel[i] == 1)
                        {
                            stringAlarm += (i + 1) + "\\";
                        }
                    }
                    break;
                case 1:
                    stringAlarm = "HDD full，alarm disk number：";
                    for (i = 0; i < CHCNetSDK.MAX_DISKNUM_V30; i++)
                    {
                        if (struAlarmInfoV30.byDiskNumber[i] == 1)
                        {
                            stringAlarm += (i + 1) + " ";
                        }
                    }
                    break;
                case 2:
                    stringAlarm = "Video loss，alarm channel：";
                    for (i = 0; i < CHCNetSDK.MAX_CHANNUM_V30; i++)
                    {
                        if (struAlarmInfoV30.byChannel[i] == 1)
                        {
                            stringAlarm += (i + 1) + " \\ ";
                        }
                    }
                    break;
                case 3:
                    stringAlarm = "Motion detection，alarm channel：";
                    for (i = 0; i < CHCNetSDK.MAX_CHANNUM_V30; i++)
                    {
                        if (struAlarmInfoV30.byChannel[i] == 1)
                        {
                            stringAlarm += (i + 1) + " \\ ";
                        }
                    }
                    break;
                case 4:
                    stringAlarm = "HDD unformatted, alarm disk number：";
                    for (i = 0; i < CHCNetSDK.MAX_DISKNUM_V30; i++)
                    {
                        if (struAlarmInfoV30.byDiskNumber[i] == 1)
                        {
                            stringAlarm += (i + 1) + " \\ ";
                        }
                    }
                    break;
                case 5:
                    stringAlarm = "Read or Write HDD error，alarm disk number：";
                    for (i = 0; i < CHCNetSDK.MAX_DISKNUM_V30; i++)
                    {
                        if (struAlarmInfoV30.byDiskNumber[i] == 1)
                        {
                            stringAlarm += (i + 1) + " \\ ";
                        }
                    }
                    break;
                case 6:
                    stringAlarm = "Tampering alarm，alarm channel：";
                    for (i = 0; i < CHCNetSDK.MAX_CHANNUM_V30; i++)
                    {
                        if (struAlarmInfoV30.byChannel[i] == 1)
                        {
                            stringAlarm += (i + 1) + " \\ ";
                        }
                    }
                    break;
                case 7:
                    stringAlarm = "Input or Output video standard mismatch,alarm channel:";
                    for (i = 0; i < CHCNetSDK.MAX_CHANNUM_V30; i++)
                    {
                        if (struAlarmInfoV30.byChannel[i] == 1)
                        {
                            stringAlarm += (i + 1) + " \\ ";
                        }
                    }
                    break;
                case 8:
                    stringAlarm = "illegal access";
                    break;
                case 9:
                    stringAlarm = "Video siganl exception, alarm channel:";
                    for (i = 0; i < CHCNetSDK.MAX_CHANNUM_V30; i++)
                    {
                        if (struAlarmInfoV30.byChannel[i] == 1)
                        {
                            stringAlarm += (i + 1) + " \\ ";
                        }
                    }
                    break;
                case 10:
                    stringAlarm = "recording/capture is abnormal，alarm channel:";
                    for (i = 0; i < CHCNetSDK.MAX_CHANNUM_V30; i++)
                    {
                        if (struAlarmInfoV30.byChannel[i] == 1)
                        {
                            stringAlarm += (i + 1) + " \\ ";
                        }
                    }
                    break;
                case 11:
                    stringAlarm = "Intelligent scene changed，alarm channel:";
                    for (i = 0; i < CHCNetSDK.MAX_CHANNUM_V30; i++)
                    {
                        if (struAlarmInfoV30.byChannel[i] == 1)
                        {
                            stringAlarm += (i + 1) + " \\ ";
                        }
                    }
                    break;
                case 12:
                    stringAlarm = "RAID is abnormal";
                    break;
                case 13:
                    stringAlarm = "recording resolution does not match with which of front-end camera，alarm channel:";
                    for (i = 0; i < CHCNetSDK.MAX_CHANNUM_V30; i++)
                    {
                        if (struAlarmInfoV30.byChannel[i] == 1)
                        {
                            stringAlarm += (i + 1) + " \\ ";
                        }
                    }
                    break;
                case 15:
                    stringAlarm = "VCA，alarm channel:";
                    for (i = 0; i < CHCNetSDK.MAX_CHANNUM_V30; i++)
                    {
                        if (struAlarmInfoV30.byChannel[i] == 1)
                        {
                            stringAlarm += (i + 1) + " \\ ";
                        }
                    }
                    break;
                default:
                    stringAlarm = "other unknown alarm info";
                    break;
            }

            if (InvokeRequired)
            {
                object[] paras = new object[3];
                paras[0] = DateTime.Now.ToString();
                paras[1] = strIP;
                paras[2] = stringAlarm;
                listViewAlarmInfo.BeginInvoke(new UpdateListBoxCallback(UpdateClientList), paras);
            }
            else
            {
                //创建该控件的主线程直接更新信息列表 
                UpdateClientList(DateTime.Now.ToString(), strIP, stringAlarm);
            }

        }

        private void ProcessCommAlarm_RULE(ref CHCNetSDK.NET_DVR_ALARMER pAlarmer, IntPtr pAlarmInfo, uint dwBufLen, IntPtr pUser)
        {
            CHCNetSDK.NET_VCA_RULE_ALARM struRuleAlarmInfo = new CHCNetSDK.NET_VCA_RULE_ALARM();
            struRuleAlarmInfo = (CHCNetSDK.NET_VCA_RULE_ALARM)Marshal.PtrToStructure(pAlarmInfo, typeof(CHCNetSDK.NET_VCA_RULE_ALARM));

            //报警信息
            string stringAlarm = "";
            uint dwSize = (uint)Marshal.SizeOf(struRuleAlarmInfo.struRuleInfo.uEventParam);

            switch (struRuleAlarmInfo.struRuleInfo.wEventTypeEx)
            {
                case (ushort)CHCNetSDK.VCA_RULE_EVENT_TYPE_EX.ENUM_VCA_EVENT_TRAVERSE_PLANE:
                    IntPtr ptrTraverseInfo = Marshal.AllocHGlobal((Int32)dwSize);
                    Marshal.StructureToPtr(struRuleAlarmInfo.struRuleInfo.uEventParam, ptrTraverseInfo, false);
                    m_struTraversePlane = (CHCNetSDK.NET_VCA_TRAVERSE_PLANE)Marshal.PtrToStructure(ptrTraverseInfo, typeof(CHCNetSDK.NET_VCA_TRAVERSE_PLANE));
                    stringAlarm = "Line crossing，Object ID：" + struRuleAlarmInfo.struTargetInfo.dwID;
                    //警戒面边线起点坐标: (m_struTraversePlane.struPlaneBottom.struStart.fX, m_struTraversePlane.struPlaneBottom.struStart.fY)
                    //警戒面边线终点坐标: (m_struTraversePlane.struPlaneBottom.struEnd.fX, m_struTraversePlane.struPlaneBottom.struEnd.fY)
                    break;
                case (ushort)CHCNetSDK.VCA_RULE_EVENT_TYPE_EX.ENUM_VCA_EVENT_ENTER_AREA:
                    IntPtr ptrEnterInfo = Marshal.AllocHGlobal((Int32)dwSize);
                    Marshal.StructureToPtr(struRuleAlarmInfo.struRuleInfo.uEventParam, ptrEnterInfo, false);
                    m_struVcaArea = (CHCNetSDK.NET_VCA_AREA)Marshal.PtrToStructure(ptrEnterInfo, typeof(CHCNetSDK.NET_VCA_AREA));
                    stringAlarm = "Target entering area，Object ID：" + struRuleAlarmInfo.struTargetInfo.dwID;
                    //m_struVcaArea.struRegion 多边形区域坐标
                    break;
                case (ushort)CHCNetSDK.VCA_RULE_EVENT_TYPE_EX.ENUM_VCA_EVENT_EXIT_AREA:               
                    IntPtr ptrExitInfo = Marshal.AllocHGlobal((Int32)dwSize);
                    Marshal.StructureToPtr(struRuleAlarmInfo.struRuleInfo.uEventParam, ptrExitInfo, false);
                    m_struVcaArea = (CHCNetSDK.NET_VCA_AREA)Marshal.PtrToStructure(ptrExitInfo, typeof(CHCNetSDK.NET_VCA_AREA));
                    stringAlarm = "Target leaving area，Object ID：" + struRuleAlarmInfo.struTargetInfo.dwID;
                    //m_struVcaArea.struRegion 多边形区域坐标
                    break;
                case (ushort)CHCNetSDK.VCA_RULE_EVENT_TYPE_EX.ENUM_VCA_EVENT_INTRUSION:
                    IntPtr ptrIntrusionInfo = Marshal.AllocHGlobal((Int32)dwSize);
                    Marshal.StructureToPtr(struRuleAlarmInfo.struRuleInfo.uEventParam, ptrIntrusionInfo, false);
                    m_struIntrusion = (CHCNetSDK.NET_VCA_INTRUSION)Marshal.PtrToStructure(ptrIntrusionInfo, typeof(CHCNetSDK.NET_VCA_INTRUSION));

                    int i = 0;
                    string strRegion = "";
                    for (i = 0; i < m_struIntrusion.struRegion.dwPointNum; i++)
                    {
                        strRegion = strRegion + "(" + m_struIntrusion.struRegion.struPos[i].fX + "," + m_struIntrusion.struRegion.struPos[i].fY + ")";
                    }
                    stringAlarm = "Intrusion detection，Object ID：" + struRuleAlarmInfo.struTargetInfo.dwID + "，DetectionTarget: " + m_struIntrusion.byDetectionTarget + "，Region range：" + strRegion;
                    //m_struIntrusion.struRegion 多边形区域坐标
                    break;
                default:
                    stringAlarm = "other behaviour analysis alarm，Object ID：" + struRuleAlarmInfo.struTargetInfo.dwID;
                    break;            
            }


            //报警图片保存
            if (struRuleAlarmInfo.dwPicDataLen > 0)
            {
                FileStream fs = new FileStream("Behaviour analysis alarm capture.jpg", FileMode.Create);
                int iLen = (int)struRuleAlarmInfo.dwPicDataLen;
                byte[] by = new byte[iLen];
                Marshal.Copy(struRuleAlarmInfo.pImage, by, 0, iLen);
                fs.Write(by, 0, iLen);
                fs.Close();
            }

            //报警时间：年月日时分秒
            string strTimeYear = ((struRuleAlarmInfo.dwAbsTime >> 26) + 2000).ToString();
            string strTimeMonth = ((struRuleAlarmInfo.dwAbsTime >> 22) & 15).ToString("d2");
            string strTimeDay = ((struRuleAlarmInfo.dwAbsTime >> 17) & 31).ToString("d2");
            string strTimeHour = ((struRuleAlarmInfo.dwAbsTime >> 12) & 31).ToString("d2");
            string strTimeMinute = ((struRuleAlarmInfo.dwAbsTime >> 6) & 63).ToString("d2");
            string strTimeSecond = ((struRuleAlarmInfo.dwAbsTime >> 0) & 63).ToString("d2");
            string strTime = strTimeYear + "-" + strTimeMonth + "-" + strTimeDay + " " + strTimeHour + ":" + strTimeMinute + ":" + strTimeSecond;
           
            //报警设备IP地址
            string strIP = struRuleAlarmInfo.struDevInfo.struDevIP.sIpV4;

            //将报警信息添加进列表
            if (InvokeRequired)
            {
                object[] paras = new object[3];
                paras[0] = strTime;
                paras[1] = strIP;
                paras[2] = stringAlarm;
                listViewAlarmInfo.BeginInvoke(new UpdateListBoxCallback(UpdateClientList), paras);
            }
            else
            {
                //创建该控件的主线程直接更新信息列表 
                UpdateClientList(strTime, strIP, stringAlarm);
            }
        }

        private void ProcessCommAlarm_Plate(ref CHCNetSDK.NET_DVR_ALARMER pAlarmer, IntPtr pAlarmInfo, uint dwBufLen, IntPtr pUser)
        {
            CHCNetSDK.NET_DVR_PLATE_RESULT struPlateResultInfo = new CHCNetSDK.NET_DVR_PLATE_RESULT();
            uint dwSize = (uint)Marshal.SizeOf(struPlateResultInfo);

            struPlateResultInfo = (CHCNetSDK.NET_DVR_PLATE_RESULT)Marshal.PtrToStructure(pAlarmInfo, typeof(CHCNetSDK.NET_DVR_PLATE_RESULT));

            //保存抓拍图片
            string str = "";
            if (struPlateResultInfo.byResultType == 1 && struPlateResultInfo.dwPicLen != 0)
            {
                str = "Plate_UserID_" + pAlarmer.lUserID + "_close-range picture.jpg";
                FileStream fs = new FileStream(str, FileMode.Create);
                int iLen = (int)struPlateResultInfo.dwPicLen;
                byte[] by = new byte[iLen];
                Marshal.Copy(struPlateResultInfo.pBuffer1, by, 0, iLen);
                fs.Write(by, 0, iLen);
                fs.Close();
            }
            if (struPlateResultInfo.dwPicPlateLen != 0)
            {
                str = "Plate_UserID_" + pAlarmer.lUserID + "_license plate picture.jpg";
                FileStream fs = new FileStream(str, FileMode.Create);
                int iLen = (int)struPlateResultInfo.dwPicPlateLen;
                byte[] by = new byte[iLen];
                Marshal.Copy(struPlateResultInfo.pBuffer2, by, 0, iLen);
                fs.Write(by, 0, iLen);
                fs.Close();
            }
            if (struPlateResultInfo.dwFarCarPicLen != 0)
            {
                str = "Plate_UserID_" + pAlarmer.lUserID + "_far-range picture.jpg";
                FileStream fs = new FileStream(str, FileMode.Create);
                int iLen = (int)struPlateResultInfo.dwFarCarPicLen;
                byte[] by = new byte[iLen];
                Marshal.Copy(struPlateResultInfo.pBuffer5, by, 0, iLen);
                fs.Write(by, 0, iLen);
                fs.Close();
            }

            //报警设备IP地址
            string strIP = pAlarmer.sDeviceIP;

            //抓拍时间：年月日时分秒
            string strTimeYear = System.Text.Encoding.UTF8.GetString(struPlateResultInfo.byAbsTime);

            //上传结果
            string stringPlateLicense = System.Text.Encoding.GetEncoding("GBK").GetString(struPlateResultInfo.struPlateInfo.sLicense).TrimEnd('\0');
            string stringAlarm = "capture upload，" + "license plate：" + stringPlateLicense + "，Serial number of vehicle：" + struPlateResultInfo.struVehicleInfo.dwIndex; 

            if (InvokeRequired)
            {
                object[] paras = new object[3];
                paras[0] = strTimeYear; //当前PC系统时间为DateTime.Now.ToString();
                paras[1] = strIP;
                paras[2] = stringAlarm;
                listViewAlarmInfo.BeginInvoke(new UpdateListBoxCallback(UpdateClientList), paras);
            }
            else
            {
                //创建该控件的主线程直接更新信息列表 
                UpdateClientList(DateTime.Now.ToString(), strIP, stringAlarm);
            }
        }

        private void ProcessCommAlarm_ITSPlate(ref CHCNetSDK.NET_DVR_ALARMER pAlarmer, IntPtr pAlarmInfo, uint dwBufLen, IntPtr pUser)
        {
            CHCNetSDK.NET_ITS_PLATE_RESULT struITSPlateResult = new CHCNetSDK.NET_ITS_PLATE_RESULT();
            uint dwSize = (uint)Marshal.SizeOf(struITSPlateResult);

            struITSPlateResult = (CHCNetSDK.NET_ITS_PLATE_RESULT)Marshal.PtrToStructure(pAlarmInfo, typeof(CHCNetSDK.NET_ITS_PLATE_RESULT));

            //保存抓拍图片
            for (int i = 0; i < struITSPlateResult.dwPicNum; i++)
            {
                if (struITSPlateResult.struPicInfo[i].dwDataLen != 0)
                {
                    string str = "ITS_UserID_[" + pAlarmer.lUserID + "]_Pictype_" + struITSPlateResult.struPicInfo[i].byType + "_Num" + (i + 1) + ".jpg";
                    FileStream fs = new FileStream(str, FileMode.Create);
                    int iLen = (int)struITSPlateResult.struPicInfo[i].dwDataLen;
                    byte[] by = new byte[iLen];
                    Marshal.Copy(struITSPlateResult.struPicInfo[i].pBuffer, by, 0, iLen);
                    fs.Write(by, 0, iLen);
                    fs.Close();
                }
            }
            //报警设备IP地址
            string strIP = pAlarmer.sDeviceIP;

            //抓拍时间：年月日时分秒
            string strTimeYear = string.Format("{0:D4}", struITSPlateResult.struSnapFirstPicTime.wYear) + 
                string.Format("{0:D2}", struITSPlateResult.struSnapFirstPicTime.byMonth) + 
                string.Format("{0:D2}", struITSPlateResult.struSnapFirstPicTime.byDay) + " " 
                + string.Format("{0:D2}", struITSPlateResult.struSnapFirstPicTime.byHour) + ":" 
                + string.Format("{0:D2}", struITSPlateResult.struSnapFirstPicTime.byMinute) + ":" 
                + string.Format("{0:D2}", struITSPlateResult.struSnapFirstPicTime.bySecond) + ":" 
                + string.Format("{0:D3}", struITSPlateResult.struSnapFirstPicTime.wMilliSec);

            //上传结果
            string stringPlateLicense = System.Text.Encoding.GetEncoding("GBK").GetString(struITSPlateResult.struPlateInfo.sLicense).TrimEnd('\0');
            string stringAlarm = "capture upload，" + "license plate：" + stringPlateLicense + "，Serial number of vehicle：" + struITSPlateResult.struVehicleInfo.dwIndex;

            if (InvokeRequired)
            {
                object[] paras = new object[3];
                paras[0] = strTimeYear;//当前系统时间为：DateTime.Now.ToString();
                paras[1] = strIP;
                paras[2] = stringAlarm;
                listViewAlarmInfo.BeginInvoke(new UpdateListBoxCallback(UpdateClientList), paras);
            }
            else
            {
                //创建该控件的主线程直接更新信息列表 
                UpdateClientList(DateTime.Now.ToString(), strIP, stringAlarm);
            }
        }

        private void ProcessCommAlarm_PDC(ref CHCNetSDK.NET_DVR_ALARMER pAlarmer, IntPtr pAlarmInfo, uint dwBufLen, IntPtr pUser)
        {
            CHCNetSDK.NET_DVR_PDC_ALRAM_INFO struPDCInfo = new CHCNetSDK.NET_DVR_PDC_ALRAM_INFO();
            uint dwSize = (uint)Marshal.SizeOf(struPDCInfo);
            struPDCInfo = (CHCNetSDK.NET_DVR_PDC_ALRAM_INFO)Marshal.PtrToStructure(pAlarmInfo, typeof(CHCNetSDK.NET_DVR_PDC_ALRAM_INFO));

            string stringAlarm = "People counting statistics，Enter number：" + struPDCInfo.dwEnterNum + "，Exit number：" + struPDCInfo.dwLeaveNum;

            uint dwUnionSize = (uint)Marshal.SizeOf(struPDCInfo.uStatModeParam);
            IntPtr ptrPDCUnion = Marshal.AllocHGlobal((Int32)dwUnionSize);
            Marshal.StructureToPtr(struPDCInfo.uStatModeParam, ptrPDCUnion, false);

            if (struPDCInfo.byMode == 0) //单帧统计结果，此处为UTC时间
            {              
                m_struStatFrame = (CHCNetSDK.UNION_STATFRAME)Marshal.PtrToStructure(ptrPDCUnion, typeof(CHCNetSDK.UNION_STATFRAME));
                stringAlarm = stringAlarm + "，single frame statistics，Counting relative time mark：" + m_struStatFrame.dwRelativeTime + "，Counting absolute time mark：" + m_struStatFrame.dwAbsTime;
            }
            if (struPDCInfo.byMode == 1) //最小时间段统计结果
            {
                m_struStatTime = (CHCNetSDK.UNION_STATTIME)Marshal.PtrToStructure(ptrPDCUnion, typeof(CHCNetSDK.UNION_STATTIME));

                //开始时间
                string strStartTime = string.Format("{0:D4}", m_struStatTime.tmStart.dwYear) +
                string.Format("{0:D2}", m_struStatTime.tmStart.dwMonth) +
                string.Format("{0:D2}", m_struStatTime.tmStart.dwDay) + " "
                + string.Format("{0:D2}", m_struStatTime.tmStart.dwHour) + ":"
                + string.Format("{0:D2}", m_struStatTime.tmStart.dwMinute) + ":"
                + string.Format("{0:D2}", m_struStatTime.tmStart.dwSecond);

                //结束时间
                string strEndTime = string.Format("{0:D4}", m_struStatTime.tmEnd.dwYear) +
                string.Format("{0:D2}", m_struStatTime.tmEnd.dwMonth) +
                string.Format("{0:D2}", m_struStatTime.tmEnd.dwDay) + " "
                + string.Format("{0:D2}", m_struStatTime.tmEnd.dwHour) + ":"
                + string.Format("{0:D2}", m_struStatTime.tmEnd.dwMinute) + ":"
                + string.Format("{0:D2}", m_struStatTime.tmEnd.dwSecond);

                stringAlarm = stringAlarm + "，minimum period statistics，start time：" + strStartTime + "，end time：" + strEndTime;
            }
            Marshal.FreeHGlobal(ptrPDCUnion);

            //报警设备IP地址
            string strIP = pAlarmer.sDeviceIP;


            if (InvokeRequired)
            {
                object[] paras = new object[3];
                paras[0] = DateTime.Now.ToString(); //当前PC系统时间
                paras[1] = strIP;
                paras[2] = stringAlarm;
                listViewAlarmInfo.BeginInvoke(new UpdateListBoxCallback(UpdateClientList), paras);
            }
            else
            {
                //创建该控件的主线程直接更新信息列表 
                UpdateClientList(DateTime.Now.ToString(), strIP, stringAlarm);
            }
        }


        private void ProcessCommAlarm_PARK(ref CHCNetSDK.NET_DVR_ALARMER pAlarmer, IntPtr pAlarmInfo, uint dwBufLen, IntPtr pUser)
        {
            CHCNetSDK.NET_ITS_PARK_VEHICLE struParkInfo = new CHCNetSDK.NET_ITS_PARK_VEHICLE();
            uint dwSize = (uint)Marshal.SizeOf(struParkInfo);
            struParkInfo = (CHCNetSDK.NET_ITS_PARK_VEHICLE)Marshal.PtrToStructure(pAlarmInfo, typeof(CHCNetSDK.NET_ITS_PARK_VEHICLE));

            //报警设备IP地址
            string strIP = pAlarmer.sDeviceIP;

            //保存抓拍图片
            for (int i = 0; i < struParkInfo.dwPicNum; i++)
            {
                if ((struParkInfo.struPicInfo[i].dwDataLen != 0) && (struParkInfo.struPicInfo[i].pBuffer != IntPtr.Zero))
                {
                    string str = "Device_Park_[" + strIP + "]_lUerID_[" + pAlarmer.lUserID + "]_Pictype_" + struParkInfo.struPicInfo[i].byType + "_Num" + (i + 1) + ".jpg";
                    FileStream fs = new FileStream(str, FileMode.Create);
                    int iLen = (int)struParkInfo.struPicInfo[i].dwDataLen;
                    byte[] by = new byte[iLen];
                    Marshal.Copy(struParkInfo.struPicInfo[i].pBuffer, by, 0, iLen);
                    fs.Write(by, 0, iLen);
                    fs.Close();
                }
            }

            string stringAlarm = "Parking data upload，Parking exception：" + struParkInfo.byParkError + "，Parking space No.：" + struParkInfo.byParkingNo +
                ", Parking space status：" + struParkInfo.byLocationStatus + "，License plate number：" +
                System.Text.Encoding.GetEncoding("GBK").GetString(struParkInfo.struPlateInfo.sLicense).TrimEnd('\0');         

            if (InvokeRequired)
            {
                object[] paras = new object[3];
                paras[0] = DateTime.Now.ToString(); //当前PC系统时间
                paras[1] = strIP;
                paras[2] = stringAlarm;
                listViewAlarmInfo.BeginInvoke(new UpdateListBoxCallback(UpdateClientList), paras);
            }
            else
            {
                //创建该控件的主线程直接更新信息列表 
                UpdateClientList(DateTime.Now.ToString(), strIP, stringAlarm);
            }
        }

        private void ProcessCommAlarm_VQD(ref CHCNetSDK.NET_DVR_ALARMER pAlarmer, IntPtr pAlarmInfo, uint dwBufLen, IntPtr pUser)
        {
            CHCNetSDK.NET_DVR_DIAGNOSIS_UPLOAD struVQDInfo = new CHCNetSDK.NET_DVR_DIAGNOSIS_UPLOAD();
            uint dwSize = (uint)Marshal.SizeOf(struVQDInfo);
            struVQDInfo = (CHCNetSDK.NET_DVR_DIAGNOSIS_UPLOAD)Marshal.PtrToStructure(pAlarmInfo, typeof(CHCNetSDK.NET_DVR_DIAGNOSIS_UPLOAD));

            //报警设备IP地址
            string strIP = pAlarmer.sDeviceIP;

            //开始时间
            string strCheckTime = string.Format("{0:D4}", struVQDInfo.struCheckTime.dwYear) +
            string.Format("{0:D2}", struVQDInfo.struCheckTime.dwMonth) +
            string.Format("{0:D2}", struVQDInfo.struCheckTime.dwDay) + " "
            + string.Format("{0:D2}", struVQDInfo.struCheckTime.dwHour) + ":"
            + string.Format("{0:D2}", struVQDInfo.struCheckTime.dwMinute) + ":"
            + string.Format("{0:D2}", struVQDInfo.struCheckTime.dwSecond);

            string stringAlarm = "VQD result，stream ID：" + struVQDInfo.sStreamID + "，Monitor IP：" + struVQDInfo.sMonitorIP + "，Monitor channel number：" + struVQDInfo.dwChanIndex +
                "，checking time：" + strCheckTime + "，byResult：" + struVQDInfo.byResult + "，bySignalResult：" + struVQDInfo.bySignalResult + "，byBlurResult：" + struVQDInfo.byBlurResult;

            if (InvokeRequired)
            {
                object[] paras = new object[3];
                paras[0] = DateTime.Now.ToString(); //当前PC系统时间
                paras[1] = strIP;
                paras[2] = stringAlarm;
                listViewAlarmInfo.BeginInvoke(new UpdateListBoxCallback(UpdateClientList), paras);
            }
            else
            {
                //创建该控件的主线程直接更新信息列表 
                UpdateClientList(DateTime.Now.ToString(), strIP, stringAlarm);
            }
        }

        private void ProcessCommAlarm_FaceSnap(ref CHCNetSDK.NET_DVR_ALARMER pAlarmer, IntPtr pAlarmInfo, uint dwBufLen, IntPtr pUser)
        {
            CHCNetSDK.NET_VCA_FACESNAP_RESULT struFaceSnapInfo = new CHCNetSDK.NET_VCA_FACESNAP_RESULT();
            uint dwSize = (uint)Marshal.SizeOf(struFaceSnapInfo);
            struFaceSnapInfo = (CHCNetSDK.NET_VCA_FACESNAP_RESULT)Marshal.PtrToStructure(pAlarmInfo, typeof(CHCNetSDK.NET_VCA_FACESNAP_RESULT));

            //报警设备IP地址
            string strIP = pAlarmer.sDeviceIP;

            //保存抓拍图片数据
            if ((struFaceSnapInfo.dwBackgroundPicLen != 0) && (struFaceSnapInfo.pBuffer2 != IntPtr.Zero))
            {
                string str = "Device_FaceSnap_CapPic_[" + strIP + "]_lUerID_[" + pAlarmer.lUserID + "]" + ".jpg";
                FileStream fs = new FileStream(str, FileMode.Create);
                int iLen = (int)struFaceSnapInfo.dwBackgroundPicLen;
                byte[] by = new byte[iLen];
                Marshal.Copy(struFaceSnapInfo.pBuffer2, by, 0, iLen);
                fs.Write(by, 0, iLen);
                fs.Close();
            }

            //报警时间：年月日时分秒
            string strTimeYear = ((struFaceSnapInfo.dwAbsTime >> 26) + 2000).ToString();
            string strTimeMonth = ((struFaceSnapInfo.dwAbsTime >> 22) & 15).ToString("d2");
            string strTimeDay = ((struFaceSnapInfo.dwAbsTime >> 17) & 31).ToString("d2");
            string strTimeHour = ((struFaceSnapInfo.dwAbsTime >> 12) & 31).ToString("d2");
            string strTimeMinute = ((struFaceSnapInfo.dwAbsTime >> 6) & 63).ToString("d2");
            string strTimeSecond = ((struFaceSnapInfo.dwAbsTime >> 0) & 63).ToString("d2");
            string strTime = strTimeYear + "-" + strTimeMonth + "-" + strTimeDay + " " + strTimeHour + ":" + strTimeMinute + ":" + strTimeSecond;

            string stringAlarm = "Face capture result，Front-end Device：" + struFaceSnapInfo.struDevInfo.struDevIP.sIpV4 + "，Alarm time：" + strTime;

            if (InvokeRequired)
            {
                object[] paras = new object[3];
                paras[0] = DateTime.Now.ToString(); //当前PC系统时间
                paras[1] = strIP;
                paras[2] = stringAlarm;
                listViewAlarmInfo.BeginInvoke(new UpdateListBoxCallback(UpdateClientList), paras);
            }
            else
            {
                //创建该控件的主线程直接更新信息列表 
                UpdateClientList(DateTime.Now.ToString(), strIP, stringAlarm);
            }
        }

        private void ProcessCommAlarm_FaceMatch(ref CHCNetSDK.NET_DVR_ALARMER pAlarmer, IntPtr pAlarmInfo, uint dwBufLen, IntPtr pUser)
        {
            CHCNetSDK.NET_VCA_FACESNAP_MATCH_ALARM struFaceMatchAlarm = new CHCNetSDK.NET_VCA_FACESNAP_MATCH_ALARM();
            uint dwSize = (uint)Marshal.SizeOf(struFaceMatchAlarm);
            struFaceMatchAlarm = (CHCNetSDK.NET_VCA_FACESNAP_MATCH_ALARM)Marshal.PtrToStructure(pAlarmInfo, typeof(CHCNetSDK.NET_VCA_FACESNAP_MATCH_ALARM));

            //报警设备IP地址
            string strIP = pAlarmer.sDeviceIP;

            //保存抓拍人脸子图图片数据
            if ((struFaceMatchAlarm.struSnapInfo.dwSnapFacePicLen != 0) && (struFaceMatchAlarm.struSnapInfo.pBuffer1 != IntPtr.Zero))
            {
                string str = "Device_FaceMatch_FacePic_[" + strIP + "]_lUerID_[" + pAlarmer.lUserID + "]" + ".jpg";
                FileStream fs = new FileStream(str, FileMode.Create);
                int iLen = (int)struFaceMatchAlarm.struSnapInfo.dwSnapFacePicLen;
                byte[] by = new byte[iLen];
                Marshal.Copy(struFaceMatchAlarm.struSnapInfo.pBuffer1, by, 0, iLen);
                fs.Write(by, 0, iLen);
                fs.Close();
            }

            //保存比对结果人脸库人脸图片数据
            if ((struFaceMatchAlarm.struBlockListInfo.dwBlockListPicLen != 0) && (struFaceMatchAlarm.struBlockListInfo.pBuffer1 != IntPtr.Zero))
            {
                string str = "Device_FaceMatch_BlockListPic_[" + strIP + "]_lUerID_[" + pAlarmer.lUserID + "]" + "_fSimilarity[" + struFaceMatchAlarm.fSimilarity + "].jpg";
                FileStream fs = new FileStream(str, FileMode.Create);
                int iLen = (int)struFaceMatchAlarm.struBlockListInfo.dwBlockListPicLen;
                byte[] by = new byte[iLen];
                Marshal.Copy(struFaceMatchAlarm.struBlockListInfo.pBuffer1, by, 0, iLen);
                fs.Write(by, 0, iLen);
                fs.Close();
            }

            //抓拍时间：年月日时分秒
            string strTimeYear = ((struFaceMatchAlarm.struSnapInfo.dwAbsTime >> 26) + 2000).ToString();
            string strTimeMonth = ((struFaceMatchAlarm.struSnapInfo.dwAbsTime >> 22) & 15).ToString("d2");
            string strTimeDay = ((struFaceMatchAlarm.struSnapInfo.dwAbsTime >> 17) & 31).ToString("d2");
            string strTimeHour = ((struFaceMatchAlarm.struSnapInfo.dwAbsTime >> 12) & 31).ToString("d2");
            string strTimeMinute = ((struFaceMatchAlarm.struSnapInfo.dwAbsTime >> 6) & 63).ToString("d2");
            string strTimeSecond = ((struFaceMatchAlarm.struSnapInfo.dwAbsTime >> 0) & 63).ToString("d2");
            string strTime = strTimeYear + "-" + strTimeMonth + "-" + strTimeDay + " " + strTimeHour + ":" + strTimeMinute + ":" + strTimeSecond;

            string stringAlarm = "Face comparison alarm, capture deivces：" + struFaceMatchAlarm.struSnapInfo.struDevInfo.struDevIP.sIpV4 + "，Capture time：" 
                + strTime + "，similarity：" + struFaceMatchAlarm.fSimilarity;

            if (InvokeRequired)
            {
                object[] paras = new object[3];
                paras[0] = DateTime.Now.ToString(); //当前PC系统时间
                paras[1] = strIP;
                paras[2] = stringAlarm;
                listViewAlarmInfo.BeginInvoke(new UpdateListBoxCallback(UpdateClientList), paras);
            }
            else
            {
                //创建该控件的主线程直接更新信息列表 
                UpdateClientList(DateTime.Now.ToString(), strIP, stringAlarm);
            }
        }

        private void ProcessCommAlarm_FaceDetect(ref CHCNetSDK.NET_DVR_ALARMER pAlarmer, IntPtr pAlarmInfo, uint dwBufLen, IntPtr pUser)
        {
            CHCNetSDK.NET_DVR_FACE_DETECTION struFaceDetectInfo = new CHCNetSDK.NET_DVR_FACE_DETECTION();
            uint dwSize = (uint)Marshal.SizeOf(struFaceDetectInfo);
            struFaceDetectInfo = (CHCNetSDK.NET_DVR_FACE_DETECTION)Marshal.PtrToStructure(pAlarmInfo, typeof(CHCNetSDK.NET_DVR_FACE_DETECTION));

            //报警设备IP地址
            string strIP = pAlarmer.sDeviceIP;

            //报警时间：年月日时分秒
            string strTimeYear = ((struFaceDetectInfo.dwAbsTime >> 26) + 2000).ToString();
            string strTimeMonth = ((struFaceDetectInfo.dwAbsTime >> 22) & 15).ToString("d2");
            string strTimeDay = ((struFaceDetectInfo.dwAbsTime >> 17) & 31).ToString("d2");
            string strTimeHour = ((struFaceDetectInfo.dwAbsTime >> 12) & 31).ToString("d2");
            string strTimeMinute = ((struFaceDetectInfo.dwAbsTime >> 6) & 63).ToString("d2");
            string strTimeSecond = ((struFaceDetectInfo.dwAbsTime >> 0) & 63).ToString("d2");
            string strTime = strTimeYear + "-" + strTimeMonth + "-" + strTimeDay + " " + strTimeHour + ":" + strTimeMinute + ":" + strTimeSecond;

            string stringAlarm = "Face detection result, Front-end devices：" + struFaceDetectInfo.struDevInfo.struDevIP + "，Alarm time：" + strTime;

            if (InvokeRequired)
            {
                object[] paras = new object[3];
                paras[0] = DateTime.Now.ToString(); //当前PC系统时间
                paras[1] = strIP;
                paras[2] = stringAlarm;
                listViewAlarmInfo.BeginInvoke(new UpdateListBoxCallback(UpdateClientList), paras);
            }
            else
            {
                //创建该控件的主线程直接更新信息列表 
                UpdateClientList(DateTime.Now.ToString(), strIP, stringAlarm);
            }
        }

        private void ProcessCommAlarm_CIDAlarm(ref CHCNetSDK.NET_DVR_ALARMER pAlarmer, IntPtr pAlarmInfo, uint dwBufLen, IntPtr pUser)
        {
            CHCNetSDK.NET_DVR_CID_ALARM struCIDAlarm = new CHCNetSDK.NET_DVR_CID_ALARM();
            uint dwSize = (uint)Marshal.SizeOf(struCIDAlarm);
            struCIDAlarm = (CHCNetSDK.NET_DVR_CID_ALARM)Marshal.PtrToStructure(pAlarmInfo, typeof(CHCNetSDK.NET_DVR_CID_ALARM));

            //报警设备IP地址
            string strIP = pAlarmer.sDeviceIP;

            //报警时间：年月日时分秒
            string strTimeYear = (struCIDAlarm.struTriggerTime.wYear).ToString();
            string strTimeMonth = (struCIDAlarm.struTriggerTime.byMonth).ToString("d2");
            string strTimeDay = (struCIDAlarm.struTriggerTime.byDay).ToString("d2");
            string strTimeHour = (struCIDAlarm.struTriggerTime.byHour).ToString("d2");
            string strTimeMinute = (struCIDAlarm.struTriggerTime.byMinute).ToString("d2");
            string strTimeSecond = (struCIDAlarm.struTriggerTime.bySecond).ToString("d2");
            string strTime = strTimeYear + "-" + strTimeMonth + "-" + strTimeDay + " " + strTimeHour + ":" + strTimeMinute + ":" + strTimeSecond;

            string stringAlarm = "CID event number，sCIDCode：" + System.Text.Encoding.UTF8.GetString(struCIDAlarm.sCIDCode).TrimEnd('\0')
                + "，sCIDDescribe：" + System.Text.Encoding.UTF8.GetString(struCIDAlarm.sCIDDescribe).TrimEnd('\0')
                + "，Report type：" + struCIDAlarm.byReportType + "，Zone number：" + struCIDAlarm.wDefenceNo + "，alarm triggering time：" + strTime;

            if (InvokeRequired)
            {
                object[] paras = new object[3];
                paras[0] = DateTime.Now.ToString(); //当前PC系统时间
                paras[1] = strIP;
                paras[2] = stringAlarm;
                listViewAlarmInfo.BeginInvoke(new UpdateListBoxCallback(UpdateClientList), paras);
            }
            else
            {
                //创建该控件的主线程直接更新信息列表 
                UpdateClientList(DateTime.Now.ToString(), strIP, stringAlarm);
            }
        }

        private void ProcessCommAlarm_AcsAlarm(ref CHCNetSDK.NET_DVR_ALARMER pAlarmer, IntPtr pAlarmInfo, uint dwBufLen, IntPtr pUser)
        {
            CHCNetSDK.NET_DVR_ACS_ALARM_INFO struAcsAlarm = new CHCNetSDK.NET_DVR_ACS_ALARM_INFO();
            uint dwSize = (uint)Marshal.SizeOf(struAcsAlarm);
            struAcsAlarm = (CHCNetSDK.NET_DVR_ACS_ALARM_INFO)Marshal.PtrToStructure(pAlarmInfo, typeof(CHCNetSDK.NET_DVR_ACS_ALARM_INFO));

            //报警设备IP地址
            string strIP = pAlarmer.sDeviceIP;

            //保存抓拍图片
            if ((struAcsAlarm.dwPicDataLen != 0) && (struAcsAlarm.pPicData != IntPtr.Zero))
            {
                string str = "Device_Acs_CapturePic_[" + strIP + "]_lUerID_[" + pAlarmer.lUserID + "]" + ".jpg";
                FileStream fs = new FileStream(str, FileMode.Create);
                int iLen = (int)struAcsAlarm.dwPicDataLen;
                byte[] by = new byte[iLen];
                Marshal.Copy(struAcsAlarm.pPicData, by, 0, iLen);
                fs.Write(by, 0, iLen);
                fs.Close();
            }

            //报警时间：年月日时分秒
            string strTimeYear = (struAcsAlarm.struTime.dwYear).ToString();
            string strTimeMonth = (struAcsAlarm.struTime.dwMonth).ToString("d2");
            string strTimeDay = (struAcsAlarm.struTime.dwDay).ToString("d2");
            string strTimeHour = (struAcsAlarm.struTime.dwHour).ToString("d2");
            string strTimeMinute = (struAcsAlarm.struTime.dwMinute).ToString("d2");
            string strTimeSecond = (struAcsAlarm.struTime.dwSecond).ToString("d2");
            string strTime = strTimeYear + "-" + strTimeMonth + "-" + strTimeDay + " " + strTimeHour + ":" + strTimeMinute + ":" + strTimeSecond;

            string stringAlarm = "Access controller alarm info，dwMajor：" + struAcsAlarm.dwMajor + "，dwMinor：" + struAcsAlarm.dwMinor + "，Card number：" 
                + System.Text.Encoding.UTF8.GetString(struAcsAlarm.struAcsEventInfo.byCardNo).TrimEnd('\0') + "，Card Reader number：" + 
                struAcsAlarm.struAcsEventInfo.dwCardReaderNo + "，Alarm triggering time：" + strTime;

            if (InvokeRequired)
            {
                object[] paras = new object[3];
                paras[0] = DateTime.Now.ToString(); //当前PC系统时间
                paras[1] = strIP;
                paras[2] = stringAlarm;
                listViewAlarmInfo.BeginInvoke(new UpdateListBoxCallback(UpdateClientList), paras);
            }
            else
            {
                //创建该控件的主线程直接更新信息列表 
                UpdateClientList(DateTime.Now.ToString(), strIP, stringAlarm);
            }

        }

        private void ProcessCommAlarm_IDInfoAlarm(ref CHCNetSDK.NET_DVR_ALARMER pAlarmer, IntPtr pAlarmInfo, uint dwBufLen, IntPtr pUser)
        {
            CHCNetSDK.NET_DVR_ID_CARD_INFO_ALARM struIDInfoAlarm = new CHCNetSDK.NET_DVR_ID_CARD_INFO_ALARM();
            uint dwSize = (uint)Marshal.SizeOf(struIDInfoAlarm);
            struIDInfoAlarm = (CHCNetSDK.NET_DVR_ID_CARD_INFO_ALARM)Marshal.PtrToStructure(pAlarmInfo, typeof(CHCNetSDK.NET_DVR_ID_CARD_INFO_ALARM));

            //报警设备IP地址
            string strIP = pAlarmer.sDeviceIP;

            //保存抓拍图片
            if ((struIDInfoAlarm.dwCapturePicDataLen != 0) && (struIDInfoAlarm.pCapturePicData != IntPtr.Zero))
            {
                string str = "Device_ID_CapturePic_[" + strIP + "]_lUerID_[" + pAlarmer.lUserID + "]" + ".jpg";
                FileStream fs = new FileStream(str, FileMode.Create);
                int iLen = (int)struIDInfoAlarm.dwCapturePicDataLen;
                byte[] by = new byte[iLen];
                Marshal.Copy(struIDInfoAlarm.pCapturePicData, by, 0, iLen);
                fs.Write(by, 0, iLen);
                fs.Close();
            }

            //保存身份证图片数据
            if ((struIDInfoAlarm.dwPicDataLen != 0) && (struIDInfoAlarm.pPicData != IntPtr.Zero))
            {
                string str = "Device_ID_IDPic_[" + strIP + "]_lUerID_[" + pAlarmer.lUserID + "]" + ".jpg";
                FileStream fs = new FileStream(str, FileMode.Create);
                int iLen = (int)struIDInfoAlarm.dwPicDataLen;
                byte[] by = new byte[iLen];
                Marshal.Copy(struIDInfoAlarm.pPicData, by, 0, iLen);
                fs.Write(by, 0, iLen);
                fs.Close();
            }

            //保存指纹数据
            if ((struIDInfoAlarm.dwFingerPrintDataLen != 0) && (struIDInfoAlarm.pFingerPrintData != IntPtr.Zero))
            {
                string str = "Device_ID_FingerPrint_[" + strIP + "]_lUerID_[" + pAlarmer.lUserID + "]" + ".data";
                FileStream fs = new FileStream(str, FileMode.Create);
                int iLen = (int)struIDInfoAlarm.dwFingerPrintDataLen;
                byte[] by = new byte[iLen];
                Marshal.Copy(struIDInfoAlarm.pFingerPrintData, by, 0, iLen);
                fs.Write(by, 0, iLen);
                fs.Close();
            }

            //报警时间：年月日时分秒
            string strTimeYear = (struIDInfoAlarm.struSwipeTime.wYear).ToString();
            string strTimeMonth = (struIDInfoAlarm.struSwipeTime.byMonth).ToString("d2");
            string strTimeDay = (struIDInfoAlarm.struSwipeTime.byDay).ToString("d2");
            string strTimeHour = (struIDInfoAlarm.struSwipeTime.byHour).ToString("d2");
            string strTimeMinute = (struIDInfoAlarm.struSwipeTime.byMinute).ToString("d2");
            string strTimeSecond = (struIDInfoAlarm.struSwipeTime.bySecond).ToString("d2");
            string strTime = strTimeYear + "-" + strTimeMonth + "-" + strTimeDay + " " + strTimeHour + ":" + strTimeMinute + ":" + strTimeSecond;

            string stringAlarm = "Identity Card swiping info，dwMajor：" + struIDInfoAlarm.dwMajor + "，dwMinor：" + struIDInfoAlarm.dwMinor 
                + "，Identity Card number：" + System.Text.Encoding.UTF8.GetString(struIDInfoAlarm.struIDCardCfg.byIDNum).TrimEnd('\0') +
                "，Name：" + System.Text.Encoding.UTF8.GetString(struIDInfoAlarm.struIDCardCfg.byName).TrimEnd('\0') + 
                "，Swiping card time：" + strTime;

            if (InvokeRequired)
            {
                object[] paras = new object[3];
                paras[0] = DateTime.Now.ToString(); //当前PC系统时间
                paras[1] = strIP;
                paras[2] = stringAlarm;
                listViewAlarmInfo.BeginInvoke(new UpdateListBoxCallback(UpdateClientList), paras);
            }
            else
            {
                //创建该控件的主线程直接更新信息列表 
                UpdateClientList(DateTime.Now.ToString(), strIP, stringAlarm);
            }
        }

        public void UpdateClientList(string strAlarmTime, string strDevIP, string strAlarmMsg)
        {
            //列表新增报警信息
            listViewAlarmInfo.Items.Add(new ListViewItem(new string[] { strAlarmTime, strDevIP, strAlarmMsg }));
        }

        private void btnLogin_Click(object sender, EventArgs e)
        {
            if (textBoxIP.Text == "" || textBoxPort.Text == "" ||
                textBoxUserName.Text == "" || textBoxPassword.Text == "")
            {
                MessageBox.Show("Please input IP, Port, User name and Password!");
                return;
            }

            if (iDeviceNumber >= 20)
            {
                MessageBox.Show("The Demo supports maximum 20 devices！");
                return;
            }

            string DVRIPAddress = textBoxIP.Text; //设备IP地址或者域名 Device IP
            Int16 DVRPortNumber = Int16.Parse(textBoxPort.Text);//设备服务端口号 Device Port
            string DVRUserName = textBoxUserName.Text;//设备登录用户名 User name to login
            string DVRPassword = textBoxPassword.Text;//设备登录密码 Password to login
            CHCNetSDK.NET_DVR_DEVICEINFO_V30 DeviceInfo = new CHCNetSDK.NET_DVR_DEVICEINFO_V30();

            //登录设备 Login the device
            m_lUserID = CHCNetSDK.NET_DVR_Login_V30(DVRIPAddress, DVRPortNumber, DVRUserName, DVRPassword, ref DeviceInfo);
            if (m_lUserID < 0)
            {
                iLastErr = CHCNetSDK.NET_DVR_GetLastError();
                strErr = "NET_DVR_Login_V30 failed, error code= " + iLastErr; //登录失败，输出错误号 Failed to login and output the error code
                MessageBox.Show(strErr);
                return;
            }
            else
            {
                //登录成功
                iDeviceNumber++;
                string str1 = "" + m_lUserID;
                listViewDevice.Items.Add(new ListViewItem(new string[] { str1, DVRIPAddress, "Disarmed" }));//将已注册设备添加进列表
            }
        }

        private void btn_SetAlarm_Click(object sender, EventArgs e)
        {
             CHCNetSDK.NET_DVR_SETUPALARM_PARAM struAlarmParam = new CHCNetSDK.NET_DVR_SETUPALARM_PARAM();
             struAlarmParam.dwSize = (uint)Marshal.SizeOf(struAlarmParam);
             struAlarmParam.byLevel = 1; //0- 一级布防,1- 二级布防
             struAlarmParam.byAlarmInfoType = 1;//智能交通设备有效，新报警信息类型
             struAlarmParam.byFaceAlarmDetection = 1;//1-人脸侦测

             for (int i = 0; i < iDeviceNumber; i++)
             {
                 m_lUserID = Int32.Parse(listViewDevice.Items[i].SubItems[0].Text);
                 m_lAlarmHandle[m_lUserID] = CHCNetSDK.NET_DVR_SetupAlarmChan_V41(m_lUserID, ref struAlarmParam);
                 if (m_lAlarmHandle[m_lUserID] < 0)
                 {
                     iLastErr = CHCNetSDK.NET_DVR_GetLastError();
                     strErr = "Failed to arm, Error code:" + iLastErr; //布防失败，输出错误号
                     listViewDevice.Items[i].SubItems[2].Text = strErr;              
                 }
                 else
                 {
                     listViewDevice.Items[i].SubItems[2].Text = "Arm successfully";                     
                 }
                 btn_SetAlarm.Enabled = false;
             }
         }

         private void btnCloseAlarm_Click(object sender, EventArgs e)
         {
             for (int i = 0; i < iDeviceNumber; i++)
             {
                 m_lUserID = Int32.Parse(listViewDevice.Items[i].SubItems[0].Text);
                 if (m_lAlarmHandle[m_lUserID] >= 0)
                 {
                     if (!CHCNetSDK.NET_DVR_CloseAlarmChan_V30(m_lAlarmHandle[m_lUserID]))
                     {
                         iLastErr = CHCNetSDK.NET_DVR_GetLastError();
                         strErr = "Failed to disarm, Error code:" + iLastErr; //撤防失败，输出错误号
                         listViewDevice.Items[i].SubItems[2].Text = strErr;
                     }
                     else
                     {
                         listViewDevice.Items[i].SubItems[2].Text = "Disarmed";
                         m_lAlarmHandle[i] = -1;
                     }
                 }
                 else 
                 {
                     listViewDevice.Items[i].SubItems[2].Text = "Disarmed";                 
                 }
             }
             btn_SetAlarm.Enabled = true;
         }

         private void btnExit_Click(object sender, EventArgs e)
         {
             //撤防
             btnCloseAlarm_Click(sender,e);

             //停止监听
             if (iListenHandle >= 0)
             {
                 CHCNetSDK.NET_DVR_StopListen_V30(iListenHandle);
             }

             //注销登录
             for (int i = 0; i < iDeviceNumber; i++)
             {
                 m_lUserID = Int32.Parse(listViewDevice.Items[i].SubItems[0].Text);
                 CHCNetSDK.NET_DVR_Logout(m_lUserID);                 
             }

             //释放SDK资源，在程序结束之前调用
             CHCNetSDK.NET_DVR_Cleanup();

             Application.Exit();
         }

         private void listViewDevice_MouseClick(object sender, MouseEventArgs e)
         {
             if (e.Button == MouseButtons.Right)
             {
                 if (listViewDevice.SelectedItems.Count > 0)
                 {
                     if (DialogResult.OK == MessageBox.Show("Are you sure to delete the selected devices！","Delete prompt",MessageBoxButtons.OKCancel))
                     {
                         foreach (ListViewItem item in this.listViewDevice.SelectedItems)
                         {
                             if (item.Selected)
                             {
                                 m_lUserID = Int32.Parse(item.SubItems[0].Text);
                                 CHCNetSDK.NET_DVR_CloseAlarmChan_V30(m_lAlarmHandle[m_lUserID]);
                                 CHCNetSDK.NET_DVR_Logout(m_lUserID);
                                 item.Remove();
                                 iDeviceNumber--;
                             }
                         }
                         this.listViewDevice.Refresh();
                     }                      
                 }
                 else
                 {
                     
                 }
             }
         }

         private void btnStartListen_Click(object sender, EventArgs e)
         {
             string sLocalIP = textBoxListenIP.Text;
             ushort wLocalPort = ushort.Parse(textBoxListenPort.Text);

             if (m_falarmData==null)
             {
                 m_falarmData = new CHCNetSDK.MSGCallBack(MsgCallback);
             }

             iListenHandle = CHCNetSDK.NET_DVR_StartListen_V30(sLocalIP, wLocalPort, m_falarmData, IntPtr.Zero);
             if (iListenHandle < 0)
             {
                 iLastErr = CHCNetSDK.NET_DVR_GetLastError();
                 strErr = "Failed to start listening, Error code:" + iLastErr; //撤防失败，输出错误号
                 MessageBox.Show(strErr);
             }
             else
             {
                 MessageBox.Show("Start listening successfully！");
                 btnStopListen.Enabled = true;
                 btnStartListen.Enabled = false;
             }
         }

         private void btnStopListen_Click(object sender, EventArgs e)
         {
             if (!CHCNetSDK.NET_DVR_StopListen_V30(iListenHandle))
             {
                 iLastErr = CHCNetSDK.NET_DVR_GetLastError();
                 strErr = "Failed to stop listening, Error code:" + iLastErr; //撤防失败，输出错误号
                 MessageBox.Show(strErr);
             }
             else
             {
                 MessageBox.Show("Stop listening successfully！");
                 btnStopListen.Enabled = false;
                 btnStartListen.Enabled = true;
             }
         }

         private void AlarmDemo_Load(object sender, EventArgs e)
         {

         }

         private void listViewDevice_SelectedIndexChanged(object sender, EventArgs e)
         {

         }

         private void listViewAlarmInfo_SelectedIndexChanged(object sender, EventArgs e)
         {

         }

         private void label9_Click(object sender, EventArgs e)
         {

         }
        
    }
}
