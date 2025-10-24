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
    void showMapHint(const QString& hintText);
    void checkPortalInteraction();

private slots:
    void updateGame();
    void spawnEnemy();
    void checkCollisions();

private:
    QGraphicsScene *scene;
    QGraphicsView *view;
    QTimer *gameTimer;
    QTimer *enemySpawnTimer;
    Player *player;
    Map *map;
    std::vector<Enemy*> enemies;
    std::vector<Bullet*> bullets;
    std::vector<Item*> items;
    int score;
    int wave;
    bool keys[4]; // 上下左右方向键状态
    int currentMapId;
    bool isEnterPressed;
    QGraphicsTextItem *mapHint; // 显示地图切换提示

    void initGame();
    void drawHUD();
    void endGame();
};

#endif // SURVIVORGAME_H
