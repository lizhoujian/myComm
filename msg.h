#ifndef __MSG_H__
#define __MSG_H__
#define _WIN32_WINNT 0x0501
#include <Windows.h>
#include <WindowsX.h>
#include <Dbt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#pragma warning(push)
#pragma warning(disable:4201)
struct msg_s
{
    struct
    {
        HWND hWndMain;
        HINSTANCE hInstance;
        HANDLE hComPort;
        HWND hEditRecv;
        HWND hEditRecv2;
        HFONT hFont;
    };

    int (*run_app)(void);

    int (*on_create)(HWND hWnd, HINSTANCE hInstance);
    int (*on_close)(void);
    int (*on_destroy)(void);
    int (*on_command)(HWND hwhWndCtrl, int id, int codeNotify);
    int (*on_timer)(int id);
    int (*on_device_change)(WPARAM event, DEV_BROADCAST_HDR *pDBH);
};
#pragma warning(pop)

int init_msg(void);

#ifndef __MSG_C__
extern struct msg_s msg;
#else
#undef __MSG_C__
LRESULT CALLBACK RecvEditWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//消息处理函数声明
int run_app(void);
int on_create(HWND hWnd, HINSTANCE hInstance);
int on_close(void);
int on_destroy(void);
int on_command(HWND hwhWndCtrl, int id, int codeNotify);
int on_timer(int id);
int on_activateapp(BOOL bActivate);
int on_device_change(WPARAM event, DEV_BROADCAST_HDR *pDBH);
#endif

#endif//!__MSG_H__
