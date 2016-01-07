#include "pinctrl.h"
#include "msg.h"
#include "utils.h"
#include "about.h"
#include "resource.h"

static HWND hWndPinCtrl;
static HWND hDtr, hRts;
static int axisx, axisy;
extern COMMCONFIG cconfig;


static int dtr[3] = {DTR_CONTROL_DISABLE, DTR_CONTROL_ENABLE, DTR_CONTROL_HANDSHAKE};
static int rts[3] = {RTS_CONTROL_DISABLE, RTS_CONTROL_ENABLE, RTS_CONTROL_HANDSHAKE};
static char *sdtr[3] = {"(1)DTR_CONTROL_DISABLE", "(0)DTR_CONTROL_ENABLE", "(0)DTR_CONTROL_HANDSHAKE"};
static char *srts[3] = {"(1)RTS_CONTROL_DISABLE", "(0)RTS_CONTROL_ENABLE", "(0)RTS_CONTROL_HANDSHAKE"};

INT_PTR CALLBACK PinCtrlDlgProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_CLOSE:
    {
        RECT rc;
        GetWindowRect(hWndDlg, &rc);
        axisx = rc.left;
        axisy = rc.top;
        hWndPinCtrl = NULL;
        EndDialog(hWndDlg, 0);
        return 0;
    }
    case WM_COMMAND:
    {
        switch(LOWORD(wParam))
        {
        case IDC_PINCTRL_OK:
            if(HIWORD(wParam) != BN_CLICKED)
                return 0;
            if(msg.hComPort == INVALID_HANDLE_VALUE)
            {
                MessageBox(hWndDlg, "û�д����豸����!", COMMON_NAME, MB_ICONINFORMATION);
                return 0;
            }
            cconfig.dcb.fDtrControl = dtr[ComboBox_GetCurSel(hDtr)];
            cconfig.dcb.fRtsControl = rts[ComboBox_GetCurSel(hRts)];
            if(!SetCommConfig(msg.hComPort, &cconfig, sizeof(cconfig)))
            {
                utils.msgerr("����DTR/RTSʱ����!");
                return 0;
            }
            EnableWindow(GetDlgItem(hWndDlg, IDC_PINCTRL_OK), FALSE);
            break;
        case IDC_CBO_PINCTRL_DTR:
        case IDC_CBO_PINCTRL_RTS:
            if(HIWORD(wParam) == CBN_SELENDOK)
            {
                EnableWindow(GetDlgItem(hWndDlg, IDC_PINCTRL_OK), TRUE);
                return 0;
            }
            break;
        }
        return 0;
    }
    case WM_INITDIALOG:
    {
        DWORD size = sizeof(cconfig);
        if(msg.hComPort == INVALID_HANDLE_VALUE)
        {
            utils.msgbox(MB_ICONEXCLAMATION, COMMON_NAME, "���ȴ�һ�������豸!");
            EndDialog(hWndDlg, 0);
            return 0;
        }
        if(!GetCommConfig(msg.hComPort, &cconfig, &size))
        {
            utils.msgerr("��ȡ��������ʱ����");
            EndDialog(hWndDlg, 0);
            return 0;
        }
        hDtr = GetDlgItem(hWndDlg, IDC_CBO_PINCTRL_DTR);
        hRts = GetDlgItem(hWndDlg, IDC_CBO_PINCTRL_RTS);
        for(;;)
        {
            int i;
            for(i = 0; i < 3; i++)
            {
                ComboBox_AddString(hDtr, sdtr[i]);
                ComboBox_AddString(hRts, srts[i]);
            }
            ComboBox_SetCurSel(hDtr, cconfig.dcb.fDtrControl);
            ComboBox_SetCurSel(hRts, cconfig.dcb.fRtsControl);
            break;
        }
        //...
        hWndPinCtrl = hWndDlg;
        SetWindowPos(hWndDlg, 0, axisx, axisy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        return 0;
    }
    }
    UNREFERENCED_PARAMETER(lParam);
    return 0;
}

int ShowPinCtrl(void)
{
    if(hWndPinCtrl)
    {
        SetWindowPos(hWndPinCtrl, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        return 0;
    }
    DialogBoxParam(msg.hInstance, MAKEINTRESOURCE(IDD_DLG_PINCTRL), NULL, PinCtrlDlgProc, 0);
    return 0;
}
