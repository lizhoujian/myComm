#define _WIN32_WINNT 0x0501
#include <windows.h>

#define COMMON_NAME "Com Monitor"
//#define COMMON_NAME_AND_VERSION COMMON_NAME" 1.10"
#ifdef _DEBUG 
	#define COMMON_NAME_AND_VERSION COMMON_NAME " 1.10 - Debug Mode"
#else
	#define COMMON_NAME_AND_VERSION COMMON_NAME " 1.10"
#endif

struct about_s{
	void (*show)(void);
};

void init_about(void);


#ifndef __ABOUT_C__

extern struct about_s about;

#else

#undef __ABOUT_C__

void show_about(void);
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif
