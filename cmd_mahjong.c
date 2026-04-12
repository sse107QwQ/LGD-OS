// cmd_mahjong.c - 四川麻将（血战到底）
#include "declarations.h"
#include "mahjong.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

// 全局游戏实例
static SichuanMahjong game;

// 花色名称
const char* SUIT_NAMES[] = {"筒", "条", "万"};
const char* WIN_TYPE_NAMES[] = {"平胡", "大对子", "清一色", "将对", "门清", "七对", "龙七对", "十八罗汉"};



void print_hand(Tile *hand, int count, FILE *out) {
    for (int i = 0; i < count; i++) {
        fprintf(out, "%2d:", i + 1);
        print_tile(hand[i], out);

        // 每6张牌换行
        if ((i + 1) % 6 == 0) {
            fprintf(out, "\n");
        } else {
            fprintf(out, "  ");
        }
    }

    // 如果最后一行不满6张，补换行
    if (count % 6 != 0) {
        fprintf(out, "\n");
    }
}
// 打印手牌
void print_tile(Tile tile, FILE *out) {
    if (tile.suit == TILE_NONE || tile.number == 0) {
        fprintf(out, "[无]");
        return;
    }

    // 检查花色和数字是否有效
    if (tile.suit < 0 || tile.suit >= 3) {
        fprintf(out, "[无效]");
        return;
    }

    if (tile.number < 1 || tile.number > 9) {
        fprintf(out, "[错误]");
        return;
    }

    fprintf(out, "%d%s", tile.number, SUIT_NAMES[tile.suit]);
}

// 排序手牌（按花色和数字）
void sort_hand(Tile *hand, int count) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (hand[j].suit > hand[j + 1].suit ||
                (hand[j].suit == hand[j + 1].suit && hand[j].number > hand[j + 1].number)) {
                Tile temp = hand[j];
                hand[j] = hand[j + 1];
                hand[j + 1] = temp;
            }
        }
    }
}

// 初始化牌墙
void init_wall() {
    int tile_id = 0;

    // 筒 (1-9, 每个4张)
    for (int num = 1; num <= 9; num++) {
        for (int copy = 0; copy < 4; copy++) {
            game.wall[tile_id].suit = TILE_TONG;
            game.wall[tile_id].number = num;
            game.wall[tile_id].id = tile_id;
            tile_id++;
        }
    }

    // 条 (1-9, 每个4张)
    for (int num = 1; num <= 9; num++) {
        for (int copy = 0; copy < 4; copy++) {
            game.wall[tile_id].suit = TILE_TIAO;
            game.wall[tile_id].number = num;
            game.wall[tile_id].id = tile_id;
            tile_id++;
        }
    }

    // 万 (1-9, 每个4张)
    for (int num = 1; num <= 9; num++) {
        for (int copy = 0; copy < 4; copy++) {
            game.wall[tile_id].suit = TILE_WAN;
            game.wall[tile_id].number = num;
            game.wall[tile_id].id = tile_id;
            tile_id++;
        }
    }

    game.wall_count = tile_id;
}

// 洗牌
void shuffle_wall() {
    srand((unsigned int)time(NULL));
    for (int i = 0; i < game.wall_count; i++) {
        int j = rand() % game.wall_count;
        Tile temp = game.wall[i];
        game.wall[i] = game.wall[j];
        game.wall[j] = temp;
    }
}

// 发牌
void deal_tiles() {
    int tile_index = 0;

    // 给每个玩家发牌
    for (int i = 0; i < PLAYER_COUNT; i++) {
        Player *player = &game.players[i];
        player->hand_count = 0;
        player->pg_count = 0;
        player->score = 0;
        player->is_hu = 0;
        player->que_set = 0;
        player->is_ai = (i != 0);  // 只有玩家1是真人

        // 每个玩家发13张牌
        for (int j = 0; j < HAND_SIZE; j++) {
            if (tile_index < 108) {
                player->hand[player->hand_count] = game.wall[tile_index];
                player->hand_count++;
                tile_index++;
            }
        }

        // 排序手牌
        sort_hand(player->hand, player->hand_count);
    }

    // 庄家多摸一张牌
    Player *banker = &game.players[game.banker];
    if (tile_index < 108) {
        banker->hand[banker->hand_count] = game.wall[tile_index];
        banker->hand_count++;
        tile_index++;
        sort_hand(banker->hand, banker->hand_count);
    }

    // 剩下的牌放在牌墙
    game.wall_count = 108 - tile_index;
    for (int i = 0; i < game.wall_count; i++) {
        game.wall[i] = game.wall[tile_index + i];
    }
}

// 定缺
void set_que_suit(Player *player, int player_index, FILE *in, FILE *out) {
    if (player->is_ai) {
        // AI定缺策略：选择数量最少的花色
        int counts[3] = {0, 0, 0};

        for (int i = 0; i < player->hand_count; i++) {
            if (player->hand[i].suit == TILE_TONG) counts[0]++;
            else if (player->hand[i].suit == TILE_TIAO) counts[1]++;
            else if (player->hand[i].suit == TILE_WAN) counts[2]++;
        }

        TileSuit que_suit = TILE_TONG;
        int min_count = counts[0];

        if (counts[1] < min_count) {
            min_count = counts[1];
            que_suit = TILE_TIAO;
        }
        if (counts[2] < min_count) {
            que_suit = TILE_WAN;
        }

        player->que_suit = que_suit;
        player->que_set = 1;

        fprintf(out, "玩家%d 定缺: %s\n", player_index + 1, SUIT_NAMES[que_suit]);
    } else {
        // 玩家定缺
        int valid = 0;
        char input[100];

        while (!valid) {
            fprintf(out, "\n请选择要缺的花色:\n");
            fprintf(out, "1. 筒\n");
            fprintf(out, "2. 条\n");
            fprintf(out, "3. 万\n");
            fprintf(out, "选择(1-3): ");

            fgets(input, sizeof(input), in);
            int choice = atoi(input);

            if (choice >= 1 && choice <= 3) {
                player->que_suit = choice - 1;  // 0:筒, 1:条, 2:万
                player->que_set = 1;
                fprintf(out, "您已定缺: %s\n", SUIT_NAMES[choice - 1]);
                valid = 1;
            } else {
                fprintf(out, "无效选择，请重新输入！\n");
            }
        }
    }
}

// 检查是否可碰
int can_peng(Player *player, Tile tile) {
    if (player->is_hu) return 0;  // 已胡牌不能碰

    int count = 0;
    for (int i = 0; i < player->hand_count; i++) {
        if (player->hand[i].suit == tile.suit &&
            player->hand[i].number == tile.number) {
            count++;
        }
    }
    return count >= 2;
}

// 检查是否可杠
int can_gang(Player *player, Tile tile, int *gang_type) {
    if (player->is_hu) return 0;  // 已胡牌不能杠

    // 检查明杠（别人打出的牌）
    int count = 0;
    for (int i = 0; i < player->hand_count; i++) {
        if (player->hand[i].suit == tile.suit &&
            player->hand[i].number == tile.number) {
            count++;
        }
    }

    if (count == 3) {
        if (gang_type) *gang_type = 1;  // 明杠
        return 1;
    }

    // 检查巴杠（已碰的牌又摸到第四张）
    for (int i = 0; i < player->pg_count; i++) {
        if (player->pg_records[i].type == 0 &&  // 碰
            player->pg_records[i].tile.suit == tile.suit &&
            player->pg_records[i].tile.number == tile.number) {
            for (int j = 0; j < player->hand_count; j++) {
                if (player->hand[j].suit == tile.suit &&
                    player->hand[j].number == tile.number) {
                    if (gang_type) *gang_type = 2;  // 巴杠
                    return 1;
                }
            }
        }
    }

    // 检查暗杠（手牌中有4张相同的牌）
    for (int i = 0; i <= player->hand_count - 4; i++) {
        if (player->hand[i].suit == tile.suit &&
            player->hand[i].number == tile.number &&
            player->hand[i+1].suit == tile.suit &&
            player->hand[i+1].number == tile.number &&
            player->hand[i+2].suit == tile.suit &&
            player->hand[i+2].number == tile.number &&
            player->hand[i+3].suit == tile.suit &&
            player->hand[i+3].number == tile.number) {
            if (gang_type) *gang_type = 3;  // 暗杠
            return 1;
        }
    }

    return 0;
}

// 检查是否可胡
int can_hu(Player *player, Tile tile, int is_zimo) {
    if (player->is_hu) return 0;  // 已胡牌不能再胡

    // 创建临时手牌数组
    Tile temp_hand[20];
    int temp_count = player->hand_count;

    for (int i = 0; i < player->hand_count; i++) {
        temp_hand[i] = player->hand[i];
    }

    // 如果是点炮胡，需要添加这张牌
    if (!is_zimo) {
        temp_hand[temp_count++] = tile;
        sort_hand(temp_hand, temp_count);
    }

    // 检查七对
    if (check_qidui(temp_hand, temp_count)) {
        return 1;
    }

    // 检查普通胡牌（4面子+1对子）
    if (check_normal_hu(temp_hand, temp_count)) {
        return 1;
    }

    return 0;
}

// 检查七对
int check_qidui(Tile *hand, int count) {
    if (count != 14) return 0;

    int pair_count = 0;
    for (int i = 0; i < count; i += 2) {
        if (i + 1 < count) {
            if (hand[i].suit == hand[i+1].suit &&
                hand[i].number == hand[i+1].number) {
                pair_count++;
            }
        }
    }

    if (pair_count == 7) {
        return 1;  // 七对
    }

    return 0;
}

// 检查普通胡牌
int check_normal_hu(Tile *hand, int count) {
    if (count != 14) return 0;

    int jiang_pos = -1;

    // 寻找将牌
    for (int i = 0; i < count - 1; i++) {
        if (hand[i].suit == hand[i+1].suit &&
            hand[i].number == hand[i+1].number) {
            // 尝试用这对牌做将
            Tile temp[20];
            int temp_count = 0;
            for (int j = 0; j < count; j++) {
                if (j != i && j != i+1) {
                    temp[temp_count++] = hand[j];
                }
            }

            // 检查剩下的12张牌是否能组成4个面子
            if (check_mianzi(temp, temp_count)) {
                return 1;
            }
        }
    }

    return 0;
}

// 检查是否能组成面子
int check_mianzi(Tile *hand, int count) {
    if (count == 0) return 1;

    // 尝试找刻子
    for (int i = 0; i <= count - 3; i++) {
        if (hand[i].suit == hand[i+1].suit && hand[i].suit == hand[i+2].suit &&
            hand[i].number == hand[i+1].number && hand[i].number == hand[i+2].number) {
            // 移除这3张牌
            Tile temp[20];
            int temp_count = 0;
            for (int j = 0; j < count; j++) {
                if (j < i || j > i+2) {
                    temp[temp_count++] = hand[j];
                }
            }
            if (check_mianzi(temp, temp_count)) {
                return 1;
            }
        }
    }

    // 尝试找顺子
    for (int i = 0; i <= count - 3; i++) {
        for (int j = i+1; j < count; j++) {
            for (int k = j+1; k < count; k++) {
                if (hand[i].suit == hand[j].suit && hand[i].suit == hand[k].suit) {
                    int nums[3] = {hand[i].number, hand[j].number, hand[k].number};
                    // 排序
                    for (int a = 0; a < 2; a++) {
                        for (int b = 0; b < 2 - a; b++) {
                            if (nums[b] > nums[b+1]) {
                                int temp_num = nums[b];
                                nums[b] = nums[b+1];
                                nums[b+1] = temp_num;
                            }
                        }
                    }
                    if (nums[1] == nums[0] + 1 && nums[2] == nums[1] + 1) {
                        // 移除这3张牌
                        Tile temp[20];
                        int temp_count = 0;
                        for (int l = 0; l < count; l++) {
                            if (l != i && l != j && l != k) {
                                temp[temp_count++] = hand[l];
                            }
                        }
                        if (check_mianzi(temp, temp_count)) {
                            return 1;
                        }
                    }
                }
            }
        }
    }

    return 0;
}

// 判断牌型
WinType get_win_type(Player *player, Tile *hand, int count, int is_zimo) {
    // 检查清一色
    int is_qingyise = 1;
    TileSuit first_suit = hand[0].suit;
    for (int i = 1; i < count; i++) {
        if (hand[i].suit != first_suit) {
            is_qingyise = 0;
            break;
        }
    }

    // 检查大对子
    int is_dadui = 0;
    Tile sorted[20];
    for (int i = 0; i < count; i++) sorted[i] = hand[i];
    sort_hand(sorted, count);

    if (check_normal_hu(hand, count)) {
        // 检查是否全是刻子
        int i = 0;
        is_dadui = 1;
        while (i < count) {
            if (i + 2 < count && sorted[i].suit == sorted[i+1].suit &&
                sorted[i].suit == sorted[i+2].suit &&
                sorted[i].number == sorted[i+1].number &&
                sorted[i].number == sorted[i+2].number) {
                i += 3;  // 刻子
            } else if (sorted[i].suit == sorted[i+1].suit &&
                      sorted[i].number == sorted[i+1].number) {
                i += 2;  // 将牌
            } else {
                is_dadui = 0;
                break;
            }
        }
    }

    // 检查七对
    int is_qidui = check_qidui(hand, count);

    // 检查龙七对
    int is_longqidui = 0;
    if (is_qidui) {
        // 检查是否有4张相同的牌
        for (int i = 0; i < count; i++) {
            int same_count = 0;
            for (int j = 0; j < count; j++) {
                if (sorted[i].suit == sorted[j].suit &&
                    sorted[i].number == sorted[j].number) {
                    same_count++;
                }
            }
            if (same_count == 4) {
                is_longqidui = 1;
                break;
            }
        }
    }

    // 检查将对
    int is_jiangdui = 1;
    for (int i = 0; i < count; i++) {
        if (sorted[i].number != 2 && sorted[i].number != 5 && sorted[i].number != 8) {
            is_jiangdui = 0;
            break;
        }
    }

    // 检查门清
    int is_menqing = (player->pg_count == 0);

    if (is_qingyise && is_dadui) return WIN_QINGYISE;
    if (is_qingyise && is_qidui) return WIN_QINGYISE;
    if (is_qingyise) return WIN_QINGYISE;
    if (is_longqidui) return WIN_LONGQIDUI;
    if (is_dadui) return WIN_DADUI;
    if (is_jiangdui) return WIN_JIANGDUI;
    if (is_menqing) return WIN_MENQING;
    if (is_qidui) return WIN_QIDUI;

    return WIN_PINGHU;
}

// 计算番数
int calculate_fan(WinType win_type, int is_zimo, int is_gangshang, int is_haidi) {
    int fan = 1;  // 基础番

    switch (win_type) {
        case WIN_PINGHU: fan = 1; break;
        case WIN_DADUI: fan = 2; break;
        case WIN_QINGYISE: fan = 4; break;
        case WIN_JIANGDUI: fan = 3; break;
        case WIN_MENQING: fan = 2; break;
        case WIN_QIDUI: fan = 4; break;
        case WIN_LONGQIDUI: fan = 8; break;
        case WIN_SHIBALUOHAN: fan = 16; break;
        default: fan = 1;
    }

    // 自摸加番
    if (is_zimo) fan++;

    // 杠上开花
    if (is_gangshang) fan++;

    // 海底捞月
    if (is_haidi) fan++;

    return fan;
}

// 显示游戏状态
void show_game_state(int player_index, FILE *out) {
    fprintf(out, "\n");
    fprintf(out, "========== 四川麻将 - 血战到底 ==========\n");
    fprintf(out, "当前局数: 第%d局  庄家: 玩家%d\n", game.round + 1, game.banker + 1);
    fprintf(out, "剩余牌数: %d张\n", game.wall_count);
    fprintf(out, "当前玩家: 玩家%d\n", player_index + 1);
    fprintf(out, "\n");

    // 显示自己
    fprintf(out, "=== 您的手牌 ===\n");
    print_hand(game.players[0].hand, game.players[0].hand_count, out);

    if (game.players[0].que_set) {
        fprintf(out, "定缺花色: %s\n", SUIT_NAMES[game.players[0].que_suit]);
    }

    if (game.players[0].pg_count > 0) {
        fprintf(out, "碰杠牌组: ");
        for (int i = 0; i < game.players[0].pg_count; i++) {
            if (game.players[0].pg_records[i].type == 0) {
                fprintf(out, "碰:");
            } else if (game.players[0].pg_records[i].type == 1) {
                fprintf(out, "明杠:");
            } else if (game.players[0].pg_records[i].type == 2) {
                fprintf(out, "巴杠:");
            } else {
                fprintf(out, "暗杠:");
            }
            print_tile(game.players[0].pg_records[i].tile, out);
            fprintf(out, " ");
        }
        fprintf(out, "\n");
    }

    // 显示其他玩家
    fprintf(out, "\n=== 其他玩家 ===\n");
    for (int i = 1; i < PLAYER_COUNT; i++) {
        if (game.players[i].is_hu) {
            fprintf(out, "玩家%d: 已胡牌", i + 1);
        } else {
            fprintf(out, "玩家%d: %d张牌", i + 1, game.players[i].hand_count);
        }

        if (game.players[i].pg_count > 0) {
            fprintf(out, " (碰杠%d组)", game.players[i].pg_count);
        }

        if (game.players[i].que_set) {
            fprintf(out, " 缺:%s", SUIT_NAMES[game.players[i].que_suit]);
        }

        fprintf(out, "\n");
    }

    // 显示最后打出的牌 - 修复这里
    if (game.last_discard.suit != TILE_NONE && game.last_discard.number != 0) {
        fprintf(out, "\n最后打出的牌: ");
        print_tile(game.last_discard, out);
        fprintf(out, " (玩家%d)\n", game.last_discard_player + 1);
    } else {
        fprintf(out, "\n最后打出的牌: 无\n");
    }
}

// 执行碰
void do_peng(int player_index, Tile tile, int from_player) {
    Player *player = &game.players[player_index];

    // 移除手牌中的两张相同牌
    int remove_count = 0;
    for (int i = 0; i < player->hand_count && remove_count < 2; i++) {
        if (player->hand[i].suit == tile.suit &&
            player->hand[i].number == tile.number) {
            // 移除这张牌
            for (int j = i; j < player->hand_count - 1; j++) {
                player->hand[j] = player->hand[j + 1];
            }
            player->hand_count--;
            i--;
            remove_count++;
        }
    }

    // 添加碰记录
    player->pg_records[player->pg_count].type = 0;  // 碰
    player->pg_records[player->pg_count].tile = tile;
    player->pg_records[player->pg_count].from_player = from_player;
    player->pg_count++;
}

// 执行杠
void do_gang(int player_index, Tile tile, int gang_type, int from_player) {
    Player *player = &game.players[player_index];

    if (gang_type == 1) {  // 明杠
        // 移除手牌中的三张相同牌
        int remove_count = 0;
        for (int i = 0; i < player->hand_count && remove_count < 3; i++) {
            if (player->hand[i].suit == tile.suit &&
                player->hand[i].number == tile.number) {
                for (int j = i; j < player->hand_count - 1; j++) {
                    player->hand[j] = player->hand[j + 1];
                }
                player->hand_count--;
                i--;
                remove_count++;
            }
        }
    } else if (gang_type == 2) {  // 巴杠
        // 手牌中有一张，碰牌组中有三张
        // 移除手牌中的一张
        for (int i = 0; i < player->hand_count; i++) {
            if (player->hand[i].suit == tile.suit &&
                player->hand[i].number == tile.number) {
                for (int j = i; j < player->hand_count - 1; j++) {
                    player->hand[j] = player->hand[j + 1];
                }
                player->hand_count--;
                break;
            }
        }

        // 将碰改为杠
        for (int i = 0; i < player->pg_count; i++) {
            if (player->pg_records[i].type == 0 &&  // 原来是碰
                player->pg_records[i].tile.suit == tile.suit &&
                player->pg_records[i].tile.number == tile.number) {
                player->pg_records[i].type = 2;  // 改为巴杠
                break;
            }
        }
    } else if (gang_type == 3) {  // 暗杠
        // 移除手牌中的四张相同牌
        int remove_count = 0;
        for (int i = 0; i < player->hand_count && remove_count < 4; i++) {
            if (player->hand[i].suit == tile.suit &&
                player->hand[i].number == tile.number) {
                for (int j = i; j < player->hand_count - 1; j++) {
                    player->hand[j] = player->hand[j + 1];
                }
                player->hand_count--;
                i--;
                remove_count++;
            }
        }

        // 添加暗杠记录
        player->pg_records[player->pg_count].type = 3;  // 暗杠
        player->pg_records[player->pg_count].tile = tile;
        player->pg_records[player->pg_count].from_player = -1;  // 暗杠
        player->pg_count++;
    }
}

// 摸牌
Tile draw_card() {
    if (game.wall_count <= 0) {
        Tile empty = {TILE_NONE, 0, -1};
        return empty;
    }

    Tile card = game.wall[0];
    for (int i = 0; i < game.wall_count - 1; i++) {
        game.wall[i] = game.wall[i + 1];
    }
    game.wall_count--;

    return card;
}

// 检查定缺
int check_que_suit(Player *player) {
    for (int i = 0; i < player->hand_count; i++) {
        if (player->hand[i].suit == player->que_suit) {
            return 0;  // 还有缺门花色的牌
        }
    }
    return 1;  // 已打光缺门
}

// AI自动出牌
// AI自动出牌
Tile ai_discard(int player_index, FILE *out) {
    Player *player = &game.players[player_index];


    /*fprintf(out, "\n玩家%d 手牌: ", player_index + 1);
    for (int i = 0; i < player->hand_count; i++) {
        print_tile(player->hand[i], out);
        fprintf(out, " ");
    }
    fprintf(out, "\n");*/
    fprintf(out, "玩家%d 定缺: %s\n", player_index + 1, SUIT_NAMES[player->que_suit]);

    // 简单AI策略
    // 1. 如果有缺门花色的牌，优先打出
    for (int i = 0; i < player->hand_count; i++) {
        if (player->hand[i].suit == player->que_suit) {
            Tile discard = player->hand[i];
            // 从手牌中移除
            for (int j = i; j < player->hand_count - 1; j++) {
                player->hand[j] = player->hand[j + 1];
            }
            player->hand_count--;

            fprintf(out, "玩家%d 打出: ", player_index + 1);
            print_tile(discard, out);
            fprintf(out, "\n");
            return discard;
        }
    }

    // 2. 否则打出最孤立的牌
    int best_index = 0;
    int best_value = 100;

    for (int i = 0; i < player->hand_count; i++) {
        int value = 0;
        Tile current = player->hand[i];

        // 计算孤立程度
        for (int j = 0; j < player->hand_count; j++) {
            if (i != j) {
                Tile other = player->hand[j];
                if (current.suit == other.suit) {
                    int diff = abs(current.number - other.number);
                    if (diff <= 2) {
                        value -= (3 - diff);  // 相邻牌减少孤立值
                    }
                }
            }
        }

        if (value < best_value) {
            best_value = value;
            best_index = i;
        }
    }

    Tile discard = player->hand[best_index];
    // 从手牌中移除
    for (int j = best_index; j < player->hand_count - 1; j++) {
        player->hand[j] = player->hand[j + 1];
    }
    player->hand_count--;

    fprintf(out, "玩家%d 打出: ", player_index + 1);
    print_tile(discard, out);
    fprintf(out, "\n");
    return discard;
}

// 玩家回合
int player_turn(int player_index, FILE *in, FILE *out) {
    Player *player = &game.players[player_index];
    Tile discard;
    if (player->is_hu) {
        fprintf(out, "玩家%d 已胡牌，跳过回合\n", player_index + 1);
        return 0;
    }

    // 摸牌
    Tile drawn = draw_card();
    if (drawn.suit == TILE_NONE) {
        fprintf(out, "牌墙已空，流局！\n");
        game.game_state = 1;
        return 0;
    }

    player->hand[player->hand_count++] = drawn;
    sort_hand(player->hand, player->hand_count);

    // 在player_turn函数中，修改玩家出牌部分
    if (player_index == 0) {  // 玩家
        int valid = 0;
        char input[100];

        while (!valid) {
            fprintf(out, "\n请选择要打出的牌（输入序号）: \n");
            print_hand(player->hand, player->hand_count, out);
            fprintf(out, "选择: ");

            fgets(input, sizeof(input), in);
            int choice = atoi(input) - 1;

            if (choice >= 0 && choice < player->hand_count) {
                // 检查定缺规则
                int has_que = 0;
                for (int i = 0; i < player->hand_count; i++) {
                    if (player->hand[i].suit == player->que_suit) {
                        has_que = 1;
                        break;
                    }
                }

                if (has_que && player->hand[choice].suit != player->que_suit) {
                    fprintf(out, "您还有%s，必须优先打出缺门花色的牌！\n",
                    SUIT_NAMES[player->que_suit]);
                    continue;
                }

                discard = player->hand[choice];
                // 从手牌中移除
                for (int j = choice; j < player->hand_count - 1; j++) {
                    player->hand[j] = player->hand[j + 1];
                }
                player->hand_count--;
                valid = 1;

                fprintf(out, "您打出: ");
                print_tile(discard, out);
                fprintf(out, "\n");
            } else {
                fprintf(out, "无效选择！请输入1-%d之间的数字。\n", player->hand_count);
            }
        }
    }

    // 检查是否可胡（自摸）
    if (can_hu(player, drawn, 1)) {
        if (player_index == 0) {
            fprintf(out, "您自摸了！是否胡牌？(y/n): ");
            char input[10];
            fgets(input, sizeof(input), in);
            if (input[0] == 'y' || input[0] == 'Y') {
                // 胡牌
                game.hu_players[game.hu_count].hu_player = player_index;
                game.hu_players[game.hu_count].dianpao_player = -1;  // 自摸
                game.hu_players[game.hu_count].win_type = get_win_type(player, player->hand, player->hand_count, 1);
                game.hu_players[game.hu_count].fan = calculate_fan(game.hu_players[game.hu_count].win_type, 1, 0, 0);
                game.hu_count++;
                player->is_hu = 1;

                fprintf(out, "\n恭喜！您自摸胡牌！\n");
                fprintf(out, "胡牌牌型: %s  %d番\n",
                       WIN_TYPE_NAMES[game.hu_players[game.hu_count-1].win_type],
                       game.hu_players[game.hu_count-1].fan);
                return 1;
            }
        } else {
            // AI自动胡牌
            fprintf(out, "玩家%d 自摸胡牌！\n", player_index + 1);
            game.hu_players[game.hu_count].hu_player = player_index;
            game.hu_players[game.hu_count].dianpao_player = -1;
            game.hu_players[game.hu_count].win_type = get_win_type(player, player->hand, player->hand_count, 1);
            game.hu_players[game.hu_count].fan = calculate_fan(game.hu_players[game.hu_count].win_type, 1, 0, 0);
            game.hu_count++;
            player->is_hu = 1;
            return 1;
        }
    }

    // 检查是否可杠
    int gang_type = 0;
    if (can_gang(player, drawn, &gang_type)) {
        if (player_index == 0) {
            fprintf(out, "您可以杠这张牌，是否杠牌？(y/n): ");
            char input[10];
            fgets(input, sizeof(input), in);
            if (input[0] == 'y' || input[0] == 'Y') {
                do_gang(player_index, drawn, gang_type, -1);
                fprintf(out, "您杠牌成功！\n");
                return 0;  // 杠牌后继续
            }
        } else {
            // AI自动杠牌
            do_gang(player_index, drawn, gang_type, -1);
            fprintf(out, "玩家%d 杠牌！\n", player_index + 1);
            return 0;  // 杠牌后继续
        }
    }

    // 出牌
    if (player_index == 0) {  // 玩家
        int valid = 0;
        char input[100];

        while (!valid) {
            fprintf(out, "\n请选择要打出的牌（输入序号）: ");
            print_hand(player->hand, player->hand_count, out);
            fprintf(out, "选择: ");

            fgets(input, sizeof(input), in);
            int choice = atoi(input) - 1;

            if (choice >= 0 && choice < player->hand_count) {
                // 检查是否已定缺
                if (!player->que_set) {
                    fprintf(out, "请先定缺！\n");
                    set_que_suit(player, player_index, in, out);
                } else {
                    // 检查是否还有缺门花色的牌
                    int has_que = 0;
                    for (int i = 0; i < player->hand_count; i++) {
                        if (player->hand[i].suit == player->que_suit) {
                            has_que = 1;
                            break;
                        }
                    }

                    if (has_que && player->hand[choice].suit != player->que_suit) {
                        fprintf(out, "您还有%s，必须优先打出缺门花色的牌！\n", SUIT_NAMES[player->que_suit]);
                        continue;
                    }
                }

                discard = player->hand[choice];
                // 从手牌中移除
                for (int j = choice; j < player->hand_count - 1; j++) {
                    player->hand[j] = player->hand[j + 1];
                }
                player->hand_count--;
                valid = 1;

                fprintf(out, "您打出: ");
                print_tile(discard, out);
                fprintf(out, "\n");
            } else {
                fprintf(out, "无效选择！\n");
            }
        }
    } else {  // AI
        discard = ai_discard(player_index, out);
    }

    // 记录最后打出的牌
    game.last_discard = discard;
    game.last_discard_player = player_index;

    // 检查其他玩家是否可以碰、杠、胡
    for (int i = 0; i < PLAYER_COUNT; i++) {
        if (i == player_index || game.players[i].is_hu) continue;

        Player *other = &game.players[i];

        // 检查胡牌
        if (can_hu(other, discard, 0)) {
            if (i == 0) {  // 玩家
                fprintf(out, "您可以胡这张牌！是否胡牌？(y/n): ");
                char input[10];
                fgets(input, sizeof(input), in);
                if (input[0] == 'y' || input[0] == 'Y') {
                    // 胡牌
                    game.hu_players[game.hu_count].hu_player = i;
                    game.hu_players[game.hu_count].dianpao_player = player_index;
                    game.hu_players[game.hu_count].win_type = get_win_type(other, other->hand, other->hand_count, 0);
                    game.hu_players[game.hu_count].fan = calculate_fan(game.hu_players[game.hu_count].win_type, 0, 0, 0);
                    game.hu_count++;
                    other->is_hu = 1;

                    fprintf(out, "\n恭喜！您胡牌了！\n");
                    fprintf(out, "胡牌牌型: %s  %d番\n",
                           WIN_TYPE_NAMES[game.hu_players[game.hu_count-1].win_type],
                           game.hu_players[game.hu_count-1].fan);
                    fprintf(out, "点炮者: 玩家%d\n", player_index + 1);
                    return 1;
                }
            } else {  // AI
                fprintf(out, "玩家%d 胡牌！\n", i + 1);
                game.hu_players[game.hu_count].hu_player = i;
                game.hu_players[game.hu_count].dianpao_player = player_index;
                game.hu_players[game.hu_count].win_type = get_win_type(other, other->hand, other->hand_count, 0);
                game.hu_players[game.hu_count].fan = calculate_fan(game.hu_players[game.hu_count].win_type, 0, 0, 0);
                game.hu_count++;
                other->is_hu = 1;
                return 1;
            }
        }

        // 检查杠
        int gang_type = 0;
        if (can_gang(other, discard, &gang_type)) {
            if (i == 0) {  // 玩家
                fprintf(out, "您可以杠这张牌，是否杠牌？(y/n): ");
                char input[10];
                fgets(input, sizeof(input), in);
                if (input[0] == 'y' || input[0] == 'Y') {
                    do_gang(i, discard, gang_type, player_index);
                    fprintf(out, "您杠牌成功！\n");
                    game.current_player = i;  // 杠牌者继续
                    return 0;
                }
            } else {  // AI
                do_gang(i, discard, gang_type, player_index);
                fprintf(out, "玩家%d 杠牌！\n", i + 1);
                game.current_player = i;  // 杠牌者继续
                return 0;
            }
        }

        // 检查碰
        if (can_peng(other, discard)) {
            if (i == 0) {  // 玩家
                fprintf(out, "您可以碰这张牌，是否碰牌？(y/n): ");
                char input[10];
                fgets(input, sizeof(input), in);
                if (input[0] == 'y' || input[0] == 'Y') {
                    do_peng(i, discard, player_index);
                    fprintf(out, "您碰牌成功！\n");
                    game.current_player = i;  // 碰牌者继续
                    return 0;
                }
            } else {  // AI
                do_peng(i, discard, player_index);
                fprintf(out, "玩家%d 碰牌！\n", i + 1);
                game.current_player = i;  // 碰牌者继续
                return 0;
            }
        }
    }

    return 0;
}

// 计分
void calculate_score() {
    fprintf(stdout, "\n========== 本局结算 ==========\n");

    for (int i = 0; i < game.hu_count; i++) {
        HuInfo *hu = &game.hu_players[i];
        Player *hu_player = &game.players[hu->hu_player];

        fprintf(stdout, "玩家%d ", hu->hu_player + 1);
        if (hu->dianpao_player == -1) {
            fprintf(stdout, "自摸 ");
        } else {
            fprintf(stdout, "胡玩家%d的牌 ", hu->dianpao_player + 1);
        }
        fprintf(stdout, "%s %d番\n", WIN_TYPE_NAMES[hu->win_type], hu->fan);

        // 计算得分
        int base_score = hu->fan * 2;  // 每番2分
        if (hu->dianpao_player == -1) {  // 自摸
            for (int j = 0; j < PLAYER_COUNT; j++) {
                if (j != hu->hu_player && !game.players[j].is_hu) {
                    hu_player->score += base_score;
                    game.players[j].score -= base_score;
                }
            }
        } else {  // 点炮
            hu_player->score += base_score * 3;  // 点炮者付3倍
            game.players[hu->dianpao_player].score -= base_score * 3;
        }
    }

    // 查叫（未胡牌玩家要给已胡牌玩家付基本分）
    for (int i = 0; i < PLAYER_COUNT; i++) {
        if (!game.players[i].is_hu) {
            for (int j = 0; j < PLAYER_COUNT; j++) {
                if (game.players[j].is_hu) {
                    game.players[i].score -= 2;  // 基本分2分
                    game.players[j].score += 2;
                }
            }
        }
    }

    fprintf(stdout, "\n====== 最终得分 ======\n");
    for (int i = 0; i < PLAYER_COUNT; i++) {
        fprintf(stdout, "玩家%d: %d分\n", i + 1, game.players[i].score);
    }
}

// 主游戏循环
void play_sichuan_mahjong(FILE *in, FILE *out) {
    fprintf(out, "========== 四川麻将 - 血战到底 ==========\n");
    fprintf(out, "游戏规则：\n");
    fprintf(out, "1. 只有筒、条、万三种花色\n");
    fprintf(out, "2. 必须定缺一门花色\n");
    fprintf(out, "3. 可碰、可杠、可胡\n");
    fprintf(out, "4. 没有'吃'操作\n");
    fprintf(out, "5. 血战到底：一家胡牌后游戏继续\n");
    fprintf(out, "6. 查叫：未胡牌玩家要付分\n");
    fprintf(out, "\n按回车键开始游戏...");
    getchar();

    // 初始化游戏
    memset(&game, 0, sizeof(game));
    init_wall();
    shuffle_wall();
    deal_tiles();

    game.banker = 0;  // 玩家1是庄家
    game.current_player = 0;
    game.round = 0;
    game.game_state = 0;

    // 初始化 last_discard
    game.last_discard.suit = TILE_NONE;  // 修复：初始化为无牌
    game.last_discard.number = 0;
    game.last_discard.id = -1;

    // 设置玩家
    for (int i = 0; i < PLAYER_COUNT; i++) {
        game.players[i].is_ai = (i != 0);
        // 手牌数量已经在 deal_tiles 中设置
    }

    // 定缺 - 让玩家先看牌
    fprintf(out, "\n====== 定缺阶段 ======\n");

    // 先显示玩家的手牌
    fprintf(out, "\n您的手牌：\n");
    print_hand(game.players[0].hand, game.players[0].hand_count, out);

    // 玩家定缺
    set_que_suit(&game.players[0], 0, in, out);

    // AI定缺
    for (int i = 1; i < PLAYER_COUNT; i++) {
        set_que_suit(&game.players[i], i, in, out);
    }

    // 游戏主循环
    int hu_count = 0;
    int is_banker_first_turn = 1;  // 标记庄家第一次出牌

    while (game.game_state == 0) {
        show_game_state(game.current_player, out);

        if (is_banker_first_turn && game.current_player == game.banker) {
            // 庄家第一次出牌，不摸牌
            is_banker_first_turn = 0;

            // 直接出牌
            Tile discard;
            if (game.current_player == 0) {  // 玩家
                int valid = 0;
                char input[100];
                Player *player = &game.players[0];

                while (!valid) {
                    fprintf(out, "\n您（庄家）第一次出牌，请选择要打出的牌（输入序号）: \n");
                    print_hand(player->hand, player->hand_count, out);
                    fprintf(out, "选择: ");

                    fgets(input, sizeof(input), in);
                    int choice = atoi(input) - 1;

                    if (choice >= 0 && choice < player->hand_count) {
                        // 检查定缺规则
                        int has_que = 0;
                        for (int i = 0; i < player->hand_count; i++) {
                            if (player->hand[i].suit == player->que_suit) {
                                has_que = 1;
                                break;
                            }
                        }

                        if (has_que && player->hand[choice].suit != player->que_suit) {
                            fprintf(out, "您还有%s，必须优先打出缺门花色的牌！\n",
                                   SUIT_NAMES[player->que_suit]);
                            continue;
                        }

                        discard = player->hand[choice];
                        // 从手牌中移除
                        for (int j = choice; j < player->hand_count - 1; j++) {
                            player->hand[j] = player->hand[j + 1];
                        }
                        player->hand_count--;
                        valid = 1;

                        fprintf(out, "您打出: ");
                        print_tile(discard, out);
                        fprintf(out, "\n");
                    } else {
                        fprintf(out, "无效选择！请输入1-%d之间的数字。\n", player->hand_count);
                    }
                }
            } else {  // AI
                discard = ai_discard(game.current_player, out);
            }

            // 记录最后打出的牌
            game.last_discard = discard;
            game.last_discard_player = game.current_player;

            int processed_action = 0;  // 标记是否处理了碰、杠、胡

            // 检查其他玩家是否可以碰、杠、胡
            for (int i = 0; i < PLAYER_COUNT; i++) {
                if (i == game.current_player || game.players[i].is_hu) continue;

                Player *other = &game.players[i];

                // 检查胡牌
                if (can_hu(other, discard, 0)) {
                    processed_action = 1;
                    if (i == 0) {  // 玩家
                        fprintf(out, "您可以胡这张牌！是否胡牌？(y/n): ");
                        char input[10];
                        fgets(input, sizeof(input), in);
                        if (input[0] == 'y' || input[0] == 'Y') {
                            // 胡牌
                            game.hu_players[game.hu_count].hu_player = i;
                            game.hu_players[game.hu_count].dianpao_player = game.current_player;
                            game.hu_players[game.hu_count].win_type = get_win_type(other, other->hand, other->hand_count, 0);
                            game.hu_players[game.hu_count].fan = calculate_fan(game.hu_players[game.hu_count].win_type, 0, 0, 0);
                            game.hu_count++;
                            other->is_hu = 1;
                            hu_count++;

                            fprintf(out, "\n恭喜！您胡牌了！\n");
                            fprintf(out, "胡牌牌型: %s  %d番\n",
                                   WIN_TYPE_NAMES[game.hu_players[game.hu_count-1].win_type],
                                   game.hu_players[game.hu_count-1].fan);
                            fprintf(out, "点炮者: 玩家%d\n", game.current_player + 1);

                            // 检查是否结束
                            if (hu_count >= PLAYER_COUNT - 1) {
                                fprintf(out, "\n三家已胡牌，本局结束！\n");
                                game.game_state = 2;
                                break;
                            }
                        }
                    } else {  // AI
                        fprintf(out, "玩家%d 胡牌！\n", i + 1);
                        game.hu_players[game.hu_count].hu_player = i;
                        game.hu_players[game.hu_count].dianpao_player = game.current_player;
                        game.hu_players[game.hu_count].win_type = get_win_type(other, other->hand, other->hand_count, 0);
                        game.hu_players[game.hu_count].fan = calculate_fan(game.hu_players[game.hu_count].win_type, 0, 0, 0);
                        game.hu_count++;
                        other->is_hu = 1;
                        hu_count++;

                        // 检查是否结束
                        if (hu_count >= PLAYER_COUNT - 1) {
                            fprintf(out, "\n三家已胡牌，本局结束！\n");
                            game.game_state = 2;
                            break;
                        }
                    }
                    break;  // 有人胡牌，不再检查其他人
                }

                // 检查杠
                int gang_type = 0;
                if (can_gang(other, discard, &gang_type)) {
                    processed_action = 1;
                    if (i == 0) {  // 玩家
                        fprintf(out, "您可以杠这张牌，是否杠牌？(y/n): ");
                        char input[10];
                        fgets(input, sizeof(input), in);
                        if (input[0] == 'y' || input[0] == 'Y') {
                            do_gang(i, discard, gang_type, game.current_player);
                            fprintf(out, "您杠牌成功！\n");
                            game.current_player = i;  // 杠牌者继续
                            break;  // 跳出循环，不切换到下家
                        } else {
                            processed_action = 0;  // 玩家选择不杠，继续检查碰
                        }
                    } else {  // AI
                        do_gang(i, discard, gang_type, game.current_player);
                        fprintf(out, "玩家%d 杠牌！\n", i + 1);
                        game.current_player = i;  // 杠牌者继续
                        break;  // 跳出循环，不切换到下家
                    }
                }

                // 检查碰
                if (can_peng(other, discard)) {
                    processed_action = 1;
                    if (i == 0) {  // 玩家
                        fprintf(out, "您可以碰这张牌，是否碰牌？(y/n): ");
                        char input[10];
                        fgets(input, sizeof(input), in);
                        if (input[0] == 'y' || input[0] == 'Y') {
                            do_peng(i, discard, game.current_player);
                            fprintf(out, "您碰牌成功！\n");
                            game.current_player = i;  // 碰牌者继续
                            break;  // 跳出循环，不切换到下家
                        } else {
                            processed_action = 0;  // 玩家选择不碰
                        }
                    } else {  // AI
                        do_peng(i, discard, game.current_player);
                        fprintf(out, "玩家%d 碰牌！\n", i + 1);
                        game.current_player = i;  // 碰牌者继续
                        break;  // 跳出循环，不切换到下家
                    }
                }
            }

            // 如果没有处理任何动作（碰、杠、胡），则切换到下家
            if (!processed_action) {
                // 下一位玩家
                do {
                    game.current_player = (game.current_player + 1) % PLAYER_COUNT;
                } while (game.players[game.current_player].is_hu);
            }

            // 检查游戏是否结束
            if (game.game_state == 2) {
                break;
            }

            // 继续下一轮循环
            continue;
        } else {
            // 正常回合
            int hu = player_turn(game.current_player, in, out);
            if (hu) {
                hu_count++;
                fprintf(out, "\n玩家%d 胡牌！\n", game.current_player + 1);

                // 检查是否结束
                if (hu_count >= PLAYER_COUNT - 1) {
                    fprintf(out, "\n三家已胡牌，本局结束！\n");
                    game.game_state = 2;
                    break;
                }
            }

            // 下一位玩家
            do {
                game.current_player = (game.current_player + 1) % PLAYER_COUNT;
            } while (game.players[game.current_player].is_hu);
        }

        // 检查牌墙是否为空
        if (game.wall_count <= 0) {
            fprintf(out, "\n牌墙已空，本局结束！\n");
            game.game_state = 1;
            break;
        }
    }

    // 计分
    calculate_score();

    fprintf(out, "\n按回车键返回主菜单...");
    getchar();
}

// 显示帮助
void show_help(FILE *out) {
    fprintf(out, "四川麻将命令帮助：\n");
    fprintf(out, "用法: mahjong [选项]\n");
    fprintf(out, "选项：\n");
    fprintf(out, "  start   开始新游戏\n");
    fprintf(out, "  help    显示此帮助\n");
    fprintf(out, "  rules   显示游戏规则\n");
    fprintf(out, "  quit    退出\n");
}

// 显示规则
void show_rules(FILE *out) {
    fprintf(out, "四川麻将（血战到底）规则\n");
    fprintf(out, "=======================\n");
    fprintf(out, "\n基本规则：\n");
    fprintf(out, "1. 使用108张牌（筒、条、万，每种1-9各4张）\n");
    fprintf(out, "2. 4人游戏，庄家14张，闲家13张\n");
    fprintf(out, "3. 必须定缺一门花色，定缺后不能打出该花色\n");
    fprintf(out, "\n操作：\n");
    fprintf(out, "1. 碰：有2张相同的牌，可以碰其他玩家打出的相同牌\n");
    fprintf(out, "2. 杠：有3张相同的牌，可以杠其他玩家打出的相同牌（明杠）\n");
    fprintf(out, "3. 巴杠：已碰的牌，自己摸到第4张\n");
    fprintf(out, "4. 暗杠：手中有4张相同的牌\n");
    fprintf(out, "5. 胡牌：完成牌型\n");
    fprintf(out, "\n没有'吃'操作！\n");
    fprintf(out, "\n胡牌牌型：\n");
    fprintf(out, "1. 平胡：基本牌型，1番\n");
    fprintf(out, "2. 大对子（碰碰胡）：全部是刻子+将牌，2番\n");
    fprintf(out, "3. 清一色：全部是一种花色，4番\n");
    fprintf(out, "4. 将对：全部是2、5、8，3番\n");
    fprintf(out, "5. 门清：没有碰杠，2番\n");
    fprintf(out, "6. 七对：7个对子，4番\n");
    fprintf(out, "7. 龙七对：7对，其中有一杠牌，8番\n");
    fprintf(out, "8. 十八罗汉：4个杠+1对，16番\n");
    fprintf(out, "\n特殊规则：\n");
    fprintf(out, "1. 血战到底：一家胡牌后游戏继续\n");
    fprintf(out, "2. 查叫：未胡牌玩家要给已胡牌玩家付分\n");
    fprintf(out, "3. 自摸加番：自摸加1番\n");
    fprintf(out, "4. 杠上花：杠牌后摸牌胡牌，加1番\n");
    fprintf(out, "5. 海底捞：最后一张牌胡牌，加1番\n");
}

// 主麻将游戏命令
int cmd_mahjong(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out) {
    (void)ctx;

    if (argc < 2) {
        show_help(out);
        return 0;
    }

    if (strcmp(argv[1], "start") == 0) {
        play_sichuan_mahjong(in, out);
    } else if (strcmp(argv[1], "help") == 0) {
        show_help(out);
    } else if (strcmp(argv[1], "rules") == 0) {
        show_rules(out);
    } else if (strcmp(argv[1], "quit") == 0) {
        fprintf(out, "退出麻将游戏\n");
    } else {
        fprintf(out, "未知选项: %s\n", argv[1]);
        fprintf(out, "使用 'mahjong help' 查看可用选项\n");
        return 1;
    }

    return 0;
}
