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

private slots:
    void updateGame();
    void spawnEnemy();
    void checkCollisions();
    building* checkCollisions_buildings();
    void shiftToMap(int mapId);
    void handleEnterPressed();
    void handleBuildingInteraction();

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

    void initGame();
    void drawHUD();
    void endGame();
};

#endif // SURVIVORGAME_H
