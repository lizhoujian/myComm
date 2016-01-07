#define __COMM_C__
#include "comm.h"
#include "msg.h"
#include "utils.h"
#include "about.h"
#include "deal.h"
#include "timeouts.h"
#include "pinctrl.h"
#include "resource.h"

struct comm_s comm;

//������������
char *aBaudRate[] = {"110", "300", "600", "1200", "2400", "4800", "9600", "14400", "19200", "38400", "57600", "74880", "115200", "128000", "256000", NULL};
DWORD iBaudRate[] = {CBR_110, CBR_300, CBR_600, CBR_1200, CBR_2400, CBR_4800, CBR_9600, CBR_14400, CBR_19200, CBR_38400, CBR_57600, CBR_74880, CBR_115200, CBR_128000, CBR_256000};
char *aParity[] = {"��У��", "żУ��", "��У��", "���У��", "�ո�У��", NULL};
BYTE iParity[] = {NOPARITY, EVENPARITY, ODDPARITY, MARKPARITY, SPACEPARITY};
char *aStopBit[] = {"1λ", "1.5λ", "2λ", NULL};
BYTE iStopBit[] = {ONESTOPBIT, ONE5STOPBITS, TWOSTOPBITS};
char *aDataSize[] = {"8λ", "7λ", "6λ", "5λ", NULL};
BYTE iDataSize[] = {8, 7, 6, 5};

//�ؼ����
HWND hWndMain;
HWND hComPort;
HWND hBaudRate;
HWND hParity;
HWND hDataSize;
HWND hStopBit;

//���ݽṹ
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
    //���ݳ�Ա
    comm.data_fmt_recv = 1;//16����
    comm.data_fmt_send = 1;//16����
    comm.fAutoSend = 0;			//���Զ�����
    comm.fShowDataReceived = 1;	//��ʾ���յ�������
    comm.data_count = 0;;
    comm.fDisableChinese = 1;	//Ĭ�ϲ�������ʾ����
}

/**************************************************
��  ��:update_config@-
��  ��:���´��򿪵Ĵ����豸����ز�������
��  ��:only_update:!0:ֻ�Ǹ��µ��ṹ��,�����µ��豸
����ֵ:�ɹ�:!0,ʧ��:0
˵  ��:
**************************************************/
int update_config(int only_update)
{
    int index;
    if(!only_update)
    {
        unsigned long size = sizeof(cconfig);;
        if(!GetCommConfig(msg.hComPort, &cconfig, &size))
        {
            utils.msgerr("ȡ�ô���Ĭ������ʱ����");
            return 0;
        }
        if(!GetCommState(msg.hComPort, &cconfig.dcb))
        {
            utils.msgerr("ȡ�ô���״̬����");
            return 0;
        }
    }
    //����ΪTRUE
    cconfig.dcb.fBinary = TRUE;

    //������
    index = ComboBox_GetCurSel(hBaudRate);
    cconfig.dcb.BaudRate = iBaudRate[index];
    //У��λ
    index = ComboBox_GetCurSel(hParity);
    cconfig.dcb.Parity = iParity[index];
    cconfig.dcb.fParity = index == 0 ? FALSE : TRUE;
    //���ݳ���
    index = ComboBox_GetCurSel(hDataSize);
    cconfig.dcb.ByteSize = iDataSize[index];
    //ֹͣλ
    index = ComboBox_GetCurSel(hStopBit);
    cconfig.dcb.StopBits = iStopBit[index];

    if(!only_update)
    {
        //����Ҫ������SetCommState��?
        if(!SetCommConfig(msg.hComPort, &cconfig, sizeof(cconfig)))
        {
            utils.msgerr("COM���ô���");
            return 0;
        }
        if(!SetCommMask(msg.hComPort, EV_RXCHAR/*|EV_BREAK*/))
        {
            utils.msgerr("SetCommMask����");
            return 0;
        }
        //��ʱ
        if(!SetCommTimeouts(msg.hComPort, &ctimeouts))
        {
            utils.msgerr("���ó�ʱ����!\n�����ص�Ĭ������!");
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
��  ��:init@-
��  ��:ֻ�����ڳ����ʼ��ʱ����ʼ������,��WM_CREATE����
��  ��:(none)
����ֵ:(none)
˵  ��:����ʱ������
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

    _SETLIST(aBaudRate, hBaudRate, 6);	//������,9600bps
    _SETLIST(aParity, hParity, 0);		//У��λ,��
    _SETLIST(aStopBit, hStopBit, 0);		//ֹͣλ,0
    _SETLIST(aDataSize, hDataSize, 0);	//���ݳ���,8λ/�ֽ�
#undef _SETLIST
#pragma warning(pop)

    //16����/�ַ�ģʽѡ��,��ʼ��:16���Ʒ���, 16���ƽ���,��Ҫͬʱ����init_commʱ�ĳ�ʼ�����ݳ�Ա��ֵ
    CheckRadioButton(hWndMain, IDC_RADIO_SEND_HEX, IDC_RADIO_SEND_CHAR, IDC_RADIO_SEND_HEX);
    CheckRadioButton(hWndMain, IDC_RADIO_RECV_HEX, IDC_RADIO_RECV_CHAR, IDC_RADIO_RECV_HEX);
    //�Զ�����ʱ��
    SetDlgItemText(hWndMain, IDC_EDIT_DELAY, "1000");
}

/**************************************************
��  ��:get_comm_list@4
��  ��:����ϵͳ�����豸�������б�
��  ��:
	which:
		1) ��0<=which<64,�򷵻ش����豸��
		2) ��which==-1,�������б���ʾ��
		3) ��Ϊ��Ч���,�򷵻��ɸ�whichָ���Ĵ����豸�����
			0<=which<MAX_COM:���ں�
			MAX_COM<=which<MAX_COM+MAX_COM:��������
����ֵ:(��which����),����ֵ��Ч
˵  ��:���Ż�,���Ե������
	2013-03-04:����,�رպ�,��Ȼ��ʾ�ر�ǰ���豸(�������)
**************************************************/
/*
int get_comm_list(int* which)
{
	static int com_port[64];
	unsigned int com_index=0;
	int now_sel=-1;//2013-03-04:��ʾ����ǰ��ѡ�е��豸���Ѳ�����,�����ʾComboBox����
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
			//2013-02-25�޸�
			len_name = sizeof(name);
			len_value = sizeof(value);
		}
		RegCloseKey(hKeyComm);
	}else{
		//utils.msgerr("�޷�ö�ٴ����豸");
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
		_snprintf(str,__ARRAY_SIZE(str),"���ҵ������豸 %d ��!",com_index);
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

    int now_sel = -1; //2013-03-04:��ʾ����ǰ��ѡ�е��豸���Ѳ�����,�����ʾComboBox����
    int new_sel = 0;

    //SetupApi
    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA spdata = {0};

    //0~MAX_COM:���ش��ں�,-1:������ǰ����
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

    //���������豸,ͨ��SetupApi
    com_list.count = 0;
    hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, 0, 0, DIGCF_PRESENT);
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
            //2013-03-09�޸�δ��Ⲣ�ڵĴ���
            pch = strstr(&com_list.com_name[com_index][0], "LPT");
            if(pch) continue;//����
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
        _snprintf(str, __ARRAY_SIZE(str), "���ҵ������豸 %d ��!", com_index);
        deal.update_status(str);
    }
    return 1;
}

/**************************************************
��  ��:open@-
��  ��:�򿪴���
��  ��:(none)
����ֵ:�ɹ�:!0,ʧ��:0
˵  ��:
**************************************************/
int open(void)
{
    char str[64];
    int index = ComboBox_GetCurSel(hComPort);
    if(index == CB_ERR)
    {
        utils.msgbox(MB_ICONINFORMATION, COMMON_NAME, "û���κο��õĴ���!");
        return 0;
    }
    index = get_comm_list((int *)index);
    sprintf(str, "\\\\.\\COM%d", index);
    msg.hComPort = CreateFile(str, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0/*FILE_FLAG_OVERLAPPED*/, NULL);
    if(msg.hComPort == INVALID_HANDLE_VALUE)
    {
        DWORD lasterror = GetLastError();
        utils.msgerr("�򿪴���ʱ����");
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
    SetDlgItemText(msg.hWndMain, IDC_BTN_OPEN, "�رմ���");
    deal.update_status("�����Ѵ�!");
    comm.fCommOpened = 1;

    CreatePipe(&comm.hPipeRead, &comm.hPipeWrite, NULL, 0);

    _beginthreadex(NULL, 0, deal.thread_read, NULL, 0, NULL);
    _beginthreadex(NULL, 0, deal.thread_write, NULL, 0, NULL);

    deal.check_auto_send();

    deal.start_timer(1);

    return 1;
}

/**************************************************
��  ��:close@-
��  ��:�ر��Ѿ��򿪵Ĵ���
��  ��:(none)
����ֵ:(none)
˵  ��:
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
    //�ؼ�ʹ��
    EnableWindow(hComPort, TRUE);
    EnableWindow(hBaudRate, TRUE);
    EnableWindow(hParity, TRUE);
    EnableWindow(hDataSize, TRUE);
    EnableWindow(hStopBit, TRUE);
    SetDlgItemText(msg.hWndMain, IDC_BTN_OPEN, "�򿪴���");
    //deal.update_status("�����ѹر�!");

    deal.start_timer(0);
    return 1;
}

/**************************************************
��  ��:save_to_file@-
��  ��:������������ݵ�ָ���ļ�
��  ��:(none)
����ֵ:�ɹ�:!0;ʧ��:0
˵  ��:
**************************************************/
int save_to_file(void)
{
    int opentype = 0;
    char *file = NULL;
    char *title = "ѡ��Ҫ������ļ���";
    char *filter = "���и�ʽ���ļ�(*.*)\x00*.*\x00";

    int length = 0;
    char *buffer = NULL;
    unsigned char *bytearray = NULL;
    HWND hEdit = comm.data_fmt_recv ? msg.hEditRecv : msg.hEditRecv2;

    //ȡ�ý��������ı�����
    length = GetWindowTextLength(hEdit);
    if(length == 0)
    {
        utils.msgbox(MB_ICONEXCLAMATION, NULL, "������Ϊ��, ����Ҫ����!");
        return 0;
    }
    //ȡ�ñ����ļ���
    file = utils.get_file_name(title, filter, 1, &opentype);
    if(file == NULL) return 0;

    //2013-03-18 ����:��Ϊ�ַ���ʾ��ʽ,16���Ʒ�ʽ��������,��Ϊ��ʽ�����ϲ�����!
    if(opentype == 0 && !comm.data_fmt_recv)
    {
        utils.msgbox(MB_ICONEXCLAMATION, NULL, "�ַ���ʽ�����ݲ��ܱ�����Ϊ16����(�������ļ�)��ʽ!\n\n�뿼�Ǳ���Ϊ�ı���ʽ!");
        return 0;
    }

    //ȡ���ڴ�
    buffer = (char *)utils.get_mem(length + 1);
    if(buffer == NULL)
        return 0;
    //ȡ������
    GetWindowText(hEdit, buffer, length + 1);

    //���ݱ��淽ʽ���浽�ļ�
    if(opentype == 0) //16����
    {
        int ret;
        ret = utils.str2hex(buffer, &bytearray);
        if(!(ret & 0x80000000)) //��������
        {
            utils.msgbox(MB_ICONERROR, "����",
                         "�����ı�ʱ����,����ʧ��!\n\n"
                         "�ѽ�������(�ֽ���):%d", ret & 0x7FFFFFFF);
            free(buffer);
            return 0;
        }
        length = ret & 0x7FFFFFFF; //������Ҫ����ĳ���
    }
    for(;;) //���浽�ļ�
    {
        //��Windowsƽ̨,��rt�ķ�ʽ����'\n'ʱ�ᱻ��չ,ֱ�ӱ���Ϊ16����,ԭ�ı����淽ʽ������
        int result;
        FILE *fp = fopen(file, "wb");
        if(fp == NULL)
        {
            utils.msgbox(MB_ICONHAND, NULL, "д�ļ�ʱ����:%s", file);
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
                         "�ļ� %s �ѱ���!\n\n"
                         "д���ֽ���:%d", file, result);
        }
        else
        {
            utils.msgbox(MB_ICONERROR, COMMON_NAME, "�ļ� %s δ����ȷ����!", file);
        }
        return result == length;
        //break;
    }
}

/**************************************************
��  ��:load_from_file@-
��  ��:�����ļ�����������
��  ��:(none)
����ֵ:�ɹ�:!0;ʧ��:0
˵  ��:
**************************************************/
int load_from_file(void)
{
    int opentype = 0;//0-16����,1-ascii
    char *file =  NULL;
    char *title = "ѡ��Ҫ���ص��ļ���";
    char *filter = "�����ļ�(*.*)\x00*.*\x00";
    //2013-01-17 BUG fix:
    //    ����ÿ�е�16���Ƹ���Ӧ����򿪵�16�����ļ�������С, ����Ӧ��������
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
        utils.msgbox(MB_ICONHAND, "����", "�޷����ļ� %s ���ж�����!", file);
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    //rewind(fp);
    fseek(fp, 0, SEEK_SET);
    //������ֻ��??��С
    if((opentype == 1 && length > COMMON_MAX_LOAD_SIZE) || //��Ϊ�ı���ʽ�����1M
            (opentype == 0 && length > hexfilesize)) //��Ϊ16���ƴ�
    {
        utils.msgbox(MB_ICONHAND, "�򿪵��ļ�����!",
                     "�ļ�:%s\n"
                     "�Ѿ��ﵽ�����ļ��Ĵ�С!\n\n"
                     "�ı���ʽ:���%d�ֽ�\n"
                     "16���Ʒ�ʽ:���%d�ֽ�\n\n"
                     "��ǰ���ط�ʽ:%s", file, COMMON_MAX_LOAD_SIZE,
                     hexfilesize, opentype == 0 ? "16����" : "�ı���ʽ");
        MessageBeep(MB_ICONEXCLAMATION);
        if(utils.msgbox(MB_ICONQUESTION | MB_YESNO, "��������?",
                        "%s\n\n"
                        "���� ��:�������ļ�,���ļ��ᱻ�ض�(��)\n"
                        "���� ��:���������ļ�����", file) == IDNO)
        {
            fclose(fp);
            return 0;
        }
        else  //�ض��ļ�
        {
            length = (size_t)(opentype == 0 ? hexfilesize : COMMON_MAX_LOAD_SIZE);
            //fseek(fp, length, SEEK_SET);
        }
    }
    //��ȡָ�����ȵ��ļ���������
    buffer = (unsigned char *)utils.get_mem(length + 1); //+1���ֽ��Ƕ����,�Ա�֤�����ı���ȡ,�����ַ�Ϊ0
    if(buffer == NULL)
    {
        fclose(fp);
        return 0;
    }

    if(length != fread(buffer, 1, length, fp))
    {
        utils.msgbox(MB_ICONHAND, NULL, "��ȡ�ļ�ʱ��ȡ���ļ�������Ԥ�ڵĳ��Ȳ�һ��! ����!");
        free(buffer);
        fclose(fp);
        return 0;
    }
    fclose(fp);
    //����16��ʽ��,��16���Ƹ�ʽ��ʽ�����ı�
    if(opentype == 0) //16���Ʒ�ʽ��
    {
        hexstr = utils.hex2str(buffer, (int *)&length, linecch, 0, NULL, 0); //ÿ����ʾ16��16����
        if(hexstr == NULL)
        {
            utils.msgbox(MB_ICONHAND, NULL, "�ڽ�16����ת��Ϊ�ַ���ʱ��������!");
            free(buffer);
            return 0;
        }
    }
    else  //�����ı���ʽ����16��������, �򵥵���֤�����Ƿ�Ϸ�
    {
        unsigned char *xx = NULL;
        int xx_ret;
        xx_ret = utils.str2hex((char *)buffer, &xx);
        if(!(xx_ret & 0x80000000)) //����ʧ������,���ݲ��Ϸ�
        {
            //��ʾ�����Ϸ����ı��Ŀ�ʼ��һ����(�����ֽ�,����ȡ64���ֽ�)������
            int parsed = xx_ret & 0x7FFFFFFF;
            if((int)(length - parsed) >= 64)
            {
                //δ���������ļ����г���64�ֽ�~~~
                //�ӵ�60�ֽڴ��ض�,��ʾ����С����
                int xx_i;
                for(xx_i = 0; xx_i < 3; xx_i++)
                {
                    buffer[parsed+60+xx_i] = '.';
                }
                buffer[parsed+60+xx_i] = 0;
            }
            else
            {
                //����64���ֽ�ֱ����ʾ
            }
            xx = buffer + parsed;
            utils.msgbox(MB_ICONHAND, "16�����ı���������",
                         "�������ַ���ʼΪ����ȷ�ĸ�ʽ:\n\n%s", xx);
            free(buffer);
            return 0;
        }
        else
        {
            //��ʽ����,��16�������ݲ���Ҫ
            free(xx);
        }
    }
    //2013-02-27:
    //ȡ���Զ�����
    //�����ݸ��µ��������ı���
    SetDlgItemText(msg.hWndMain, IDC_EDIT_SEND, (opentype == 0 ? hexstr : (char *)buffer));
    free(buffer);
    if(opentype == 0)
        free(hexstr);
    return 1;
}

/**************************************************
��  ��:set_data_fmt@-
��  ��:ȡ�õ�ǰ����/�������ݵĸ�ʽ
��  ��:(none)
����ֵ:(none)
˵  ��:
**************************************************/
void set_data_fmt(void)
{
    comm.data_fmt_send = IsDlgButtonChecked(msg.hWndMain, IDC_RADIO_SEND_HEX);
    comm.data_fmt_recv = IsDlgButtonChecked(msg.hWndMain, IDC_RADIO_RECV_HEX);
    ShowWindow(msg.hEditRecv, comm.data_fmt_recv > 0);
    ShowWindow(msg.hEditRecv2, comm.data_fmt_recv == 0);
    SetDlgItemText(msg.hWndMain, IDC_STATIC_RECV, comm.data_fmt_recv ? "���ݽ��� - 16����ģʽ" : "���ݽ��� - �ַ�ģʽ");
}

/**************************************************
��  ��:switch_disp@-
��  ��:���ĵ�ǰ�ַ���ʾ�ķ�ʽ,������Ascii
��  ��:(none)
����ֵ:(none)
˵  ��:
**************************************************/
int switch_disp(void)
{
    if(comm.fDisableChinese) //��ǰΪ��ʾ�ַ�
    {
        comm.fDisableChinese = 0;
    }
    else  //��ǰΪ��ʾ����
    {
        comm.fDisableChinese = 1;
    }
    return 0;
}
/**************************************************
��  ��:hardware_config@-
��  ��:�ṩӲ������֧�ֵ�����
��  ��:(none)
����ֵ:(useless)
˵  ��:
**************************************************/
int hardware_config(void)
{
    int comid;
    char str[64];
    HWND hcp = GetDlgItem(msg.hWndMain, IDC_CBO_CP);

    if(msg.hComPort != INVALID_HANDLE_VALUE)
    {
        utils.msgbox(MB_ICONINFORMATION, COMMON_NAME, "������Ҫ�ȱ��ر�!");
        return 0;
    }

    comid = ComboBox_GetCurSel(hcp);
    if(comid == CB_ERR)
    {
        utils.msgbox(MB_ICONEXCLAMATION, COMMON_NAME, "����ѡ��һ�������豸!");
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
					utils.msgbox(MB_ICONERROR,COMMON_NAME,"("#field")�����ṩ�˵�ǰ����֧�ֵ�����! ����!");\
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
            utils.msgerr("�������öԻ������");
        }
    }
    return 0;
}

