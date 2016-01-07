#define __DEAL_C__
#include "deal.h"
#include "msg.h"
#include "utils.h"
#include "comm.h"
#include "about.h"
#include "debug.h"
#include "resource.h"

#pragma comment(lib,"WinMM")

struct deal_s deal;

void init_deal(void)
{
	memset(&deal,0,sizeof(deal));
	deal.do_check_recv_buf = do_check_recv_buf;
	deal.do_buf_recv = do_buf_recv;
	deal.do_buf_send = do_buf_send;
	deal.update_savebtn_status = update_savebtn_status;
	deal.update_status = update_status;
	deal.thread_read = thread_read;
	deal.thread_write = thread_write;
	deal.cancel_auto_send = cancel_auto_send;
	deal.check_auto_send = check_auto_send;
	deal.do_send = do_send;
	deal.start_timer = start_timer;
	deal.last_show = 1;
}

//���� ���浽�ļ� ��ť��״̬
void update_savebtn_status(void)
{
	HWND hSave = GetDlgItem(msg.hWndMain, IDC_BTN_SAVEFILE);
	//����û�򿪻���ʾ����
	EnableWindow(hSave,msg.hComPort==INVALID_HANDLE_VALUE||!comm.fShowDataReceived);
}

//NULL:���¼���״̬
void update_status(char* str)
{
	static char status[128] = {" ״̬:"};
	static HWND hStatus;
	//TODO:
	if(!hStatus)
		hStatus = GetDlgItem(msg.hWndMain, IDC_STATIC_STATUS);
	if(str == NULL)//���¼���״̬
		sprintf(status+6, "���ռ���:%u,���ͼ���:%u,�ȴ�����:%u", comm.cchReceived, comm.cchSent,comm.cchNotSend);
	else//���״̬���
		_snprintf(status+6, sizeof(status), "%s", str);
	SetWindowText(hStatus, status);
}
/*
��  ��:do_buf_recv
��  ��:����ֹͣ��ʾ�������
��  ��:
	chs:�ַ�ָ��,���������
	cb:�ֽ���
	action:����, ������:
		0 - ��ӻ����ڴ�, �޷���ֵ
		1 - ȡ�û���������, ����ֵΪunsigned char*
		2 - ȡ�û������ĳ���, ����int
		3 - �ͷŻ������ڴ�
����ֵ:
	int - �ο�action��������
*/
int do_buf_recv(unsigned char* chs, int cb, int action)
{
	static unsigned char* str = NULL;
	static unsigned char* pstr = NULL;
	//unsigned int length = 0;
	if(str == NULL && action==0){
		//TODO:
		str = utils.get_mem(COMMON_INTERNAL_RECV_BUF_SIZE);
		if(str==NULL) return 0;
		pstr = str;
	}
	switch(action)
	{
	case 0://��ӻ����ַ�
		if(pstr-str + cb > COMMON_INTERNAL_RECV_BUF_SIZE){
			int ret;
			//TODO:ѯ���Ƿ���µ���ʾ���ټ���,����ɾ��
			ret = utils.msgbox(MB_ICONERROR|MB_YESNO, "����",
				"ֹͣ��ʾ��, ��ʾ�����ݱ����浽�˳����ڲ��Ļ�����, ��:\n"
				"�ڲ�������Ĭ�ϵ�1M�ռ�����, ���ݿ����Ѳ��ֶ�ʧ!\n\n"
				"�Ƿ�Ҫ����ڲ�������?"
			);
			if(ret == IDYES){
				free(str);
				str = NULL;
			}
			return 0;
		}
		memcpy(pstr,chs,cb);
		pstr += cb;
		//length = sprintf(pstr, chs);
		//pstr += length;
		break;
	case 1://ȡ�û���������
		return (int)str;
	case 2://ȡ�û���������
		return (int)(pstr-(unsigned int)str/*+1*/);
	case 3://�ͷŻ�����
		/*if(str){
			free(str);
			str = NULL;
		}*/
		utils.free_mem((void**)&str,"<do_buf_recv>");
		return 0;
	}
	return 0;
}

/**************************************************
��  ��:do_check_recv_buf@-
��  ��:����ֹͣ��ʾ�ָ���������ʾʱ��⻺�����Ƿ��б��������
��  ��:(none)
����ֵ:(none)
˵  ��:(none)
**************************************************/
void do_check_recv_buf(void)
{
	unsigned char* saved = (unsigned char*)do_buf_recv(NULL, 0, 1);
	if(saved != NULL){//�ڲ�������������
		int ret;
		ret = utils.msgbox(MB_ICONINFORMATION|MB_YESNO, "��ʾ",
			"��~\n\n"
			"��ֹͣ��ʾ���ݵĹ�����, δ����ʾ�����ݱ����浽�˳�����ڲ���������,\n\n"
			"��Ҫ���±���������ݵ���������?\n\n"
			"�����ѡ���˷�, �ⲿ�����ݽ������ٱ�����!");
		if(ret == IDYES){//ϣ����ʾ���������˵�����
			/*int len1 = Edit_GetTextLength(msg.hEditRecv);
			int len2 = do_buf_recv(NULL,0, 2);
			if(len1+len2 > COMMON_RECV_BUF_SIZE){//��������װ������
				int ret;
				ret = utils.msgbox(MB_ICONEXCLAMATION|MB_YESNO, COMMON_NAME,
					"�~~~\n\n"
					"�������ĳ����Ѵﵽ�������, ���ݿ��ܲ��ᱻ��������ʾ����������, Ҫ��ʾ�ضϺ��������?\n\n"
					"���ѡ���˷�, �ڲ�����Ļ��������ݻᱻ�����Ŷ~");
				if(ret == IDNO){//ȡ����ʾ�����
					do_buf_recv(NULL,0, 3);
					return;
				}else{//ѡ���˽ض���ʾ����
					
				}
			}
			Edit_SetSel(msg.hEditRecv, len1, len1);
			Edit_ReplaceSel(msg.hEditRecv, saved);*/
			add_text(saved,do_buf_recv(NULL,0,2));
			do_buf_recv(NULL, 0, 3);
		}else{//����Ҫ��ʾ���������,�����
			do_buf_recv(NULL, 0, 3);
		}
	}
}

#define SEND_DATA_SIZE 100
int do_buf_send(int action)
{
	static int insertl=0;
	static int removel=0;
	static SEND_DATA* send_data[SEND_DATA_SIZE];
	switch(action)
	{
	case 0://��ʼ��
		{
			int it;
			int len = sizeof(SEND_DATA)*SEND_DATA_SIZE;
			void* pv=utils.get_mem(len);
			if(!pv) return 0;
			for(it=0; it<SEND_DATA_SIZE; it++){
				send_data[it] = (SEND_DATA*)((unsigned char*)pv+it*sizeof(SEND_DATA));
				send_data[it]->flag = -1;
			}
			return 1;
		}
	case 1://ȡ�û�����
		{
			if(insertl==50) insertl=0;
			if(send_data[insertl]->flag!=-1)
				return 0;
			else{
				SEND_DATA* psd=send_data[insertl];
				psd->flag = 1;
				insertl++;
				return (int)psd;
			}
		}
	case 2://�黹������
		{
			send_data[removel]->flag=-1;
			if(++removel == SEND_DATA_SIZE)
				removel = 0;
			return 1;
		}
	case 3://�ͷ����л�����
		free(send_data[0]);
		return 1;
	}
	return 0;
}

/**************************************************
��  ��:add_text@8
��  ��:���baָ������ݵ���ʾ��(16���ƺ��ַ�ģʽ)
��  ��:ba - 16��������,cb - �ֽ���
����ֵ:(none)
˵  ��:
	2013-03-10:���˴����޸�, ò�����ڲ��ٶ���?�ҵ����
BUG�ɻ����Ҳ���ʱ��!!!
**************************************************/
void add_text(unsigned char* ba, int cb)
{
	//2012-03-19:���ӵ�10KB�ռ�
	static char inner_str[10240];
	if(cb==0) return;
	if(comm.fShowDataReceived){
		if(comm.data_fmt_recv){//16����
			char* str=NULL;
			DWORD len,cur_pos;
			len = comm.data_count;//Edit_GetTextLength(msg.hEditRecv);
			cur_pos = len % (COMMON_LINE_CCH_RECV*3+2);
			cur_pos = cur_pos/3;
			str = utils.hex2str(ba,&cb,COMMON_LINE_CCH_RECV,cur_pos,inner_str,__ARRAY_SIZE(inner_str));
			__try{
				Edit_SetSel(msg.hEditRecv, len, len);
				Edit_ReplaceSel(msg.hEditRecv, str);
				if(str!=inner_str) utils.free_mem((void**)&str,NULL);
			}
			__except(EXCEPTION_EXECUTE_HANDLER){
				utils.msgbox(MB_ICONERROR,COMMON_NAME,"add_text:Access Violation!");
			}
			InterlockedExchangeAdd((long volatile*)&comm.data_count,cb);
		}else{//�ַ�
			char* str=NULL;
			/*volatile */char* p = NULL;
			int len;
			if(comm.fDisableChinese){//��������ʾ���ĵĻ�,������>0x7F���ַ��ĳ�С����,ͬ��Ҳ���������ַ�
				int it;
				unsigned char uch;
				for(it=0; it<cb; it++){
					uch = ba[it];
					if(uch>0&&uch<32&&uch!='\n' || uch>0x7F){
						ba[it] = (unsigned char)'?';
					}
				}
			}
			
			str=utils.hex2chs(ba,cb,inner_str,__ARRAY_SIZE(inner_str));

			//2013-03-10 ����:str��������0��ʼ��, Ҳ���ܰ�������ַ���(��Ȼ��)
			//�ַ���֮��Ҳ��һ������ "1" ��'\0'�����
			p=str;
			//���ַ�������
			for(;;){
				__try{//�Թ�ǰ���'\0'
					while(!*p)
						p++;
				}
				__except(EXCEPTION_EXECUTE_HANDLER){
					utils.msgbox(MB_ICONERROR,COMMON_NAME,
						"utils.hex2chs:�ڴ�����쳣, �뱨�����!\n\n"
						"str=0x%08X",p);
					//���û�������쳣�Ļ�,pӦ��λ������'x'��
					p = 2+str+cb;
				}
				if(p-str-2>=cb){//ĩβΪ����0+1��x--->���ݴ������
					break;
				}
				len = Edit_GetTextLength(msg.hEditRecv2);
				Edit_SetSel(msg.hEditRecv2,len,len);
				Edit_ReplaceSel(msg.hEditRecv2,p);
				//SetDlgItemText(msg.hWndMain,IDC_EDIT_SEND,p);
				//MessageBox(NULL,p,NULL,0);
				while(*p++)//��λ����2��'\0',����hex2chsת����Ľ�β�ذ�������'\0'+һ��'x'
					;
			}
			if(str!=inner_str) utils.free_mem((void**)&str,NULL);
		}
	}else{
		do_buf_recv(ba,cb,0);
	}
}

//#pragma pack(push,1)
typedef struct _RECV_STRUCT
{
	int flag_use_last_char;
	int flag_left_one_char;
	unsigned char last_unresolved_char;
	unsigned char left_char;
	
}RECV_STRUCT;
//#pragma  pack(pop)

//�������߳�
unsigned int __stdcall thread_read(void* pv)
{
	DWORD nRead,nTotalRead=0;
	int retval;
	COMSTAT sta;
	DWORD comerr;
	int sehcode = 0;
	unsigned char* block_data=NULL;
	DWORD nBytesToRead;
	//OVERLAPPED ow = {0};
	
	
	block_data = (unsigned char*)utils.get_mem(COMMON_READ_BUFFER_SIZE+1);//+1��������ǰ��,������һ�ε������ַ�(���ֽ�)
	if(block_data == NULL){
		utils.msgbox(MB_ICONERROR,COMMON_NAME,"���̱߳��Ƚ���!");
		return 1;
	}

	__try{
		for(;msg.hComPort;)
		{
			if(!comm.fCommOpened) __leave;
			ClearCommError(msg.hComPort,&comerr,&sta);
			if(sta.cbInQue == 0){
				sta.cbInQue++;
			}
			
			nBytesToRead = sta.cbInQue;
			if(nBytesToRead>COMMON_READ_BUFFER_SIZE){
				nBytesToRead = COMMON_READ_BUFFER_SIZE-1;
			}

			for(nTotalRead=0;nTotalRead<nBytesToRead;){
				retval = ReadFile(msg.hComPort, &block_data[0]+nTotalRead, nBytesToRead-nTotalRead, &nRead, NULL);
				if(!retval)
				{
					if(!comm.fCommOpened)
						__leave;
					utils.msgerr("�����ڴ���:�ε�����֮ǰ���˹رմ���?\n\n");
					comm.close();
					comm.update((int*)-1);
					__leave;
				}
				if(nRead == 0){
					//Sleep(100);
					continue;
				}				
				nTotalRead += nRead;
			}

			if(!comm.fDisableChinese){
				DWORD it;
				int flag_need_one_more_byte;
				enum{
					CHARFMT_NULL,
					CHARFMT_ASCII,
					CHARFMT_OEMCP
				};
				int flag=CHARFMT_NULL;
				int flag_current=CHARFMT_NULL;
				for(it=0; it<nBytesToRead;it++){
					flag_current = block_data[it]<=0x7F?CHARFMT_ASCII:CHARFMT_OEMCP;
					switch(flag)
					{
					case CHARFMT_NULL:
						flag = flag_current;
						break;
					case CHARFMT_ASCII:
						if(flag_current == CHARFMT_ASCII){
							continue;
						}else if(flag_current == CHARFMT_OEMCP){
							flag = CHARFMT_OEMCP;
						}
						break;
					case CHARFMT_OEMCP:
						if(flag_current == CHARFMT_ASCII){
							block_data[it-1] = '?';
							flag = CHARFMT_ASCII;
						}else if(flag_current == CHARFMT_OEMCP){
							flag = CHARFMT_NULL;
						}
						break;
					}
				}
				//�������CHARFMT_OEMCP����
				flag_need_one_more_byte = flag_current == CHARFMT_OEMCP && flag==CHARFMT_OEMCP;

				if(flag_need_one_more_byte)
				{
					//int times=50;
					unsigned char read_byte=0;
					int ret=0;
					//��ѭ�� :-)
					while(1){
						ClearCommError(msg.hComPort,&comerr,&sta);
						if(sta.cbInQue){
							retval=ReadFile(msg.hComPort,&read_byte,1,&nRead,NULL);
							if(!retval)
							{
								if(!comm.fCommOpened){
									int len;
									char t[64];
									_snprintf(t,sizeof(t),"<���һ���ֽڿ��ܲ���ȷ:%02X>",block_data[nBytesToRead-1]);
									len = GetWindowTextLength(msg.hEditRecv2);
									Edit_SetSel(msg.hEditRecv2,len,len);
									Edit_ReplaceSel(msg.hEditRecv2,t);
									__leave;
								}
								utils.msgerr("�����ڴ���:�ε�����֮ǰ���˹رմ���?\n\n");
								comm.close();
								comm.update((int*)-1);
								__leave;
							}
							if(nRead == 0){
								continue;
							}
							ret=1;
							break;
						}
					}
					//debug_out(("��Ҫһ���ֽ�,���һ���ֽ���:%02X,ret=%d\n",block_data[nBytesToRead-1],ret));
					if(ret){
						block_data[nBytesToRead] = read_byte;
						nBytesToRead++;
					}else{
						//debug_out(("û�ж�ȡ�������ֽ�!\n"));
						int len;
						char t[64];
						_snprintf(t,sizeof(t),"<���һ���ֽڿ��ܲ���ȷ:%02X>",block_data[nBytesToRead-1]);
						len = GetWindowTextLength(msg.hEditRecv2);
						Edit_SetSel(msg.hEditRecv2,len,len);
						Edit_ReplaceSel(msg.hEditRecv2,t);
						block_data[nBytesToRead-1] = 'x';
					}
				}
			}
			InterlockedExchangeAdd((long volatile*)&comm.cchReceived, nBytesToRead);
			update_status(NULL);
	
			if(comm.fShowDataReceived){
				if(!deal.last_show){
					deal.do_check_recv_buf();
				}
			}
			deal.last_show = comm.fShowDataReceived;
			add_text(&block_data[0],nBytesToRead);
		}
		sehcode = 1;
	}
	__finally{
		if(block_data){
			utils.free_mem((void**)&block_data,"���߳�");
		}
	}
	UNREFERENCED_PARAMETER(pv);
	return 0;
}

//д�����߳�
unsigned int __stdcall thread_write(void* pv)
{
	DWORD nWritten,nRead;
	SEND_DATA* psd = NULL;
	BOOL bRet;
	for(;;){
		if(msg.hComPort!=INVALID_HANDLE_VALUE){

			if(!ReadFile(comm.hPipeRead,(void*)&psd,4,&nRead,NULL)){
				if(!comm.fCommOpened||!comm.hPipeRead)
					break;
				utils.msgerr("��ȡ�ܵ�ʱ����:");
			}
			if(nRead!=4)
				continue;
			//Լ��ָ��ֵΪ0x00000001ʱΪ�˳�(�Ƿ����ڴ�)
			if((unsigned long)psd == 0x00000001){
				//do_buf_send(2);
				break;
			}else{
				DWORD nWrittenData=0;
				//2013-03-23:δ���������Ӽ��� - �ѷŵ� do_send ��
				//InterlockedExchangeAdd((volatile unsigned long*)&comm.cchNotSend,psd->data_size);
				while(nWrittenData<psd->data_size){
					bRet = WriteFile(msg.hComPort, &psd->data[0]+nWrittenData,psd->data_size-nWrittenData, &nWritten, NULL);
					if(!comm.fCommOpened)
						break;
					if(!bRet){
						utils.msgerr("д����");
						//2013-03-23:
						break;
					}else{
						nWrittenData += nWritten;
						InterlockedExchangeAdd((volatile long *)&comm.cchSent,nWritten);//���ͼ���   - ����
						InterlockedExchangeAdd((volatile long *)&comm.cchNotSend,-(LONG)nWritten);//δ���ͼ��� - ����
						update_status(NULL);
					}
				}
				if(psd->flag==1)
					do_buf_send(2);
				else if(psd->flag==2)
					utils.free_mem((void**)&psd,"��д�������");
			}
		}else{
			break;
		}
	}
	UNREFERENCED_PARAMETER(pv);
	return 0;
}


/**************************************************
��  ��:cancel_auto_send@4
��  ��:ȡ���Զ����Ͳ���
��  ��:reason-ȡ������:0-check,1-�رմ���
����ֵ:(none)
˵  ��:���۴����Ƿ��
	2013-03-04����:���ڹرղ����Զ�ȡ���Զ�����(��)
**************************************************/
void cancel_auto_send(int reason)
{
	//if(!comm.fAutoSend&&msg.hComPort!=INVALID_HANDLE_VALUE) return;
	EnableWindow(GetDlgItem(msg.hWndMain,IDC_BTN_SEND),TRUE);
	EnableWindow(GetDlgItem(msg.hWndMain,IDC_EDIT_DELAY),TRUE);

	if(reason==1){

	}else if(reason == 0){
		CheckDlgButton(msg.hWndMain,IDC_CHK_AUTO_SEND,FALSE);
	}
	if(comm.fAutoSend){
		KillTimer(msg.hWndMain,0);
		comm.fAutoSend=0;
	}
}

/**************************************************
��  ��:check_auto_send@-
��  ��:ʹ���Զ�����ѡ��
��  ��:(none)
����ֵ:(none)
˵  ��:���۴����Ƿ��
**************************************************/
void check_auto_send(void)
{
	int flag;
	int elapse;
	BOOL fTranslated;


	flag = IsDlgButtonChecked(msg.hWndMain, IDC_CHK_AUTO_SEND);
	if(!flag){
		deal.cancel_auto_send(0);
		return;
	}
	elapse = GetDlgItemInt(msg.hWndMain,IDC_EDIT_DELAY,&fTranslated,FALSE);
	if(!fTranslated || (elapse>60000||elapse<50)){
		utils.msgbox(MB_ICONEXCLAMATION,COMMON_NAME,
			"�Զ�����ʱ�����ò���ȷ, �Զ����ͱ����!\nʱ�䷶ΧΪ50ms~60000ms");
		CheckDlgButton(msg.hWndMain,IDC_CHK_AUTO_SEND,FALSE);
		return;
	}
	
	EnableWindow(GetDlgItem(msg.hWndMain, IDC_EDIT_DELAY),FALSE);
	EnableWindow(GetDlgItem(msg.hWndMain, IDC_BTN_SEND),FALSE);
	if(msg.hComPort!=INVALID_HANDLE_VALUE){
		SetTimer(msg.hWndMain,0,elapse,NULL);
		comm.fAutoSend = 1;
	}
}


/**************************************************
��  ��:do_send@-
��  ��:���͵�����
��  ��:(none)
����ֵ:(none)
˵  ��:
**************************************************/
void do_send(void)
{
	char* buff = NULL;
	HWND hSend = NULL;
	DWORD len;
	SEND_DATA* psd = NULL;
	DWORD nWritten;
	unsigned char* bytearray = NULL;

	if(msg.hComPort==INVALID_HANDLE_VALUE)
	{
		//����һ��ԭ���Ƕ��̹߳ر��˸þ��(��Ϊ����, ���紮�ڱ��Ƴ�)
		char text[32];
		GetWindowText(GetDlgItem(msg.hWndMain, IDC_BTN_OPEN), text, sizeof(text));
		if(strcmp(text, "�رմ���") == 0){ //˵���������ڴ�������
			msg.on_command(NULL, IDC_BTN_OPEN, 0);
			return;
		}
		utils.msgbox(MB_ICONEXCLAMATION, "����", "���ȴ򿪴����豸!");
		deal.update_status("����������Ӧ����Ȼ��򿪴����ٷ���!");
		return;
	}
	
	hSend = GetDlgItem(msg.hWndMain, IDC_EDIT_SEND);
	len = GetWindowTextLength(hSend);
	if(len==0){
		deal.cancel_auto_send(0);
		deal.update_status("���������ݺ��ٷ���!");
		return;
	}
	//TODO:��ʵ����Ӧ�ò��ᷢ��?
	if(len>COMMON_MAX_LOAD_SIZE){
		utils.msgbox(MB_ICONEXCLAMATION, NULL, "�������ݹ���!");
		return;
	}
	buff = (char*)utils.get_mem(len+1);
	if(!buff) return;
	GetWindowText(hSend, buff, len+1);

	if(comm.data_fmt_send){//16���Ʒ�ʽ����
		int ret;
		int length;
		ret = utils.str2hex(buff, &bytearray);
		if(!(ret&0x80000000)){
			//TODO:
			if(comm.fAutoSend){
				deal.cancel_auto_send(0);
			}
			utils.msgbox(MB_ICONEXCLAMATION, NULL, "�����������ݽ�������, ����!\n\n�ǲ���ѡ���˷������ݵĸ�ʽ\?");
			free(buff);
			return;
		}
		//������ȷ����������
		length = ret&0x7FFFFFFF;	
		len = length;
		free(buff);
	}
	
	//TODO:
	if(len<1024){
		psd = (SEND_DATA*)deal.do_buf_send(1);
		if(!psd){
			free(comm.data_fmt_send?bytearray:(unsigned char*)buff);
			deal.cancel_auto_send(0);
			utils.msgbox(MB_ICONEXCLAMATION,"��ȵ�...","�����ٶȹ���,`��̫��������ۻ�����δ�����͵�����!\n\n"
				"����ѿ����Զ�����,���Զ����ͱ�ȡ��!");
			return;
		}
		memcpy(psd->data,comm.data_fmt_send?bytearray:(unsigned char*)buff,len);
		psd->data_size = len;
		psd->flag = 1;
	}else{
		psd = (SEND_DATA*)utils.get_mem(sizeof(SEND_DATA)+len-1024);
		if(!psd) {
			free(comm.data_fmt_send?bytearray:(unsigned char*)buff);
			return;
		}
		psd->flag = 2;
		psd->data_size = len;
		memcpy(psd->data,buff,len);
	}
	//TODO:
	WriteFile(comm.hPipeWrite,&psd,4,&nWritten,NULL);

	//2013-03-23:δ���������Ӽ���
	InterlockedExchangeAdd((volatile long*)&comm.cchNotSend,psd->data_size);
	
	return;
}

/**************************************************
��  ��:start_timer@4
��  ��:������ʱ��
��  ��:start:!0-����,0-�ر�
����ֵ:(none)
˵  ��:
**************************************************/
static void __stdcall TimeProc(
	UINT uID,      
	UINT uMsg,     
	DWORD dwUser,  
	DWORD dw1,     
	DWORD dw2      
	)
{
	unsigned char *second,*minute,*hour;
	char str[9];
	second = (unsigned char *)((unsigned long)&deal.conuter + 0);
	minute = (unsigned char *)((unsigned long)&deal.conuter + 1);
	hour   = (unsigned char *)((unsigned long)&deal.conuter + 2);
	if(++*second == 60){
		*second = 0;
		if(++*minute == 60){
			*minute = 0;
			if(++*hour == 24){
				*hour = 0;
			}
		}
	}
	sprintf(&str[0],"%02d:%02d:%02d",*hour,*minute,*second);
	SetDlgItemText(msg.hWndMain,IDC_STATIC_TIMER,str);
	UNREFERENCED_PARAMETER(uID);
	UNREFERENCED_PARAMETER(uMsg);
	UNREFERENCED_PARAMETER(dwUser);
	UNREFERENCED_PARAMETER(dw1);
	UNREFERENCED_PARAMETER(dw2);
}

void start_timer(int start)
{
	static UINT timer_id;
	if(start){
		InterlockedExchange((volatile long*)&deal.conuter,0);
		SetDlgItemText(msg.hWndMain,IDC_STATIC_TIMER,"00:00:00");
		timer_id=timeSetEvent(1000,0,TimeProc,0,TIME_PERIODIC|TIME_CALLBACK_FUNCTION);
		if(timer_id == 0){
			//...
		}
	}else{
		//SetDlgItemText(msg.hWndMain,IDC_STATIC_TIMER,"00:00:00");
		if(timer_id){
			timeKillEvent(timer_id);
			timer_id = 0;
		}
	}
}
