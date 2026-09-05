// cmd_ssh.c - 精简版，仅支持版本交换
#include "declarations.h"
#include "cmd_ssh.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#define closesocket close
#endif

int cmd_ssh(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out) {
    (void)in;
    (void)ctx;

    if (argc < 2) {
        fprintf(out, "用法: ssh <host> [port]\n");
        fprintf(out, "  SSH客户端 - 当前仅支持版本交换\n");
        return 1;
    }

    char *host = argv[1];
    int port = 22;
    if (argc >= 3) port = atoi(argv[2]);

    // 初始化Winsock（Windows）
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        fprintf(out, "WSAStartup失败\n");
        return 1;
    }
#endif

    // 创建socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(out, "创建socket失败\n");
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    // 解析主机名
    struct hostent *he = gethostbyname(host);
    if (!he) {
        fprintf(out, "无法解析主机名 '%s'\n", host);
        closesocket(sockfd);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);

    // 连接
    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fprintf(out, "连接到 %s:%d 失败\n", host, port);
        closesocket(sockfd);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    fprintf(out, "已连接到 %s:%d\n", host, port);

    // 发送SSH版本标识
    const char *client_id = "SSH-2.0-LGD_OS_SSH_Client_0.1\r\n";
    send(sockfd, client_id, strlen(client_id), 0);
    fprintf(out, "[发送] %s", client_id);

    // 接收服务器版本标识
    char server_buf[256] = {0};
    int received = recv(sockfd, server_buf, sizeof(server_buf)-1, 0);
    if (received > 0) {
        server_buf[received] = '\0';
        // 去除末尾的CR/LF
        char *nl = strchr(server_buf, '\r');
        if (nl) *nl = '\0';
        nl = strchr(server_buf, '\n');
        if (nl) *nl = '\0';
        fprintf(out, "[接收] %s\n", server_buf);
    } else {
        fprintf(out, "接收服务器版本失败\n");
    }

    // 版本交换成功，但KEX未实现
    fprintf(out, "\n SSH 版本交换成功！\n");
    fprintf(out, "[\033[33m提示\033[0m] SSH 密钥交换(KEX)暂未实现，连接终止。\n");
    fprintf(out, "      当前仅支持到 SSH-2.0 版本协商阶段。\n");

    closesocket(sockfd);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
