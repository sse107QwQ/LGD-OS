// cmd_game.c
#include "declarations.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define BOARD_SIZE 9
#define MINE_COUNT 20

typedef struct {
    int is_mine;
    int is_revealed;
    int is_flagged;
    int neighbor_mines;
} Cell;

typedef struct {
    Cell board[BOARD_SIZE][BOARD_SIZE];
    int game_over;
    int game_won;
    int mines_left;
    int first_move;
} MinesweeperGame;

void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void init_game(MinesweeperGame *game) {
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++) {
            game->board[i][j].is_mine = 0;
            game->board[i][j].is_revealed = 0;
            game->board[i][j].is_flagged = 0;
            game->board[i][j].neighbor_mines = 0;
        }
    game->game_over = 0;
    game->game_won = 0;
    game->mines_left = MINE_COUNT;
    game->first_move = 1;
}

void place_mines(MinesweeperGame *game, int safe_row, int safe_col) {
    int placed = 0;
    while (placed < MINE_COUNT) {
        int r = rand() % BOARD_SIZE;
        int c = rand() % BOARD_SIZE;
        if (abs(r - safe_row) <= 1 && abs(c - safe_col) <= 1)
            continue;
        if (!game->board[r][c].is_mine) {
            game->board[r][c].is_mine = 1;
            placed++;
        }
    }
}

void calculate_neighbors(MinesweeperGame *game) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (game->board[i][j].is_mine) continue;
            int cnt = 0;
            for (int di = -1; di <= 1; di++)
                for (int dj = -1; dj <= 1; dj++) {
                    int ni = i + di, nj = j + dj;
                    if (ni >= 0 && ni < BOARD_SIZE && nj >= 0 && nj < BOARD_SIZE)
                        if (game->board[ni][nj].is_mine) cnt++;
                }
            game->board[i][j].neighbor_mines = cnt;
        }
    }
}

void flood_fill(MinesweeperGame *game, int start_r, int start_c) {
    int queue[81][2];
    int head = 0, tail = 0;
    queue[tail][0] = start_r;
    queue[tail][1] = start_c;
    tail++;

    while (head < tail) {
        int r = queue[head][0];
        int c = queue[head][1];
        head++;

        if (r < 0 || r >= BOARD_SIZE || c < 0 || c >= BOARD_SIZE)
            continue;

        Cell *cell = &game->board[r][c];
        if (cell->is_revealed || cell->is_flagged || cell->is_mine)
            continue;

        cell->is_revealed = 1;

        if (cell->neighbor_mines == 0) {
            for (int dr = -1; dr <= 1; dr++) {
                for (int dc = -1; dc <= 1; dc++) {
                    if (dr == 0 && dc == 0) continue;
                    int nr = r + dr;
                    int nc = c + dc;
                    if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE) {
                        Cell *nb = &game->board[nr][nc];
                        if (!nb->is_revealed && !nb->is_flagged && !nb->is_mine) {
                            queue[tail][0] = nr;
                            queue[tail][1] = nc;
                            tail++;
                        }
                    }
                }
            }
        }
    }
}

int reveal_cell(MinesweeperGame *game, int row, int col) {
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE)
        return 0;
    if (game->board[row][col].is_revealed || game->board[row][col].is_flagged)
        return 0;

    if (game->first_move) {
        place_mines(game, row, col);
        calculate_neighbors(game);
        game->first_move = 0;
    }

    if (game->board[row][col].is_mine) {
        game->board[row][col].is_revealed = 1;
        game->game_over = 1;
        return -1;
    }

    flood_fill(game, row, col);
    return 1;
}

int flag_cell(MinesweeperGame *game, int row, int col) {
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE)
        return 0;
    if (game->board[row][col].is_revealed)
        return 0;
    if (game->board[row][col].is_flagged) {
        game->board[row][col].is_flagged = 0;
        game->mines_left++;
    } else {
        game->board[row][col].is_flagged = 1;
        game->mines_left--;
    }
    return 1;
}

int check_win(MinesweeperGame *game) {
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            if (!game->board[i][j].is_mine && !game->board[i][j].is_revealed)
                return 0;
    return 1;
}

void print_board(MinesweeperGame *game, FILE *out) {
    clear_screen();
    fprintf(out, "\n   ");
    for (int j = 0; j < BOARD_SIZE; j++)
        fprintf(out, "%2d ", j);
    fprintf(out, "\n  +");
    for (int j = 0; j < BOARD_SIZE; j++)
        fprintf(out, "---");
    fprintf(out, "+\n");

    for (int i = 0; i < BOARD_SIZE; i++) {
        fprintf(out, "%d |", i);
        for (int j = 0; j < BOARD_SIZE; j++) {
            Cell *cell = &game->board[i][j];
            if (game->game_over && cell->is_mine)
                fprintf(out, " * ");
            else if (cell->is_flagged)
                fprintf(out, " ? ");
            else if (cell->is_revealed) {
                if (cell->is_mine)
                    fprintf(out, " * ");
                else if (cell->neighbor_mines == 0)
                    fprintf(out, "   ");
                else
                    fprintf(out, " %d ", cell->neighbor_mines);
            } else
                fprintf(out, " . ");
        }
        fprintf(out, "|\n");
    }
    fprintf(out, "  +");
    for (int j = 0; j < BOARD_SIZE; j++)
        fprintf(out, "---");
    fprintf(out, "+\n");
    fprintf(out, "剩余地雷: %d\n", game->mines_left);

    if (game->game_over)
        fprintf(out, "\n游戏结束！你踩到地雷了！\n");
    else if (game->game_won)
        fprintf(out, "\n恭喜！你赢了！\n");
}

void print_help(FILE *out) {
    fprintf(out, "\n=== 扫雷游戏帮助 ===\n");
    fprintf(out, "游戏规则：\n");
    fprintf(out, "  - %dx%d 网格，有%d个地雷\n", BOARD_SIZE, BOARD_SIZE, MINE_COUNT);
    fprintf(out, "  - 目标：找出所有地雷而不触发它们\n");
    fprintf(out, "\n操作说明：\n");
    fprintf(out, "  - 输入格式: 行 列 操作\n");
    fprintf(out, "  - 行和列范围: 0-%d\n", BOARD_SIZE-1);
    fprintf(out, "  - 操作: 1=挖开, 2=标记/取消标记\n");
    fprintf(out, "  - 示例: 4 5 1  (挖开第4行第5列的格子)\n");
    fprintf(out, "          2 3 2  (标记第2行第3列的格子)\n");
    fprintf(out, "  - 输入 'quit' 退出游戏\n");
    fprintf(out, "  - 输入 'help' 显示此帮助\n");
    fprintf(out, "\n符号说明：\n");
    fprintf(out, "  . 未翻开\n");
    fprintf(out, "  ? 标记\n");
    fprintf(out, "  * 地雷\n");
    fprintf(out, "  数字 周围地雷数\n");
    fprintf(out, "===================\n");
}

int cmd_game(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out) {
    (void)ctx; (void)argc; (void)argv;

    MinesweeperGame game;
    char input[100];
    int row, col, action, result;

    srand((unsigned int)time(NULL));
    init_game(&game);

    // 显示欢迎界面和帮助（不进入循环，先显示一次）
    clear_screen();
    fprintf(out, "\n");
    fprintf(out, "╔══════════════════════════════════════════════════════════════════╗\n");
    fprintf(out, "║                      欢迎来到扫雷游戏！                            ║\n");
    fprintf(out, "║                         %dx%d 扫雷游戏                             ║\n", BOARD_SIZE, BOARD_SIZE);
    fprintf(out, "║                         %d个地雷                                   ║\n", MINE_COUNT);
    fprintf(out, "╚══════════════════════════════════════════════════════════════════╝\n");
    print_help(out);
    fprintf(out, "\n按回车键开始游戏...");
    fflush(out);
    getchar();  // 等待用户按回车

    // 主游戏循环
    while (!game.game_over && !game.game_won) {
        print_board(&game, out);
        fprintf(out, "\n输入操作 (行 列 操作) 或输入 'help'/'quit': ");
        fflush(out);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
            fprintf(out, "\n退出游戏。\n");
            return 0;
        }
        if (strcmp(input, "help") == 0) {
            // 显示帮助，不清屏，等待用户按回车后继续
            print_help(out);
            fprintf(out, "\n按回车键继续游戏...");
            fflush(out);
            getchar();  // 消耗回车
            continue;
        }

        if (sscanf(input, "%d %d %d", &row, &col, &action) != 3) {
            fprintf(out, "错误: 输入格式不正确。使用: 行 列 操作 (例如: 4 5 1)\n");
            continue;
        }

        if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
            fprintf(out, "错误: 行和列必须在 0-%d 范围内\n", BOARD_SIZE-1);
            continue;
        }

        if (action != 1 && action != 2) {
            fprintf(out, "错误: 操作必须是 1(挖开) 或 2(标记)\n");
            continue;
        }

        if (action == 1) {
            result = reveal_cell(&game, row, col);
            if (result == -1)
                game.game_over = 1;
            else if (result == 0)
                fprintf(out, "这个位置不能挖开！\n");
        } else {
            if (!flag_cell(&game, row, col))
                fprintf(out, "这个位置不能标记！\n");
        }

        if (!game.game_over && check_win(&game))
            game.game_won = 1;
    }

    print_board(&game, out);
    if (game.game_won)
        fprintf(out, "\n恭喜！你成功完成了扫雷游戏！\n");
    else if (game.game_over)
        fprintf(out, "\n很遗憾，你踩到了地雷！\n");
    fprintf(out, "\n游戏结束。\n");
    return 0;
}
