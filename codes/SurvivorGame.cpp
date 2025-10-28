#include "SurvivorGame.h"
#include "define.h"
#include <QGraphicsTextItem>
#include <QFont>
#include <QMouseEvent>
#include <QRandomGenerator>
#include <QRectF>

SurvivorGame::SurvivorGame(QWidget *parent)
    : QMainWindow(parent), score(0), wave(1), currentMapId(1), isEnterPressed(false), mapHint(nullptr)
{
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

    // 设置敌人生成计时器
    enemySpawnTimer = new QTimer(this);
    connect(enemySpawnTimer, &QTimer::timeout, this, &SurvivorGame::spawnEnemy);
    enemySpawnTimer->start(INITIAL_ENEMY_SPAWN_INTERVAL); // 每2秒生成一个敌人
}

SurvivorGame::~SurvivorGame()
{
    delete gameTimer;
    delete enemySpawnTimer;
    delete player;
    delete map;
    // 清理所有敌人、子弹和物品
    for (auto enemy : enemies) delete enemy;
    for (auto bullet : bullets) delete bullet;
    for (auto item : items) delete item;
}

void SurvivorGame::initGame()
{
    initGameWithMap(currentMapId);
}

void SurvivorGame::initGameWithMap(int mapId)
{
    // 清空场景
    scene->clear();
    enemies.clear();
    bullets.clear();
    items.clear();

    if (map != nullptr) {
        delete map;
        map = nullptr;
    }

    // 设置场景大小
    scene->setSceneRect(0, 0, GAME_WIDTH, GAME_HEIGHT);

    // 保存当前地图ID
    currentMapId = mapId;

    // 创建并添加地图
    map = new Map(mapId, scene);

    // 创建并添加玩家
    player = new Player();
    player->setPos(GAME_WIDTH / 2, GAME_HEIGHT / 2);
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

    // 如果是第一张地图，显示HUD和敌人生成
    if (mapId == 1) {
        // 重置分数和波次
        score = 0;
        wave = 1;

        // 绘制HUD
        drawHUD();

        // 启动敌人生成计时器
        int spawnInterval = INITIAL_ENEMY_SPAWN_INTERVAL - (wave - 1) * WAVE_SPAWN_INTERVAL_DECREASE;
        if (spawnInterval < MIN_ENEMY_SPAWN_INTERVAL) {
            spawnInterval = MIN_ENEMY_SPAWN_INTERVAL;
        }
        //enemySpawnTimer->start(spawnInterval);
    } else {
        // 第二张地图，暂停敌人生成
        enemySpawnTimer->stop();

        // 显示地图提示
        showMapHint("这是第二章地图\n移动到底部传送门按Enter返回第一张地图");
    }

    // 启动游戏计时器
    //gameTimer->start(1000 / FPS);
}

void SurvivorGame::showMapHint(const QString &text)
{
    // if (mapHint) {
    //     scene->removeItem(mapHint);
    //     delete mapHint;
    //     mapHint = nullptr;
    // }

    mapHint = new QGraphicsTextItem(text);
    QFont font("Arial", 16, QFont::Bold);
    mapHint->setFont(font);
    mapHint->setDefaultTextColor(Qt::white);
    mapHint->setPos(GAME_WIDTH / 2 - mapHint->boundingRect().width() / 2, 20);
    mapHint->setZValue(100); // 确保提示在最上层
    scene->addItem(mapHint);
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
        break;
    case Qt::Key_Escape:
        close();
        break;
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
    if (event->button() == Qt::LeftButton) {
        // 计算鼠标相对于场景的位置
        QPointF mousePos = view->mapToScene(event->pos());
        // 玩家射击
        Bullet *bullet = player->shoot(mousePos);
        if (bullet) {
            bullets.push_back(bullet);
            scene->addItem(bullet);
        }
    }
}

void SurvivorGame::updateGame()
{
    // 更新玩家移动
    player->updateMovement(keys);

    // 检测传送门交互
    checkPortalInteraction();

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
        if (enemies.empty() && score > 0) {
            wave++;
            enemySpawnTimer->setInterval(INITIAL_ENEMY_SPAWN_INTERVAL - (wave * WAVE_SPAWN_INTERVAL_DECREASE)); // 每波加快生成速度
            if (enemySpawnTimer->interval() < MIN_ENEMY_SPAWN_INTERVAL) enemySpawnTimer->setInterval(MIN_ENEMY_SPAWN_INTERVAL);
        }
    }
}

void SurvivorGame::spawnEnemy()
{
    // 随机生成敌人位置（屏幕边缘）
    int side = QRandomGenerator::global()->bounded(4);
    QPointF pos;

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

    // 每波生成更多敌人
    if (wave > 1 && QRandomGenerator::global()->bounded(100) < wave * 5) {
        spawnEnemy();
    }
}

void SurvivorGame::checkCollisions()
{
    // 子弹与敌人碰撞
    auto bulleti=bullets.begin();
    for (auto bullet : bullets) {
        for (auto it = enemies.begin(); it != enemies.end();) {
            if (bullet->collidesWithItem(*it)) {
                (*it)->takeDamage(bullet->getDamage());
                scene->removeItem(bullet);
                bullets.erase(bulleti);
                delete bullet;
                bullet = nullptr;
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
    for (auto it = items.begin(); it != items.end();) {
        if ((*it)->collidesWithItem(player)) {
            (*it)->applyEffect(player);
            scene->removeItem(*it);
            delete *it;
            it = items.erase(it);
        } else {
            ++it;
        }
    }
}

void SurvivorGame::drawHUD()
{
    // 清除旧的HUD元素
    for (auto item : scene->items()) {
        if (QGraphicsTextItem *textItem = dynamic_cast<QGraphicsTextItem*>(item)) {
            scene->removeItem(textItem);
            delete textItem;
        }
    }

    // 绘制分数
    QGraphicsTextItem *scoreText = new QGraphicsTextItem();
    scoreText->setPlainText("分数: " + QString::number(score));
    scoreText->setDefaultTextColor(Qt::white);
    scoreText->setFont(QFont("Arial", 16));
    scoreText->setPos(10, 10);
    scene->addItem(scoreText);

    // 绘制生命值
    QGraphicsTextItem *healthText = new QGraphicsTextItem();
    healthText->setPlainText("生命值: " + QString::number(player->getHealth()));
    healthText->setDefaultTextColor(Qt::red);
    healthText->setFont(QFont("Arial", 16));
    healthText->setPos(10, 40);
    scene->addItem(healthText);

    // 绘制波次
    QGraphicsTextItem *waveText = new QGraphicsTextItem();
    waveText->setPlainText("波次: " + QString::number(wave));
    waveText->setDefaultTextColor(Qt::yellow);
    waveText->setFont(QFont("Arial", 16));
    waveText->setPos(10, 70);
    scene->addItem(waveText);

    // 绘制弹药
    QGraphicsTextItem *ammoText = new QGraphicsTextItem();
    ammoText->setPlainText("弹药: " + QString::number(player->getAmmo()));
    ammoText->setDefaultTextColor(Qt::green);
    ammoText->setFont(QFont("Arial", 16));
    ammoText->setPos(10, 100);
    scene->addItem(ammoText);
}

void SurvivorGame::endGame()
{
    gameTimer->stop();
    enemySpawnTimer->stop();

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

    if (currentMapId == 1) {
        portalPos = QPointF(TELEPORT_MAP_1_POS_X, TELEPORT_MAP_1_POS_Y);
    } else if (currentMapId == 2) {
        portalPos = QPointF(TELEPORT_MAP_2_POS_X, TELEPORT_MAP_2_POS_Y);
    }

    // 计算玩家与传送门的距离
    qreal distance = qSqrt(qPow(playerPos.x() - portalPos.x(), 2) + qPow(playerPos.y() - portalPos.y(), 2));

    // 如果玩家在传送门附近
    if (distance < TELEPORT_INTERACTION_RADIUS) {
        // 显示提示
        QString hintText = currentMapId == 1 ? "按Enter前往第二章地图" : "按Enter返回第一章地图";
        showMapHint(hintText);

        // 如果按下了Enter键，切换地图
        if (isEnterPressed) {
            if(currentMapId == 1){
                enemySpawnTimer->stop();
            }
            int targetMapId = (currentMapId == 1) ? 2 : 1;
            initGameWithMap(targetMapId);
            isEnterPressed = false;
        }
    }
}
