#include "timeouts.h"
#include "msg.h"
#include "utils.h"
#include "about.h"
#include "resource.h"

/**********************************************************
�ļ�����:timeouts.c
�ļ�·��:./common/timeouts.c
����ʱ��:2013-2-27,16:44:25
�ļ�����:Ů������
�ļ�˵��:���ڳ�ʱ����
**********************************************************/

extern COMMTIMEOUTS ctimeouts;
static HWND hWndTimeouts;

INT_PTR CALLBACK TimeoutsDlgProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


INT_PTR CALLBACK TimeoutsDlgProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
    {
        char str[16];
#pragma warning(push)
#pragma warning(disable:4127)
#define _SETTEXT(timeout,id) \
do{\
	_snprintf(str,__ARRAY_SIZE(str),"%d",timeout);\
	SetDlgItemText(hWndDlg,id,str);\
}while(0)
        _SETTEXT(ctimeouts.ReadIntervalTimeout, IDC_CTO_EDIT_READ_INTERVAL);
        _SETTEXT(ctimeouts.ReadTotalTimeoutMultiplier, IDC_CTO_EDIT_READ_MULTIPLIER);
        _SETTEXT(ctimeouts.ReadTotalTimeoutConstant, IDC_CTO_EDIT_READ_CONSTANT);
        _SETTEXT(ctimeouts.WriteTotalTimeoutMultiplier, IDC_CTO_EDIT_WRITE_MULTIPLIER);
        _SETTEXT(ctimeouts.WriteTotalTimeoutConstant, IDC_CTO_EDIT_WRITE_CONSTANT);
#undef _SETTEXT
#pragma warning(pop)
        EnableMenuItem(GetSystemMenu(hWndDlg, FALSE), SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        utils.center_window(hWndDlg, msg.hWndMain);
        SetFocus(GetDlgItem(hWndDlg, IDC_CTO_BTN_CANCEL));
        //...
        hWndTimeouts = hWndDlg;
        return 0;
    }
    case WM_COMMAND:
    {
        if(HIWORD(wParam) != BN_CLICKED)
            return 0;
        switch(LOWORD(wParam))
        {
        case IDC_CTO_BTN_CANCEL:
            hWndTimeouts = NULL;
            EndDialog(hWndDlg, 0);
            return 0;
        case IDC_CTO_BTN_OK:
        {
            int value;
#pragma warning(push)
#pragma warning(disable:4127)
#define _SETVALUE(timeout,id) \
							do{\
BOOL bTranslated;\
value=GetDlgItemInt(hWndDlg,id,&bTranslated,FALSE);\
if(!bTranslated){\
		MessageBox(hWndDlg,#timeout" ��������!",COMMON_NAME,MB_ICONEXCLAMATION);\
		return 0;\
}else{\
	timeout=value;\
}\
							}while(0)
            _SETVALUE(ctimeouts.ReadIntervalTimeout, IDC_CTO_EDIT_READ_INTERVAL);
            _SETVALUE(ctimeouts.ReadTotalTimeoutMultiplier, IDC_CTO_EDIT_READ_MULTIPLIER);
            _SETVALUE(ctimeouts.ReadTotalTimeoutConstant, IDC_CTO_EDIT_READ_CONSTANT);
            _SETVALUE(ctimeouts.WriteTotalTimeoutMultiplier, IDC_CTO_EDIT_WRITE_MULTIPLIER);
            _SETVALUE(ctimeouts.WriteTotalTimeoutConstant, IDC_CTO_EDIT_WRITE_CONSTANT);
#undef _SETVALUE
#pragma warning(pop)
            if(msg.hComPort != INVALID_HANDLE_VALUE)
            {
                int ret;
                ret = MessageBox(hWndDlg,
                                 "��ǰ���д��ڱ���,��ʱ����Ҫ������Ч?\n\n"
                                 "[  ��]   ������Ч\n"
                                 "[  ��]   �´δ򿪴����豸ʱ��Ч\n"
                                 "[ȡ��]    �ص���ʱ����",
                                 "��ʱ����",
                                 MB_YESNOCANCEL | MB_ICONEXCLAMATION);
                if(ret == IDYES)
                {
                    if(msg.hComPort == INVALID_HANDLE_VALUE)
                    {
                        utils.msgbox(MB_ICONEXCLAMATION, COMMON_NAME,
                                     "�ڵȴ����̴򿪵Ĵ������Ѿ����ر�, ��һ�δ�ʱ�Ż���Ч!");
                    }
                    else
                    {
                        if(!SetCommTimeouts(msg.hComPort, &ctimeouts))
                        {
                            utils.msgerr("���ó�ʱʧ��");
                            return 0;
                        }
                    }
                }
                else if(ret == IDNO)
                {
                }
                else if(ret == IDCANCEL)
                {
                    return 0;
                }
            }
            else
            {
                MessageBox(hWndDlg, "��ʱ���ý����´δ򿪴����豸ʱ��Ч!", COMMON_NAME, MB_ICONINFORMATION);
            }
            hWndTimeouts = NULL;
            EndDialog(hWndDlg, 0);
            return 0;
        }
        case IDC_CTO_BTN_DEFAULT:
            SetDlgItemText(hWndDlg, IDC_CTO_EDIT_READ_INTERVAL, "0");
            SetDlgItemText(hWndDlg, IDC_CTO_EDIT_READ_MULTIPLIER, "1");
            SetDlgItemText(hWndDlg, IDC_CTO_EDIT_READ_CONSTANT, "0");
            SetDlgItemText(hWndDlg, IDC_CTO_EDIT_WRITE_MULTIPLIER, "1");
            SetDlgItemText(hWndDlg, IDC_CTO_EDIT_WRITE_CONSTANT, "0");
            MessageBox(hWndDlg, "�ѻ�ԭΪĬ��ֵ,��Ҫ�������� ȷ��!", COMMON_NAME, MB_ICONINFORMATION);
            return 0;
        }
    }
    }
    UNREFERENCED_PARAMETER(lParam);
    return 0;
}

int ShowTimeouts(void)
{
    /*if(msg.hComPort!=INVALID_HANDLE_VALUE){
    	utils.msgbox(MB_ICONEXCLAMATION,COMMON_NAME,"���ȹر��Ѿ��򿪵Ĵ����豸!");
    	return 0;
    }*/
    if(hWndTimeouts)
    {
        SetWindowPos(hWndTimeouts, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        return 0;
    }
    DialogBoxParam(msg.hInstance, MAKEINTRESOURCE(IDD_DLG_TIMEOUTS), NULL, TimeoutsDlgProc, 0);
    return 0;
}