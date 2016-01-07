#define __DEAL_C__
#include "deal.h"
#include "msg.h"
#include "utils.h"
#include "comm.h"
#include "about.h"
#include "debug.h"
#include "resource.h"

#pragma comment(lib,"WinMM")
#pragma comment(lib, "Kernel32")

struct deal_s deal;

void do_send_fx(void);

void init_deal(void)
{
    memset(&deal, 0, sizeof(deal));
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
    deal.do_send_fx = do_send_fx;
    deal.start_timer = start_timer;
    deal.last_show = 1;
}

//更新 保存到文件 按钮的状态
void update_savebtn_status(void)
{
    HWND hSave = GetDlgItem(msg.hWndMain, IDC_BTN_SAVEFILE);
    //串口没打开或不显示数据
    EnableWindow(hSave, msg.hComPort == INVALID_HANDLE_VALUE || !comm.fShowDataReceived);
}

//NULL:更新计数状态
void update_status(char *str)
{
    static char status[128] = {" 状态: "};
    static HWND hStatus;
    //TODO:
    if(!hStatus)
        hStatus = GetDlgItem(msg.hWndMain, IDC_STATIC_STATUS);
    if(str == NULL)//更新计数状态
        sprintf(status + 6, "已接收:%u, 已发送:%u, 未发送:%u", comm.cchReceived, comm.cchSent, comm.cchNotSend);
    else//输出状态语句
        _snprintf(status + 6, sizeof(status), "%s", str);
    SetWindowText(hStatus, status);
}

/*
函  数:do_buf_recv
功  能:缓冲停止显示后的数据
参  数:
	chs:字符指针,跟动作相关
	cb:字节数
	action:动作, 有如下:
		0 - 添加缓冲内存, 无返回值
		1 - 取得缓冲区内容, 返回值为unsigned char*
		2 - 取得缓冲区的长度, 返回int
		3 - 释放缓冲区内存
返回值:
	int - 参看action动作部分
*/
int do_buf_recv(unsigned char *chs, int cb, int action)
{
    static unsigned char *str = NULL;
    static unsigned char *pstr = NULL;
    //unsigned int length = 0;
    if(str == NULL && action == 0)
    {
        //TODO:
        str = utils.get_mem(COMMON_INTERNAL_RECV_BUF_SIZE);
        if(str == NULL) return 0;
        pstr = str;
    }
    switch(action)
    {
    case 0://添加缓冲字符
        if(pstr - str + cb > COMMON_INTERNAL_RECV_BUF_SIZE)
        {
            int ret;
            //TODO:询问是否更新到显示后再继续,不必删除
            ret = utils.msgbox(MB_ICONERROR | MB_YESNO, "警告",
                               "停止显示后, 显示的数据被保存到了程序内部的缓冲区, 但:\n"
                               "内部缓冲区默认的1M空间已满, 数据可能已部分丢失!\n\n"
                               "是否要清空内部缓冲区?"
                              );
            if (ret == IDYES)
            {
                free(str);
                str = NULL;
            }
            return 0;
        }
        memcpy(pstr, chs, cb);
        pstr += cb;
        //length = sprintf(pstr, chs);
        //pstr += length;
        break;
    case 1://取得缓冲区内容
        return (int)str;
    case 2://取得缓冲区长度
        return (int)(pstr - (unsigned int)str/*+1*/);
    case 3://释放缓冲区
        /*if(str){
        	free(str);
        	str = NULL;
        }*/
        utils.free_mem((void **)&str, "<do_buf_recv>");
        return 0;
    }
    return 0;
}

/**************************************************
函  数:do_check_recv_buf@-
功  能:在由停止显示恢复到继续显示时检测缓冲区是否有保存的数据
参  数:(none)
返回值:(none)
说  明:(none)
**************************************************/
void do_check_recv_buf(void)
{
    unsigned char *saved = (unsigned char *)do_buf_recv(NULL, 0, 1);
    if(saved != NULL) //内部缓冲区有数据
    {
        int ret;
        ret = utils.msgbox(MB_ICONINFORMATION | MB_YESNO, "提示",
                           "嗯~\n\n"
                           "在停止显示数据的过程中, 未被显示的数据被保存到了程序的内部缓冲区中,\n\n"
                           "需要更新被保存的数据到接收区吗?\n\n"
                           "如果您选择了否, 这部分数据将不会再被保存!");
        if(ret == IDYES) //希望显示保存起来了的数据
        {
            /*int len1 = Edit_GetTextLength(msg.hEditRecv);
            int len2 = do_buf_recv(NULL,0, 2);
            if(len1+len2 > COMMON_RECV_BUF_SIZE){//但缓冲区装不下了
            	int ret;
            	ret = utils.msgbox(MB_ICONEXCLAMATION|MB_YESNO, COMMON_NAME,
            		"喔~~~\n\n"
            		"缓冲区的长度已达到最大限制, 数据可能不会被完整地显示到接收区中, 要显示截断后的内容吗?\n\n"
            		"如果选择了否, 内部保存的缓冲区数据会被清除的哦~");
            	if(ret == IDNO){//取消显示并清空
            		do_buf_recv(NULL,0, 3);
            		return;
            	}else{//选择了截断显示数据

            	}
            }
            Edit_SetSel(msg.hEditRecv, len1, len1);
            Edit_ReplaceSel(msg.hEditRecv, saved);*/
            add_text(saved, do_buf_recv(NULL, 0, 2));
            do_buf_recv(NULL, 0, 3);
        }
        else  //不需要显示保存的数据,则清空
        {
            do_buf_recv(NULL, 0, 3);
        }
    }
}

#define SEND_DATA_SIZE 100
int do_buf_send(int action, int len)
{
#if 0
    static int insertl = 0;
    static int removel = 0;
    static SEND_DATA *send_data[SEND_DATA_SIZE];
    switch(action)
    {
    case 0://初始化
    {
        int it;
        int len = sizeof(SEND_DATA) * SEND_DATA_SIZE;
        void *pv = utils.get_mem(len);
        if(!pv) return 0;
        for (it = 0; it < SEND_DATA_SIZE; it++)
        {
            send_data[it] = (SEND_DATA *)((unsigned char *)pv + it * sizeof(SEND_DATA));
            send_data[it]->flag = -1;
        }
        return 1;
    }
    case 1://取得缓冲区
    {
        if(insertl == SEND_DATA_SIZE) insertl = 0;
        if(send_data[insertl]->flag != -1)
            return 0;
        else
        {
            SEND_DATA *psd = send_data[insertl];
            send_data[insertl]->data = utils.get_mem(len);
            if (send_data[insertl]->data){
                psd->flag = 1;
                insertl++;
            }
            return (int)psd;
        }
    }
    case 2://归还缓冲区
    {
        if (send_data[removel]->flag != -1) {
            if (send_data[removel]->data) {
                utils.free_mem(&send_data[removel]->data, "<do_buf_send>");
                send_data[removel]->data = NULL;
            }
        }
        send_data[removel]->flag = -1;
        if(++removel == SEND_DATA_SIZE)
            removel = 0;
        return 1;
    }
    case 3://释放所有缓冲区
        free(send_data[0]);
        return 1;
    }
#else
    if (action == 1) {
        SEND_DATA *p = (SEND_DATA*)utils.get_mem(sizeof(SEND_DATA) + len + 1);
        if (p) {
            memset(p, 0, sizeof(SEND_DATA) + len + 1);
            p->data = (unsigned char*)p + sizeof(SEND_DATA);
            return (int)p;
        }
    }
#endif
    return 0;
}

/**************************************************
函  数:add_text@8
功  能:添加ba指向的数据到显示区(16进制和字符模式)
参  数:ba - 16进制数据,cb - 字节数
返回值:(none)
说  明:
	2013-03-10:作了大量修改, 貌似现在不再丢包?找到这个
BUG可花了我不少时间!!!
**************************************************/
void add_text(unsigned char *ba, int cb)
{
    //2012-03-19:增加到10KB空间
    static char inner_str[10240];
    if(cb == 0) return;
    if(comm.fShowDataReceived)
    {
        if(comm.data_fmt_recv) //16进制
        {
            char *str = NULL;
            DWORD len, cur_pos;
            len = comm.data_count;//Edit_GetTextLength(msg.hEditRecv);
            cur_pos = len % (COMMON_LINE_CCH_RECV * 3 + 2);
            cur_pos = cur_pos / 3;
            str = utils.hex2str(ba, &cb, COMMON_LINE_CCH_RECV, cur_pos, inner_str, __ARRAY_SIZE(inner_str));
            __try
            {
                Edit_SetSel(msg.hEditRecv, len, len);
                Edit_ReplaceSel(msg.hEditRecv, str);
                if(str != inner_str) utils.free_mem((void **)&str, NULL);
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                utils.msgbox(MB_ICONERROR, COMMON_NAME, "add_text:Access Violation!");
            }
            InterlockedExchangeAdd((long volatile *)&comm.data_count, cb);
        }
        else  //字符
        {
            char *str = NULL;
            /*volatile */
            char *p = NULL;
            int len;
            if(comm.fDisableChinese) //不允许显示中文的话,把所有>0x7F的字符改成小数点,同样也处理特殊字符
            {
                int it;
                unsigned char uch;
                for(it = 0; it < cb; it++)
                {
                    uch = ba[it];
                    if(uch > 0 && uch < 32 && uch != '\n' || uch > 0x7F)
                    {
                        ba[it] = (unsigned char)'?';
                    }
                }
            }

            str = utils.hex2chs(ba, cb, inner_str, __ARRAY_SIZE(inner_str));

            //2013-03-10 修正:str可能是以0开始的, 也可能包含多个字符串(必然的)
            //字符串之间也不一定是以 "1" 个'\0'间隔的
            p = str;
            //多字符串处理
            for(;;)
            {
                __try //略过前面的'\0'
                {
                    while(!*p)
                        p++;
                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    utils.msgbox(MB_ICONERROR, COMMON_NAME,
                                 "utils.hex2chs:内存访问异常, 请报告错误!\n\n"
                                 "str=0x%08X", p);
                    //如果没有遇到异常的话,p应该位于最后的'x'处
                    p = 2 + str + cb;
                }
                if(p - str - 2 >= cb) //末尾为两个0+1个x--->数据处理完毕
                {
                    break;
                }
                len = Edit_GetTextLength(msg.hEditRecv2);
                Edit_SetSel(msg.hEditRecv2, len, len);
                Edit_ReplaceSel(msg.hEditRecv2, p);
                //SetDlgItemText(msg.hWndMain,IDC_EDIT_SEND,p);
                //MessageBox(NULL,p,NULL,0);
                while(*p++)//定位到第2个'\0',所有hex2chs转换后的结尾必包含两个'\0'+一个'x'
                    ;
            }
            if(str != inner_str) utils.free_mem((void **)&str, NULL);
        }
    }
    else
    {
        do_buf_recv(ba, cb, 0);
    }
}

//#pragma pack(push,1)
typedef struct _RECV_STRUCT
{
    int flag_use_last_char;
    int flag_left_one_char;
    unsigned char last_unresolved_char;
    unsigned char left_char;

} RECV_STRUCT;
//#pragma  pack(pop)

static void TRACE(const char * sz, ...)
{
    char szData[512]={0};

    va_list args;
    va_start(args, sz);
    _vsnprintf(szData, sizeof(szData) - 1, sz, args);
    va_end(args);

    OutputDebugString(szData);
}

#include "user_fx.h"
static void recv_to_fx(unsigned char *data, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        uart_on_recv_char(data[i]);
    }
}

static void send_to_fx(unsigned char *data, int len)
{
    int nWritten = 0;
    SEND_DATA *psd = NULL;

    while (psd == NULL) {
        psd = (SEND_DATA *)deal.do_buf_send(1, len);
    }
    memcpy(psd->data, data, len);
    psd->data_size = len;
    psd->flag = 1;
    WriteFile(comm.hPipeWrite, &psd, 4, &nWritten, NULL);
    InterlockedExchangeAdd((volatile long *)&comm.cchNotSend, psd->data_size);
    //TRACE("send_to_fx request %d, sent %d bytes.\n", len, nWritten);
}

static void appendSent(unsigned char *s)
{
    int len;
    len = Edit_GetTextLength(msg.hEditRecv);
    Edit_SetSel(msg.hEditRecv, len, len);
    Edit_ReplaceSel(msg.hEditRecv, s);
}

static void appendSentHex(unsigned char *s, int len)
{
    int i;
    unsigned char *r = malloc(len * 3 + 1);
    if (r) {
        memset(r, 0, len * 3 + 1);
        for (i = 0; i < len; i++) {
            sprintf(r + i * 3, "%02x ", s[i]);
        }
        appendSent(r);
        free(r);
    }
}

static void appendSentLR(void)
{
    appendSent("\r\n");
}

static void fx_send_string(unsigned char *data, unsigned short len)
{
    appendSentHex(data, len);
    send_to_fx(data, len);
}

static void fxSend(unsigned char c)
{
    appendSentHex(&c, 1);
    send_to_fx(&c, 1);
}

static void write_read_test1(void)
{
    int data_len = 2;
    int i;
    unsigned char bytes[20] = {0x35, 0x84};

    fx_write(REG_Y, 0, bytes, data_len);
    appendSentLR();
    fx_read(REG_Y, 0, bytes, data_len);
    TRACE("read bytes is ");
    for (i = 0; i < data_len; i++) {
        TRACE("%02x ", bytes[i]);
    }
    TRACE("\n");
    appendSentLR();
}

static void write_read_test2(void)
{
    int i;
    int data_len = 4;
    unsigned char bytes[20] = {0x34, 0x12, 0xcd, 0xab};

    fx_write(REG_D, 123, bytes, data_len);
    appendSentLR();
    fx_read(REG_D, 123, bytes, data_len);
    TRACE("read bytes is ");
    for (i = 0; i < data_len; i++) {
        TRACE("%02x ", bytes[i]);
    }
    TRACE("\n");
    appendSentLR();
}

static void test_enquiry(void)
{
    fx_enquiry();
    appendSentLR();
}

static void test_onoff1(void)
{
    fx_force_on(REG_Y, 0);
    appendSentLR();
    fx_force_off(REG_Y, 0);
    appendSentLR();
}

static void test_onoff2(void)
{
    fx_force_on(REG_Y, 19); // Y23 - 8进制
    appendSentLR();
    fx_force_off(REG_Y, 19);
    appendSentLR();
}

static unsigned int __stdcall fx_send_test(void* p)
{
    uart_set_tx_cb(fxSend);
    uart_set_tx_string_cb(fx_send_string);

    test_enquiry();

    test_onoff1();
    test_onoff2();

    write_read_test1();
    write_read_test2();

    return 0;
}

//读操作线程
unsigned int __stdcall thread_read(void *pv)
{
    DWORD nRead, nTotalRead = 0;
    int retval;
    COMSTAT sta;
    DWORD comerr;
    int sehcode = 0;
    unsigned char *block_data = NULL;
    DWORD nBytesToRead;
    //OVERLAPPED ow = {0};


    block_data = (unsigned char *)utils.get_mem(COMMON_READ_BUFFER_SIZE + 1); //+1用来放在前面,保存上一次的中文字符(单字节)
    if(block_data == NULL)
    {
        utils.msgbox(MB_ICONERROR, COMMON_NAME, "读线程被迫结束!");
        return 1;
    }

    __try
    {
        for(; msg.hComPort;)
        {
            if(!comm.fCommOpened) __leave;
            ClearCommError(msg.hComPort, &comerr, &sta);
            if(sta.cbInQue == 0)
            {
                sta.cbInQue++;
            }

            nBytesToRead = sta.cbInQue;
            if(nBytesToRead > COMMON_READ_BUFFER_SIZE)
            {
                nBytesToRead = COMMON_READ_BUFFER_SIZE - 1;
            }

            for (nTotalRead = 0; nTotalRead < nBytesToRead;)
            {
                retval = ReadFile(msg.hComPort, &block_data[0] + nTotalRead, nBytesToRead - nTotalRead, &nRead, NULL);
                if (!retval)
                {
                    if(!comm.fCommOpened)
                        __leave;
                    utils.msgerr("读串口错误:拔掉串口之前忘了关闭串口?\n\n");
                    comm.close();
                    comm.update((int *) - 1);
                    __leave;
                }
                if(nRead == 0)
                {
                    //Sleep(100);
                    continue;
                }
                recv_to_fx(&block_data[0] + nTotalRead, nRead);
                nTotalRead += nRead;
            }

            if(!comm.fDisableChinese)
            {
                DWORD it;
                int flag_need_one_more_byte;
                enum
                {
                    CHARFMT_NULL,
                    CHARFMT_ASCII,
                    CHARFMT_OEMCP
                };
                int flag = CHARFMT_NULL;
                int flag_current = CHARFMT_NULL;
                for(it = 0; it < nBytesToRead; it++)
                {
                    flag_current = block_data[it] <= 0x7F ? CHARFMT_ASCII : CHARFMT_OEMCP;
                    switch(flag)
                    {
                    case CHARFMT_NULL:
                        flag = flag_current;
                        break;
                    case CHARFMT_ASCII:
                        if(flag_current == CHARFMT_ASCII)
                        {
                            continue;
                        }
                        else if(flag_current == CHARFMT_OEMCP)
                        {
                            flag = CHARFMT_OEMCP;
                        }
                        break;
                    case CHARFMT_OEMCP:
                        if(flag_current == CHARFMT_ASCII)
                        {
                            block_data[it-1] = '?';
                            flag = CHARFMT_ASCII;
                        }
                        else if(flag_current == CHARFMT_OEMCP)
                        {
                            flag = CHARFMT_NULL;
                        }
                        break;
                    }
                }
                //若最后以CHARFMT_OEMCP结束
                flag_need_one_more_byte = flag_current == CHARFMT_OEMCP && flag == CHARFMT_OEMCP;

                if (flag_need_one_more_byte)
                {
                    //int times=50;
                    unsigned char read_byte = 0;
                    int ret = 0;
                    //死循环 :-)
                    while(1)
                    {
                        ClearCommError(msg.hComPort, &comerr, &sta);
                        if (sta.cbInQue)
                        {
                            retval = ReadFile(msg.hComPort, &read_byte, 1, &nRead, NULL);
                            if (!retval)
                            {
                                if (!comm.fCommOpened)
                                {
                                    int len;
                                    char t[64];
                                    _snprintf(t, sizeof(t), "<最后一个字节可能不正确:%02X>", block_data[nBytesToRead-1]);
                                    len = GetWindowTextLength(msg.hEditRecv2);
                                    Edit_SetSel(msg.hEditRecv2, len, len);
                                    Edit_ReplaceSel(msg.hEditRecv2, t);
                                    __leave;
                                }
                                utils.msgerr("读串口错误:拔掉串口之前忘了关闭串口?\n\n");
                                comm.close();
                                comm.update((int *) - 1);
                                __leave;
                            }
                            if(nRead == 0)
                            {
                                continue;
                            }
                            ret = 1;
                            break;
                        }
                    }
                    //debug_out(("需要一个字节,最后一个字节是:%02X,ret=%d\n",block_data[nBytesToRead-1],ret));
                    if(ret)
                    {
                        block_data[nBytesToRead] = read_byte;
                        nBytesToRead++;
                    }
                    else
                    {
                        //debug_out(("没有读取到所需字节!\n"));
                        int len;
                        char t[64];
                        _snprintf(t, sizeof(t), "<最后一个字节可能不正确:%02X>", block_data[nBytesToRead-1]);
                        len = GetWindowTextLength(msg.hEditRecv2);
                        Edit_SetSel(msg.hEditRecv2, len, len);
                        Edit_ReplaceSel(msg.hEditRecv2, t);
                        block_data[nBytesToRead-1] = 'x';
                    }
                }
            }
            InterlockedExchangeAdd((long volatile *)&comm.cchReceived, nBytesToRead);
            update_status(NULL);

            if(comm.fShowDataReceived)
            {
                if(!deal.last_show)
                {
                    deal.do_check_recv_buf();
                }
            }
            deal.last_show = comm.fShowDataReceived;
            add_text(&block_data[0], nBytesToRead);
        }
        sehcode = 1;
    }
    __finally
    {
        if(block_data)
        {
            utils.free_mem((void **)&block_data, "读线程");
        }
    }
    UNREFERENCED_PARAMETER(pv);
    return 0;
}

//写操作线程
unsigned int __stdcall thread_write(void *pv)
{
    DWORD nWritten, nRead;
    SEND_DATA *psd = NULL;
    BOOL bRet;
    for(;;)
    {
        if(msg.hComPort != INVALID_HANDLE_VALUE)
        {
            if(!ReadFile(comm.hPipeRead, (void *)&psd, 4, &nRead, NULL))
            {
                if(!comm.fCommOpened || !comm.hPipeRead)
                    break;
                utils.msgerr("读取管道时错误:");
            }
            if(nRead != 4)
                continue;
            //约定指针值为0x00000001时为退出(非分配内存)
            if((unsigned long)psd == 0x00000001)
            {
                //do_buf_send(2, 0);
                break;
            }
            else
            {
                DWORD nWrittenData = 0;
                //2013-03-23:未被发送增加计数 - 已放到 do_send 中
                //InterlockedExchangeAdd((volatile unsigned long*)&comm.cchNotSend,psd->data_size);
                while(nWrittenData < psd->data_size)
                {
                    bRet = WriteFile(msg.hComPort, &psd->data[0] + nWrittenData, psd->data_size - nWrittenData, &nWritten, NULL);
                    if(!comm.fCommOpened)
                        break;
                    if(!bRet)
                    {
                        utils.msgerr("写错误");
                        //2013-03-23:
                        break;
                    }
                    else
                    {
                        nWrittenData += nWritten;
                        InterlockedExchangeAdd((volatile long *)&comm.cchSent, nWritten); //发送计数   - 增加
                        InterlockedExchangeAdd((volatile long *)&comm.cchNotSend, -(LONG)nWritten); //未发送计数 - 减少
                        update_status(NULL);
                    }
                }
                if(psd->flag == 1) {
                    //do_buf_send(2, 0);
                    //utils.free_mem(&psd->data, "<thread_write>");
                    utils.free_mem(&psd, "<thread_write>");
                } else if(psd->flag == 2) {
                    utils.free_mem((void **)&psd, "被写完的数据");
                }
            }
        }
        else
        {
            break;
        }
    }
    UNREFERENCED_PARAMETER(pv);
    return 0;
}


/**************************************************
函  数:cancel_auto_send@4
功  能:取消自动发送操作
参  数:reason-取消理由:0-check,1-关闭串口
返回值:(none)
说  明:无论串口是否打开
	2013-03-04更新:串口关闭不再自动取消自动发送(打钩)
**************************************************/
void cancel_auto_send(int reason)
{
    //if(!comm.fAutoSend&&msg.hComPort!=INVALID_HANDLE_VALUE) return;
    EnableWindow(GetDlgItem(msg.hWndMain, IDC_BTN_SEND), TRUE);
    EnableWindow(GetDlgItem(msg.hWndMain, IDC_EDIT_DELAY), TRUE);

    if(reason == 1)
    {

    }
    else if(reason == 0)
    {
        CheckDlgButton(msg.hWndMain, IDC_CHK_AUTO_SEND, FALSE);
    }
    if(comm.fAutoSend)
    {
        KillTimer(msg.hWndMain, 0);
        comm.fAutoSend = 0;
    }
}

/**************************************************
函  数:check_auto_send@-
功  能:使能自动发送选项
参  数:(none)
返回值:(none)
说  明:无论串口是否打开
**************************************************/
void check_auto_send(void)
{
    int flag;
    int elapse;
    BOOL fTranslated;


    flag = IsDlgButtonChecked(msg.hWndMain, IDC_CHK_AUTO_SEND);
    if(!flag)
    {
        deal.cancel_auto_send(0);
        return;
    }
    elapse = GetDlgItemInt(msg.hWndMain, IDC_EDIT_DELAY, &fTranslated, FALSE);
    if(!fTranslated || (elapse > 60000 || elapse < 50))
    {
        utils.msgbox(MB_ICONEXCLAMATION, COMMON_NAME,
                     "自动发送时间设置不正确, 自动发送被否决!\n时间范围为50ms~60000ms");
        CheckDlgButton(msg.hWndMain, IDC_CHK_AUTO_SEND, FALSE);
        return;
    }

    EnableWindow(GetDlgItem(msg.hWndMain, IDC_EDIT_DELAY), FALSE);
    EnableWindow(GetDlgItem(msg.hWndMain, IDC_BTN_SEND), FALSE);
    if(msg.hComPort != INVALID_HANDLE_VALUE)
    {
        SetTimer(msg.hWndMain, 0, elapse, NULL);
        comm.fAutoSend = 1;
    }
}

void do_send_fx(void)
{
    if (msg.hComPort == INVALID_HANDLE_VALUE)
    {
        //另有一个原因是读线程关闭了该句柄(因为错误, 比如串口被移除)
        char text[32];
        GetWindowText(GetDlgItem(msg.hWndMain, IDC_BTN_OPEN), text, sizeof(text));
        if(strcmp(text, "关闭串口") == 0)  //说明正是由于错误所致
        {
            msg.on_command(NULL, IDC_BTN_OPEN, 0);
            return;
        }
        utils.msgbox(MB_ICONEXCLAMATION, "警告", "请先打开串口设备!");
        deal.update_status("请先设置相应参数然后打开串口再发送!");
        return;
    }
    _beginthreadex(NULL, 0, fx_send_test, 0, 0, NULL);
}

/**************************************************
函  数:do_send@-
功  能:发送到串口
参  数:(none)
返回值:(none)
说  明:
**************************************************/
void do_send(void)
{
    char *buff = NULL;
    HWND hSend = NULL;
    DWORD len;
    SEND_DATA *psd = NULL;
    DWORD nWritten;
    unsigned char *bytearray = NULL;

    if(msg.hComPort == INVALID_HANDLE_VALUE)
    {
        //另有一个原因是读线程关闭了该句柄(因为错误, 比如串口被移除)
        char text[32];
        GetWindowText(GetDlgItem(msg.hWndMain, IDC_BTN_OPEN), text, sizeof(text));
        if(strcmp(text, "关闭串口") == 0)  //说明正是由于错误所致
        {
            msg.on_command(NULL, IDC_BTN_OPEN, 0);
            return;
        }
        utils.msgbox(MB_ICONEXCLAMATION, "警告", "请先打开串口设备!");
        deal.update_status("请先设置相应参数然后打开串口再发送!");
        return;
    }

    hSend = GetDlgItem(msg.hWndMain, IDC_EDIT_SEND);
    len = GetWindowTextLength(hSend);
    if(len == 0)
    {
        deal.cancel_auto_send(0);
        deal.update_status("请输入内容后再发送!");
        return;
    }
    //TODO:其实这里应该不会发生?
    if(len > COMMON_MAX_LOAD_SIZE)
    {
        utils.msgbox(MB_ICONEXCLAMATION, NULL, "发送内容过多!");
        return;
    }
    buff = (char *)utils.get_mem(len + 1);
    if(!buff) return;
    GetWindowText(hSend, buff, len + 1);

    if(comm.data_fmt_send) //16进制方式发送
    {
        int ret;
        int length;
        ret = utils.str2hex(buff, &bytearray);
        if(!(ret & 0x80000000))
        {
            //TODO:
            if(comm.fAutoSend)
            {
                deal.cancel_auto_send(0);
            }
            utils.msgbox(MB_ICONEXCLAMATION, NULL, "发送区的数据解析错误, 请检查!\n\n是不是选错了发送数据的格式\?");
            free(buff);
            return;
        }
        //解析正确后来到这里
        length = ret & 0x7FFFFFFF;
        len = length;
        free(buff);
    }

    //TODO:
    if(len < 1024)
    {
        psd = (SEND_DATA *)deal.do_buf_send(1, len);
        if(!psd)
        {
            free(comm.data_fmt_send ? bytearray : (unsigned char *)buff);
            deal.cancel_auto_send(0);
            utils.msgbox(MB_ICONEXCLAMATION, "请等等...", "发送速度过快,`有太多的数据累积起来未被发送到串口!\n\n"
                         "如果已开启自动发送,则自动发送被取消!");
            return;
        }
        memcpy(psd->data, comm.data_fmt_send ? bytearray : (unsigned char *)buff, len);
        psd->data_size = len;
        psd->flag = 1;
    }
    else
    {
        psd = (SEND_DATA *)utils.get_mem(sizeof(SEND_DATA) + len - 1024);
        if(!psd)
        {
            free(comm.data_fmt_send ? bytearray : (unsigned char *)buff);
            return;
        }
        psd->flag = 2;
        psd->data_size = len;
        memcpy(psd->data, buff, len);
    }
    //TODO:
    WriteFile(comm.hPipeWrite, &psd, 4, &nWritten, NULL);

    //2013-03-23:未被发送增加计数
    InterlockedExchangeAdd((volatile long *)&comm.cchNotSend, psd->data_size);

    return;
}

/**************************************************
函  数:start_timer@4
功  能:开启计时器
参  数:start:!0-开启,0-关闭
返回值:(none)
说  明:
**************************************************/
static void __stdcall TimeProc(
    UINT uID,
    UINT uMsg,
    DWORD dwUser,
    DWORD dw1,
    DWORD dw2
)
{
    unsigned char *second, *minute, *hour;
    char str[9];
    second = (unsigned char *)((unsigned long)&deal.conuter + 0);
    minute = (unsigned char *)((unsigned long)&deal.conuter + 1);
    hour   = (unsigned char *)((unsigned long)&deal.conuter + 2);
    if(++*second == 60)
    {
        *second = 0;
        if(++*minute == 60)
        {
            *minute = 0;
            if(++*hour == 24)
            {
                *hour = 0;
            }
        }
    }
    sprintf(&str[0], "%02d:%02d:%02d", *hour, *minute, *second);
    SetDlgItemText(msg.hWndMain, IDC_STATIC_TIMER, str);
    UNREFERENCED_PARAMETER(uID);
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(dwUser);
    UNREFERENCED_PARAMETER(dw1);
    UNREFERENCED_PARAMETER(dw2);
}

void start_timer(int start)
{
    static UINT timer_id;
    if(start)
    {
        InterlockedExchange((volatile long *)&deal.conuter, 0);
        SetDlgItemText(msg.hWndMain, IDC_STATIC_TIMER, "00:00:00");
        timer_id = timeSetEvent(1000, 0, TimeProc, 0, TIME_PERIODIC | TIME_CALLBACK_FUNCTION);
        if(timer_id == 0)
        {
            //...
        }
    }
    else
    {
        //SetDlgItemText(msg.hWndMain,IDC_STATIC_TIMER,"00:00:00");
        if(timer_id)
        {
            timeKillEvent(timer_id);
            timer_id = 0;
        }
    }
}
