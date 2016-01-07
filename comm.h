#ifndef __COMM_H__
#define __COMM_H__
#define _WIN32_WINNT 0x0501
#include <windows.h>
#pragma warning(push)
#pragma warning(disable:4201)
#include <SetupAPI.h>
#include <devguid.h>
#pragma warning(pop)
#include <stdio.h>

#ifndef CBR_74880
#define CBR_74880 74880
#endif

void init_comm(void);

#pragma warning(push)
#pragma warning(disable:4201)
struct comm_s
{
    void (*init)(void);
    int (*update)(int *which);
    int (*open)(void);
    int (*close)(void);
    int (*save_to_file)(void);
    int (*load_from_file)(void);
    void (*set_data_fmt)(void);
    int (*hardware_config)(void);
    int (*show_pin_ctrl)(void);
    int (*show_timeouts)(void);
    int (*update_config)(int only_update);
    int (*switch_disp)(void);
    //���ݳ�Ա
    struct
    {
        //����/�������ݸ�ʽ
        int data_fmt_send;	//0-�ַ�
        int data_fmt_recv;	//0-�ַ�
        //����/���ռ���
        unsigned int cchSent;		//��ʼ��0
        unsigned int cchReceived;	//��ʼ��0
        unsigned int cchNotSend;	//�ȴ����͵�������
        //�Ƿ��Զ�����
        int fAutoSend;				//��ʼ����
        //�Ƿ���ʾ��������
        int fShowDataReceived;		//��ʼ����
        //�����Ƿ��Ѿ���
        int fCommOpened;
        //�Ƿ�������ʾ����
        int fDisableChinese;
        //��д�ܵ�
        HANDLE hPipeRead;
        HANDLE hPipeWrite;

        DWORD data_count;


    };
};
#pragma warning(pop)

#define COMMON_MAX_LOAD_SIZE			((unsigned long)1<<20)
#define COMMON_LINE_CCH_SEND			16
#define COMMON_LINE_CCH_RECV			16
#define COMMON_SEND_BUF_SIZE			COMMON_MAX_LOAD_SIZE
#define COMMON_RECV_BUF_SIZE			(((unsigned long)1<<20)*10)
#define COMMON_INTERNAL_RECV_BUF_SIZE	((unsigned long)1<<20)
#define COMMON_READ_BUFFER_SIZE			((unsigned long)1<<20)

#ifndef __COMM_C__
extern struct comm_s comm;
#endif
//////////////////////////////////////////////////////////////////////////
#ifdef __COMM_C__
void init(void);
int open(void);
int get_comm_list(int *which);
int save_to_file(void);
int load_from_file(void);
void set_data_fmt(void);
int hardware_config(void);
int update_config(int only_update);
int close(void);
int switch_disp(void);
#endif

#endif//!__COMM_H__
