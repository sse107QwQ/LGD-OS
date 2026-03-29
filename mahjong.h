// mahjong.h
#ifndef MAHJONG_H
#define MAHJONG_H

#include <stdio.h>

// 麻将牌定义
#define TILE_TYPES 3         // 三种花色：筒、条、万
#define TILE_NUMBERS 9       // 1-9
#define TILE_COPIES 4        // 每种4张
#define TOTAL_TILES 108      // 108张牌
#define HAND_SIZE 13         // 初始手牌13张
#define PLAYER_COUNT 4       // 4人
#define MAX_GANG 4           // 最多4个杠
#define MAX_PENG 4           // 最多4个碰

// 花色枚举
typedef enum {
    TILE_TONG,       // 筒
    TILE_TIAO,       // 条
    TILE_WAN,        // 万
    TILE_NONE
} TileSuit;

// 牌结构
typedef struct {
    TileSuit suit;   // 花色
    int number;      // 数字 1-9
    int id;          // 唯一ID
} Tile;

// 动作类型
typedef enum {
    ACTION_NONE,     // 无
    ACTION_PENG,     // 碰
    ACTION_GANG,     // 杠
    ACTION_HU,       // 胡
    ACTION_GUO       // 过
} ActionType;

// 碰/杠记录
typedef struct {
    int type;        // 0:碰, 1:明杠, 2:暗杠, 3:巴杠
    Tile tile;       // 牌
    int from_player; // 从哪个玩家来
} PengGangRecord;

// 玩家结构
typedef struct {
    Tile hand[20];               // 手牌
    int hand_count;              // 手牌数
    PengGangRecord pg_records[8]; // 碰杠记录
    int pg_count;                // 碰杠数量
    TileSuit que_suit;           // 缺一门的花色
    int que_set;                 // 是否已定缺
    int score;                   // 积分
    int is_ai;                   // 是否为AI
    int is_ting;                 // 是否听牌
    int is_hu;                   // 是否已胡
    int is_zhamao;               // 是否查叫
    int action;                  // 当前可执行动作
} Player;

// 胡牌牌型
typedef enum {
    WIN_PINGHU,        // 平胡
    WIN_DADUI,         // 大对子（碰碰胡）
    WIN_QINGYISE,      // 清一色
    WIN_JIANGDUI,      // 将对（258将）
    WIN_MENQING,       // 门清
    WIN_QIDUI,         // 七对
    WIN_LONGQIDUI,     // 龙七对
    WIN_SHIBALUOHAN,  // 十八罗汉
    WIN_NONE
} WinType;

// 胡牌信息
typedef struct {
    int hu_player;     // 胡牌玩家
    int dianpao_player;// 点炮玩家（-1为自摸）
    WinType win_type;  // 胡牌牌型
    int fan;           // 番数
    int score;         // 得分
} HuInfo;

// 游戏状态
typedef struct {
    Tile wall[TOTAL_TILES];     // 牌墙
    int wall_count;             // 剩余牌数
    Player players[PLAYER_COUNT]; // 玩家
    int current_player;         // 当前玩家
    Tile last_discard;          // 最后打出的牌
    int last_discard_player;    // 最后打牌玩家
    HuInfo hu_players[PLAYER_COUNT]; // 胡牌玩家列表
    int hu_count;               // 胡牌人数
    int round;                  // 当前局数
    int banker;                 // 庄家
    int game_state;            // 游戏状态 0:进行中 1:流局 2:结束
} SichuanMahjong;

// 全局变量声明
extern const char* SUIT_NAMES[];
extern const char* WIN_TYPE_NAMES[];

// 工具函数声明
void print_tile(Tile tile, FILE *out);
void print_hand(Tile *hand, int count, FILE *out);
void sort_hand(Tile *hand, int count);

// 游戏初始化函数
void init_wall(void);
void shuffle_wall(void);
void deal_tiles(void);

// 玩家操作函数
void set_que_suit(Player *player, int player_index, FILE *in, FILE *out);
int can_peng(Player *player, Tile tile);
int can_gang(Player *player, Tile tile, int *gang_type);
int can_hu(Player *player, Tile tile, int is_zimo);
void do_peng(int player_index, Tile tile, int from_player);
void do_gang(int player_index, Tile tile, int gang_type, int from_player);
Tile draw_card(void);
Tile ai_discard(int player_index, FILE *out);
int player_turn(int player_index, FILE *in, FILE *out);

// 胡牌检查函数
int check_qidui(Tile *hand, int count);
int check_normal_hu(Tile *hand, int count);
int check_mianzi(Tile *hand, int count);
WinType get_win_type(Player *player, Tile *hand, int count, int is_zimo);
int calculate_fan(WinType win_type, int is_zimo, int is_gangshang, int is_haidi);
int check_que_suit(Player *player);

// 游戏控制函数
void show_game_state(int player_index, FILE *out);
void calculate_score(void);
void play_sichuan_mahjong(FILE *in, FILE *out);
void show_help(FILE *out);
void show_rules(FILE *out);

#endif // MAHJONG_H
