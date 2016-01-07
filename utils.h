#ifndef __UTILS_H__
#define __UTILS_H__
#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define __ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

void init_utils(void);

struct utils_s{
	int (*msgbox)(UINT msgicon, char* caption, char* fmt, ...);
	void (*msgerr)(char* prefix);
	char* (*get_file_name)(char* title, char* filter, int action, int* opentype);
	int (*set_clip_data)(char* str);
	int (*str2hex)(char* str, unsigned char** ppBuffer);
	char* (*hex2str)(unsigned char* hexarray, int* length, int linecch, int start, char* buf, int buf_size);
	char* (*hex2chs)(unsigned char* hexarray,int length,char* buf,int buf_size);
	void* (*get_mem)(size_t size);
	void (*free_mem)(void** ppv,char* prefix);
	void (*center_window)(HWND hWnd, HWND hWndOwner);
	void (*bubble_sort)(int* a, int size, int inc_or_dec);
	int (*show_expr)(void);
};

#ifndef __UTILS_C__
	extern struct utils_s utils;
#else
#undef __UTILS_C__
	
int msgbox(UINT msgicon, char* caption, char* fmt, ...);
void msgerr(char* prefix);
char* get_file_name(char* title, char* filter, int action, int* opentype);
int set_clip_data(char* str);
int str2hex(char* str, unsigned char** ppBuffer);
char* hex2str(unsigned char* hexarray, int* length, int linecch, int start, char* buf, int buf_size);
char* hex2chs(unsigned char* hexarray,int length,char* buf,int buf_size);
void* get_mem(size_t size);
void center_window(HWND hWnd, HWND hWndOwner);
void bubble_sort(int* a, int size, int inc_or_dec);
void free_mem(void** ppv,char* prefix);
#endif


#endif//!__UTILS_H__
