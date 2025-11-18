#ifndef DEFINE_H
#define DEFINE_H

// 游戏窗口常量
#define GAME_WIDTH 1280
#define GAME_HEIGHT 720
#define FPS 60

// 游戏状态常量
#define INITIAL_PLAYER_HEALTH 10000
#define INITIAL_PLAYER_AMMO 10000
#define INITIAL_ENEMY_HEALTH 50
#define INITIAL_FOOD_GAUGE 100
#define INITIAL_MONEY 100
#define FOOD_GAUGE_DECREASE 1
#define FOOD_GAUGE_CONSUME 0.5
#define FOOD_GAUGE_INTERVAL 10000
#define FOOD_GAUGE_INTERVAL_POISON 5000
#define FOOD_GAUGE_DECREASE_POISON 2
#define INTERVAL_POISON 60000

// 移动速度常量
#define PLAYER_SPEED 5
extern int ENEMY_SPEED;
#define BULLET_SPEED 10

// 碰撞伤害常量
#define PLAYER_DAMAGE 20
#define ENEMY_DAMAGE 10

// 射击相关常量
#define FIRE_RATE 200 // 毫秒
#define BULLET_LIFETIME 100 // 帧数

// 敌人生成常量
#define INITIAL_ENEMY_SPAWN_INTERVAL 2000 // 毫秒
#define MIN_ENEMY_SPAWN_INTERVAL 500 // 毫秒
#define INITIAL_ENEMIES 10//第一波敌人数量
#define MORE_DIFFICULT 2//每波敌人比上波敌人多的倍数
#define ADD_ENEMY_COINS 10 // 初始值是10金币每个enemy

// 物品相关常量
#define ITEM_DROP_CHANCE 30 // 百分比
#define HEALTH_PACK_VALUE 50
#define AMMO_PACK_VALUE 30
#define SPEED_BOOST_VALUE 20
#define MONEY_PACK_VALUE 5
#define ITEM_LIFETIME 300000 // 帧数
#define SPEEDBUFF_LIFETIME 300000

// 波次相关常量
#define WAVE_SPAWN_INTERVAL_DECREASE 100 // 每波减少的生成间隔（毫秒）

// 地图相关常量
#define MAP_1_BACKGROUND_PATH ":/map1.png"
#define MAP_2_BACKGROUND_PATH ":/map2.jpg"
#define MAP_3_BACKGROUND_PATH ":/map3.png"
#define MAP_4_BACKGROUND_PATH ":/map4.png"
#define PORTALS_PATH ":/portal.png"
#define MAP_OBSTACLE_COUNT 10
#define OBSTACLE_SIZE 2
#define MAP_GRID_SIZE 50
#define MAP_BG_TOP_COLOR QColor(40, 40, 80)
#define MAP_BG_BOTTOM_COLOR QColor(20, 20, 40)
#define MAP_GRID_COLOR QColor(60, 60, 100, 100)

// 传送门相关常量
#define TELEPORT_WIDTH 50
#define TELEPORT_HEIGHT 50
#define TELEPORT_COLOR Qt::magenta
#define TELEPORT_MAP_1_POS_X 700
#define TELEPORT_MAP_1_POS_Y 500
#define TELEPORT_MAP_2_POS_X 700
#define TELEPORT_MAP_2_POS_Y 500
#define TELEPORT_INTERACTION_RADIUS 50
#define TELEPORT_INTERVAL 5000 //传送门间隔时间，记得调整，初始5s

// 图片路径
#define RESOURCES_DIR "./resources/"
#define PLAYER_IMAGE_PATH ":/player.png"
#define ENEMY_IMAGE_PATH ":/enemy.png"
#define BULLET_IMAGE_PATH RESOURCES_DIR "bullet.png"
#define HEALTH_PACK_IMAGE_PATH RESOURCES_DIR "health_pack.png"
#define AMMO_PACK_IMAGE_PATH RESOURCES_DIR "ammo_pack.png"
#define SPEED_BOOST_IMAGE_PATH RESOURCES_DIR "speed_boost.png"
#define MONEY_PACK_IMAGE_PATH RESOURCES_DIR "money_pack.png"
#define OBSTACLE_IMAGE_PATH RESOURCES_DIR "obstacle.png"

// 游戏对象尺寸
#define PLAYER_SIZE 50
#define ENEMY_SIZE 40
#define BULLET_WIDTH 10
#define BULLET_HEIGHT 5
#define ITEM_SIZE 30

// 颜色定义
#define PLAYER_COLOR Qt::blue
#define ENEMY_COLOR Qt::red
#define BULLET_COLOR Qt::yellow
#define HEALTH_PACK_COLOR Qt::red
#define AMMO_PACK_COLOR Qt::green
#define SPEED_BOOST_COLOR Qt::blue
#define Money_Pack_COLOR Qt::yellow
#define BACKGROUND_COLOR Qt::black
#define OBSTACLE_COLOR Qt::darkGray

// 建筑相关
#define MAX_SLEEP_DURATIO 5000
#define HEALTH_RECOVER 100
#define HEALTH_INTERVAL 1000
#define RATING_OF_POISON 3

// 老师路径
#define TEACHER_1_PATH ":/teacher1.png"
#define TEACHER_2_PATH ":/teacher2.jpeg"
#define TEACHER_3_PATH ":/teacher3.jpg"
#define TEACHER_4_PATH ":/teacher4.jpg"
#define TEACHER_5_PATH ":/teacher5.jpg"
#define TEACHER_6_PATH ":/teacher6.png"

#endif // DEFINE_H
