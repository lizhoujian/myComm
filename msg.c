#include <stdio.h>
#include <ctype.h>
#include <process.h>
#define __MSG_C__
#include "msg.h"
#include "utils.h"
#include "deal.h"
#include "about.h"
#include "comm.h"
//#include "timeouts.h"
//#include "pinctrl.h"
#include "asctable.h"
#include "str2hex.h"
#include "resource.h"

struct msg_s msg;

static void TRACE(const char * sz, ...)
{
    char szData[512]={0};

    va_list args;
    va_start(args, sz);
    _vsnprintf(szData, sizeof(szData) - 1, sz, args);
    va_end(args);

    OutputDebugString(szData);
}

//��Ϣ�ṹ���ʼ��, ���캯��
int init_msg(void)
{
    memset(&msg, 0, sizeof(msg));
    msg.run_app = run_app;
    msg.on_create = on_create;
    msg.on_close = on_close;
    msg.on_destroy = on_destroy;
    msg.on_command = on_command;
    msg.on_timer = on_timer;
    msg.on_device_change = on_device_change;
    return 1;
}

/**************************************************
��  ��:RecvEditWndProc@16
��  ��:������EDIT���������,ȡ�����ѡ���ı�ʱ�Բ�����ı���ɵĸ���
��  ��:
����ֵ:
˵  ��:��EM_SETSEL �� EM_REPLACESEL����䲻����ѡ��
**************************************************/
WNDPROC OldRecvEditWndProc = NULL;
LRESULT CALLBACK RecvEditWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static int fEnableSelect = 1;
    switch(uMsg)
    {
    case EM_SETSEL:
        fEnableSelect = 0;
        return CallWindowProc(OldRecvEditWndProc, hWnd, uMsg, wParam, lParam);
    case EM_REPLACESEL:
    {
        LRESULT ret;
        ret = CallWindowProc(OldRecvEditWndProc, hWnd, uMsg, wParam, lParam);
        fEnableSelect = 1;
        return ret;
    }
    case WM_LBUTTONDOWN:
    case WM_MOUSEMOVE:/*
        if(fEnableSelect == 0 || comm.fShowDataReceived && msg.hComPort != INVALID_HANDLE_VALUE)
        {
            return 0;
        }*/
        break;
    case WM_CONTEXTMENU:/*
        if(comm.fShowDataReceived && msg.hComPort != INVALID_HANDLE_VALUE)
            return 0;
        else*/
            break;
    }
    return CallWindowProc(OldRecvEditWndProc, hWnd, uMsg, wParam, lParam);
}

WNDPROC OldRecv2EditWndProc = NULL;
LRESULT CALLBACK Recv2EditWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static int fEnableSelect = 1;
    switch(uMsg)
    {
    case EM_SETSEL:
        fEnableSelect = 0;
        return CallWindowProc(OldRecv2EditWndProc, hWnd, uMsg, wParam, lParam);
    case EM_REPLACESEL:
    {
        LRESULT ret;
        ret = CallWindowProc(OldRecv2EditWndProc, hWnd, uMsg, wParam, lParam);
        fEnableSelect = 1;
        return ret;
    }
    case WM_LBUTTONDOWN:
    case WM_MOUSEMOVE:/*
        if(fEnableSelect == 0 || comm.fShowDataReceived && msg.hComPort != INVALID_HANDLE_VALUE)
        {
            return 0;
        }*/
        break;
    case WM_CONTEXTMENU:
/*
        if(msg.hComPort != INVALID_HANDLE_VALUE && comm.fShowDataReceived)
        {
            POINT pt;
            HMENU hEditMenu;
            hEditMenu = GetSubMenu(LoadMenu(msg.hInstance, MAKEINTRESOURCE(IDR_MENU_EDIT_RECV)), 0);
            GetCursorPos(&pt);
            CheckMenuItem(hEditMenu, MENU_EDIT_CHINESE, MF_BYCOMMAND | (comm.fDisableChinese ? MF_UNCHECKED : MF_CHECKED));
            TrackPopupMenu(hEditMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, msg.hWndMain, NULL);
            return 0;
        }
        else*/
        {
            break;
        }

    }
    return CallWindowProc(OldRecv2EditWndProc, hWnd, uMsg, wParam, lParam);
}

#define _SETRESULT(_msg,_result,_msgret) \
	case _msg:SetDlgMsgResult(hWnd,_msg,_result);return _msgret
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //TRACE("hwnd = %x, umsg = %x, wparam = %x, lparam = %x\n", hWnd, uMsg, wParam, lParam);
    switch(uMsg)
    {
        _SETRESULT(WM_CLOSE, msg.on_close(), 0);
        _SETRESULT(WM_DESTROY, msg.on_destroy(), 0);
        _SETRESULT(WM_INITDIALOG, msg.on_create(hWnd, (HINSTANCE)lParam), 0);
        _SETRESULT(WM_COMMAND, msg.on_command((HWND)lParam, LOWORD(wParam), HIWORD(wParam)), 0);
        _SETRESULT(WM_TIMER, msg.on_timer((int)wParam), 0);
        _SETRESULT(WM_DEVICECHANGE, msg.on_device_change(wParam, (DEV_BROADCAST_HDR *)lParam), 0);
    default:
        return 0;
    }
}
#undef _SETRESULT

int run_app(void)
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    return DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DLG_MAIN1), NULL, (DLGPROC)MainWndProc, (LPARAM)hInstance);
}

int on_timer(int id)
{
    if(!comm.fAutoSend || msg.hComPort == INVALID_HANDLE_VALUE)
    {
        deal.cancel_auto_send(0);
        return 0;
    }
    deal.do_send();
    UNREFERENCED_PARAMETER(id);
    return 0;
}

int on_create(HWND hWnd, HINSTANCE hInstance)
{
    HICON hIcon = NULL;
    //��ʼ�����
    msg.hWndMain = hWnd;
    msg.hInstance = hInstance;
    msg.hEditRecv = GetDlgItem(hWnd, IDC_EDIT_RECV);
    msg.hEditRecv2 = GetDlgItem(hWnd, IDC_EDIT_RECV2);
    msg.hComPort = INVALID_HANDLE_VALUE;

    //�Ѵ����ƶ�����Ļ����
    utils.center_window(hWnd, NULL);
    //������ͼ��
    hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    SendMessage(msg.hWndMain, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    SetWindowText(hWnd, COMMON_NAME_AND_VERSION);
    SetFocus(GetDlgItem(hWnd, IDC_BTN_OPEN));
    //SetClassLong(hWnd,gcl_)

    //������ʾ������
    //�ȿ�����
    msg.hFont = CreateFont(
                    10, 5, /*Height,Width*/
                    0, 0, /*escapement,orientation*/
                    FW_REGULAR, FALSE, FALSE, FALSE, /*weight, italic, underline, strikeout*/
                    ANSI_CHARSET, OUT_DEVICE_PRECIS, CLIP_MASK, /*charset, precision, clipping*/
                    DEFAULT_QUALITY, DEFAULT_PITCH, /*quality, and pitch*/
                    "Courier"); /*font name*/
    SendDlgItemMessage(msg.hWndMain, IDC_EDIT_RECV, WM_SETFONT, (WPARAM)msg.hFont, MAKELPARAM(TRUE, 0));
    SendDlgItemMessage(msg.hWndMain, IDC_EDIT_RECV2, WM_SETFONT, (WPARAM)msg.hFont, MAKELPARAM(TRUE, 0));
    SendDlgItemMessage(msg.hWndMain, IDC_EDIT_SEND, WM_SETFONT, (WPARAM)msg.hFont, MAKELPARAM(TRUE, 0));
    //�ı���ʾ�������뷢�ͻ����С
    SendDlgItemMessage(msg.hWndMain, IDC_EDIT_RECV, EM_SETLIMITTEXT, (WPARAM)COMMON_RECV_BUF_SIZE, 0);
    SendDlgItemMessage(msg.hWndMain, IDC_EDIT_SEND, EM_SETLIMITTEXT, (WPARAM)COMMON_SEND_BUF_SIZE, 0);
    SendDlgItemMessage(msg.hWndMain, IDC_EDIT_RECV2, EM_SETLIMITTEXT, (WPARAM)COMMON_RECV_BUF_SIZE, 0);
    OldRecvEditWndProc = (WNDPROC)SetWindowLong(GetDlgItem(msg.hWndMain, IDC_EDIT_RECV), GWL_WNDPROC, (LONG)RecvEditWndProc);
    OldRecv2EditWndProc = (WNDPROC)SetWindowLong(GetDlgItem(msg.hWndMain, IDC_EDIT_RECV2), GWL_WNDPROC, (LONG)Recv2EditWndProc);
    ShowWindow(msg.hEditRecv2, TRUE);
    //TODO:
    comm.init();
    comm.update((int *) - 1);
    if(ComboBox_GetCount(GetDlgItem(hWnd, IDC_CBO_CP)) == 0)
        deal.update_status("û���κο��õĴ���!");
    deal.init_ui();
    deal.do_buf_send(0, 0);
    return 0;
}

int on_close(void)
{
    if(msg.hComPort != INVALID_HANDLE_VALUE)
    {
        int ret;
        ret = utils.msgbox(MB_ICONEXCLAMATION | MB_OKCANCEL, "��ʾ",
                           "��ǰ�����ڵ���״̬, ��ȷ��Ҫ�˳���?");
        if(ret != IDOK)
            return 0;
    }
    if(comm.fCommOpened)
        comm.close();
    deal.do_buf_send(3, 0);
    deal.do_buf_recv(NULL, 0, 3);

    if(msg.hFont) //ɾ�������ĵȿ�����
    {
        DeleteObject(SelectObject(GetDC(GetDlgItem(msg.hWndMain, IDC_EDIT_RECV)), GetStockObject(NULL_PEN)));
        msg.hFont = NULL;
    }
    DestroyWindow(msg.hWndMain);
    return 0;
}

int on_destroy(void)
{
    PostQuitMessage(0);
    return 0;
}

int on_command(HWND hWndCtrl, int id, int codeNotify)
{
    if(!hWndCtrl && !codeNotify) //Menu
    {
        switch(id)
        {
            //Menu - Other
        case MENU_OTHER_HELP:
            about.show();
            break;
        case MENU_OTHER_ASCII:
            ShowAsciiTable();
            break;
        case MENU_OTHER_CALC:
            utils.show_expr();
            break;
        case MENU_OTHER_NOTEPAD:
            ShellExecute(NULL, "open", "notepad", NULL, NULL, SW_SHOWNORMAL);
            break;
        case MENU_OTHER_DEVICEMGR:
            ShellExecute(NULL, "open", "devmgmt.msc", NULL, NULL, SW_SHOWNORMAL);
            break;
            //Menu - More Settings
        case MENU_MORE_TIMEOUTS:
            comm.show_timeouts();
            break;
        case MENU_MORE_DRIVER:
            comm.hardware_config();
            break;
        case MENU_MORE_PINCTRL:
            comm.show_pin_ctrl();
            break;
            //case MENU_OTHER_STR2HEX:ShowStr2Hex();break;
            //Menu - EditBox
        case MENU_EDIT_CHINESE:
            comm.switch_disp();
            break;
        }
        return 0;
    }
    switch(id)
    {
    case IDC_RADIO_SEND_CHAR:
    case IDC_RADIO_SEND_HEX:
    case IDC_RADIO_RECV_CHAR:
    case IDC_RADIO_RECV_HEX:
        comm.set_data_fmt();
        return 0;
    case IDC_BTN_STOPDISP:
    {
        if(comm.fShowDataReceived) //�����ֹͣ��ʾ,���뵽��ͣģʽ
        {
            SetWindowText(hWndCtrl, "������ʾ");
            comm.fShowDataReceived = 0;
        }
        else  //����˼�����ʾ, ���뵽��ʾģʽ
        {
            //deal.do_check_recv_buf();
            SetWindowText(hWndCtrl, "ֹͣ��ʾ");
            comm.fShowDataReceived = 1;
        }
        deal.update_savebtn_status();
        return 0;
    }
    case IDC_BTN_COPY_RECV:
    case IDC_BTN_COPY_SEND:
    {
        char *buffer = NULL;
        int length = 0;
        if(id == IDC_BTN_COPY_RECV && comm.fShowDataReceived && msg.hComPort != INVALID_HANDLE_VALUE)
        {
            utils.msgbox(MB_ICONEXCLAMATION, COMMON_NAME,
                         "����ʾģʽ�²������ƽ���������!\n"
                         "���� ֹͣ��ʾ �л�����ͣ��ʾģʽ.");
            return 0;
        }
        length = GetWindowTextLength(GetDlgItem(msg.hWndMain, id == IDC_BTN_COPY_RECV ? (comm.data_fmt_recv ? IDC_EDIT_RECV : IDC_EDIT_RECV2) : IDC_EDIT_SEND));
        if(length == 0)
        {
            MessageBeep(MB_ICONINFORMATION);
            utils.msgbox(MB_ICONQUESTION, COMMON_NAME, "ʲô��û��,���븴��ɶ?");
            return 0;
        }
        buffer = (char *)utils.get_mem(length + 1);
        if(buffer == NULL) return 0;

        GetDlgItemText(msg.hWndMain, id == IDC_BTN_COPY_RECV ? IDC_EDIT_RECV : IDC_EDIT_SEND, buffer, length + 1);
        if(!utils.set_clip_data(buffer))
        {
            utils.msgbox(MB_ICONHAND, NULL, "����ʧ��!");
        }
        else
        {
            utils.msgbox(MB_ICONINFORMATION, COMMON_NAME,
                         "�Ѹ��� %s�� ���ݵ�������!", id == IDC_BTN_COPY_RECV ? "����" : "����");
        }
        free(buffer);
        return 0;
    }
    case IDC_BTN_LOADFILE:
        comm.load_from_file();
        return 0;
    case IDC_BTN_SAVEFILE:
        comm.save_to_file();
        return 0;
    case IDC_BTN_HELP:
    {
        HMENU hMenu;
        POINT pt;
        hMenu = GetSubMenu(LoadMenu(msg.hInstance, MAKEINTRESOURCE(IDR_MENU_OTHER)), 0);
        GetCursorPos(&pt);
        TrackPopupMenu(hMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, msg.hWndMain, NULL);
        return 0;
    }
    case IDC_EDIT_RECV:
        if(codeNotify == EN_ERRSPACE || codeNotify == EN_MAXTEXT)
        {
            int ret;
            ret = utils.msgbox(MB_ICONEXCLAMATION | MB_YESNOCANCEL, NULL,
                               "���ջ���������, ����ս��������ݻ��Ǳ���,�ֻ���ȡ��?\n\n"
                               "��ѡ��   ��:��Ҫ�󱣴����������,����ս���������\n"
                               "��ѡ��   ��:���������ݽ��ᱻ���,���ݲ�������\n"
                               "��ѡ�� ȡ��:���������ݽ�������,�����ݽ��޷�����ʾ\n\n"
                               "���ǽ�����û������Ҫ����,�����벻Ҫ���ȡ��!");
            if(ret == IDYES)
            {
                if(comm.save_to_file())
                {
                    msg.on_command(NULL, IDC_BTN_CLR_RECV, BN_CLICKED);
                }
                else
                {
                    int a;
                    a = utils.msgbox(MB_ICONQUESTION | MB_YESNO, COMMON_NAME, "����û�б�����,Ҫ������ս��ջ�����ô?");
                    if(a == IDYES)
                    {
                        msg.on_command(NULL, IDC_BTN_CLR_RECV, BN_CLICKED);
                    }
                }
            }
            else if(ret == IDNO)
            {
                msg.on_command(NULL, IDC_BTN_CLR_RECV, BN_CLICKED);
            }
            else if(ret == IDCANCEL)
            {
                //ȡ��...
            }
        }
        return 0;
    case IDC_EDIT_SEND:
        if(codeNotify == EN_ERRSPACE || codeNotify == EN_MAXTEXT)
        {
            utils.msgbox(MB_ICONEXCLAMATION | MB_YESNO, NULL, "���ͻ�������!");
        }
        else if(codeNotify == EN_CHANGE)
        {
            if(comm.fAutoSend)
            {
                deal.cancel_auto_send(0);
                utils.msgbox(MB_ICONINFORMATION, COMMON_NAME, "���ڷ��������Ѹı�, �Զ��ط���ȡ��!");
            }
        }
        return 0;
    case IDC_BTN_SEND:
        deal.do_send();
        return 0;
	case IDC_BTN_BIT_EXEC:
		deal.do_fx_bit();
		return 0;
	case IDC_BTN_BYTE_READ:
		deal.do_fx_byte_read();
		return 0;
	case IDC_BTN_BYTE_WRITE:
		deal.do_fx_byte_write();
		return 0;
    case IDC_BTN_CLIENT_SEARCH:
        deal.do_client_search();
        return;
    case IDC_BTN_SEND2:
        deal.do_send_fx();
        return 0;
    case IDC_BTN_CLR_COUNTER:
        InterlockedExchange((long volatile *)&comm.cchSent, 0);
        InterlockedExchange((long volatile *)&comm.cchReceived, 0);
        deal.update_status(NULL);
        return 0;
    case IDC_BTN_CLR_SEND:
        deal.cancel_auto_send(0);
        SetDlgItemText(msg.hWndMain, IDC_EDIT_SEND, "");
        return 0;
    case IDC_BTN_CLR_RECV:
        SetDlgItemText(msg.hWndMain, comm.data_fmt_recv ? IDC_EDIT_RECV : IDC_EDIT_RECV2, "");
        if(comm.data_fmt_recv) InterlockedExchange((long volatile *)&comm.data_count, 0);
        return 0;
    case IDC_CHK_TOP:
    {
        int flag = IsDlgButtonChecked(msg.hWndMain, IDC_CHK_TOP);
        SetWindowPos(msg.hWndMain, flag ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        return 0;
    }
    case IDC_CHK_AUTO_SEND:
        deal.check_auto_send();
        return 0;
    case IDC_BTN_OPEN:
    {
        if(comm.fCommOpened)
        {
            comm.close();
            comm.update((int *) - 1);
        }
        else
        {
            comm.open();
        }
        deal.update_savebtn_status();
        return 0;
    }
    case IDC_BTN_MORE_SETTINGS:
    {
        POINT pt;
        HMENU hMenu;
        GetCursorPos(&pt);
        hMenu = GetSubMenu(LoadMenu(msg.hInstance, MAKEINTRESOURCE(IDR_MENU_MORE)), 0);
        TrackPopupMenu(hMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, msg.hWndMain, NULL);
        return 0;
    }
    case IDC_CBO_CP:
    {
        static RECT rc;
        if(codeNotify == CBN_DROPDOWN)
        {
            GetWindowRect(hWndCtrl, &rc);
            ShowWindow(msg.hEditRecv, FALSE);
            ShowWindow(msg.hEditRecv2, TRUE);
            ShowWindow(GetDlgItem(msg.hWndMain, IDC_STATIC_RECV), FALSE);
            SetWindowPos(hWndCtrl, 0, 0, 0, rc.right - rc.left + 300, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);
            //utils.msgbox(0,NULL,(char*)comm.update((int*)(16+ComboBox_GetCurSel(hWndCtrl))));
            return 0;
        }
        else if(codeNotify == CBN_SELENDOK || codeNotify == CBN_SELENDCANCEL)
        {
            SetWindowPos(hWndCtrl, 0, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);
            ShowWindow(comm.data_fmt_recv ? msg.hEditRecv : msg.hEditRecv2, TRUE);
            ShowWindow(GetDlgItem(msg.hWndMain, IDC_STATIC_RECV), TRUE);
            return 0;
        }
        break;
    }
    case IDC_REG_CLIENT_LIST:
    {
        switch (codeNotify) {
            case CBN_SELCHANGE:
            {
                HWND h = GetDlgItem(msg.hWndMain, IDC_EDIT_IP);
                if (h) {
                    char buf[100] = {0,};
                    HWND cb = GetDlgItem(msg.hWndMain, IDC_REG_CLIENT_LIST);
                    ComboBox_GetLBText(cb, ComboBox_GetCurSel(cb), buf);
                    SetWindowText(h, buf);
                }
                break;
            }
        }
    }
    }
    return 0;
}
/**************************************************
��  ��:on_device_change@8
��  ��:���豸�����ı��⴮���豸�ĸĶ�
��  ��:	event - �豸�¼�
		pDBH - DEV_BROADCAST_HDR*
����ֵ:��MSDN
˵  ��:
**************************************************/
int on_device_change(WPARAM event, DEV_BROADCAST_HDR *pDBH)
{
    TRACE("device change, event = %d\n", event);
    if(msg.hComPort == INVALID_HANDLE_VALUE)
    {
        if(event == DBT_DEVICEARRIVAL)
        {
            if(pDBH->dbch_devicetype == DBT_DEVTYP_PORT)
            {
                DEV_BROADCAST_PORT *pPort = (DEV_BROADCAST_PORT *)pDBH;
                char *name = &pPort->dbcp_name[0];
                if(strnicmp("COM", name, 3) == 0)
                {
                    int com_id;
                    char buff[32];
                    extern HWND hComPort;
                    _snprintf(buff, sizeof(buff), "�Ѽ�⵽�����豸 %s �Ĳ���!", name);
                    deal.update_status(buff);
                    com_id = atoi(name + 3);
                    if(comm.update((int *)&com_id))
                        ComboBox_SetCurSel(hComPort, com_id);
                    SetFocus(GetDlgItem(msg.hWndMain, IDC_EDIT_SEND));
                }
            }
        }
        else if(event == DBT_DEVICEREMOVECOMPLETE)
        {
            if(pDBH->dbch_devicetype == DBT_DEVTYP_PORT)
            {
                DEV_BROADCAST_PORT *pPort = (DEV_BROADCAST_PORT *)pDBH;
                char *name = &pPort->dbcp_name[0];
                if(strnicmp("COM", name, 3) == 0)
                {
                    char buff[32];
                    extern HWND hComPort;
                    _snprintf(buff, sizeof(buff), "�����豸 %s ���Ƴ�!", name);
                    deal.update_status(buff);
                    comm.update((HANDLE) - 1);
                    if(ComboBox_GetCount(hComPort))
                        ComboBox_SetCurSel(hComPort, 0);
                }
            }
        }
    }
    return TRUE;
}
