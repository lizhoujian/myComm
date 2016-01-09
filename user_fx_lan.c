#include "ghttp/ghttp.h"
#include "ghttp/http_hdrs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <unistd.h>
#include "user_fx.h"
#include "user_fx_lan.h"
#include "cJSON/cJSON.h"

#define MAX_TIMEOUT 20
#define URL "http://%s/config?command=fx2n"

#include <stdarg.h>
#include <stdio.h>
#include <windows.h>
#include <process.h>
static void TRACE(const char * sz, ...)
{
    char szData[512]={0};

    va_list args;
    va_start(args, sz);
    _vsnprintf(szData, sizeof(szData) - 1, sz, args);
    va_end(args);

    OutputDebugString(szData);
}

static u8 *create_url(u8 *ip)
{
    u8 *r;
    int len;

    len = strlen(ip) + strlen(URL) + 1;
    r = (u8*)malloc(len);
    if (r) {
        memset(r, 0, len);
        sprintf(r, URL, ip);
    }
    
    return r;
}

static cJSON *create_json(u8 cmd, u8 addr_type, u16 addr, u8 *data, u16 len)
{
    int i;
    u8 *tmp;
    cJSON *root = NULL;

    root = cJSON_CreateObject();
    if (root) {
        cJSON_AddNumberToObject(root, "cmd", cmd);
        cJSON_AddNumberToObject(root, "addr_type", addr_type);
        cJSON_AddNumberToObject(root, "addr", addr);
        if (data && len > 0) {
            tmp = (u8*)malloc(len * 2 + 1);
            if (tmp) {
                memset(tmp, 0, len * 2 + 1);
                for (i = 0; i < len; i++) {
                    sprintf(tmp + i * 2, "%02x", data[i]);
                }
                cJSON_AddStringToObject(root, "data", (const char*)tmp);
                free(tmp);
            } else {
                TRACE("alloc tmp mem for data failed.\n");
            }
        }
        cJSON_AddNumberToObject(root, "len", len);
    }
    return root;
}

static u8 *post_request(u8 *ip, u8 cmd, u8 addr_type, u16 addr, u8 *data, u16 len)
{
    u8 *url;
    cJSON *json;
    u8 *params;
    u8 *r;

    url = create_url(ip);
    if (!url) {
        return NULL;
    }

    json = create_json(cmd, addr_type, addr, data, len);
    if (!json) {
        return NULL;
    }

    params = (u8*)cJSON_Print(json);
    if (params) {
        TRACE("post: %s\n params: %s\n", url, params);
    }
    r = fx_lan_post(url, params, MAX_TIMEOUT);

    if (params) {
        free(params);
    }
    cJSON_Delete(json);
    free(url);

    if (r) {
        TRACE("recv from client: \n%s\n", r);
    }

    return r;
}

static bool post_simple(u8 *ip, u8 cmd, u8 addr_type, u16 addr, u8 *data, u16 len)
{
    bool ret = false;
    cJSON *json;
    cJSON *sub;
    u8 *value;
    u8 *r;

    r = post_request(ip, cmd, addr_type, addr, (cmd == ACTION_WRITE ? data : NULL), len);
    if (r) {
        json = cJSON_Parse((const char*)r);
        if (json) {
            sub = cJSON_GetObjectItem(json, "result");
            ret = (sub && sub->valueint);
            if (ret) {
                sub = cJSON_GetObjectItem(json, "value");
                if (sub) {
                    value = (u8*)sub->valuestring;
                    if (value && data) {
                        strcpy((char*)data, (const char*)value);
                    }
                }
            }
            cJSON_Delete(json);
        }
    }

    return ret;
}

bool fx_lan_enquiry(u8 *ip)
{
    return post_simple(ip, ENQ, 0, 0, NULL, 0);
}

bool fx_lan_force_on(u8 *ip, u8 addr_type, u16 addr)
{
    return post_simple(ip, ACTION_FORCE_ON, addr_type, addr, NULL, 0);
}

bool fx_lan_force_off(u8 *ip, u8 addr_type, u16 addr)
{
    return post_simple(ip, ACTION_FORCE_OFF, addr_type, addr, NULL, 0);
}

bool fx_lan_read(u8 *ip, u8 addr_type, u16 addr, u8 *out, u16 len)
{
    return post_simple(ip, ACTION_READ, addr_type, addr, out, len);
}

bool fx_lan_write(u8 *ip, u8 addr_type, u16 addr, u8 *data, u16 len)
{
    return post_simple(ip, ACTION_WRITE, addr_type, addr, data, len);
}

u8 *fx_lan_get(u8 *uri, u8 *params, int timeout)
{
    ghttp_request *request = NULL;
    u8 *tmp;
    char timeout_str[10] = {0,};
    u8 *result;
    int len, result_len = 0;
    u8 *data = NULL;

    request = ghttp_request_new();
    if (!request) {
        return NULL;
    }
    if (params != NULL && strlen(params) > 0) {
        len = strlen(uri) + strlen(params) + 2;
        tmp = (char*)malloc(len);
        if (tmp) {
            memset(tmp, 0, len);
            strcpy(tmp, uri);
            if (strchr(tmp, '?') == NULL) {
                strcat(tmp, "?");
            }
            strcat(tmp, params);
            ghttp_set_uri(request, tmp);
            free(tmp);
        }
    } else {
        ghttp_set_uri(request, uri);
    }

    ghttp_set_type(request, ghttp_type_get);
    ghttp_set_header(request, http_hdr_Connection, "close");
    sprintf(timeout_str, "%d", timeout);
    ghttp_set_header(request, http_hdr_Timeout, (const char *)timeout_str);

    ghttp_prepare(request);
    if (ghttp_process(request) != ghttp_done) {
        goto __exit;
    }

    result_len = ghttp_get_body_len(request);
    result = (u8*)ghttp_get_body(request);

    len = result_len + 1;
    data = (u8 *)malloc(len);
    memset(data, 0, len + 1);
    data[len] = '\0';
    memcpy(data, result, result_len);

__exit:
    ghttp_clean(request);
    ghttp_request_destroy(request);

    return data;
}

u8 *fx_lan_post(u8 *uri, u8 *params, int timeout)
{
    char timeout_str[10] = {0,};
    ghttp_request *request = NULL;
    ghttp_status status;
    int len;
    u8 *result;
    int result_len = 0;
    u8 *data = NULL;

    request = ghttp_request_new();
    if (!request) {
        return NULL;
    }
    if (ghttp_set_uri(request, uri) == -1) {
        return NULL;
    }
    if (ghttp_set_type(request, ghttp_type_post) == -1) {
        return NULL;
    }

    ghttp_set_header(request, http_hdr_Content_Type, "application/x-www-form-urlencoded");
    sprintf(timeout_str, "%d", timeout);
    ghttp_set_header(request, http_hdr_Timeout, (const char *)timeout_str);
    //ghttp_set_sync(request, ghttp_sync); //set sync

    len = strlen(params);
    ghttp_set_body(request, params, len);

    ghttp_prepare(request);
    status = ghttp_process(request);
    if (status != ghttp_done) {
        goto __exit;
    }

    result_len = ghttp_get_body_len(request);
    result = (u8*)ghttp_get_body(request);
    if (result) {
        data = (u8 *)malloc(result_len + 1);
        memset(data, 0, result_len + 1);
        data[result_len] = '\0';
        memcpy(data, result, result_len);
    }

__exit:
    ghttp_clean(request);
    ghttp_request_destroy(request);

    return data;
}
