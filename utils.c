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
函  数:msgbox
功  能:显示消息框
参  数:
	msgicon:消息光标
	caption:对话框标题
	fmt:格式字符串
	...:变参
返回值:
	用户点击的按钮对应的值(MessageBox)
说  明:
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
函  数:msgerr
功  能:显示带prefix前缀的系统错误消息
参  数:prefix-前缀字符串
返回值:(无)
说  明:
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
函  数:get_file_name
功  能:显示打开/保存文件对话框
参  数:
	title:对话框标题
	filter:对话框过虑选项
	action:打开的动作: 0-打开,1-保存
	opentype:0-16进制,1-文本
返回值:
	NULL:没有正确地选择
	否则:选择的文件名字符串
说  明:我去,从一开始到现在(2013-02-06),才找到那个无法显示
	OFN_EXPLORER风格的原因,就一个_WIN32_WINNT啊,啊啊啊~
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
				MessageBox(GetParent(hdlg), "请在下面的列表中选择文件打开/保存的方式!", "打开/保存", MB_ICONEXCLAMATION);
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
		ComboBox_AddString(hCboOpenType, "十六进制, 任意二进制文件");
		ComboBox_AddString(hCboOpenType, "文本字符, 包含16进制序列的文本文件");

		popentype = (int*)((OPENFILENAME*)lParam)->lCustData;
		return 0;
	}
	UNREFERENCED_PARAMETER(wParam);
	return 0;
}

/***************************************************
函  数:set_clip_data
功  能:复制str指向的字符串到剪贴板
参  数:
	str:字符串,以0结尾
返回值:
	成功:非零
	失败:零
说  明:
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
函  数:str2hex
功  能:转换16进制字符串到16进制值数组
参  数:
	str:指向包含16进制的字符串,格式:2个16进制值+1个空格
	ppBuffer:指向char*的指针,用来保存转换后的结果的缓冲区
返回值:
	成功:最高位为1,低31位表示得到的16进制数的个数
	失败:最高位为0,低31位表示已经解析的字符串的长度
说  明:如果换用sscanf应该要快些,不过刚开始写的时候没考虑到
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
	//空间分配失败
	if(buffer == NULL){
		return 0;
	}
	for(;;){//遍历每个字符进行解析
		register unsigned char ch1 = pstr[0], ch2 = pstr[1];
		//保证数据格式为2个16进制值+1个空格(或没有空格,最后一个)
		if(ch1 && ch2 && (pstr[2]==0x20||!pstr[2]||pstr[2]=='\r')){
			if(isxdigit(ch1) && isxdigit(ch2)){//为正确的16进制值
				if(isdigit(ch1)){//第1个字符为数字
					if(isdigit(ch2)){//第2个字符也为数字
						hex = (unsigned char)((ch1-48)*16 + (ch2-48));
					}else{//第2个数字为字母
						ch2 = (char)toupper(ch2);
						hex = (unsigned char)((ch1-48)*16 + (ch2-55));
					}
				}else{//第1个字符为字母
					if(isdigit(ch2)){//第二个字符也为数字
						hex = (unsigned char)((ch1-55)*16 + (ch2-48));
					}else{//第二个字符为字母
						ch2 = (unsigned char)toupper(ch2);
						hex = (unsigned char)((ch1-55)*16 + (ch2-55));
					}
				}
				//正确解析
				pstr += 2;
			}else{//2个字符中有一个格式不正确
				free(buffer);
				*ppBuffer = NULL;
				return (unsigned int)(pstr-str)&0x7FFFFFFF;
			}
		}else if(ch1){//不完整的16进制格式
			free(buffer);
			*ppBuffer = NULL;
			return (unsigned int)(pstr-str)&0x7FFFFFFF;
		}else{//解析完成
			break;
		}
		//只有正确解析后才会来到这里,保存起来
		*pb++ = hex;//保存值
		count++;//统计计数
	
		//Skip 16进制内容后面的第1个空格
		if(*pstr == 0x20) pstr++;
		//遇到回车换行
		//if(*pstr == '\r') pstr += 2;
		if(*pstr=='\r') pstr++;//可能会是Linux文本(如果可能,just in case)
		if(*pstr=='\n') pstr++;
	}
	//解析已完成
	*ppBuffer = buffer;
	return count|0x80000000;
}

/**************************************************
函  数:hex2chs
功  能:转换16进制数组到字符字符串
参  数:	hexarray - 16进制数组
		length - 长度
		buf - 默认缓冲空间
		buf_size - 默认空间大小
返回值:字符串
说  明:2013-03-10:作了很多修改,大量减少丢包
2013-03-23 修正:
	用C语言的人们都习惯使用'\n'作为换行符,我也这样使用,
但偏偏Windows的编辑框以'\r\n'作为换行符,没有办法,我不得
不把所有的'\n'换成'\r\n',效率必然会下降,而且我不得不计算出
\n的个数先 --> 为了计算所需缓冲区的大小
**************************************************/
char* hex2chs(unsigned char* hexarray,int length,char* buf,int buf_size)
{
	char* buffer=NULL;
	int total_length;
	int line_r;			// \n的个数 -> 全部转换为\r\n
	//计算\n的个数
	do{
		int len=0;
		for(line_r=0; len<length; len++){
			if(hexarray[len]=='\n'){
				line_r++;
			}
		}
	}while((0));

	total_length = length*1 + 1 + 1 + 1 + line_r;//以两个\0 + 1个 x结尾    +  每个\n换成\r\n <= line_r * 1
	if(total_length<=buf_size && buf){
		buffer = buf;
		//memset(buffer,0,buf_size);
	}else{
		buffer = (char*)utils.get_mem(total_length);
		if(!buffer) return NULL;
	}

	//memcpy(buffer,hexarray,length);
	//把所有的\n换成\r\n
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

	buffer[total_length-1] = 'x';//保证数据不全为0,仅作为结束标志
	buffer[total_length-2] = '\0';
	buffer[total_length-3] = '\0';
	return buffer;
}

/*************************************************
函  数:hex2str
功  能:转换16进制值数组到16进制字符串
参  数:
	hexarray:16进制数组
	*length:16进制数组长度
	linecch:每行的16进制的个数,为0表示不换行
	start:开始于第几个16进制序列
	buf:默认空间,如果空间大小合适,则采用此空间
	buf_size:空间大小
返回值:
	成功:字符串指针(如果不是默认缓冲区,需要手动释放)
	失败:NULL
	*length 返回返回字符串的长度
说  明:
	2013-03-05:修正, 由于可能添加较频繁,但每次的数据又很少,
所以现在可以传入用户定义的缓冲来保存数据了,若返回值==buf,
说明用户空间被使用
	2013-03-10:以前少加了一句:*pb='\0'; 
		导致接收区总是显示乱码(数据量不正确),找了好久才发现....
**************************************************/
char* hex2str(unsigned char* hexarray, int* length, int linecch, int start, char* buf, int buf_size)
{
	char* buffer = NULL;
	char* pb = NULL;
	int count = start;
	int total_length;
	//int ret_length=0;
	int k;

	//2013-01-17更新计算错误:
	//	每字节占用2个ASCII+1个空格:length*3
	//  换行字符占用:length/linecch*2
	total_length = *length*3 + *length/linecch*2+1+2;//+1:最后1个'\0';+2:可能是第1个\r\n
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
		//换行处理
		if(linecch && ++count == linecch){
			pb[0] = '\r';
			pb[1] = '\n';
			pb += 2;
			count = 0;
		}
	}
	//2013-03-10:以前少加了一句:*pb='\0'; 
	//导致接收区总是显示乱码(数据量不正确),找了好久才发现....
	*pb = '\0';
	*length = pb-buffer;
	return buffer;
}

/**************************************************
函  数:get_mem@4
功  能:分配内存,并清零
参  数:size-待分配的大小
返回值:内存区指针
说  明:
**************************************************/
void* get_mem(size_t size)
{
	void* pv = malloc(size);
	if(!pv){
		utils.msgbox(MB_ICONERROR,NULL,"内存分配错误");
		return NULL;
	}
	memset(pv,0,size);
	return pv;
}

/**************************************************
函  数:free_mem@8
功  能:释放内存区域,异常处理
参  数:void* pv:指针,char* prefix:说明
返回值:
说  明:
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
			"%s:指针被错误地释放,请报告异常!",prefix?prefix:"<null-function-name>");
	}
}

/**************************************************
函  数:center_window@8
功  能:把指定窗口居中于指定的另一窗口
参  数:	hWnd - 待居中的窗口句柄
		hWndOwner - 参考窗口句柄
返回值:
说  明:若居中于屏幕,置hWndOwner为NULL
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
函  数:bubble_sort@12
功  能:冒泡排序
参  数:a-指针,size-元素个数,inc_or_dec-0:降序,!0:升序
返回值:(无)
说  明:元素大小为4个字节,粗略写的,写得模糊,不知道有问题没
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
