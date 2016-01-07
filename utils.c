#define __UTILS_C__
#include "utils.h"
#include "msg.h"
#include "expr.h"
#include "about.h"
#include "resource.h"

struct utils_s utils;

void init_utils(void)
{
	memset(&utils, 0, sizeof(utils));
	utils.msgbox = msgbox;
	utils.msgerr = msgerr;
	utils.get_file_name = get_file_name;
	utils.set_clip_data = set_clip_data;
	utils.str2hex = str2hex;
	utils.hex2str = hex2str;
	utils.get_mem = get_mem;
	utils.center_window = center_window;
	utils.bubble_sort = bubble_sort;
	utils.show_expr = ShowExpr;
	utils.hex2chs = hex2chs;
	utils.free_mem = free_mem;
	return;
}

/***************************************************
��  ��:msgbox
��  ��:��ʾ��Ϣ��
��  ��:
	msgicon:��Ϣ���
	caption:�Ի������
	fmt:��ʽ�ַ���
	...:���
����ֵ:
	�û�����İ�ť��Ӧ��ֵ(MessageBox)
˵  ��:
***************************************************/
int msgbox(UINT msgicon, char* caption, char* fmt, ...)
{
	va_list va;
	char smsg[1024]={0};
	va_start(va, fmt);
	_vsnprintf(smsg, sizeof(smsg), fmt, va);
	va_end(va);
	return MessageBox(msg.hWndMain, smsg, caption, msgicon);
}
/***************************************************
��  ��:msgerr
��  ��:��ʾ��prefixǰ׺��ϵͳ������Ϣ
��  ��:prefix-ǰ׺�ַ���
����ֵ:(��)
˵  ��:
***************************************************/
void msgerr(char* prefix)
{
	char* buffer = NULL;
	if(!prefix) prefix = "";
	if(FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,GetLastError(),MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),(LPSTR)&buffer,1,NULL)) 
	{
		utils.msgbox(MB_ICONHAND, NULL, "%s:%s", prefix, buffer);
		LocalFree(buffer);
	}
}

/***************************************************
��  ��:get_file_name
��  ��:��ʾ��/�����ļ��Ի���
��  ��:
	title:�Ի������
	filter:�Ի������ѡ��
	action:�򿪵Ķ���: 0-��,1-����
	opentype:0-16����,1-�ı�
����ֵ:
	NULL:û����ȷ��ѡ��
	����:ѡ����ļ����ַ���
˵  ��:��ȥ,��һ��ʼ������(2013-02-06),���ҵ��Ǹ��޷���ʾ
	OFN_EXPLORER����ԭ��,��һ��_WIN32_WINNT��,������~
***************************************************/
UINT_PTR __stdcall OFNHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);
char* get_file_name(char* title, char* filter, int action, int* opentype)
{
	OPENFILENAME ofn = {0};
	int ret;
	static char buffer[MAX_PATH];
	*buffer = 0;
	ofn.lStructSize = sizeof(ofn);
	ofn.hInstance = msg.hInstance;
	if(action == 0) ofn.Flags = OFN_ENABLESIZING|OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_ENABLEHOOK|OFN_ENABLETEMPLATE;
	else ofn.Flags = OFN_PATHMUSTEXIST|OFN_ENABLESIZING|OFN_EXPLORER|OFN_NOREADONLYRETURN|OFN_ENABLETEMPLATE|OFN_ENABLEHOOK|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;
	ofn.hwndOwner = msg.hWndMain;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = &buffer[0];
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = title;
	ofn.lpfnHook = OFNHookProc;
	ofn.lCustData = (LPARAM)opentype;
	ofn.lpTemplateName = MAKEINTRESOURCE(IDD_DLG_TEMPLATE);
	ret = action?GetSaveFileName(&ofn):GetOpenFileName(&ofn);
	return ret?&buffer[0]:NULL;
}

UINT_PTR CALLBACK OFNHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND hParent = NULL;
	static int* popentype = NULL;
	if(uiMsg == WM_NOTIFY){
		LPOFNOTIFY pofn = (LPOFNOTIFY)lParam;
		if(pofn->hdr.code == CDN_FILEOK){
			HWND hCboOpenType = GetDlgItem(hdlg, IDC_HOOK_COMBO);
			int index = ComboBox_GetCurSel(hCboOpenType);
			if(index == -1){
				MessageBox(GetParent(hdlg), "����������б���ѡ���ļ���/����ķ�ʽ!", "��/����", MB_ICONEXCLAMATION);
				//2013-01-17
				SetFocus(hCboOpenType);
				SetWindowLong(hdlg, DWL_MSGRESULT, 1);
				return 1;
			}
			*popentype = index;
			return 0;
		}
	}else if(uiMsg == WM_SIZE){
		HWND hCboCurFlt = GetDlgItem(GetParent(hdlg), cmb1);
		HWND hCboOpenType = GetDlgItem(hdlg, IDC_HOOK_COMBO);
		RECT rcCboFlt;
		GetWindowRect(hCboCurFlt, &rcCboFlt);
		SetWindowPos(hCboOpenType,HWND_NOTOPMOST,0,0,rcCboFlt.right-rcCboFlt.left,rcCboFlt.bottom-rcCboFlt.top, SWP_NOMOVE|SWP_NOZORDER);
		return 0;
	}else if(uiMsg == WM_INITDIALOG){
		HWND hCboOpenType = GetDlgItem(hdlg, IDC_HOOK_COMBO);
		ComboBox_AddString(hCboOpenType, "ʮ������, ����������ļ�");
		ComboBox_AddString(hCboOpenType, "�ı��ַ�, ����16�������е��ı��ļ�");

		popentype = (int*)((OPENFILENAME*)lParam)->lCustData;
		return 0;
	}
	UNREFERENCED_PARAMETER(wParam);
	return 0;
}

/***************************************************
��  ��:set_clip_data
��  ��:����strָ����ַ�����������
��  ��:
	str:�ַ���,��0��β
����ֵ:
	�ɹ�:����
	ʧ��:��
˵  ��:
***************************************************/
int set_clip_data(char* str)
{
	HGLOBAL hGlobalMem = NULL;
	char* pMem = NULL;
	int lenstr;

	if(str == NULL) return 1;
	if(!OpenClipboard(NULL)) return 0;

	lenstr = strlen(str)+1;//Makes it null-terminated
	hGlobalMem = GlobalAlloc(GHND, lenstr);
	if(!hGlobalMem) return 0;
	pMem = (char*)GlobalLock(hGlobalMem);
	EmptyClipboard();
	memcpy(pMem, str, lenstr);
	SetClipboardData(CF_TEXT, hGlobalMem);
	CloseClipboard();
	GlobalFree(hGlobalMem);
	return 1;
}

/**************************************************
��  ��:str2hex
��  ��:ת��16�����ַ�����16����ֵ����
��  ��:
	str:ָ�����16���Ƶ��ַ���,��ʽ:2��16����ֵ+1���ո�
	ppBuffer:ָ��char*��ָ��,��������ת����Ľ���Ļ�����
����ֵ:
	�ɹ�:���λΪ1,��31λ��ʾ�õ���16�������ĸ���
	ʧ��:���λΪ0,��31λ��ʾ�Ѿ��������ַ����ĳ���
˵  ��:�������sscanfӦ��Ҫ��Щ,�����տ�ʼд��ʱ��û���ǵ�
***************************************************/
int str2hex(char* str, unsigned char** ppBuffer)
{
	unsigned char hex;
	unsigned int count = 0;
	unsigned char* buffer = NULL;
	unsigned char* pb = NULL;
	register char* pstr = str;

	buffer=(unsigned char*)utils.get_mem(strlen(str)/3+1);
	pb = buffer;
	//�ռ����ʧ��
	if(buffer == NULL){
		return 0;
	}
	for(;;){//����ÿ���ַ����н���
		register unsigned char ch1 = pstr[0], ch2 = pstr[1];
		//��֤���ݸ�ʽΪ2��16����ֵ+1���ո�(��û�пո�,���һ��)
		if(ch1 && ch2 && (pstr[2]==0x20||!pstr[2]||pstr[2]=='\r')){
			if(isxdigit(ch1) && isxdigit(ch2)){//Ϊ��ȷ��16����ֵ
				if(isdigit(ch1)){//��1���ַ�Ϊ����
					if(isdigit(ch2)){//��2���ַ�ҲΪ����
						hex = (unsigned char)((ch1-48)*16 + (ch2-48));
					}else{//��2������Ϊ��ĸ
						ch2 = (char)toupper(ch2);
						hex = (unsigned char)((ch1-48)*16 + (ch2-55));
					}
				}else{//��1���ַ�Ϊ��ĸ
					if(isdigit(ch2)){//�ڶ����ַ�ҲΪ����
						hex = (unsigned char)((ch1-55)*16 + (ch2-48));
					}else{//�ڶ����ַ�Ϊ��ĸ
						ch2 = (unsigned char)toupper(ch2);
						hex = (unsigned char)((ch1-55)*16 + (ch2-55));
					}
				}
				//��ȷ����
				pstr += 2;
			}else{//2���ַ�����һ����ʽ����ȷ
				free(buffer);
				*ppBuffer = NULL;
				return (unsigned int)(pstr-str)&0x7FFFFFFF;
			}
		}else if(ch1){//��������16���Ƹ�ʽ
			free(buffer);
			*ppBuffer = NULL;
			return (unsigned int)(pstr-str)&0x7FFFFFFF;
		}else{//�������
			break;
		}
		//ֻ����ȷ������Ż���������,��������
		*pb++ = hex;//����ֵ
		count++;//ͳ�Ƽ���
	
		//Skip 16�������ݺ���ĵ�1���ո�
		if(*pstr == 0x20) pstr++;
		//�����س�����
		//if(*pstr == '\r') pstr += 2;
		if(*pstr=='\r') pstr++;//���ܻ���Linux�ı�(�������,just in case)
		if(*pstr=='\n') pstr++;
	}
	//���������
	*ppBuffer = buffer;
	return count|0x80000000;
}

/**************************************************
��  ��:hex2chs
��  ��:ת��16�������鵽�ַ��ַ���
��  ��:	hexarray - 16��������
		length - ����
		buf - Ĭ�ϻ���ռ�
		buf_size - Ĭ�Ͽռ��С
����ֵ:�ַ���
˵  ��:2013-03-10:���˺ܶ��޸�,�������ٶ���
2013-03-23 ����:
	��C���Ե����Ƕ�ϰ��ʹ��'\n'��Ϊ���з�,��Ҳ����ʹ��,
��ƫƫWindows�ı༭����'\r\n'��Ϊ���з�,û�а취,�Ҳ���
�������е�'\n'����'\r\n',Ч�ʱ�Ȼ���½�,�����Ҳ��ò������
\n�ĸ����� --> Ϊ�˼������軺�����Ĵ�С
**************************************************/
char* hex2chs(unsigned char* hexarray,int length,char* buf,int buf_size)
{
	char* buffer=NULL;
	int total_length;
	int line_r;			// \n�ĸ��� -> ȫ��ת��Ϊ\r\n
	//����\n�ĸ���
	do{
		int len=0;
		for(line_r=0; len<length; len++){
			if(hexarray[len]=='\n'){
				line_r++;
			}
		}
	}while((0));

	total_length = length*1 + 1 + 1 + 1 + line_r;//������\0 + 1�� x��β    +  ÿ��\n����\r\n <= line_r * 1
	if(total_length<=buf_size && buf){
		buffer = buf;
		//memset(buffer,0,buf_size);
	}else{
		buffer = (char*)utils.get_mem(total_length);
		if(!buffer) return NULL;
	}

	//memcpy(buffer,hexarray,length);
	//�����е�\n����\r\n
	do{
		unsigned char* pch=(unsigned char*)buffer;
		int itx;
		for(itx=0; itx<length; itx++){
			if(hexarray[itx]=='\n'){
				*pch++ = '\r';
				*pch++ = '\n';
			}else{
				*pch++ = hexarray[itx];
			}
		}
	}while((0));

	buffer[total_length-1] = 'x';//��֤���ݲ�ȫΪ0,����Ϊ������־
	buffer[total_length-2] = '\0';
	buffer[total_length-3] = '\0';
	return buffer;
}

/*************************************************
��  ��:hex2str
��  ��:ת��16����ֵ���鵽16�����ַ���
��  ��:
	hexarray:16��������
	*length:16�������鳤��
	linecch:ÿ�е�16���Ƶĸ���,Ϊ0��ʾ������
	start:��ʼ�ڵڼ���16��������
	buf:Ĭ�Ͽռ�,����ռ��С����,����ô˿ռ�
	buf_size:�ռ��С
����ֵ:
	�ɹ�:�ַ���ָ��(�������Ĭ�ϻ�����,��Ҫ�ֶ��ͷ�)
	ʧ��:NULL
	*length ���ط����ַ����ĳ���
˵  ��:
	2013-03-05:����, ���ڿ�����ӽ�Ƶ��,��ÿ�ε������ֺ���,
�������ڿ��Դ����û�����Ļ���������������,������ֵ==buf,
˵���û��ռ䱻ʹ��
	2013-03-10:��ǰ�ټ���һ��:*pb='\0'; 
		���½�����������ʾ����(����������ȷ),���˺þòŷ���....
**************************************************/
char* hex2str(unsigned char* hexarray, int* length, int linecch, int start, char* buf, int buf_size)
{
	char* buffer = NULL;
	char* pb = NULL;
	int count = start;
	int total_length;
	//int ret_length=0;
	int k;

	//2013-01-17���¼������:
	//	ÿ�ֽ�ռ��2��ASCII+1���ո�:length*3
	//  �����ַ�ռ��:length/linecch*2
	total_length = *length*3 + *length/linecch*2+1+2;//+1:���1��'\0';+2:�����ǵ�1��\r\n
	if(buf_size>=total_length && buf){
		buffer = buf;
		//memset(buffer,0,buf_size);
	}else{
		buffer=(char*)get_mem(total_length);
		if(buffer == NULL) return NULL;
	}
	//memset(buffer,0,total_length);
	for(k=0,pb=buffer; k<*length; k++){
		sprintf(pb, "%02X ", hexarray[k]);
		pb += 3;
		//���д���
		if(linecch && ++count == linecch){
			pb[0] = '\r';
			pb[1] = '\n';
			pb += 2;
			count = 0;
		}
	}
	//2013-03-10:��ǰ�ټ���һ��:*pb='\0'; 
	//���½�����������ʾ����(����������ȷ),���˺þòŷ���....
	*pb = '\0';
	*length = pb-buffer;
	return buffer;
}

/**************************************************
��  ��:get_mem@4
��  ��:�����ڴ�,������
��  ��:size-������Ĵ�С
����ֵ:�ڴ���ָ��
˵  ��:
**************************************************/
void* get_mem(size_t size)
{
	void* pv = malloc(size);
	if(!pv){
		utils.msgbox(MB_ICONERROR,NULL,"�ڴ�������");
		return NULL;
	}
	memset(pv,0,size);
	return pv;
}

/**************************************************
��  ��:free_mem@8
��  ��:�ͷ��ڴ�����,�쳣����
��  ��:void* pv:ָ��,char* prefix:˵��
����ֵ:
˵  ��:
**************************************************/
void free_mem(void** ppv,char* prefix)
{
	if(ppv==NULL)
		return;
	__try{
		free(*ppv);
		*ppv = NULL;
	}
	__except(EXCEPTION_EXECUTE_HANDLER){
		//*ppv=NULL;
		utils.msgbox(MB_ICONERROR,COMMON_NAME,
			"%s:ָ�뱻������ͷ�,�뱨���쳣!",prefix?prefix:"<null-function-name>");
	}
}

/**************************************************
��  ��:center_window@8
��  ��:��ָ�����ھ�����ָ������һ����
��  ��:	hWnd - �����еĴ��ھ��
		hWndOwner - �ο����ھ��
����ֵ:
˵  ��:����������Ļ,��hWndOwnerΪNULL
**************************************************/
void center_window(HWND hWnd, HWND hWndOwner)
{
	RECT rchWnd,rchWndOwner;
	int width,height;
	int x,y;

	if(!IsWindow(hWnd)) return;
	GetWindowRect(hWnd,&rchWnd);

	if(!hWndOwner||!IsWindow(hWndOwner)){
		int scrWidth,scrHeight;
		scrWidth = GetSystemMetrics(SM_CXSCREEN);
		scrHeight = GetSystemMetrics(SM_CYSCREEN);
		SetRect(&rchWndOwner,0,0,scrWidth,scrHeight);
	}else{
		GetWindowRect(hWndOwner,&rchWndOwner);
	}
	width = rchWnd.right-rchWnd.left;
	height = rchWnd.bottom-rchWnd.top;
	
	x = (rchWndOwner.right-rchWndOwner.left-width)/2+rchWndOwner.left;
	y = (rchWndOwner.bottom-rchWndOwner.top-height)/2+rchWndOwner.top;

	MoveWindow(hWnd,x,y,width,height,TRUE);
}

/*************************************************
��  ��:bubble_sort@12
��  ��:ð������
��  ��:a-ָ��,size-Ԫ�ظ���,inc_or_dec-0:����,!0:����
����ֵ:(��)
˵  ��:Ԫ�ش�СΪ4���ֽ�,����д��,д��ģ��,��֪��������û
**************************************************/
void bubble_sort(int* a, int size, int inc_or_dec)
{
	int x,y,tmp;
	if(size<1) return;
	for(x=0; x<size-1; x++){
		for(y=0; y<size-x-1; y++){
			if(inc_or_dec){
				if(a[y]>a[y+1]){
					tmp=a[y+1];
					a[y+1]=a[y];
					a[y]=tmp;
				}
			}else{
				if(a[y]<a[y+1]){
					tmp=a[y+1];
					a[y+1]=a[y];
					a[y]=tmp;
				}
			}
		}
	}
}
