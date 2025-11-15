#ifndef SURVIVORGAME_H
#define SURVIVORGAME_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <QKeyEvent>
#include <vector>
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"
#include "Item.h"
#include "Map.h"
#include <QObject>
#include "building.h"
#include "playground.h"
#include "library.h"
#include "teacher.h"

class SurvivorGame : public QMainWindow
{
    Q_OBJECT

public:
    SurvivorGame(QWidget *parent = nullptr);
    ~SurvivorGame();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void initGameWithMap(int mapId);
    void checkPortalInteraction();
    void initMap2Buildings();
    void learnNewSkill();

private slots:
    void updateGame();
    void spawnEnemy();
    void checkCollisions();
    building* checkCollisions_buildings();
    void shiftToMap(int mapId);
    void handleEnterPressed();
    void handleBuildingInteraction();
    void updateFadeEffect();

private:
    QGraphicsScene *scene;
    QGraphicsView *view;
    QTimer *gameTimer;
    QTimer *enemySpawnTimer;
    QTimer *teleportInterval;
    QTimer *foodGaugeInterval;
    QTimer *foodGaugeIntervalPoisoned;
    QTimer *intervalBetweenPoinsoned;
    bool isTeleporting = false;
    Player *player;
    Map *map;
    std::vector<Enemy*> enemies;
    std::vector<Bullet*> bullets;
    std::vector<Item*> items;
    std::vector<building*> buildings;
    bool is_in_building;
    building* the_building;
    int score;
    int wave;
    bool keys[4]; // 上下左右方向键状态
    int currentMapId;
    bool isEnterPressed;
    int sum_of_enemies_this_wave,sum_of_enemies_now;
    QGraphicsTextItem *mapHint; // 显示地图切换提示
    bool isPoisoned = false; // 中毒状况
    QGraphicsRectItem* blackMask = nullptr; // 黑色遮罩
    QTimer* fadeTimer; // 渐变动画定时器
    qreal maskOpacity; // 遮罩透明度
    bool isSleeping = false; // 睡眠状态标记
    QTimer* sleepTimer; // 控制自动恢复的计时器
    QTimer *healthRecover;
    QGraphicsTextItem *healText = nullptr;
    QGraphicsTextItem *canteenText = nullptr;
    QTimer *canteenTextInterval;
    teacher *newTeacher = nullptr;
    QGraphicsTextItem *teacherOccurText = nullptr;
    QGraphicsRectItem *foodButton = nullptr;
    QGraphicsRectItem *bulletButton = nullptr;
    QGraphicsTextItem *foodText = nullptr;
    QGraphicsTextItem *bulletText = nullptr;
    bool inSupermarketInterface = false;
    bool haveLearned = false;
    void initGame();
    void drawHUD();
    void endGame();
    void createSupermarketInterface();
    void removeSupermarketInterface();
    void handleSupermarketButtonClick(QPointF clickPos);
};

#endif // SURVIVORGAME_H
