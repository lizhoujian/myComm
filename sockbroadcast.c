#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
#include <Windows.h>
#include <process.h>

#pragma comment(lib, "ws2_32.lib")

static int thread_state = 0;
static void (*append_list_cb)(const char *addr);

static void TRACE(const char * sz, ...)
{
    char szData[512]={0};

    va_list args;
    va_start(args, sz);
    _vsnprintf(szData, sizeof(szData) - 1, sz, args);
    va_end(args);

    OutputDebugString(szData);
}

static void append_client_list(const char *addr)
{
    if (append_list_cb) {
        append_list_cb(addr);
    }
}

static void parse_braodcast_recv(struct sockaddr_in *from, const char *msg)
{
    TRACE("recv from host:%s, post:%d, msg:%s\n", inet_ntoa(from->sin_addr), ntohs(from->sin_port), msg);
    append_client_list(inet_ntoa(from->sin_addr));
}

static unsigned int __stdcall client_search_thread(void* p)
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int ret;
    int sock;
    int so_broadcast;
    int recTimeOut = 3000;

    struct sockaddr_in addr;
    struct sockaddr_in b_addr;
    
    char buff[] = "Are You Espressif IOT Smart Device?";
    struct sockaddr_in from;
    char recv[128] = {0,};
    int len;

    wVersionRequested = MAKEWORD(2,2);
    ret = WSAStartup(wVersionRequested,&wsaData);
    if (ret != NO_ERROR) {
        TRACE(L"WSAStartup failed with error: %d\n", ret);
        return 1;
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        TRACE("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    //允许发送广播消息
    so_broadcast = TRUE;
    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&so_broadcast, sizeof(so_broadcast));

    addr.sin_family = AF_INET; //使用互联网际协议，即IP协议
    addr.sin_addr.S_un.S_addr = inet_addr("192.168.0.108");//htonl(INADDR_ANY); 
    addr.sin_port = htons(42312);

    //如果仅仅是发送广播，这一步可有可无。没有绑定也能发送广播
    ret = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == SOCKET_ERROR) {
        TRACE("bind failed with error %u\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    b_addr.sin_family = AF_INET;
    b_addr.sin_addr.S_un.S_addr = htonl(INADDR_BROADCAST);
    b_addr.sin_port = htons(1025);
    
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&recTimeOut, sizeof(recTimeOut));

    while (thread_state == 1)
    {
        memset(recv, 0, sizeof(recv));
        ret = sendto(sock, buff, strlen(buff), 0, (struct sockaddr*)&b_addr, sizeof(b_addr));
        if (ret != SOCKET_ERROR) {
            len = sizeof(b_addr);
            ret = recvfrom(sock, recv, sizeof(recv) - 1, 0, (struct sockaddr *)&from, &len);
            if (ret > 0) {
                parse_braodcast_recv(&from, recv);
            } else {
                TRACE("recv failed, error: %d\n", WSAGetLastError());
            }
        } else {
            TRACE("send failed, error: %d\n", WSAGetLastError());
        }
        Sleep(1000);
    }

    TRACE("client search thread exit.\n");
    closesocket(sock);
    WSACleanup();
    
    thread_state = 3;
    
    return 0;
}

void stop_client_search(void)
{
    thread_state = 2;
    Sleep(100);
}

void start_client_search(void (*append_list)(const char *addr))
{
    append_list_cb = append_list;
    thread_state = 1;
    _beginthreadex(NULL, 0, client_search_thread, 0, 0, NULL);
}
