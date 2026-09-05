#ifndef CMD_SSH_H
#define CMD_SSH_H

// SSH消息类型（仅保留基础定义，以备将来扩展）
#define SSH_MSG_KEXINIT       20
#define SSH_MSG_NEWKEYS       21
#define SSH_MSG_KEXDH_INIT    30
#define SSH_MSG_KEXDH_REPLY   31
#define SSH_MSG_SERVICE_REQUEST 5
#define SSH_MSG_SERVICE_ACCEPT  6
#define SSH_MSG_USERAUTH_REQUEST 50
#define SSH_MSG_USERAUTH_SUCCESS 52
#define SSH_MSG_USERAUTH_FAILURE 51
#define SSH_MSG_CHANNEL_OPEN      90
#define SSH_MSG_CHANNEL_OPEN_CONFIRMATION 91
#define SSH_MSG_CHANNEL_OPEN_FAILURE      92
#define SSH_MSG_CHANNEL_DATA      94
#define SSH_MSG_CHANNEL_EOF       96
#define SSH_MSG_CHANNEL_CLOSE     97

// 连接上下文（可保留，但当前未使用）
typedef struct {
    int socket_fd;
    char client_version[256];
    char server_version[256];
    int is_connected;
} SshConnection;

// 命令函数
int cmd_ssh(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);

#endif
