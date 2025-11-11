#include "SurvivorGame.h"
#include "define.h"
#include <QGraphicsTextItem>
#include <QFont>
#include <QMouseEvent>
#include <QRandomGenerator>
#include <QRectF>
#include "building.h"
#include "playground.h"
#include "hostel.h"
#include "supermarket.h"
SurvivorGame::SurvivorGame(QWidget *parent)
    : QMainWindow(parent), score(0), wave(1), currentMapId(1), isEnterPressed(false),sum_of_enemies_this_wave(INITIAL_ENEMIES),sum_of_enemies_now(0), mapHint(nullptr)
{

    //初始化第二章地图的建筑
    building *tmp =new playground;
    buildings.push_back(tmp);
    tmp = new hostel;
    buildings.push_back(tmp);
    tmp = new Supermarket;
    buildings.push_back(tmp);
    is_in_building=0;
    // 初始化按键状态
    for (int i = 0; i < 4; i++) {
        keys[i] = false;
    }

    // 创建场景和视图
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, GAME_WIDTH, GAME_HEIGHT);
    view = new QGraphicsView(scene, this);
    view->setRenderHint(QPainter::Antialiasing);
    view->setCacheMode(QGraphicsView::CacheBackground);
    view->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    view->setDragMode(QGraphicsView::NoDrag);
    view->setFixedSize(GAME_WIDTH, GAME_HEIGHT);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setCentralWidget(view);
    setFixedSize(GAME_WIDTH, GAME_HEIGHT);
    setWindowTitle("幸存者游戏");

    // 初始化游戏
    initGame();

    // 设置游戏计时器
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &SurvivorGame::updateGame);
    gameTimer->start(1000 / FPS); // 60FPS
    teleportInterval = new QTimer(this);
    teleportInterval->setSingleShot(true);
    foodGaugeInterval = new QTimer(this);
    foodGaugeInterval->setInterval(FOOD_GAUGE_INTERVAL);
    foodGaugeIntervalPoisoned = new QTimer(this);
    foodGaugeIntervalPoisoned->setInterval(FOOD_GAUGE_INTERVAL_POISON);
    intervalBetweenPoinsoned = new QTimer(this);
    if(isPoisoned){
        connect(foodGaugeIntervalPoisoned, &QTimer::timeout, this, [=](){player->takeFoodGauge(FOOD_GAUGE_DECREASE_POISON);});
    } else{
        connect(foodGaugeInterval, &QTimer::timeout, this, [=](){player->takeFoodGauge(FOOD_GAUGE_DECREASE);});
    }
    foodGaugeInterval->start();
    intervalBetweenPoinsoned->setSingleShot(true);
    connect(intervalBetweenPoinsoned, &QTimer::timeout, this, [=](){isPoisoned = false; foodGaugeIntervalPoisoned->stop(); foodGaugeInterval->start();});

    // 设置敌人生成计时器
    enemySpawnTimer = new QTimer(this);
    connect(enemySpawnTimer, &QTimer::timeout, this, &SurvivorGame::spawnEnemy);
    enemySpawnTimer->start(INITIAL_ENEMY_SPAWN_INTERVAL); // 每2秒生成一个敌人

    //初始化场景转换提示文本
    mapHint=new QGraphicsTextItem();
}

SurvivorGame::~SurvivorGame()
{
    delete gameTimer;
    delete enemySpawnTimer;
    delete teleportInterval;
    delete foodGaugeInterval;
    delete foodGaugeIntervalPoisoned;
    delete intervalBetweenPoinsoned;
    delete player;
    delete map;
    // 清理所有敌人、子弹和物品
    for (auto enemy : enemies) delete enemy;
    for (auto bullet : bullets) delete bullet;
    for (auto item : items) delete item;
    //清理建筑
    for (auto building : buildings) delete building;
}

void SurvivorGame::initGame()
{
    initGameWithMap(currentMapId);
}

void SurvivorGame::shiftToMap(int mapId)
{
    for (auto enemy : enemies) {
        scene->removeItem(enemy);
        delete enemy;
    }
    enemies.clear();

    for (auto bullet : bullets) {
        scene->removeItem(bullet);
        delete bullet;
    }
    bullets.clear();

    for (auto item : items) {
        scene->removeItem(item);
        delete item;
    }
    items.clear();

    // 清理地图
    if (map) {
        scene->removeItem(map);
        delete map;
        map = nullptr;
    }

    //qDebug()<<"map succeed";
    // 清理提示文本
    if (mapHint) {
        // 不管是否在场景中，先尝试移除（避免残留）
        if (scene->items().contains(mapHint)) {
            scene->removeItem(mapHint);
        }
        delete mapHint; // 强制删除指针
        mapHint = nullptr; // 置空，避免野指针
    }
    //qDebug()<<"mapHint succeed";

    // 清理所有文本项（包括可能残留的mapHint副本或未清理的文本）
    QList<QGraphicsItem*> allItems = scene->items();
    for (QGraphicsItem* item : allItems) {
        if (QGraphicsTextItem *textItem = dynamic_cast<QGraphicsTextItem*>(item)) {
            // 移除所有文本项（包括mapHint，双重保险）
            scene->removeItem(textItem);
            delete textItem;
        }
    }
    //qDebug()<<"items succeed";

    // 设置场景大小
    scene->setSceneRect(0, 0, GAME_WIDTH, GAME_HEIGHT);

    // 保存当前地图ID
    currentMapId = mapId;

    //qDebug()<<currentMapId;
    // 创建并添加地图
    map = new Map(mapId, scene);

    // 确保玩家属于当前场景
    if (player->scene() != scene) {
        scene->addItem(player);
    }
    player->setPos(GAME_WIDTH / 2, GAME_HEIGHT / 2);

    // 设置玩家为焦点
    player->setFlag(QGraphicsItem::ItemIsFocusable);
    player->setFocus();

    // 重置方向键状态
    for (int i = 0; i < 4; i++) {
        keys[i] = false;
    }

    foodGaugeInterval->stop();
    foodGaugeIntervalPoisoned->stop();

    // 重置Enter键状态
    isEnterPressed = false;

    if (mapId == 1) {
        // 第一张地图：启动敌人生成
        enemySpawnTimer->start(INITIAL_ENEMY_SPAWN_INTERVAL);
        if (isPoisoned) {
            foodGaugeIntervalPoisoned->start();
            intervalBetweenPoinsoned->start();
        } else {
            foodGaugeInterval->start();
        }
    } else{
        // 第二张地图：停止敌人生成
        enemySpawnTimer->stop();
    }
    //qDebug()<<"before drawHUD()";

    drawHUD();
    //qDebug()<<"drawHUD succeed";
}

void SurvivorGame::initGameWithMap(int mapId)
{
    // 保存当前地图ID
    currentMapId = mapId;

    // 创建并添加地图
    map = new Map(mapId, scene);

    // 创建并添加玩家
    player = new Player();
    player->setPos(GAME_WIDTH / 2, GAME_HEIGHT / 2);
    player->setZValue(2);
    scene->addItem(player);

    // 设置玩家为焦点
    player->setFlag(QGraphicsItem::ItemIsFocusable);
    player->setFocus();

    // 重置方向键状态
    for (int i = 0; i < 4; i++) {
        keys[i] = false;
    }

    // 重置Enter键状态
    isEnterPressed = false;

    score = 0;
    wave = 1;
    drawHUD();
}

void SurvivorGame::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
    case Qt::Key_W:
        keys[0] = true;
        break;
    case Qt::Key_Down:
    case Qt::Key_S:
        keys[1] = true;
        break;
    case Qt::Key_Left:
    case Qt::Key_A:
        keys[2] = true;
        break;
    case Qt::Key_Right:
    case Qt::Key_D:
        keys[3] = true;
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        isEnterPressed = true;
        handleEnterPressed();
        break;
    case Qt::Key_Escape:
        close();
        break;
    }
}

void SurvivorGame::handleEnterPressed(){
    if(currentMapId == 2){
        handleBuildingInteraction();
    }
    checkPortalInteraction();
}

void SurvivorGame::handleBuildingInteraction(){
    //第二张地图进入地图
    building *targetBuilding = checkCollisions_buildings();
    if(targetBuilding){
        int targetMapId = targetBuilding->getTeleportTarget();
        isEnterPressed = false;
        if(targetMapId == 3 || targetMapId == 4) {
            //qDebug()<<"currentMapId"<<currentMapId<<" to "<<targetMapId;
            shiftToMap(targetMapId);
        }
    }
}

void SurvivorGame::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
    case Qt::Key_W:
        keys[0] = false;
        break;
    case Qt::Key_Down:
    case Qt::Key_S:
        keys[1] = false;
        break;
    case Qt::Key_Left:
    case Qt::Key_A:
        keys[2] = false;
        break;
    case Qt::Key_Right:
    case Qt::Key_D:
        keys[3] = false;
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        isEnterPressed = false;
        break;
    }
}

void SurvivorGame::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && currentMapId == 1) {
        // 计算鼠标相对于场景的位置
        QPointF mousePos = view->mapToScene(event->pos());
        // 玩家射击
        Bullet *bullet = player->shoot(mousePos);
        if (bullet) {
            bullets.push_back(bullet);
            scene->addItem(bullet);
        }
        player->takeFoodGauge(FOOD_GAUGE_CONSUME);
    }
}

void SurvivorGame::updateGame()
{
    // 更新玩家移动
    player->updateMovement(keys);
    //qDebug()<<player->pos()<<"\n";
    //qDebug()<<"currentMapId: "<<currentMapId;

    // 如果是第一张地图，更新敌人和游戏逻辑
    if (currentMapId == 1) {
        // 更新所有敌人
        for (auto it = enemies.begin(); it != enemies.end();) {
            (*it)->update();
            if ((*it)->isDead()) {
                scene->removeItem(*it);
                delete *it;
                it = enemies.erase(it);
                score += 10;
            } else {
                ++it;
            }
        }

        // 更新所有子弹
        for (auto it = bullets.begin(); it != bullets.end();) {
            (*it)->update();
            if ((*it)->shouldRemove()) {
                scene->removeItem(*it);
                delete *it;
                it = bullets.erase(it);
            } else {
                ++it;
            }
        }

        // 更新所有物品
        for (auto it = items.begin(); it != items.end();) {
            (*it)->update();
            if ((*it)->shouldRemove()) {
                scene->removeItem(*it);
                delete *it;
                it = items.erase(it);
            } else {
                ++it;
            }
        }

        // 检查碰撞
        checkCollisions();

        // 更新HUD
        drawHUD();

        // 检查游戏结束
        if (player->isDead()) {
            endGame();
        }

        // 更新波次
        if (enemies.empty() && sum_of_enemies_now>=sum_of_enemies_this_wave && score > 0) {
            wave++;
            sum_of_enemies_this_wave*=MORE_DIFFICULT;
            sum_of_enemies_now=0;
            enemySpawnTimer->setInterval(INITIAL_ENEMY_SPAWN_INTERVAL - (wave * WAVE_SPAWN_INTERVAL_DECREASE)); // 每波加快生成速度
            if (enemySpawnTimer->interval() < MIN_ENEMY_SPAWN_INTERVAL) enemySpawnTimer->setInterval(MIN_ENEMY_SPAWN_INTERVAL);
            spawnEnemy();
        }
    }
}

void SurvivorGame::spawnEnemy()
{
    if (sum_of_enemies_this_wave<=sum_of_enemies_now) {
        return ;//每波仅生成固定数量的敌人
    }
    // 随机生成敌人位置（屏幕边缘）
    int side = QRandomGenerator::global()->bounded(4);
    QPointF pos;
    sum_of_enemies_now++;
    switch (side) {
    case 0: // 顶部
        pos = QPointF(QRandomGenerator::global()->bounded(GAME_WIDTH), 0);
        break;
    case 1: // 底部
        pos = QPointF(QRandomGenerator::global()->bounded(GAME_WIDTH), GAME_HEIGHT);
        break;
    case 2: // 左侧
        pos = QPointF(0, QRandomGenerator::global()->bounded(GAME_HEIGHT));
        break;
    case 3: // 右侧
        pos = QPointF(GAME_WIDTH, QRandomGenerator::global()->bounded(GAME_HEIGHT));
        break;
    }

    Enemy *enemy = new Enemy(player);
    enemy->setPos(pos);
    scene->addItem(enemy);
    enemies.push_back(enemy);


}


void SurvivorGame::checkCollisions()
{
    // 子弹与敌人碰撞
    auto bulleti=bullets.begin();
    for (auto bullet : bullets) {
        for (auto it = enemies.begin(); it != enemies.end();) {
            if (bullet->collidesWithItem(*it)) {
                (*it)->takeDamage(bullet->getDamage(),items);
                scene->removeItem(bullet);
                bullets.erase(bulleti);
                auto tmp=bullet;
                ++bullet;
                delete tmp;
                break;
            }
            ++it;
        }
        ++bulleti;
    }

    // 敌人与玩家碰撞
    for (auto enemy : enemies) {
        if (enemy->collidesWithItem(player)) {
            player->takeDamage(enemy->getDamage());
        }
    }

    // 玩家与物品碰撞
    auto iti=items.begin();
    for (auto it:items) {
        if (it->collidesWithItem(player)) {
            scene->removeItem(it);
            it->applyEffect(player);
            auto tmp=it;++it;
            items.erase(iti);
            delete tmp;
            break;

        } else {
            ++it;
        }
        ++iti;
    }

}

building* SurvivorGame::checkCollisions_buildings(){
    qreal distance;
    for(auto it:buildings){
        distance=qSqrt(qPow(it->pos().x()-player->pos().x(),2)+qPow(it->pos().y()-player->pos().y(),2));
        if(distance<=TELEPORT_INTERACTION_RADIUS){
            return it;
        }
    }
    return nullptr;
}

void SurvivorGame::drawHUD()
{

    if (mapHint) {
        if (scene->items().contains(mapHint)) {
            scene->removeItem(mapHint);
        }
        delete mapHint;
        mapHint = nullptr;
    }
    //qDebug()<<"drawHUD(): "<<"delete mapHint succeed";
    // 清理旧的 HUD 文本项（只删 ZValue 10-20 的文本，不删 mapHint）
    QList<QGraphicsItem*> allItems = scene->items(); // 先复制列表，避免遍历中删除导致迭代器失效
    for (QGraphicsItem* item : allItems) {
        if (QGraphicsTextItem *textItem = dynamic_cast<QGraphicsTextItem*>(item)) {
            // 只清理 HUD 相关文本（ZValue 10-20），mapHint 的 ZValue 是 100，不清理
            if (textItem->scene() == scene && textItem->zValue() >= 10 && textItem->zValue() <= 20) {
                scene->removeItem(textItem);
                delete textItem;
            }
        }
    }
    //qDebug()<<"drawHUD(): "<<"delete items succeed";

    if (currentMapId == 1) {
        // 绘制地图提示（ZValue 100，不被 HUD 清理影响）
        mapHint = new QGraphicsTextItem("这是第一张地图\n移动到底部传送门按Enter进入第二张地图");
        mapHint->setDefaultTextColor(Qt::white);
        mapHint->setFont(QFont("Arial", 16, QFont::Bold));
        mapHint->setPos(GAME_WIDTH / 2 - mapHint->boundingRect().width() / 2, 20);
        mapHint->setZValue(100);
        scene->addItem(mapHint);

        // 绘制分数（ZValue 10，属于 HUD 清理范围）
        QGraphicsTextItem *scoreText = new QGraphicsTextItem();
        scoreText->setPlainText("分数: " + QString::number(score));
        scoreText->setDefaultTextColor(Qt::white);
        scoreText->setFont(QFont("Arial", 16));
        scoreText->setPos(10, 10);
        scoreText->setZValue(10);
        scene->addItem(scoreText);

        // 绘制生命值
        QGraphicsTextItem *healthText = new QGraphicsTextItem();
        healthText->setPlainText("生命值: " + QString::number(player->getHealth()));
        healthText->setDefaultTextColor(Qt::red);
        healthText->setFont(QFont("Arial", 16));
        healthText->setPos(10, 40);
        healthText->setZValue(10);
        scene->addItem(healthText);

        // 绘制波次
        QGraphicsTextItem *waveText = new QGraphicsTextItem();
        waveText->setPlainText("波次: " + QString::number(wave));
        waveText->setDefaultTextColor(Qt::yellow);
        waveText->setFont(QFont("Arial", 16));
        waveText->setPos(10, 70);
        waveText->setZValue(10);
        scene->addItem(waveText);

        // 绘制弹药
        QGraphicsTextItem *ammoText = new QGraphicsTextItem();
        ammoText->setPlainText("弹药: " + QString::number(player->getAmmo()));
        ammoText->setDefaultTextColor(Qt::green);
        ammoText->setFont(QFont("Arial", 16));
        ammoText->setPos(10, 100);
        ammoText->setZValue(10);
        scene->addItem(ammoText);

        // 绘制攻击力
        QGraphicsTextItem *damageText = new QGraphicsTextItem();
        damageText->setPlainText("攻击力: " + QString::number(player->getDamage()));
        damageText->setDefaultTextColor(Qt::white);
        damageText->setFont(QFont("Arial", 16));
        damageText->setPos(10, 130);
        damageText->setZValue(10);
        scene->addItem(damageText);

        // 绘制饱食度
        QGraphicsTextItem *foodGaugeText = new QGraphicsTextItem();
        foodGaugeText->setPlainText("饱食度: " + QString::number(player->getFoodGauge()));
        foodGaugeText->setDefaultTextColor(Qt::blue);
        foodGaugeText->setFont(QFont("Arial", 16));
        foodGaugeText->setPos(10, 160);
        foodGaugeText->setZValue(10);
        scene->addItem(foodGaugeText);
    } else if (currentMapId == 2) {
        // 第二张地图：重新创建 mapHint（避免被误删后无提示）
        mapHint = new QGraphicsTextItem("这是第二张地图\n移动到底部传送门按Enter返回第一张地图");
        mapHint->setDefaultTextColor(Qt::white);
        mapHint->setFont(QFont("Arial", 16, QFont::Bold));
        mapHint->setPos(GAME_WIDTH / 2 - mapHint->boundingRect().width() / 2, 20);
        mapHint->setZValue(100);
        scene->addItem(mapHint);
    }
}

void SurvivorGame::endGame()
{
    gameTimer->stop();
    enemySpawnTimer->stop();
    foodGaugeInterval->stop();
    foodGaugeIntervalPoisoned->stop();
    intervalBetweenPoinsoned->stop();
    teleportInterval->stop();

    // 显示游戏结束画面
    QGraphicsTextItem *gameOverText = new QGraphicsTextItem();
    gameOverText->setPlainText("游戏结束!\n最终分数: " + QString::number(score) + "\n最终波次: " + QString::number(wave));
    gameOverText->setDefaultTextColor(Qt::red);
    gameOverText->setFont(QFont("Arial", 32, QFont::Bold));
    gameOverText->setPos(GAME_WIDTH / 2 - gameOverText->boundingRect().width() / 2, GAME_HEIGHT / 2 - 50);
    scene->addItem(gameOverText);

    QGraphicsTextItem *restartText = new QGraphicsTextItem();
    restartText->setPlainText("按ESC退出");
    restartText->setDefaultTextColor(Qt::white);
    restartText->setFont(QFont("Arial", 20));
    restartText->setPos(GAME_WIDTH / 2 - restartText->boundingRect().width() / 2, GAME_HEIGHT / 2 + 50);
    scene->addItem(restartText);
}

void SurvivorGame::checkPortalInteraction()
{
    // 检查玩家是否在传送门附近并且按下了Enter键
    QPointF playerPos = player->pos();
    QPointF portalPos;
    int targetMapId = -1;

    if (currentMapId == 1 || currentMapId == 3 || currentMapId == 4) {
        portalPos = QPointF(TELEPORT_MAP_1_POS_X, TELEPORT_MAP_1_POS_Y);
        targetMapId = 2;
    } else if (currentMapId == 2) {
        portalPos = QPointF(TELEPORT_MAP_2_POS_X, TELEPORT_MAP_2_POS_Y);
        targetMapId = 1;
    }

    // 计算玩家与传送门的距离
    qreal distance = qSqrt(qPow(playerPos.x() - portalPos.x(), 2) + qPow(playerPos.y() - portalPos.y(), 2));

    // 如果玩家在传送门附近
    if (distance < TELEPORT_INTERACTION_RADIUS) {
        // 如果按下了Enter键，切换地图
        if (isEnterPressed) {
            if((currentMapId == 1 || (currentMapId == 2 && targetMapId == 1)) && teleportInterval->isActive()) return ;
            if(currentMapId == 1 || (currentMapId == 2 && targetMapId == 1)) {
                teleportInterval->start(TELEPORT_INTERVAL);
            }
            isEnterPressed = false;
            //qDebug()<<"currentMapId"<<currentMapId<<" to "<<targetMapId;
            //qDebug()<<"QTimer is "<<teleportInterval->isActive();
            shiftToMap(targetMapId);
        }
    }
}
