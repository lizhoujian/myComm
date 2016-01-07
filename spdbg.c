//#include <vld.h>

#include "msg.h"
#include "utils.h"
#include "about.h"
#include "comm.h"
#include "deal.h"
#pragma warning(disable:4100) //unreferenced formal parameter(s)

int CALLBACK WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd)
{
	init_msg();
	init_utils();
	init_about();
	init_comm();
	init_deal();
	msg.run_app();
	MessageBeep(MB_OK);
	return 0;
}
