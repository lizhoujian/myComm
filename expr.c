#include "expr.h"
#include "msg.h"
#include "stack.h"
#include "resource.h"

/**********************************************************
�ļ�����:expr.c
�ļ�·��:./common/expr.c
����ʱ��:2013-3-1,15:17:42
�ļ�����:Ů������
�ļ�˵��:���ʽ��ֵ,���ܼܺ�,��һ��д���ֳ���
**********************************************************/

static HWND hWndExpr = NULL;
static WNDPROC origEditProc = NULL;
static HWND hEditExpr;
static HWND hEditResult;
static int axisx, axisy;

int is_operator(char ch);
int privilege(char opcode1, char opcode2);
void evaluate(char *str);
int calc(int oprand1, int oprand2, char opcode);

void add_result(char *fmt, ...)
{
    va_list va;
    int len;
    char expr[1024];
    va_start(va, fmt);
    len = _vsnprintf(expr, sizeof(expr) - 2, fmt, va);
    strcpy(expr + len, "\r\n");
    va_end(va);
    len = Edit_GetTextLength(hEditResult);
    Edit_SetSel(hEditResult, len, len);
    Edit_ReplaceSel(hEditResult, expr);
}

static LRESULT __stdcall ExprEditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_KEYDOWN:
        if(wParam == VK_RETURN)
        {
            char expr[1024];
            GetWindowText(hEditExpr, expr, sizeof(expr));
            if(!*expr) return 0;
            evaluate(expr);
            add_result("-------------------------------\r\n\r\n");
            return 0;
        }
        break;
    }
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    return CallWindowProc(origEditProc, hWnd, uMsg, wParam, lParam);
}

INT_PTR CALLBACK ExprDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_CLOSE:
    {
        RECT rc;
        GetWindowRect(hWndDlg, &rc);
        axisx = rc.left;
        axisy = rc.top;
        hWndExpr = NULL;
        EndDialog(hWndDlg, 0);
        return 0;
    }
    case WM_INITDIALOG:
        hWndExpr = hWndDlg;
        hEditExpr = GetDlgItem(hWndDlg, IDC_EXPR_EDIT_EXPR);
        hEditResult = GetDlgItem(hWndDlg, IDC_EXPR_EDIT_RESULT);
        SendMessage(hEditExpr, WM_SETFONT, (WPARAM)msg.hFont, 0);
        SendMessage(hEditResult, WM_SETFONT, (WPARAM)msg.hFont, 0);
        origEditProc = (WNDPROC)SetWindowLong(hEditExpr, GWL_WNDPROC, (long)ExprEditProc);
        SetWindowPos(hWndDlg, 0, axisx, axisy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        add_result("���ʽ����\r\n֧�ֵ������:\r\n+,-,*,/(����),%%\r\n&,|,^,<(����),>(����)\r\n��֧�ָ�����\r\n"
                   "���س�������!\r\n��֧��10���Ʋ�����!\r\n��֧������!");
        SetDlgItemText(hWndDlg, IDC_EXPR_EDIT_EXPR, "���ʽ���������ڹ��ܸĽ�Ŀǰ������!");
        return 0;
    case WM_LBUTTONDOWN:
        SendMessage(hWndDlg, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        return 0;
    }
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    return 0;
}

int ShowExpr(void)
{
    if(hWndExpr)
    {
        if(IsIconic(hWndExpr))
            ShowWindow(hWndExpr, SW_SHOWNORMAL);
        else
            SetWindowPos(hWndExpr, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        return 0;
    }
    DialogBoxParam(msg.hInstance, MAKEINTRESOURCE(IDD_DLG_EXPR), NULL, ExprDialogProc, 0);
    return 0;
}

//////////////////////////////////////////////////////////////////////////

typedef enum _var_type
{
    MVT_NULL,
    MVT_VALUE,
    MVT_OPERATOR,
} var_type;

typedef struct _var_num
{
    int bits;
    char data[11];
    int sign;
} var_num;

void show_result(int data)
{
    char binary[33];
    int remain;
    int it = 31;
    unsigned int divider = (unsigned int)data;
    binary[32] = '\0';
    while(divider > 0)
    {
        remain = divider % 2;
        divider /= 2;
        binary[it--] = (char)(remain + '0');
    }
    //printf("dec:%d, oct:%o, hex:0x%X, bin:%s\n",data,data,data,binary+it+1);
    add_result("dec:%d\r\noct:0%o\r\nhex:0x%X\r\nbin:%s", data, data, data, binary + it + 1);
}

int is_operator(char ch)
{
    static char s_op[] = {'+', '-', '*', '/', '%', '&', '|', '^', '<', '>', '(', ')'};
    int i;
    for(i = 0; i < sizeof(s_op); i++)
    {
        if(s_op[i] == ch)
        {
            return i + 1;
        }
    }
    return 0;
}


//1:>,0:=;-1;<
int privilege(char opcode1, char opcode2)
{
    static char s_privilege[12][12] =
    {
        //��Ϊopcode2,��Ϊopcode1
        /*  +   -   *   /   %   &   |   ^   <   >   (    ) */
        /* + */{  0,  0, -1, -1, -1,  1,  1,  1,  1,  1, -1,  1 },
        /* - */{  0,  0, -1, -1, -1,  1,  1,  1,  1,  1, -1,  1 },
        /* * */{  1,  1,  0,  0,  0,  1,  1,  1,  1,  1, -1,  1 },
        /* / */{  1,  1,  0,  0,  0,  1,  1,  1,  1,  1, -1,  1 },
        /* % */{  1,  1,  0,  0,  0,  1,  1,  1,  1,  1, -1,  1 },
        /* & */{ -1, -1, -1, -1, -1,  0,  1,  1, -1, -1, -1,  1 },
        /* | */{ -1, -1, -1, -1, -1, -1,  0,  1, -1, -1, -1,  1 },
        /* ^ */{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1 }, //������ҽ��
        /* < */{ -1, -1, -1, -1, -1,  1,  1,  1,  0,  0, -1,  1 },
        /* > */{ -1, -1, -1, -1, -1,  1,  1,  1,  0,  0, -1,  1 },
        /* ( */{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0 },
        /* ) */{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  1 },
    };
    opcode1 = (char)is_operator(opcode1);
    opcode2 = (char)is_operator(opcode2);
    return s_privilege[opcode1-1][opcode2-1];
}

int calc(int oprand1, int oprand2, char opcode)
{
    switch(opcode)
    {
    case '+':
        return oprand1 + oprand2;
    case '-':
        return oprand1 - oprand2;
    case '*':
        return oprand1 * oprand2;
    case '/':
        return (int)(oprand1 / (float)oprand2);
    case '%':
return oprand1 % (oprand2 == 0 ? oprand1 == 0 ? 1 : oprand1 : oprand2);
    case '&':
        return oprand1 & oprand2;
    case '|':
        return oprand1 | oprand2;
    case '^':
        return oprand1 ^ oprand2;
    case '<':
        return oprand1 << oprand2;
    case '>':
        return oprand1 >> oprand2;
    }
    return 0;
}

void evaluate(char *str)
{
    var_type type = MVT_NULL;
    var_num num = {0, {0}};
    stack stk;
    stack opcstk;
    char ch;
    char *p = str;
    stack_init(&stk);
    stack_init(&opcstk);
    for(;;)
    {
        ch = *p;
        switch(type)
        {
        case MVT_VALUE:
        {
            if(!ch)
            {
                int value;
                num.data[num.bits] = '\0';
                value = atoi(num.data);
                stack_push(&stk, num.sign ? value : -value);
                num.bits = 0;
                for(;;)
                {
                    if(!stack_is_empty(&stk) && !stack_is_empty(&opcstk))
                    {
                        char opcode = (char)stack_pop(&opcstk);
                        int oprand1, oprand2;
                        oprand2 = stack_pop(&stk);
                        if(!stack_is_empty(&stk))
                        {
                            int result;
                            oprand1 = stack_pop(&stk);
                            result = calc(oprand1, oprand2, opcode);
                            //printf("%d %c %d = %d\n",oprand1,opcode,oprand2,result);
                            add_result("%d %c %d = %d\n", oprand1, opcode, oprand2, result);
                            stack_push(&stk, result);
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else if(isdigit(ch))
            {
                num.data[num.bits++] = ch;
            }
            else if(is_operator(ch))
            {
                int value;
                num.data[num.bits] = '\0';
                value = atoi(num.data);
                stack_push(&stk, num.sign ? value : -value);
                //num.bits=0;
                for(;;)
                {
                    if(!stack_is_empty(&opcstk))
                    {
                        int cmp = privilege((char)stack_get_top(&opcstk), ch);
                        if(cmp >= 0) //ǰһ����������ȼ����ڵ��ں���
                        {
                            if(stack_get_top(&opcstk) != '(')
                            {
                                int oprand1, oprand2;
                                char opcode;
                                oprand2 = stack_pop(&stk);
                                if(!stack_is_empty(&stk))
                                {
                                    int result;
                                    oprand1 = stack_pop(&stk);
                                    opcode = (char)stack_pop(&opcstk);
                                    result = calc(oprand1, oprand2, opcode);
                                    //printf("%d %c %d = %d\n",oprand1,opcode,oprand2,result);
                                    add_result("%d %c %d = %d\n", oprand1, opcode, oprand2, result);
                                    //stack_push(&stk,result);
                                    itoa(result, num.data, 10);
                                    num.bits = strlen(num.data);
                                    num.sign = result > 0;
                                    type =  MVT_VALUE;
                                    //break;
                                }
                                //stack_push(&opcstk,ch);
                            }
                            else
                            {
                                if(ch == ')')
                                {
                                    stack_pop(&opcstk);
                                    type = MVT_VALUE;
                                    break;
                                }
                                else
                                {
                                    stack_push(&opcstk, ch);
                                    type = MVT_OPERATOR;
                                    //break;
                                }
                                //break;
                            }
                        }
                        else  //ǰһ����������ȼ����ں���
                        {
                            stack_push(&opcstk, ch);
                            type = MVT_OPERATOR;
                            break;
                        }
                    }
                    else
                    {
                        stack_push(&opcstk, ch);
                        type = MVT_OPERATOR;
                        break;
                    }
                }
                //type = MVT_OPERATOR;
            }
            break;
        }
        case MVT_OPERATOR:
        {
            if(!ch)
            {
                //printf("�����λ�����,����!\n");
                //add_result("�����λ�����,����!");
            }
            else if(isdigit(ch))
            {
                type = MVT_VALUE;
                num.bits = 0;
                num.data[num.bits++] = ch;
            }
            else if(is_operator(ch))
            {
                //printf("%c(***error***)",ch);
                if(ch == '+' || ch == '-')
                {
                    num.sign = ch == '+';
                }
                else if(ch == '(')
                {
                    stack_push(&opcstk, ch);
                }
                else
                {
                    //printf("%c(ERROR)",ch);
                    add_result("%c(����������!)", ch);
                }
            }
            break;
        }
        case MVT_NULL:
        {
            if(isdigit(ch))
            {
                //printf("%c",ch);
                num.bits = 0;
                num.sign = 1;
                num.data[num.bits++] = ch;
                type = MVT_VALUE;
            }
            else if(ch == '+' || ch == '-')
            {
                num.sign = ch == '+';
                type = MVT_OPERATOR;
            }
            else if(ch == '(')
            {
                stack_push(&opcstk, ch);
                num.sign = 1;
                num.bits = 0;
                type = MVT_OPERATOR;
            }
            else
            {
                num.sign = 1;
                //add_result("��1�����������!");
                type = MVT_NULL;
            }
            break;
        }
        }
        if(!ch)
        {
            break;
        }
        else
        {
            p++;
        }
    }
    //printf("result:%d\n",stack_pop(&stk));
    if(type != MVT_NULL)
        show_result(stack_pop(&stk));
}


#if 0
int main(void)
{
    char str[1024];
    char *pch = NULL;
    for(;;)
    {
        fflush(stdin);
        printf("input expr:");
        fgets(&str[0], sizeof(str), stdin);
        pch = strrchr(str, '\n');
        if(pch) *pch = 0;
        if(!*str)
            continue;
        evaluate(str);
        //show_result(atoi(str));
        printf("\n");
    }
    return 0;
}
#endif
