// Copyright (C) 2015 zhangyongzhi 

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "uci.h"

//根据选项来访问服务器，
struct Hello
{
    char agent[50]; //  代理字符串。
    char url[256];  //  访问的url。
    int delay;      //  启动后延迟多长时间访问。
};

int getValue(struct uci_context *ctx, char *key, char*value, int n)
{
    char strKey[100];
    struct uci_ptr ptr;

    snprintf(strKey, sizeof(strKey), "hello.globe.%s",key);
    if (uci_lookup_ptr(ctx, &ptr, strKey, true) == UCI_OK)
    {
        printf("%s\n", ptr.o->v.string);
        strncpy(value, ptr.o->v.string, n-1);
    }
    return 0;
}

int read_conf( struct Hello *hello)
{
    struct uci_context *ctx = uci_alloc_context();
    if (!ctx) 
    {
        fprintf(stderr, "No memory\n");
        return 1;
    }
    getValue(ctx, "agent", hello->agent, sizeof(hello->agent));
    getValue(ctx, "url", hello->url, sizeof(hello->url));
    char delay[20];
    getValue(ctx, "delay", delay, sizeof(delay));
    hello->delay = atoi(delay);

    uci_free_context(ctx);    
    return 0;
}

struct Hello hello;
int main(int argc, char* argv[])
{
/*
    char agent[50] = "openwrt";
    char url[100] = "http://bjbook.net/bk/openwrt";
    
*/
    char cmd[512] = {0};
    //从/etc/config/hello中读取参数
    read_conf(&hello); 
    printf("agent=%s\n", hello.agent);
    printf("url=%s\n", hello.url);
    printf("delay=%d\n", hello.delay);

    snprintf(cmd, sizeof(cmd), "wget --user-agent=%s %s", 
        hello.agent, hello.url);
    //system("wget --user-agent=agent-string http://bjbook.net/bk/openwrt");
    printf("cmd=%s\n", cmd);
    
    //srand(time(NULL));
    int delay_time = rand() % hello.delay;
    printf("delay_time=%d\n", delay_time);
    sleep(delay_time);
    
    system(cmd);
    return 0;
}
