#define __COMM_C__
#include "comm.h"
#include "msg.h"
#include "utils.h"
#include "about.h"
#include "deal.h"
#include "timeouts.h"
#include "pinctrl.h"
#include "resource.h"
#include "user_fx.h"

struct comm_s comm;

//串口设置数据
char *aBaudRate[] = {"110", "300", "600", "1200", "2400", "4800", "9600", "14400", "19200", "38400", "57600", "74880", "115200", "128000", "256000", NULL};
DWORD iBaudRate[] = {CBR_110, CBR_300, CBR_600, CBR_1200, CBR_2400, CBR_4800, CBR_9600, CBR_14400, CBR_19200, CBR_38400, CBR_57600, CBR_74880, CBR_115200, CBR_128000, CBR_256000};
char *aParity[] = {"无校验", "偶校验", "奇校验", "标记校验", "空格校验", NULL};
BYTE iParity[] = {NOPARITY, EVENPARITY, ODDPARITY, MARKPARITY, SPACEPARITY};
char *aStopBit[] = {"1位", "1.5位", "2位", NULL};
BYTE iStopBit[] = {ONESTOPBIT, ONE5STOPBITS, TWOSTOPBITS};
char *aDataSize[] = {"8位", "7位", "6位", "5位", NULL};
BYTE iDataSize[] = {8, 7, 6, 5};

//控件句柄
HWND hWndMain;
HWND hComPort;
HWND hBaudRate;
HWND hParity;
HWND hDataSize;
HWND hStopBit;

//数据结构
//DCB cdcb;
COMMTIMEOUTS ctimeouts = {0, 1, 0, 1, 0};
COMMCONFIG cconfig;
//COMSTAT cstate;

void init_comm(void)
{
    memset(&comm, 0, sizeof(comm));
    comm.init = init;
    comm.update = get_comm_list;
    comm.open = open;
    comm.save_to_file = save_to_file;
    comm.load_from_file = load_from_file;
    comm.set_data_fmt = set_data_fmt;
    comm.close = close;
    comm.hardware_config = hardware_config;
    comm.show_pin_ctrl = ShowPinCtrl;
    comm.show_timeouts = ShowTimeouts;
    comm.update_config = update_config;
    comm.switch_disp = switch_disp;
    //数据成员
    comm.data_fmt_recv = 1;//16进制
    comm.data_fmt_send = 1;//16进制
    comm.fAutoSend = 0;			//非自动发送
    comm.fShowDataReceived = 1;	//显示接收到的数据
    comm.data_count = 0;;
    comm.fDisableChinese = 1;	//默认不允许显示中文
}

/**************************************************
函  数:update_config@-
功  能:更新待打开的串口设备的相关操作设置
参  数:only_update:!0:只是更新到结构体,不更新到设备
返回值:成功:!0,失败:0
说  明:
**************************************************/
int update_config(int only_update)
{
    int index;
    if(!only_update)
    {
        unsigned long size = sizeof(cconfig);;
        if(!GetCommConfig(msg.hComPort, &cconfig, &size))
        {
            utils.msgerr("取得串口默认配置时出错");
            return 0;
        }
        if(!GetCommState(msg.hComPort, &cconfig.dcb))
        {
            utils.msgerr("取得串口状态错误");
            return 0;
        }
    }
    //必须为TRUE
    cconfig.dcb.fBinary = TRUE;

    //波特率
    index = ComboBox_GetCurSel(hBaudRate);
    cconfig.dcb.BaudRate = iBaudRate[index];
    //校验位
    index = ComboBox_GetCurSel(hParity);
    cconfig.dcb.Parity = iParity[index];
    cconfig.dcb.fParity = index == 0 ? FALSE : TRUE;
    //数据长度
    index = ComboBox_GetCurSel(hDataSize);
    cconfig.dcb.ByteSize = iDataSize[index];
    //停止位
    index = ComboBox_GetCurSel(hStopBit);
    cconfig.dcb.StopBits = iStopBit[index];

    if(!only_update)
    {
        //还需要再设置SetCommState吗?
        if(!SetCommConfig(msg.hComPort, &cconfig, sizeof(cconfig)))
        {
            utils.msgerr("COM配置错误");
            return 0;
        }
        if(!SetCommMask(msg.hComPort, EV_RXCHAR/*|EV_BREAK*/))
        {
            utils.msgerr("SetCommMask错误");
            return 0;
        }
        //超时
        if(!SetCommTimeouts(msg.hComPort, &ctimeouts))
        {
            utils.msgerr("设置超时错误!\n请检查或回到默认设置!");
            ShowTimeouts();
            return 0;
        }
        if(!SetupComm(msg.hComPort, COMMON_READ_BUFFER_SIZE, COMMON_READ_BUFFER_SIZE))
        {
            utils.msgerr("SetupComm");
        }
        PurgeComm(msg.hComPort, PURGE_RXABORT | PURGE_RXCLEAR);
        PurgeComm(msg.hComPort, PURGE_TXABORT | PURGE_TXCLEAR);
    }
    return 1;
}

/**************************************************
函  数:init@-
功  能:只负责在程序初始化时作初始化操作,被WM_CREATE调用
参  数:(none)
返回值:(none)
说  明:其它时候别调用
**************************************************/
void init(void)
{
    int it;
    hWndMain = msg.hWndMain;
#define _GETHWND(name,id) \
		name = GetDlgItem(hWndMain,id)
    _GETHWND(hComPort, IDC_CBO_CP);
    _GETHWND(hBaudRate, IDC_CBO_BR);
    _GETHWND(hParity, IDC_CBO_CHK);
    _GETHWND(hDataSize, IDC_CBO_DATA);
    _GETHWND(hStopBit, IDC_CBO_STOP);
#undef _GETHWND
#pragma warning(push)
#pragma warning(disable:4127)
#define _SETLIST(_array,_hwnd,_init) \
		do{\
			for(it=0; _array[it]; it++)\
				ComboBox_AddString(_hwnd,_array[it]);\
			ComboBox_SetCurSel(_hwnd,_init);\
		}while(0)

    _SETLIST(aBaudRate, hBaudRate, 6);	//波特率,9600bps
    _SETLIST(aParity, hParity, 1);		//校验位,偶校验
    _SETLIST(aStopBit, hStopBit, 0);		//停止位,0
    _SETLIST(aDataSize, hDataSize, 1);	//数据长度,7位/字节
#undef _SETLIST
#pragma warning(pop)

    //16进制/字符模式选择,初始化:16进制发送, 16进制接收,需要同时设置init_comm时的初始化数据成员的值
    CheckRadioButton(hWndMain, IDC_RADIO_SEND_HEX, IDC_RADIO_SEND_CHAR, IDC_RADIO_SEND_HEX);
    CheckRadioButton(hWndMain, IDC_RADIO_RECV_HEX, IDC_RADIO_RECV_CHAR, IDC_RADIO_RECV_HEX);
    //自动发送时间
    SetDlgItemText(hWndMain, IDC_EDIT_DELAY, "1000");
}

/**************************************************
函  数:get_comm_list@4
功  能:更新系统串口设备到串口列表
参  数:
	which:
		1) 若0<=which<64,则返回串口设备号
		2) 若which==-1,仅更新列表到显示区
		3) 若为有效句柄,则返回由该which指定的串口设备的序号
			0<=which<MAX_COM:串口号
			MAX_COM<=which<MAX_COM+MAX_COM:串口名字
返回值:(见which参数),其它值无效
说  明:待优化,粗略地完成了
	2013-03-04:更新,关闭后,依然显示关闭前的设备(如果存在)
**************************************************/
/*
int get_comm_list(int* which)
{
	static int com_port[64];
	unsigned int com_index=0;
	int now_sel=-1;//2013-03-04:表示更新前被选中的设备现已不存在,否则表示ComboBox索引
	int new_sel=0;
	HKEY hKeyComm=NULL;
	char* pSubKey = "Hardware\\DeviceMap\\SerialComm";
	if((int)which<64 && (int)which>=0){
		return com_port[(int)which];
	}else if((int)which==-1){
		now_sel = ComboBox_GetCurSel(hComPort);
		if(now_sel!=-1)
			now_sel = com_port[now_sel];
	}
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,pSubKey,0,KEY_QUERY_VALUE,&hKeyComm)==ERROR_SUCCESS){
		char name[64],value[64];
		DWORD len_name=sizeof(name),len_value=sizeof(value);
		DWORD increase=0;
		while(RegEnumValue(hKeyComm,increase++,name,&len_name,NULL,NULL,(UCHAR*)value,&len_value)==ERROR_SUCCESS){
			char* pch = value;
			int comid;
			__try{
				while(*pch && !isdigit(*pch))
					pch++;
			}
			__except(EXCEPTION_EXECUTE_HANDLER){
				utils.msgbox(MB_ICONHAND,COMMON_NAME,"Access Violation!(get_comm_list)");
			}
			comid = atoi(pch);
			com_port[com_index++] = comid;
			if(com_index==__ARRAY_SIZE(com_port))
				break;
			//2013-02-25修复
			len_name = sizeof(name);
			len_value = sizeof(value);
		}
		RegCloseKey(hKeyComm);
	}else{
		//utils.msgerr("无法枚举串口设备");
	}
	utils.bubble_sort(com_port,com_index,1);
	ComboBox_ResetContent(hComPort);
	for(;;){
		unsigned int it;
		char str[6]; //"COMxx"
		for(it=0; it<com_index; it++){
			sprintf(str,"COM%d",com_port[it]);
			ComboBox_AddString(hComPort,str);
			if(which==(int*)-1){
				if(now_sel!=-1 && com_port[it]==now_sel){
					new_sel = it;
				}
			}else{
				if(com_port[it] == *which){
					*which = it;
				}
			}
		}
		break;
	}
	ComboBox_SetCurSel(hComPort,now_sel!=-1?new_sel:0);
	if(now_sel==-1){
		char str[64];
		_snprintf(str,__ARRAY_SIZE(str),"共找到串口设备 %d 个!",com_index);
		deal.update_status(str);
	}
	return 1;
}
*/

#define MAX_COM 16
struct COM_LIST
{
    int count;
    int com_id[MAX_COM];
    char com_name[MAX_COM][256];
    //int com[MAX_COM];
};

int get_comm_list(int *which)
{
    int it;

    static struct COM_LIST com_list;
    unsigned int com_index = 0;

    int now_sel = -1; //2013-03-04:表示更新前被选中的设备现已不存在,否则表示ComboBox索引
    int new_sel = 0;

    //SetupApi
    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA spdata = {0};

    //0~MAX_COM:返回串口号,-1:保留当前更新
    if((int)which < MAX_COM && (int)which >= 0)
    {
        return com_list.com_id[(int)which];
    }
    else if((int)which == -1)
    {
        now_sel = ComboBox_GetCurSel(hComPort);
        if(now_sel != -1)
            now_sel = com_list.com_id[now_sel];
    }
    else if((int)which >= MAX_COM && (int)which < 2 * MAX_COM)
    {
        return (int)com_list.com_name[(int)which-MAX_COM];
    }

    //遍历串口设备,通过SetupApi
    com_list.count = 0;
    hDevInfo = SetupDiGetClassDevs(&GUID_CLASS_COMPORT, 0, 0, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
    if(hDevInfo == INVALID_HANDLE_VALUE)
    {
        utils.msgerr("SetupApi");
        com_list.count = 0;
        return 0;
    }
    spdata.cbSize = sizeof(spdata);
    for(it = 0; SetupDiEnumDeviceInfo(hDevInfo, it, &spdata); it++)
    {
        //char* buffer = NULL;
        //DWORD buffer_size = 0;
        //char* pch = NULL;

        if(SetupDiGetDeviceRegistryProperty(hDevInfo, &spdata, SPDRP_FRIENDLYNAME,
                                            NULL, (PBYTE)&com_list.com_name[com_index][0], sizeof(com_list.com_name[0]), NULL))
        {
            char *pch = NULL;
            int tmp_id = 0;
            //2013-03-09修复未检测并口的错误
            pch = strstr(&com_list.com_name[com_index][0], "LPT");
            if(pch) continue;//并口
            pch = strstr(&com_list.com_name[com_index][0], "COM");
            __try
            {
                tmp_id = atoi(pch + 3);
                *(pch - 1) = 0;
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                utils.msgbox(MB_ICONERROR, COMMON_NAME, "SetupDiGetDeviceRegistryProperty:Access Violation! Please Report This Bug!");
            }
            com_list.com_id[com_index] = tmp_id;
            com_index++;
        }
    }
    SetupDiDestroyDeviceInfoList(hDevInfo);
    com_list.count = com_index;

    //utils.bubble_sort(com_port,com_index,1);

    ComboBox_ResetContent(hComPort);
    for(;;)
    {
        unsigned int it;
        char str[300]; //"COMxx"
        for(it = 0; it < com_index; it++)
        {
            sprintf(str, "COM%2d            %s", com_list.com_id[it], com_list.com_name[it]);
            ComboBox_AddString(hComPort, str);
            if(which == (int *) - 1)
            {
                if(now_sel != -1 && com_list.com_id[it] == now_sel)
                {
                    new_sel = it;
                }
            }
            else
            {
                if(com_list.com_id[it] == *which)
                {
                    *which = it;
                }
            }
        }
        break;
    }
    ComboBox_SetCurSel(hComPort, now_sel != -1 ? new_sel : 0);
    if(now_sel == -1 && (int)which == -1)
    {
        char str[64];
        _snprintf(str, __ARRAY_SIZE(str), "共找到串口设备 %d 个!", com_index);
        deal.update_status(str);
    }
    return 1;
}

/**************************************************
函  数:open@-
功  能:打开串口
参  数:(none)
返回值:成功:!0,失败:0
说  明:
**************************************************/
int open(void)
{
    char str[64];
    int index = ComboBox_GetCurSel(hComPort);
    if(index == CB_ERR)
    {
        utils.msgbox(MB_ICONINFORMATION, COMMON_NAME, "没有任何可用的串口!");
        return 0;
    }
    index = get_comm_list((int *)index);
    sprintf(str, "\\\\.\\COM%d", index);
    msg.hComPort = CreateFile(str, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0/*FILE_FLAG_OVERLAPPED*/, NULL);
    if(msg.hComPort == INVALID_HANDLE_VALUE)
    {
        DWORD lasterror = GetLastError();
        utils.msgerr("打开串口时错误");
        if(lasterror == ERROR_FILE_NOT_FOUND)
        {
            comm.update((int *) - 1);
        }
        return 0;
    }
    if(!update_config(0))
    {
        CloseHandle(msg.hComPort);
        msg.hComPort = INVALID_HANDLE_VALUE;
        return 0;
    }
    deal.update_savebtn_status();
    EnableWindow(hComPort, FALSE);
    EnableWindow(hBaudRate, FALSE);
    EnableWindow(hParity, FALSE);
    EnableWindow(hDataSize, FALSE);
    EnableWindow(hStopBit, FALSE);
    SetDlgItemText(msg.hWndMain, IDC_BTN_OPEN, "关闭串口");
    deal.update_status("串口已打开!");
    comm.fCommOpened = 1;

    CreatePipe(&comm.hPipeRead, &comm.hPipeWrite, NULL, 0);

    _beginthreadex(NULL, 0, deal.thread_read, NULL, 0, NULL);
    _beginthreadex(NULL, 0, deal.thread_write, NULL, 0, NULL);

    deal.check_auto_send();

    deal.start_timer(1);

    return 1;
}

/**************************************************
函  数:close@-
功  能:关闭已经打开的串口
参  数:(none)
返回值:(none)
说  明:
**************************************************/
int close(void)
{
    deal.cancel_auto_send(1);
    comm.fCommOpened = 0;
    PurgeComm(msg.hComPort, PURGE_RXCLEAR | PURGE_RXABORT);
    PurgeComm(msg.hComPort, PURGE_TXCLEAR | PURGE_TXABORT);
    CloseHandle(msg.hComPort);
    msg.hComPort = INVALID_HANDLE_VALUE;
    for(;;)
    {
        int psd = 1;
        DWORD nWritten;
        WriteFile(comm.hPipeWrite, &psd, 4, &nWritten, NULL);
        Sleep(100);
        break;
    }
    CloseHandle(comm.hPipeRead);
    CloseHandle(comm.hPipeWrite);
    comm.hPipeRead = NULL;
    comm.hPipeWrite = NULL;
    deal.update_status(NULL);
    deal.update_savebtn_status();
    //控件使能
    EnableWindow(hComPort, TRUE);
    EnableWindow(hBaudRate, TRUE);
    EnableWindow(hParity, TRUE);
    EnableWindow(hDataSize, TRUE);
    EnableWindow(hStopBit, TRUE);
    SetDlgItemText(msg.hWndMain, IDC_BTN_OPEN, "打开串口");
    //deal.update_status("串口已关闭!");

    deal.start_timer(0);
    return 1;
}

/**************************************************
函  数:save_to_file@-
功  能:保存接收区数据到指定文件
参  数:(none)
返回值:成功:!0;失败:0
说  明:
**************************************************/
int save_to_file(void)
{
    int opentype = 0;
    char *file = NULL;
    char *title = "选择要保存的文件名";
    char *filter = "所有格式的文件(*.*)\x00*.*\x00";

    int length = 0;
    char *buffer = NULL;
    unsigned char *bytearray = NULL;
    HWND hEdit = comm.data_fmt_recv ? msg.hEditRecv : msg.hEditRecv2;

    //取得接收区的文本长度
    length = GetWindowTextLength(hEdit);
    if(length == 0)
    {
        utils.msgbox(MB_ICONEXCLAMATION, NULL, "缓冲区为空, 不需要保存!");
        return 0;
    }
    //取得保存文件名
    file = utils.get_file_name(title, filter, 1, &opentype);
    if(file == NULL) return 0;

    //2013-03-18 更正:若为字符显示方式,16进制方式不被允许,因为格式基本上不满足!
    if(opentype == 0 && !comm.data_fmt_recv)
    {
        utils.msgbox(MB_ICONEXCLAMATION, NULL, "字符格式的数据不能被保存为16进制(二进制文件)形式!\n\n请考虑保存为文本格式!");
        return 0;
    }

    //取得内存
    buffer = (char *)utils.get_mem(length + 1);
    if(buffer == NULL)
        return 0;
    //取得内容
    GetWindowText(hEdit, buffer, length + 1);

    //根据保存方式保存到文件
    if(opentype == 0) //16进制
    {
        int ret;
        ret = utils.str2hex(buffer, &bytearray);
        if(!(ret & 0x80000000)) //解析错误
        {
            utils.msgbox(MB_ICONERROR, "错误",
                         "解析文本时出错,保存失败!\n\n"
                         "已解析长度(字节数):%d", ret & 0x7FFFFFFF);
            free(buffer);
            return 0;
        }
        length = ret & 0x7FFFFFFF; //更新需要保存的长度
    }
    for(;;) //保存到文件
    {
        //在Windows平台,以rt的方式保存'\n'时会被扩展,直接保存为16进制,原文本保存方式已抛弃
        int result;
        FILE *fp = fopen(file, "wb");
        if(fp == NULL)
        {
            utils.msgbox(MB_ICONHAND, NULL, "写文件时错误:%s", file);
            if(opentype == 0) free(bytearray);
            free(buffer);
            return 0;
        }
        result = fwrite(opentype == 0 ? bytearray : (unsigned char *)buffer, 1, length, fp);
        fclose(fp);
        if(opentype == 0) free(bytearray);
        free(buffer);
        if(result == length)
        {
            utils.msgbox(MB_ICONINFORMATION, COMMON_NAME,
                         "文件 %s 已保存!\n\n"
                         "写入字节数:%d", file, result);
        }
        else
        {
            utils.msgbox(MB_ICONERROR, COMMON_NAME, "文件 %s 未能正确保存!", file);
        }
        return result == length;
        //break;
    }
}

/**************************************************
函  数:load_from_file@-
功  能:加载文件到发送区域
参  数:(none)
返回值:成功:!0;失败:0
说  明:
**************************************************/
int load_from_file(void)
{
    int opentype = 0;//0-16进制,1-ascii
    char *file =  NULL;
    char *title = "选择要加载的文件名";
    char *filter = "所有文件(*.*)\x00*.*\x00";
    //2013-01-17 BUG fix:
    //    由于每行的16进制个数应参与打开的16进制文件的最大大小, 所以应该先声明
    //
    size_t linecch = COMMON_LINE_CCH_SEND; // count of chars per line
    size_t hexfilesize = (COMMON_SEND_BUF_SIZE - COMMON_SEND_BUF_SIZE / linecch * 2) / 3;

    FILE *fp = NULL;
    unsigned char *buffer = NULL;
    char *hexstr = NULL;
    size_t length;

    file = utils.get_file_name(title, filter, 0, &opentype);
    if(file == NULL) return 0;
    fp = fopen(file, "rb");
    if(fp == NULL)
    {
        utils.msgbox(MB_ICONHAND, "错误", "无法打开文件 %s 进行读操作!", file);
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    //rewind(fp);
    fseek(fp, 0, SEEK_SET);
    //缓冲区只有??大小
    if((opentype == 1 && length > COMMON_MAX_LOAD_SIZE) || //作为文本方式打开最大1M
            (opentype == 0 && length > hexfilesize)) //作为16进制打开
    {
        utils.msgbox(MB_ICONHAND, "打开的文件过大!",
                     "文件:%s\n"
                     "已经达到最大打开文件的大小!\n\n"
                     "文本方式:最大%d字节\n"
                     "16进制方式:最大%d字节\n\n"
                     "当前加载方式:%s", file, COMMON_MAX_LOAD_SIZE,
                     hexfilesize, opentype == 0 ? "16进制" : "文本方式");
        MessageBeep(MB_ICONEXCLAMATION);
        if(utils.msgbox(MB_ICONQUESTION | MB_YESNO, "继续操作?",
                        "%s\n\n"
                        "单击 是:继续打开文件,但文件会被截断(短)\n"
                        "单击 否:放弃加载文件操作", file) == IDNO)
        {
            fclose(fp);
            return 0;
        }
        else  //截断文件
        {
            length = (size_t)(opentype == 0 ? hexfilesize : COMMON_MAX_LOAD_SIZE);
            //fseek(fp, length, SEEK_SET);
        }
    }
    //读取指定长度的文件到缓冲区
    buffer = (unsigned char *)utils.get_mem(length + 1); //+1个字节是多余的,以保证若是文本读取,最后的字符为0
    if(buffer == NULL)
    {
        fclose(fp);
        return 0;
    }

    if(length != fread(buffer, 1, length, fp))
    {
        utils.msgbox(MB_ICONHAND, NULL, "读取文件时读取的文件长度与预期的长度不一致! 错误!");
        free(buffer);
        fclose(fp);
        return 0;
    }
    fclose(fp);
    //若以16方式打开,对16进制格式格式化到文本
    if(opentype == 0) //16进制方式打开
    {
        hexstr = utils.hex2str(buffer, (int *)&length, linecch, 0, NULL, 0); //每行显示16个16进制
        if(hexstr == NULL)
        {
            utils.msgbox(MB_ICONHAND, NULL, "在将16进制转换为字符串时遇到错误!");
            free(buffer);
            return 0;
        }
    }
    else  //若以文本方式加载16进制内容, 简单地验证内容是否合法
    {
        unsigned char *xx = NULL;
        int xx_ret;
        xx_ret = utils.str2hex((char *)buffer, &xx);
        if(!(xx_ret & 0x80000000)) //解析失败了呢,内容不合法
        {
            //显示出不合法的文本的开始的一部分(若干字节,这里取64个字节)的内容
            int parsed = xx_ret & 0x7FFFFFFF;
            if((int)(length - parsed) >= 64)
            {
                //未被解析的文件还有超过64字节~~~
                //从第60字节处截断,显示三个小数点
                int xx_i;
                for(xx_i = 0; xx_i < 3; xx_i++)
                {
                    buffer[parsed+60+xx_i] = '.';
                }
                buffer[parsed+60+xx_i] = 0;
            }
            else
            {
                //不足64个字节直接显示
            }
            xx = buffer + parsed;
            utils.msgbox(MB_ICONHAND, "16进制文本解析错误",
                         "从以下字符开始为不正确的格式:\n\n%s", xx);
            free(buffer);
            return 0;
        }
        else
        {
            //格式无误,但16进制内容不需要
            free(xx);
        }
    }
    //2013-02-27:
    //取消自动发送
    //把内容更新到发送区文本框
    SetDlgItemText(msg.hWndMain, IDC_EDIT_SEND, (opentype == 0 ? hexstr : (char *)buffer));
    free(buffer);
    if(opentype == 0)
        free(hexstr);
    return 1;
}

/**************************************************
函  数:set_data_fmt@-
功  能:取得当前发送/接收数据的格式
参  数:(none)
返回值:(none)
说  明:
**************************************************/
void set_data_fmt(void)
{
    comm.data_fmt_send = IsDlgButtonChecked(msg.hWndMain, IDC_RADIO_SEND_HEX);
    comm.data_fmt_recv = IsDlgButtonChecked(msg.hWndMain, IDC_RADIO_RECV_HEX);
    ShowWindow(msg.hEditRecv, comm.data_fmt_recv > 0);
    ShowWindow(msg.hEditRecv2, comm.data_fmt_recv == 0);
    SetDlgItemText(msg.hWndMain, IDC_STATIC_RECV, comm.data_fmt_recv ? "数据接收 - 16进制模式" : "数据接收 - 字符模式");
}

/**************************************************
函  数:switch_disp@-
功  能:更改当前字符显示的方式,中文与Ascii
参  数:(none)
返回值:(none)
说  明:
**************************************************/
int switch_disp(void)
{
    if(comm.fDisableChinese) //当前为显示字符
    {
        comm.fDisableChinese = 0;
    }
    else  //当前为显示中文
    {
        comm.fDisableChinese = 1;
    }
    return 0;
}
/**************************************************
函  数:hardware_config@-
功  能:提供硬件驱动支持的配置
参  数:(none)
返回值:(useless)
说  明:
**************************************************/
int hardware_config(void)
{
    int comid;
    char str[64];
    HWND hcp = GetDlgItem(msg.hWndMain, IDC_CBO_CP);

    if(msg.hComPort != INVALID_HANDLE_VALUE)
    {
        utils.msgbox(MB_ICONINFORMATION, COMMON_NAME, "串口需要先被关闭!");
        return 0;
    }

    comid = ComboBox_GetCurSel(hcp);
    if(comid == CB_ERR)
    {
        utils.msgbox(MB_ICONEXCLAMATION, COMMON_NAME, "请先选择一个串口设备!");
        return 0;
    }
    comid = comm.update((int *)comid);
    _snprintf(str, __ARRAY_SIZE(str), "COM%d", comid);
    cconfig.dwSize = sizeof(cconfig);
    comm.update_config(1);
    if(CommConfigDialog(str, msg.hWndMain, (LPCOMMCONFIG)&cconfig))
    {
        int it;
#pragma warning(push)
#pragma warning(disable:4127)
#define _SETVALUE(a,field,hwnd) \
			do{\
				for(it=0; it<__ARRAY_SIZE(a); it++){\
					if(a[it]==field){\
						ComboBox_SetCurSel(hwnd,it);\
						break;\
					}\
				}\
				if(it==__ARRAY_SIZE(a)){\
					utils.msgbox(MB_ICONERROR,COMMON_NAME,"("#field")驱动提供了当前程序不支持的设置! 错误!");\
					return 0;\
				}\
			}while(0)
        _SETVALUE(iBaudRate, cconfig.dcb.BaudRate, hBaudRate);
        _SETVALUE(iParity, cconfig.dcb.Parity, hParity);
        _SETVALUE(iStopBit, cconfig.dcb.StopBits, hStopBit);
        _SETVALUE(iDataSize, cconfig.dcb.ByteSize, hDataSize);
#undef _SETVALUE
#pragma warning(pop)
    }
    else
    {
        if(GetLastError() != ERROR_CANCELLED)
        {
            utils.msgerr("调用配置对话框错误");
        }
    }
    return 0;
}

