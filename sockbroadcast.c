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

static const char **get_local_all_ip(void)
{
    int ret;
    char szHost[128];   
    struct hostent *pHost;  
    struct in_addr addr;

    int i;
    char **pAlias;
    int c;
    static char **list = NULL;

    if (list) return list;

    ret = gethostname(szHost, 128);
    if (ret == SOCKET_ERROR) {
        TRACE("gethostname failed, error=%d\n", WSAGetLastError());
        return NULL;
    }

    pHost = gethostbyname(szHost);
    if (!pHost) {
        TRACE("gethostbyname failed, error=%d\n", WSAGetLastError());
        return NULL;
    }

    i = 0;
    for (pAlias = pHost->h_aliases; *pAlias != 0; pAlias++) {
        TRACE("\tAlternate name #%d: %s\n", ++i, *pAlias);
    }

    if (pHost->h_addrtype == AF_INET) {
        i = 0; c = 0;
        while (pHost->h_addr_list[i] != 0) {
            i++;
        }
        c = i + 1; // append null at the end
        list = (char**)malloc(c * sizeof(char*) + c * 20);
        if (!(char*)list) {
            TRACE("malloc failed.\n");
            return NULL;
        }
        memset((char*)list, 0, c * sizeof(char*) + c * 20);
        for (i = 0; i < c - 1; i++) {
            list[i] = (char*)(list + i * 20 + c * sizeof(char*));
        }

        i = 0;
        while (pHost->h_addr_list[i] != 0) {
            addr.s_addr = *(u_long *) pHost->h_addr_list[i];
            strcpy(list[i], inet_ntoa(addr));
            TRACE("\tIP Address #%d: %s\n", i + 1, list[i]);
            i++;
        }
        return list;
    } else if (pHost->h_addrtype == AF_NETBIOS) {
        TRACE("NETBIOS address was returned\n");
        return NULL;
    } else {
        TRACE("unkown addrtype = %d\n", pHost->h_addrtype);
        return NULL;
    }
}

static unsigned int __stdcall client_search_thread(void* p)
{
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

    if (!p) return 0;

    TRACE("broadcast for %s\n", p);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        TRACE("socket failed with error: %ld\n", WSAGetLastError());
        return 1;
    }

    //允许发送广播消息
    so_broadcast = TRUE;
    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&so_broadcast, sizeof(so_broadcast));

    addr.sin_family = AF_INET; //使用互联网际协议，即IP协议
    addr.sin_addr.S_un.S_addr = inet_addr((const char*)p); //inet_addr("192.168.0.108");//htonl(INADDR_ANY); 
    addr.sin_port = htons(42312);

    //如果仅仅是发送广播，这一步可有可无。没有绑定也能发送广播
    ret = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == SOCKET_ERROR) {
        TRACE("bind failed with error %u\n", WSAGetLastError());
        closesocket(sock);
        return 1;
    }

    b_addr.sin_family = AF_INET;
    b_addr.sin_addr.S_un.S_addr = htonl(INADDR_BROADCAST);
    b_addr.sin_port = htons(1025);
    
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&recTimeOut, sizeof(recTimeOut));

    while (thread_state == 1) {
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

    return 0;
}

static unsigned int __stdcall client_search_start(void* p)
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int i;
    const char **list;
    int ret;

    wVersionRequested = MAKEWORD(2,2);
    ret = WSAStartup(wVersionRequested,&wsaData);
    if (ret != NO_ERROR) {
        TRACE("WSAStartup failed with error: %d\n", ret);
        return 1;
    }

    list = get_local_all_ip();
    if (!list) {
        TRACE("get locall all ip failed.\n");
        return 0;
    }

    for (i = 0; list[i] != NULL; i++) {
        _beginthreadex(NULL, 0, client_search_thread, (void*)list[i], 0, NULL);
    }

    while (thread_state == 1) {
        Sleep(1000);
    }

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
    _beginthreadex(NULL, 0, client_search_start, 0, 0, NULL);
}
