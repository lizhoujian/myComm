#include "str2hex.h"
#include "msg.h"
#include "utils.h"
#include "resource.h"

static HWND hWndStr2Hex;
static int axisx, axisy;
static HWND hSrc, hDst;


INT_PTR CALLBACK Str2HexDlgProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_SYSCOMMAND:
        if(wParam == SC_MINIMIZE)
        {
            char str[1024];
            char result[sizeof(str)+2];
            char *p = NULL;
            int len = GetWindowTextLength(hSrc);
            if(len == 0 || len > sizeof(str))
            {
                break;
            }
            GetWindowText(hSrc, str, __ARRAY_SIZE(str));
            p = utils.hex2str((unsigned char *)str, &len, 16, 0, result, __ARRAY_SIZE(result));
            if(p)
            {
                SetWindowText(hDst, p);
                if(p != result)
                {
                    utils.free_mem((void **)&p, "<str2hex dlg>");
                }
            }
            SetDlgMsgResult(hWndDlg, WM_SYSCOMMAND, 0);
            return 1;
        }
        return 0;
    case WM_CLOSE:
    {
        RECT rc;
        GetWindowRect(hWndDlg, &rc);
        axisx = rc.left;
        axisy = rc.top;
        hWndStr2Hex = NULL;
        EndDialog(hWndDlg, 0);
        return 0;
    }
    case WM_INITDIALOG:
    {
        hWndStr2Hex = hWndDlg;
        hSrc = GetDlgItem(hWndDlg, IDC_EDIT_STR2HEX_SRC);
        hDst = GetDlgItem(hWndDlg, IDC_EDIT_STR2HEX_DST);
        SetWindowPos(hWndDlg, 0, axisx, axisy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        return 0;
    }
    }
    UNREFERENCED_PARAMETER(lParam);
    return 0;
}

int ShowStr2Hex(void)
{
    if(hWndStr2Hex)
    {
        SetWindowPos(hWndStr2Hex, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        return 0;
    }
    DialogBoxParam(msg.hInstance, MAKEINTRESOURCE(IDD_DLG_STR2HEX), NULL, Str2HexDlgProc, 0);
    return 0;
}