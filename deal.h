#ifndef __DEAL_H__
#define __DEAL_H__

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

void init_deal(void);

typedef struct _SEND_DATA
{
    DWORD data_size;
    int flag;
    unsigned char *data;
} SEND_DATA;

struct deal_s
{
    void (*do_check_recv_buf)(void);
    int (*do_buf_send)(int action, int len);
    int (*do_buf_recv)(unsigned char *chs, int cb, int action);
    void (*update_status)(char *str);
    void (*update_savebtn_status)(void);
    void (*cancel_auto_send)(int reason);
    void (*check_auto_send)(void);
    unsigned int (__stdcall *thread_read)(void *pv);
    unsigned int (__stdcall *thread_write)(void *pv);
    void (*do_send)(void);
    void (*init_ui)(void);
    void (*do_send_fx)(void);
	void (*do_fx_bit)(void);
	void (*do_fx_byte_read)(void);
	void (*do_fx_byte_write)(void);
	void (*do_client_search)(void);
    void (*start_timer)(int start);
    //....
    int last_show;
    //¼ÆÊ±Æ÷
    unsigned int conuter;
};

#ifndef __DEAL_C__
extern struct deal_s deal;
#endif

#ifdef __DEAL_C__
#undef __DEAL_C__
int do_buf_send(int action);
int do_buf_recv(unsigned char *chs, int cb, int action);
void do_check_recv_buf(void);

void update_status(char *str);
void update_savebtn_status(void);
void cancel_auto_send(int reason);
void check_auto_send(void);

unsigned int __stdcall thread_read(void *pv);
unsigned int __stdcall thread_write(void *pv);

void do_send(void);

//void add_ch(unsigned char ch);
void add_text(unsigned char *ba, int cb);

void start_timer(int start);

#endif
#endif//__DEAL_H__
