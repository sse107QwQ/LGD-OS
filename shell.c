// 在 shell.c 顶部
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "declarations.h"
#include "log.h"
#include <time.h>
#include <stdarg.h>
// 定义常量
#define MAX_INPUT_LEN 1024
#define MAX_TOKENS 100
#define MAX_PATH_LEN 1024
#define MAX_CMD_LEN 1024
#ifdef _WIN32
#include <windows.h>
#define sleep(seconds) Sleep((seconds) * 1000)
#endif
// 定义全局命令表
Command commands[] = {
    {"help", cmd_help,
     "显示帮助信息",
     "用法: help [command]\n"
     "  显示所有命令或特定命令的帮助信息。\n"
     "  参数:\n"
     "    command    (可选) 要获取帮助的命令名称\n"
     "  示例:\n"
     "    help        显示所有命令\n"
     "    help cd     显示cd命令的详细帮助\n"
     "    help all    显示所有命令的详细帮助"},

    {"exit", cmd_exit,
     "退出Shell",
     "用法: exit\n"
     "  退出LGD-OS Shell。\n"
     "  示例:\n"
     "    exit        退出系统"},

    {"cd", cmd_cd,
     "切换当前目录",
     "用法: cd [目录]\n"
     "  切换当前工作目录。\n"
     "  特殊目录:\n"
     "    /       - 根目录\n"
     "    ~       - 用户主目录\n"
     "    .       - 当前目录\n"
     "    ..      - 上级目录\n"
     "  示例:\n"
     "    cd /        切换到根目录\n"
     "    cd ..       切换到上级目录\n"
     "    cd /usr    切换到/usr目录"},

    {"ls", cmd_ls,
     "列出目录内容",
     "用法: ls [选项] [目录]\n"
     "  列出指定目录的内容。\n"
     "  选项:\n"
     "    -l     显示详细信息\n"
     "    -a     显示所有文件（包括隐藏文件）\n"
     "    -h     显示文件大小（人类可读格式）\n"
     "  示例:\n"
     "    ls          列出当前目录\n"
     "    ls -l      详细列表\n"
     "    ls /usr    列出指定目录"},

    {"pwd", cmd_pwd,
     "显示当前目录",
     "用法: pwd\n"
     "  显示当前工作目录的路径。\n"
     "  示例:\n"
     "    pwd        显示当前目录路径"},

    {"mkdir", cmd_mkdir,
     "创建目录",
     "用法: mkdir [选项] <目录名> [目录2 ...]\n"
     "  创建一个或多个目录。\n"
     "  选项:\n"
     "    -p     递归创建父目录\n"
     "  示例:\n"
     "    mkdir dir1            创建单个目录\n"
     "    mkdir dir1 dir2 dir3  创建多个目录"},

    {"rmdir", cmd_rmdir,
     "删除空目录",
     "用法: rmdir <目录名> [目录2 ...]\n"
     "  删除一个或多个空目录。\n"
     "  示例:\n"
     "    rmdir emptydir        删除空目录"},

    {"cp", cmd_cp,
     "复制文件或目录",
     "用法: cp [选项] <源> <目标>\n"
     "  或: cp [选项] <源1> <源2> ... <目标目录>\n"
     "  复制文件或目录。\n"
     "  选项:\n"
     "    -r, -R 递归复制目录\n"
     "    -f     强制覆盖目标文件\n"
     "  示例:\n"
     "    cp file.txt backup.txt     复制文件\n"
     "    cp -r dir1 dir2            复制目录\n"
     "    cp file1.txt file2.txt dir/ 复制多个文件到目录"},

    {"mv", cmd_mv,
     "移动/重命名文件或目录",
     "用法: mv [选项] <源> <目标>\n"
     "  或: mv [选项] <源1> <源2> ... <目标目录>\n"
     "  移动或重命名文件/目录。\n"
     "  选项:\n"
     "    -f 强制覆盖目标文件\n"
     "  示例:\n"
     "    mv old.txt new.txt      重命名文件\n"
     "    mv file.txt dir/        移动文件到目录\n"
     "    mv dir1 dir2            移动/重命名目录"},

    {"cat", cmd_cat,
     "显示文件内容",
     "用法: cat [文件1] [文件2 ...]\n"
     "  显示一个或多个文件的内容。\n"
     "  示例:\n"
     "    cat file.txt            显示单个文件\n"
     "    cat file1.txt file2.txt 显示多个文件\n"
     "    cat                     从标准输入读取（Ctrl+Z结束）"},

    {"touch", cmd_touch,
     "创建文件或更新修改时间",
     "用法: touch <文件名> [文件2 ...]\n"
     "  创建文件或更新其时间戳。\n"
     "  示例:\n"
     "    touch newfile.txt      创建新文件\n"
     "    touch file1.txt file2.txt 创建多个文件\n"
     "    touch existing.txt     更新文件时间戳"},

    {"rm", cmd_rm,
     "删除文件或目录",
     "用法: rm [选项] <文件/目录> [文件2/目录2 ...]\n"
     "  删除文件或目录。\n"
     "  选项:\n"
     "    -f  强制删除，不提示\n"
     "    -r  递归删除目录\n"
     "  示例:\n"
     "    rm file.txt          删除文件\n"
     "    rm -f old.txt        强制删除文件\n"
     "    rm -r dir            递归删除目录"},

    {"info", cmd_info,
     "显示文件/目录信息",
     "用法: info <文件/目录> [文件2/目录2 ...]\n"
     "  显示文件或目录的详细信息。\n"
     "  信息包括：大小、修改时间、权限、内容等。\n"
     "  示例:\n"
     "    info file.txt     显示文件信息\n"
     "    info dir/         显示目录信息"},

    {"edit", cmd_edit,
     "编辑文本文件",
     "用法: edit <文件> [文件2 ...]\n"
     "  使用系统默认文本编辑器打开文件。\n"
     "  示例:\n"
     "    edit file.txt      编辑文件\n"
     "    edit newfile.txt   创建并编辑新文件"},

    {"echo", cmd_echo,
     "显示文本",
     "用法: echo [选项] [文本]\n"
     "  显示指定的文本。\n"
     "  选项:\n"
     "    -n 输出末尾不换行\n"
     "  示例:\n"
     "    echo Hello World          输出Hello World\n"
     "    echo -n \"Hello \"        输出Hello 不换行"},

    {"grep", cmd_grep,
     "文本搜索",
     "用法: grep [选项] <模式> [文件] [文件2 ...]\n"
     "  在文件中搜索指定模式。\n"
     "  选项:\n"
     "    -i  忽略大小写\n"
     "    -n  显示行号\n"
     "    -c  只显示匹配行数\n"
     "  示例:\n"
     "    grep \"text\" file.txt       搜索文本\n"
     "    grep -i \"TEXT\" file.txt   忽略大小写搜索\n"
     "    grep -n \"text\" file.txt   显示行号"},

    {"wc", cmd_wc,
     "统计文件信息",
     "用法: wc [选项] [文件] [文件2 ...]\n"
     "  统计文件的行数、单词数、字符数。\n"
     "  选项:\n"
     "    -l  只统计行数\n"
     "    -w  只统计单词数\n"
     "    -c  只统计字节数\n"
     "    -m  只统计字符数\n"
     "  默认显示：行数 单词数 字符数 文件名\n"
     "  示例:\n"
     "    wc file.txt          统计文件信息\n"
     "    wc -l file.txt       只统计行数\n"
     "    wc -w file.txt       只统计单词数"},

    {"more", cmd_more,
     "分页显示文件",
     "用法: more [选项] [文件] [文件2 ...]\n"
     "  分页显示文件内容。\n"
     "  选项:\n"
     "    -n  NUM  每页显示行数（默认24行）\n"
     "  按键:\n"
     "    空格键    显示下一页\n"
     "    q        退出\n"
     "    Enter    显示下一行\n"
     "  示例:\n"
     "    more file.txt       分页显示文件\n"
     "    cat file.txt | more 通过管道分页显示"},

    {"game", cmd_game,
     "扫雷游戏",
     "用法: game\n"
     "  启动9x9扫雷游戏，包含10个雷。\n"
     "  游戏规则：\n"
     "    1. 目标：找出所有没有雷的格子\n"
     "    2. 标记：F=标记为雷，?=可疑标记\n"
     "    3. 操作：输入 行 列 操作\n"
     "    4. 数字：表示周围雷的数量\n"
     "    5. 标记：?表示不确定的格子\n"
     "  游戏内命令：\n"
     "    help    显示游戏帮助\n"
     "    quit    退出游戏\n"
     "  示例：\n"
     "    game             开始游戏\n"
     "    4 5 1            翻开第4行第5列的格子\n"
     "    2 3 2            标记第2行第3列为可疑"},

    {"tree", cmd_tree,
     "树状显示目录结构",
     "用法: tree [选项] [路径]\n"
     "  树状显示目录结构。\n"
     "  选项:\n"
     "    -a      显示所有文件（包括隐藏文件）\n"
     "    -d      只显示目录\n"
     "    -L  NUM 显示的最大层级\n"
     "    -h, --help 显示此帮助信息\n"
     "  示例:\n"
     "    tree          显示当前目录树\n"
     "    tree /usr     显示指定目录树\n"
     "    tree -a       显示所有文件\n"
     "    tree -d       只显示目录\n"
     "    tree -L 2     只显示2层"},

    {"osinfo", cmd_osinfo,
     "显示系统信息",
     "用法: osinfo\n"
     "  显示LGD-OS的详细信息。\n"
     "  信息包括：\n"
     "    - 系统版本和编译时间\n"
     "    - 主机和用户信息\n"
     "    - 系统功能特性\n"
     "    - 目录结构\n"
     "    - 使用说明\n"
     "  示例:\n"
     "    osinfo      显示系统信息"},

    {"find", cmd_find,
     "查找文件或目录",
     "用法: find [路径] [选项]...\n"
     "  在指定目录中查找文件或目录。\n"
     "  选项:\n"
     "    -name 模式    按名称查找（区分大小写）\n"
     "    -iname 模式   按名称查找（不区分大小写）\n"
     "    -type 类型    按类型查找（d=目录，f=文件）\n"
     "    -count        只显示匹配数量\n"
     "  示例:\n"
     "    find -name test.txt         查找文件\n"
     "    find /home -name *.txt      在指定目录查找\n"
     "    find -iname test            不区分大小写查找\n"
     "    find -type d                查找所有目录"},

    {"ll", cmd_ll,
     "详细列表（ls -l的别名）",
     "用法: ll [目录]\n"
     "  显示目录的详细列表。\n"
     "  等同于: ls -l\n"
     "  示例:\n"
     "    ll          当前目录详细列表\n"
     "    ll /usr     指定目录详细列表"},
     {"version", cmd_version,
    "显示系统版本信息",
    "version: 显示LGD-OS版本信息\n"
    "  用法: version\n"
    "  功能: 显示当前系统的版本号、编译时间等信息\n"
    "  示例:\n"
    "    version          显示系统版本\n"},
    {"clear", cmd_clear,
    "清空终端屏幕",
    "clear: 清空终端屏幕内容\n"
    "  用法: clear\n"
    "  功能: 清除当前终端屏幕上的所有内容，将光标移动到屏幕顶部\n"
    "  示例:\n"
    "    clear          清空屏幕\n"
    "  注意: 此命令会清除所有当前显示的内容，但不会影响系统状态\n"},
     {"calc", cmd_calc,
    "科学计算器，支持表达式计算和方程求解",
    "用法: calc [选项] <表达式>\n"
    "  科学计算器，支持表达式计算和方程求解\n"
    "\n"
    "选项:\n"
    "  -D          默认模式（数学表达式计算）\n"
    "  -E          方程模式（一元一次/二次方程）\n"
    "  -S          线性方程组模式（N元一次方程组）\n"
    "\n"
    "默认模式支持的运算:\n"
    "  + - * /     加减乘除\n"
    "  ^           幂运算 2^3=8\n"
    "  [           开方运算 2[4=2 (2次开方)\n"
    "               [4=2 (默认平方根)\n"
    "               3[8=2 (3次开方)\n"
    "  ()          括号\n"
    "\n"
    "方程模式支持的格式:\n"
    "  一元一次方程: ax + b = 0 或 ax = b\n"
    "  一元二次方程: ax^2 + bx + c = 0\n"
    "  注意: 系数需明确写出 2x^2+3x-5=0\n"
    "        支持省略系数: x^2+3x-5=0 (a=1)\n"
    "        支持负系数: -x^2+3x-5=0\n"
    "\n"
    "线性方程组模式（-S）:\n"
    "  格式: 方程1;方程2;方程3;...\n"
    "  示例: 2x1+3x2=8;4x1+x2=6\n"
    "        x1+x2+x3=6;2x1+x2+3x3=13;x1+2x2+x3=8\n"
    "  注意: 变量必须为x1,x2,x3,...形式\n"
    "        系数必须明确写出，但如果是1不需要\n"
    "        方程用分号分隔\n"
    "\n"
    "示例:\n"
    "  calc 2+3 * 4               # 表达式计算\n"
    "  calc -D 2+3 * 4            # 表达式计算\n"
    "  calc -E \"2x^2+3x-5=0\"   # 一元二次方程\n"
    "  calc -E \"4x+2=0\"        # 一元一次方程\n"
    "  calc -E \"x^2-4=0\"       # 解方程\n"
    "  calc -S \"2x1+3x2=8;4x1+x2=6\"  # 二元一次方程组"}//,
    /*{"mahjong", cmd_mahjong,
    "四川麻将游戏",
    "四川麻将（血战到底，缺一门）\n"
    "  用法: mahjong [选项]\n"
    "  选项:\n"
    "    start   开始新游戏\n"
    "    help    显示帮助信息\n"
    "    rules   显示详细游戏规则\n"
    "    quit    退出游戏\n"
    "\n"
    "游戏特色:\n"
    "纯四川麻将，只有筒、条、万\n"
    "必须定缺一门花色\n"
    "可碰、可杠、可胡，没有'吃'\n"
    "血战到底：一家胡牌后游戏继续\n"
    "支持多种胡牌牌型：\n"
    "    - 平胡、大对子、清一色\n"
    "    - 七对、龙七对、将对\n"
    "    - 门清、十八罗汉\n"
    "4人游戏（1人+3AI）\n"
    "\n"
    "示例:\n"
    "  mahjong start     开始四川麻将\n"
    "  mahjong rules     查看详细规则\n"
    "  mahjong help      查看命令帮助\n"}*/
};

int command_count = sizeof(commands) / sizeof(commands[0]);


// 解析输入字符串为令牌数组
int parse_input(char *input, int *argc, char *argv[]) {
    char *token;
    int count = 0;

    // 分割输入字符串
    token = strtok(input, " \t\n\r");
    while (token != NULL && count < MAX_ARGS - 1) {
        argv[count] = token;
        count++;
        token = strtok(NULL, " \t\n\r");
    }

    *argc = count;
    argv[count] = NULL;  // 以NULL结尾

    return count;
}



// shell_execute_command 函数
int shell_execute_command(ShellContext *ctx, const char *cmd_str, FILE *in, FILE *out) {
    int argc = 0;
    char *argv[MAX_ARGS];
    char input[MAX_INPUT_LEN];

    // 复制输入以便修改
    strncpy(input, cmd_str, MAX_INPUT_LEN - 1);
    input[MAX_INPUT_LEN - 1] = '\0';

    // 解析输入 - 修正参数传递
    parse_input(input, &argc, argv);

    if (argc == 0) {
        return 0;  // 空输入
    }

    // 查找并执行命令
    for (int i = 0; i < command_count; i++) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            if (ctx->logger) {
                LOG_INFO(ctx->logger, "执行命令: %s", commands[i].name);
            }
            return commands[i].func(ctx, argc, argv, in, out);
        }
    }

    // 命令未找到，使用非静态版本的 handle_unknown_command
    return handle_unknown_command(ctx, argv[0], argc, argv, in, out);
}
int handle_unknown_command(ShellContext *ctx, const char *cmd, int argc, char *argv[], FILE *in, FILE *out) {
    (void)ctx;  // 未使用
    (void)in;   // 未使用
    (void)argc; // 未使用
    (void)argv; // 未使用

    fprintf(out, "未知命令: '%s'\n", cmd);
    fprintf(out, "可用命令:\n");

    // 按字母顺序排序显示命令
    for (int i = 0; i < command_count; i++) {
        if (i % 4 == 0) fprintf(out, "  ");
        fprintf(out, "%-8s", commands[i].name);
        if ((i + 1) % 4 == 0 || i == command_count - 1) {
            fprintf(out, "\n");
        } else {
            fprintf(out, "  ");
        }
    }

    fprintf(out, "\n输入 'help' 获取详细帮助\n");
    return 1;
}
int execute_command(ShellContext *ctx, const char *input, FILE *in, FILE *out) {
    // 这里只需要调用 shell_execute_command
    return shell_execute_command(ctx, input, in, out);
}
// 节日彩蛋函数
void check_holiday_egg(FILE *out){
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    int month = tm->tm_mon + 1;  // tm_mon 是0-11
    int day = tm->tm_mday;
    int weekday = tm->tm_wday;  // 0=周日, 1=周一, ...
    int hour = tm->tm_hour;

    fprintf(out, "\n");  // 先换行，美观

    // 检查今天是否是特殊日期
    int holiday_found = 0;

    if (month == 1 && day == 1) {  // 元旦
        fprintf(out, "========================================\n");
        fprintf(out, "       Happy New Year 2024!\n");
        fprintf(out, "========================================\n");
        fprintf(out, "\n");
        fprintf(out, "    +--------------+\n");
        fprintf(out, "   | 新年快乐！    |\n");
        fprintf(out, "   | 恭喜发财！    |\n");
        fprintf(out, "   | 万事如意！    |\n");
        fprintf(out, "    +--------------+\n");
        fprintf(out, "\n");
        fprintf(out, "新年新气象，LGD-OS 祝您新年快乐！\n");
        fprintf(out, "========================================\n");
        holiday_found = 1;
    }

    if (month == 2 && day == 14) {  // 情人节
        fprintf(out, "========================================\n");
        fprintf(out, "       Happy Valentine's Day!\n");
        fprintf(out, "========================================\n");
        fprintf(out, "\n");
        fprintf(out, "   *****       *****\n");
        fprintf(out, "  *******     *******\n");
        fprintf(out, " *********   *********\n");
        fprintf(out, "***********************\n");
        fprintf(out, " *********************\n");
        fprintf(out, "  *******************\n");
        fprintf(out, "   *****************\n");
        fprintf(out, "    ***************\n");
        fprintf(out, "     *************\n");
        fprintf(out, "      ***********\n");
        fprintf(out, "       *********\n");
        fprintf(out, "        *******\n");
        fprintf(out, "         *****\n");
        fprintf(out, "          ***\n");
        fprintf(out, "           *\n");
        fprintf(out, "\n");
        fprintf(out, "愿您的生活充满爱与幸福\n");
        fprintf(out, "========================================\n");
        holiday_found = 1;
    }
    if (month == 3 && day == 14) {  //圆周率日
        fprintf(out, "========================================\n");
        fprintf(out, "     Happy Pi Day! (3.14)\n");
        fprintf(out, "========================================\n");
        fprintf(out, "\n");
        fprintf(out, "      _..._\n");
        fprintf(out, "    .'     '.\n");
        fprintf(out, "   /  o   o  \\\n");
        fprintf(out, "  |           |\n");
        fprintf(out, "  |     \\     |\n");
        fprintf(out, "   \\   '. .' /\n");
        fprintf(out, "    '..___.'\n");
        fprintf(out, "\n");
        fprintf(out, "圆周率π前100位：\n");
        fprintf(out, "3.1415926535 8979323846 2643383279 5028841971 6939937510\n");
        fprintf(out, "5820974944 5923078164 0628620899 8628034825 3421170679\n");
        fprintf(out, "========================================\n");
    }
    if (month == 3 && day == 8) {  // 妇女节
        fprintf(out, "========================================\n");
        fprintf(out, "       Happy Women's Day!\n");
        fprintf(out, "========================================\n");
        fprintf(out, "\n");
        fprintf(out, "     * * * * * *\n");
        fprintf(out, "   * * * * * * * *\n");
        fprintf(out, "  * * * * * * * * *\n");
        fprintf(out, " * * * * * * * * * *\n");
        fprintf(out, "* * * 节日快乐 * * *\n");
        fprintf(out, " * * * * * * * * * *\n");
        fprintf(out, "  * * * * * * * * *\n");
        fprintf(out, "   * * * * * * * *\n");
        fprintf(out, "     * * * * * *\n");
        fprintf(out, "\n");
        fprintf(out, "祝所有的女性朋友节日快乐！\n");
        fprintf(out, "========================================\n");
        holiday_found = 1;
    }

    if (month == 4 && day == 1) {  // 愚人节
        fprintf(out, "========================================\n");
        fprintf(out, "       April Fool's Day!\n");
        fprintf(out, "========================================\n");
        fprintf(out, "\n");
        fprintf(out, "   .-\"\"\"\"\"-.\n");
        fprintf(out, "  /         \\\n");
        fprintf(out, " |  O   O  |\n");
        fprintf(out, " |    >    |\n");
        fprintf(out, "  \\  ___  /\n");
        fprintf(out, "   '.___.'\n");
        fprintf(out, "\n");
        fprintf(out, "系统消息：发现外星信号...\n");
        sleep(1);
        fprintf(out, "正在接收信息...\n");
        sleep(1);
        fprintf(out, "外星人说：愚人节快乐！\n");
        fprintf(out, "========================================\n");
        holiday_found = 1;
    }

    if (month == 5 && day == 1) {  // 劳动节
        fprintf(out, "========================================\n");
        fprintf(out, "       International Workers' Day!\n");
        fprintf(out, "========================================\n");
        fprintf(out, "\n");
        fprintf(out, "      |---|\n");
        fprintf(out, "      |   |\n");
        fprintf(out, "      |   |\n");
        fprintf(out, "   ___|   |___\n");
        fprintf(out, "  |___     ___|\n");
        fprintf(out, "      |   |\n");
        fprintf(out, "      |   |\n");
        fprintf(out, "      |   |\n");
        fprintf(out, "     _|   |_\n");
        fprintf(out, "\n");
        fprintf(out, "劳动最光荣！\n");
        fprintf(out, "========================================\n");
        holiday_found = 1;
    }

    if (month == 6 && day == 1) {  // 儿童节
        fprintf(out, "========================================\n");
        fprintf(out, "       Happy Children's Day!\n");
        fprintf(out, "========================================\n");
        fprintf(out, "\n");
        fprintf(out, "     ^  ^\n");
        fprintf(out, "    ( o o )\n");
        fprintf(out, "     (   )\n");
        fprintf(out, "     |   |\n");
        fprintf(out, "    /|   |\\\n");
        fprintf(out, "   / |   | \\\n");
        fprintf(out, "     |   |\n");
        fprintf(out, "     |   |\n");
        fprintf(out, "    /     \\\n");
        fprintf(out, "   /       \\\n");
        fprintf(out, "\n");
        fprintf(out, "愿您永远保持童心！\n");
        fprintf(out, "========================================\n");
        holiday_found = 1;
    }

    if (month == 10 && day == 1) {  // 国庆节
        fprintf(out, "========================================\n");
        fprintf(out, "        National Day!\n");
        fprintf(out, "========================================\n");
        fprintf(out, "\n");
        fprintf(out, "    ****************\n");
        fprintf(out, "   ******************\n");
        fprintf(out, "  ********************\n");
        fprintf(out, "  ********************\n");
        fprintf(out, "  ********************\n");
        fprintf(out, "   ******************\n");
        fprintf(out, "    ****************\n");
        fprintf(out, "      ************\n");
        fprintf(out, "        ********\n");
        fprintf(out, "          ****\n");
        fprintf(out, "\n");
        fprintf(out, "祝福祖国繁荣昌盛！\n");
        fprintf(out, "========================================\n");
        holiday_found = 1;
    }

    if (month == 10 && day == 31) {  // 万圣节
        fprintf(out, "========================================\n");
        fprintf(out, "        Happy Halloween!\n");
        fprintf(out, "========================================\n");
        fprintf(out, "\n");
        fprintf(out, "    .---.\n");
        fprintf(out, "   /     \\\n");
        fprintf(out, "  |  o o  |\n");
        fprintf(out, "  |   ^   |\n");
        fprintf(out, "  |  '-'  |\n");
        fprintf(out, "   \\_____/\n");
        fprintf(out, "\n");
        fprintf(out, "  不给糖就捣蛋！\n");
        fprintf(out, "========================================\n");
        holiday_found = 1;
    }

    if (month == 12 && day == 25) {  // 圣诞节
        fprintf(out, "========================================\n");
        fprintf(out, "       Merry Christmas!\n");
        fprintf(out, "========================================\n");
        fprintf(out, "\n");
        fprintf(out, "       *\n");
        fprintf(out, "      / \\\n");
        fprintf(out, "     /   \\\n");
        fprintf(out, "    /     \\\n");
        fprintf(out, "   /       \\\n");
        fprintf(out, "  /         \\\n");
        fprintf(out, "  -----------\n");
        fprintf(out, "     |   |\n");
        fprintf(out, "     |   |\n");
        fprintf(out, "     |   |\n");
        fprintf(out, "    =======\n");
        fprintf(out, "\n");
        fprintf(out, "圣诞快乐！\n");
        fprintf(out, "========================================\n");
        holiday_found = 1;
    }

    if (month == 12 && day == 31) {  // 新年夜
        fprintf(out, "========================================\n");
        fprintf(out, "     Happy New Year's Eve!\n");
        fprintf(out, "========================================\n");
        fprintf(out, "\n");
        fprintf(out, "      _\\|/_\n");
        fprintf(out, "     (o o)\n");
        fprintf(out, " +----oOO-{_}-OOo----+\n");
        fprintf(out, " | 新年倒计时开始！  |\n");
        fprintf(out, " +-------------------+\n");
        fprintf(out, "\n");
        fprintf(out, " 新年快乐！\n");
        fprintf(out, "========================================\n");
        holiday_found = 1;
    }

    // 如果不是节日，根据时间显示问候
    if (!holiday_found) {
        if (hour >= 22 && hour < 23||hour >= 0 && hour < 5) {  // 凌晨
            fprintf(out, "========================================\n");
            fprintf(out, "        夜深了，注意休息！\n");
            fprintf(out, "========================================\n");
            fprintf(out, "\n");
            fprintf(out, "       z   z\n");
            fprintf(out, "        z z\n");
            fprintf(out, "         z\n");
            fprintf(out, "   Zzzzzzzzzzzzzz\n");
            fprintf(out, "\n");
            fprintf(out, "健康是革命的本钱，熬夜伤身哦！\n");
            fprintf(out, "========================================\n");
        } else if (hour >= 5 && hour < 12) {  // 早上
            fprintf(out, "========================================\n");
            fprintf(out, "        早上好！\n");
            fprintf(out, "========================================\n");
            fprintf(out, "\n");
            fprintf(out, "       \\     /\n");
            fprintf(out, "        \\   /\n");
            fprintf(out, "         \\ /\n");
            fprintf(out, "       .-----.\n");
            fprintf(out, "      | ^   ^ |\n");
            fprintf(out, "      |   o   |\n");
            fprintf(out, "      |  \\_/  |\n");
            fprintf(out, "       '-----'\n");
            fprintf(out, "\n");
            fprintf(out, "新的一天，新的开始！\n");
            fprintf(out, "========================================\n");
        } else if (hour >= 12 && hour < 18) {  // 下午
            fprintf(out, "========================================\n");
            fprintf(out, "        下午好！\n");
            fprintf(out, "========================================\n");
            fprintf(out, "\n");
            fprintf(out, "       .------.\n");
            fprintf(out, "      | ^  ^  |\n");
            fprintf(out, "      |   o   |\n");
            fprintf(out, "      |  ---  |\n");
            fprintf(out, "       '------'\n");
            fprintf(out, "\n");
            fprintf(out, "保持好心情，效率加倍！\n");
            fprintf(out, "========================================\n");
        } else {  // 晚上
            fprintf(out, "========================================\n");
            fprintf(out, "        晚上好！\n");
            fprintf(out, "========================================\n");
            fprintf(out, "\n");
            fprintf(out, "       .---.\n");
            fprintf(out, "      | ^ ^ |\n");
            fprintf(out, "      |  o  |\n");
            fprintf(out, "      |  -  |\n");
            fprintf(out, "       '---'\n");
            fprintf(out, "\n");
            fprintf(out, "放松一下，享受休闲时光！\n");
            fprintf(out, "========================================\n");
        }
    }

    fprintf(out, "\n");  // 结尾再换一行
}
// 初始化Shell
ShellContext* shell_init(void) {
    ShellContext *ctx = (ShellContext *)malloc(sizeof(ShellContext));
    if (!ctx) {
        fprintf(stderr, "错误: 内存分配失败\n");
        return NULL;
    }

    // 初始化成员
    strcpy(ctx->current_dir, "/");
    strcpy(ctx->username, "lgd");
    strcpy(ctx->hostname, "lgd-pc");
    strcpy(ctx->home_dir, "/home/lgd");
    ctx->running = 1;
    ctx->history_count = 0;
    ctx->logger = NULL;  // 初始化为NULL

    // 初始化历史记录数组
    for (int i = 0; i < MAX_HISTORY; i++) {
        ctx->history[i] = NULL;
    }

    // 可以在这里初始化日志器
    // ctx->logger = create_logger("shell.log");

    printf("LGD-OS Shell 已启动 (版本: %s)\n", LGD_OS_VERSION);
    printf("输入 'help' 获取可用命令列表\n");
    printf("输入 'exit' 退出\n");
    printf("--------------------------------\n");

    return ctx;
}

// 清理Shell
void shell_cleanup(ShellContext *ctx) {
    if (!ctx) return;

    // 释放历史记录
    for (int i = 0; i < ctx->history_count && i < MAX_HISTORY; i++) {
        if (ctx->history[i]) {
            free(ctx->history[i]);
        }
    }

    // 释放日志器
    if (ctx->logger) {
        destroy_logger(ctx->logger);  // 注意：这里应该调用 destroy_logger
    }

    free(ctx);
}
// shell.c
void shell_display_prompt(ShellContext *ctx) {
    char hostname[100] = "sse_107";
    char username[100] = "SSE_107";

    // 获取主机名和用户名
    #ifdef _WIN32
        DWORD size = sizeof(hostname);
        GetComputerNameA(hostname, &size);

        DWORD user_size = sizeof(username);
        GetUserNameA(username, &user_size);
    #endif

    // 输出带颜色的提示符
    printf("\033[32m%s@%s\033[0m:\033[34m%s\033[0m$ ", username, hostname, ctx->current_dir);
    fflush(stdout);
}


// 执行管道命令
int shell_execute_pipeline(ShellContext *ctx, const char *pipeline_cmd) {
    char cmd1[MAX_CMD_LEN];
    char cmd2[MAX_CMD_LEN];
    char *pipe_pos;

    // 查找管道符
    pipe_pos = strchr(pipeline_cmd, '|');
    if (pipe_pos == NULL) {
        // 没有管道，直接执行单条命令
        return shell_execute_command(ctx, pipeline_cmd, stdin, stdout);
    }

    // 分割命令
    int len = pipe_pos - pipeline_cmd;
    strncpy(cmd1, pipeline_cmd, len);
    cmd1[len] = '\0';

    // 跳过管道符和空格
    char *cmd2_start = pipe_pos + 1;
    while (*cmd2_start == ' ') {
        cmd2_start++;
    }
    strcpy(cmd2, cmd2_start);

    // 提取命令名（第一个单词）
    char cmd1_name[100];
    sscanf(cmd1, "%99s", cmd1_name);  // 从cmd1中读取第一个单词

    char cmd2_name[100];
    sscanf(cmd2, "%99s", cmd2_name);  // 从cmd2中读取第一个单词

    // 检查是否包含game命令
    if (strcmp(cmd1_name, "game") == 0) {
        fprintf(stderr, "错误: 游戏命令 'game' 是交互式程序，不能与管道一起使用。\n");
        fprintf(stderr, "请直接运行 'game' 命令进行游戏。\n");
        return 1;
    }

    if (strcmp(cmd2_name, "game") == 0) {
        fprintf(stderr, "错误: 游戏命令 'game' 是交互式程序，不能与管道一起使用。\n");
        fprintf(stderr, "请直接运行 'game' 命令进行游戏。\n");
        return 1;
    }
    /*if (strcmp(cmd1_name, "mahjong") == 0) {
        fprintf(stderr, "错误: 游戏命令 'mahjong' 是交互式程序，不能与管道一起使用。\n");
        fprintf(stderr, "请直接运行 'mahjong' 命令进行游戏。\n");
        return 1;
    }
    if (strcmp(cmd2_name, "mahjong") == 0) {
        fprintf(stderr, "错误: 游戏命令 'mahjong' 是交互式程序，不能与管道一起使用。\n");
        fprintf(stderr, "请直接运行 'mahjong' 命令进行游戏。\n");
        return 1;
    }*/
    // 创建临时文件
    char temp_filename[MAX_PATH_LEN];
    FILE *temp_file;

    #ifdef _WIN32
        char temp_path[MAX_PATH_LEN];
        GetTempPathA(MAX_PATH_LEN, temp_path);
        GetTempFileNameA(temp_path, "lgd", 0, temp_filename);
    #else
        strcpy(temp_filename, "/tmp/lgd_temp_XXXXXX");
        int fd = mkstemp(temp_filename);
        if (fd == -1) {
            fprintf(stderr, "错误: 无法创建临时文件\n");
            return 1;
        }
        close(fd);
    #endif

    // 执行第一个命令，输出到临时文件
    temp_file = fopen(temp_filename, "w");
    if (temp_file == NULL) {
        fprintf(stderr, "错误: 无法打开临时文件\n");
        return 1;
    }

    int status = shell_execute_command(ctx, cmd1, stdin, temp_file);
    fclose(temp_file);

    if (status != 0) {
        remove(temp_filename);
        return status;
    }

    // 执行第二个命令，从临时文件读取
    temp_file = fopen(temp_filename, "r");
    if (temp_file == NULL) {
        fprintf(stderr, "错误: 无法打开临时文件\n");
        remove(temp_filename);
        return 1;
    }

    status = shell_execute_command(ctx, cmd2, temp_file, stdout);
    fclose(temp_file);

    // 删除临时文件
    remove(temp_filename);

    return status;
}

// 运行Shell
// Shell主循环
void shell_run(ShellContext *ctx) {
    char cmd_line[MAX_CMD_LEN];
    int result;  // 添加变量来存储命令执行结果
    check_holiday_egg(stdout);
    while (1) {
        shell_display_prompt(ctx);

        if (fgets(cmd_line, MAX_CMD_LEN, stdin) == NULL) {
            break;  // EOF
        }

        // 去除换行符
        cmd_line[strcspn(cmd_line, "\n")] = '\0';

        // 空命令
        if (strlen(cmd_line) == 0) {
            continue;
        }

        // 判断是否有管道
        if (strchr(cmd_line, '|') != NULL) {
            result = shell_execute_pipeline(ctx, cmd_line);
        } else {
            result = shell_execute_command(ctx, cmd_line, stdin, stdout);
        }

        // 检查是否需要退出
        if (result == 2) {  // exit命令返回2
            printf("退出LGD-OS。\n");
            break;
        }
    }
}
int cmd_version(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out) {
    (void)ctx;  // 未使用参数
    (void)argc;
    (void)argv;
    (void)in;

    fprintf(out, "\n");
    fprintf(out, "========================================\n");
    fprintf(out, "版本：%s\n", LGD_OS_VERSION);
    fprintf(out, "========================================\n");
    fprintf(out, "\n");

    return 0;
}
int cmd_clear(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out) {
    (void)ctx;  // 未使用参数
    (void)argc;
    (void)argv;
    (void)in;
    (void)out;  // 清屏命令通常不使用文件输出

    // 跨平台清屏实现
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif

    return 0;
}
