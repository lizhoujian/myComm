#define __ABOUT_C__
#include "about.h"
#include "msg.h"
#include "utils.h"
#include "resource.h"

struct about_s about;

void init_about(void)
{
    memset(&about, 0, sizeof(about));
    about.show = show_about;
}

void show_about(void)
{
    char *help_msg =
        "说在前面的话:\r\n"
        "    该程序目前正处理开发的初期, 还有很多的错误未发现, 可能会给您的使用过程带来"
        "不便, 或者发生意想不到的错误, 对此引起的错误我表示歉意. 但, 非常感谢您对本程序的支持和所做的测试工作,"
        "发现了任何的软件错误, 请及时向作者提交, 以尽量减少错误的发生.\r\n\r\n"
        "-----------------------------------------------------\r\n"
        "软件说明:\r\n"
        "   软件用C语言+SDK方式完成, 开源软件, 绿色小巧, 不会给系统带来任何的垃圾文件."
        "\r\n\r\n"
        "-----------------------------------------------------\r\n"
        "使用帮助:\r\n"
        "    先设置好各个串口的参数之后打开串口进行读与写\r\n"
        "    一般设置(需参考硬件):\r\n"
        "        波特率:9600\r\n"
        "        校验位:无\r\n"
        "        数据位:8位\r\n"
        "        停止位:1位\r\n\r\n"
        "-----------------------------------------------------\r\n"
        "其它:\r\n"
        "    自动发送时间间隔范围:50ms~60000ms\r\n"
        "    开启自动发送后将不允许手动发送\r\n"
        "    若修改发送数据, 自动发送将被取消\r\n"
        "    16进制发送格式为:2个16进制位+1个空格\r\n"
        "    字符发送时回车换行会被发送,16进制发送时不会\r\n\r\n"
        "-----------------------------------------------------\r\n"
        "更新:\r\n"
        "2012-12-24 1.0.0.0:\r\n"
        "    发布第1个版本\r\n"
        "2012-12-26:\r\n"
        "    自动识别当前存在,插入,移除的串口号\r\n"
        //"    关闭串口后接收与发送缓冲计数器现已自动清零(2013-01-05)"
        "2013-01-11 1.0.0.1:\r\n"
        "    增加保存接收区数据到文件(16进制/文本形式)\r\n"
        "    增加从文件读数据到发送区(16进制/文本形式)\r\n"
        "    增加暂停显示功能\r\n"
        "    增加复制发送/接收区数据到剪贴板\r\n"
        "2013-01-18 1.0.0.2:\r\n"
        "    修复:文本文件,16二进制文件读取错误\r\n"
        "    修复:程序内部缓冲区满后使程序进入死循环\r\n"
        "    修复:文本字符方式显示接收的数据时产生不正确的换行符的错误,若要产生换行符, 请使用\"\\n\"\r\n"
        "2013-02-08 1.0.0.3:\r\n"
        "    内部程序作了许多的优化工作,包含数据的发送方式等\r\n"
        "    修复接收数据时鼠标在接收区的文本选择造成的干扰\r\n"
        "2013-02-14 1.0.0.4:\r\n"
        "    增加显示出0~127号ASCII对应8,10,16进制功能\r\n"
        "2013-02-24 1.0.0.5,今天元宵节:\r\n"
        "    更改原来的1~64串口列表到自动检测计算机上的可用串口\r\n"
        "2013-02-27 1.0.0.6:\r\n"
        "    若发送文本,则自动发送被自动取消(若自动发送选项已打开)\r\n"
        "    在显示模式下不允许对接收区数据进行选择操作\r\n"
        "    提供硬件支持的串口设备设置\r\n"
        "    为用户提供串口超时设置\r\n"
        "    提供手动设置DTR/RTS引脚电平\r\n"
        "2013-03-01  1.0.0.7:\r\n"
        "    修改原计算器(系统)为表达式求值计算器(简单版本)\r\n"
        "2013-03-03:\r\n"
        "    添加:<其它>菜单添加<设备管理器>\r\n"
        "    修改:在关闭串口后自动发送前面的钩不再自动取消(如果已经选中)\r\n"
        "    修改:串口被关闭/移除后串口列表回到第一个串口设备的BUG\r\n"
        "2013-03-04:\r\n"
        "    修改:现在在串口列表中可以显示串口在设备管理器中的名字了\r\n"
        "    修正:无法显示 MSP430-FETUIF Debugger 的串口号(现在调用SetupApi更新列表)\r\n"
        "2013-03-05:\r\n"
        "    为了方便数据的统计与显示,16进制内容与字符内容被显示到不同的编辑框中\r\n"
        "2013-03-09 1.0.0.8:\r\n"
        "    修正在使用SetupApi枚举串口设备时未检测并口设备而造成的内存异常访问错误\r\n"
        "    减少在某些波特率(如:19200bps)下丢包严重的情况(如:MSP430串口),有时候还是会发生,等待修复.某些软件(如:SComAssistant"
        "采用每次只读一个字节的办法效果还行, 就是速度有点慢. 我改成了WaitCommEvent函数调用了(原来是Pending ReadFile),减少了CPU占用(有些串口驱动并不总是支持同步操作).\r\n"
        "    以前只管ReadFile+输出nRead字节,这里错误,ReadFile并不保证读取到要求的数据量后才返回,这里会导致严重丢包,WriteFile亦然.\r\n"
        "    速度减慢,但数据更完整\r\n"
        "2013-03-10 1.0.0.9:\r\n"
        "    修正:因为在格式化字符串的最后少写了一句 *pb = \'\\0\',导致接收区数据显示错误!"
        "    修复:对utils.hex2chs和add_text作了大量修改,大大减少数据丢包,貌似没有丢包?,细节处理参见源程序\r\n"
        "    1.0.0.8版本因为内部原因速度严重减慢, 1.0.0.9回到原来的快速!\r\n"
        "2013-03-18:\r\n"
        "    更正:若为字符显示方式,16进制方式保存不被允许,因为格式基本上不满足!\r\n"
        "2013-03-23 1.10:\r\n"
        "    添加:工作模式中,右键点击接收区字符文本框可以使能中文显示模式(不推荐),由于中文字符由两个字节构成,所以:一旦在某一次接收过程中只"
        "接收到了中文字符的一个字节,那么数据就会显示出错, 这个无法避免, 所以建议尽量不使能中文显示模式.\r\n"
        "    修正:用C语言的人们都习惯使用'\\n'作为换行符,我也这样使用,"
        "但偏偏Windows的编辑框以'\\r\\n'作为换行符,没有办法,我不得"
        "不把所有的'\\n'换成'\\r\\n',效率必然会下降,而且我不得不计算出"
        "\\n的个数先 --> 为了计算所需缓冲区的大小.\r\n"
        "    添加:现在可以显示出还未被发送出去的数据计数.\r\n"
        "    添加:新增计时器,打开串口后开始计时,关闭后停止计时.\r\n"
        "2013-03-25:\r\n"
        "    修正:大大减少中文乱码的问题.细节处理见代码.\r\n"
        "    增加:字符串转16进制数组功能,工具菜单里面.\r\n"
        ;
    DialogBoxParam(msg.hInstance, MAKEINTRESOURCE(IDD_MSG), msg.hWndMain, DialogProc, (LPARAM)help_msg);
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
    {
        //extern HFONT hFont;
        SetWindowText(hwndDlg, "关于 "COMMON_NAME_AND_VERSION);
        SetDlgItemText(hwndDlg, IDC_STATIC_VERSION, COMMON_NAME_AND_VERSION"  编译时间:"__DATE__" - "__TIME__);
        SetDlgItemText(hwndDlg, IDC_MSG_EDIT_MSG, (char *)lParam);
        //SendDlgItemMessage(hwndDlg,IDC_MSG_EDIT_MSG,WM_SETFONT,(WPARAM)hFont,0);
        SetFocus(GetDlgItem(hwndDlg, IDC_MSG_OK));
        utils.center_window(hwndDlg, msg.hWndMain);
        return 0;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HICON hIcon;
        HDC hDC = BeginPaint(hwndDlg, &ps);
        hIcon = LoadIcon(msg.hInstance, MAKEINTRESOURCE(IDI_ICON1));
        DrawIcon(hDC, 10, 10 , hIcon);
        EndPaint(hwndDlg, &ps);
        DestroyIcon(hIcon);
        return 0;
    }
    case WM_CLOSE:
        EndDialog(hwndDlg, 0);
        return 0;
    case WM_COMMAND:
        if(LOWORD(wParam) == IDC_MSG_OK && HIWORD(wParam) == BN_CLICKED)
        {
            SendMessage(hwndDlg, WM_CLOSE, 0, 0);
            return 0;
        }
        else if(LOWORD(wParam) == IDC_MSG_HOTLINK && HIWORD(wParam) == BN_CLICKED)
        {
            char *web = "http://www.cnblogs.com/nbsofer/archive/2012/12/24/2831700.html";
            ShellExecute(NULL, "open", web, NULL, NULL, SW_SHOWNORMAL);
            return 0;
        }
        return 0;
    default:
        return 0;
    }
}
