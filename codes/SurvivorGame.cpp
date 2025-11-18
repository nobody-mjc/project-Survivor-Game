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
#include "canteen.h"
#include "classroom.h"
#include "library.h"
#include <QRandomGenerator>
#include <algorithm>
#include <QCursor>
const int MAP_WIDTH = 3000;
const int MAP_HEIGHT = 3000;


SurvivorGame::SurvivorGame(QWidget *parent)
    : QMainWindow(parent), score(0), wave(1), currentMapId(1), isEnterPressed(false),sum_of_enemies_this_wave(INITIAL_ENEMIES),sum_of_enemies_now(0), mapHint(nullptr),inSupermarketInterface(false)
{
    initBackgroundMusic();
    //初始化第二章地图的建筑
    building *tmp =new playground;
    buildings.push_back(tmp);
    tmp = new hostel;
    buildings.push_back(tmp);
    tmp = new Supermarket;
    buildings.push_back(tmp);
    tmp = new Canteen();
    buildings.push_back(tmp);
    tmp = new Classroom;
    buildings.push_back(tmp);
    tmp = new Library;
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
    view->setFixedSize(0.9*GAME_WIDTH, 0.9*GAME_HEIGHT);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setCentralWidget(view);
    setFixedSize(0.9*GAME_WIDTH, 0.9*GAME_HEIGHT);
    setWindowTitle("幸存者游戏");
    // === 创建 HUD 覆盖层 ===
    hud = new HUDWidget(this);
    hud->setGeometry(0, 0, GAME_WIDTH, GAME_HEIGHT);
    hud->show();

    // 初始化游戏
    initGame();
    playBackgroundMusic();
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
    connect(foodGaugeIntervalPoisoned, &QTimer::timeout, this, [=](){player->takeFoodGauge(FOOD_GAUGE_DECREASE_POISON);});
    connect(foodGaugeInterval, &QTimer::timeout, this, [=](){player->takeFoodGauge(FOOD_GAUGE_DECREASE);});
    foodGaugeInterval->start();
    intervalBetweenPoinsoned->setSingleShot(true);
    intervalBetweenPoinsoned->setInterval(INTERVAL_POISON);
    connect(intervalBetweenPoinsoned, &QTimer::timeout, this, [=](){isPoisoned = false; foodGaugeIntervalPoisoned->stop(); foodGaugeInterval->start();});

    // 初始化黑色遮罩
    blackMask = new QGraphicsRectItem(0, 0, GAME_WIDTH, GAME_HEIGHT);
    blackMask->setBrush(Qt::black);
    blackMask->setOpacity(0.0);
    blackMask->setZValue(200); // 确保在最上层
    scene->addItem(blackMask);
    fadeTimer = new QTimer(this);
    fadeTimer->setInterval(30); // 每30ms更新一次
    connect(fadeTimer, &QTimer::timeout, this, &SurvivorGame::updateFadeEffect);
    // 配置自动恢复计时器（超时后自动结束休眠）
    sleepTimer = new QTimer(this);
    sleepTimer->setSingleShot(true); // 只触发一次
    connect(sleepTimer, &QTimer::timeout, this, [=](){
        if (isSleeping) { // 若仍在休眠状态，自动恢复
            isSleeping = false;
            healthRecover->stop();
            if(healText){
                if (scene->items().contains(healText)) {
                    scene->removeItem(healText);
                }
                delete healText;
                healText = nullptr;
            }
            fadeTimer->start(); // 开始渐变恢复
        }
    });
    healthRecover = new QTimer(this);
    healthRecover->setInterval(HEALTH_INTERVAL);
    connect(healthRecover, &QTimer::timeout, this, [=](){
        building *tmp = new hostel();
        //qDebug()<<"tmp succeed";
        int healthBeforeHeal = player->getHealth();
        QString end = tmp->update(player);
        int healthAfetHeal = player->getHealth();
        //qDebug()<<"end succeed";
        if(healText){
            //qDebug()<<"healText exists";
            if (scene->items().contains(healText)) {
                scene->removeItem(healText);
            }
            delete healText;
            healText = nullptr;
        }
        healText = new QGraphicsTextItem(end + "   " +QString::number(healthBeforeHeal) + " to " + QString::number(healthAfetHeal));
        //qDebug()<<"healText succeed";
        healText->setDefaultTextColor(Qt::white);
        healText->setFont(QFont("Arial", 16));
        healText->setPos(GAME_WIDTH/2 - 160, 30);
        healText->setZValue(300);
        scene->addItem(healText);
        delete tmp;
        //qDebug()<<"tmp delete";
    });

    // 设置敌人生成计时器
    enemySpawnTimer = new QTimer(this);
    connect(enemySpawnTimer, &QTimer::timeout, this, &SurvivorGame::spawnEnemy);
    enemySpawnTimer->start(INITIAL_ENEMY_SPAWN_INTERVAL); // 每2秒生成一个敌人

    // 食堂标语计时器
    canteenTextInterval = new QTimer(this);
    canteenTextInterval->setSingleShot(true);
    canteenTextInterval->setInterval(400);
    connect(canteenTextInterval, &QTimer::timeout, this, [=](){
        if(canteenText){
            if (scene->items().contains(canteenText)) {
                scene->removeItem(canteenText);
            }
            delete canteenText;
            canteenText = nullptr;
        }
    });

    //初始化场景转换提示文本
    mapHint=new QGraphicsTextItem();
}
// ========== 添加音乐初始化函数实现 ==========
void SurvivorGame::initBackgroundMusic()
{
    backgroundMusic = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);

    // 设置音频输出
    backgroundMusic->setAudioOutput(audioOutput);

    // 设置音量 (0.0 - 1.0)
    audioOutput->setVolume(0.3); // 30% 音量

    // 设置音乐文件 - 从资源文件加载
    // 注意：您需要将音乐文件添加到 res.qrc 中，路径为 ":/sounds/background_music.mp3"
    backgroundMusic->setSource(QUrl("qrc:/sounds/background_music.mp3"));

    // 循环播放
    backgroundMusic->setLoops(QMediaPlayer::Infinite);

}

void SurvivorGame::playBackgroundMusic()
{
    if (backgroundMusic) {
        backgroundMusic->play();
    }
}

void SurvivorGame::pauseBackgroundMusic()
{
    if (backgroundMusic) {
        backgroundMusic->pause();
    }
}

void SurvivorGame::stopBackgroundMusic()
{
    if (backgroundMusic) {
        backgroundMusic->stop();
    }
}

void SurvivorGame::setMusicVolume(float volume)
{
    if (audioOutput) {
        audioOutput->setVolume(volume);
    }
}


SurvivorGame::~SurvivorGame()
{
    // ========== 清理音乐资源 ==========
    if (backgroundMusic) {
        backgroundMusic->stop();
        delete backgroundMusic;
    }
    if (audioOutput) {
        delete audioOutput;
    }

    delete gameTimer;
    delete enemySpawnTimer;
    delete teleportInterval;
    delete foodGaugeInterval;
    delete foodGaugeIntervalPoisoned;
    delete intervalBetweenPoinsoned;
    delete player;
    delete map;
    delete blackMask;
    delete fadeTimer;
    delete sleepTimer;
    delete healthRecover;
    delete canteenTextInterval;
    if(healText){
        if (scene->items().contains(healText)) {
            scene->removeItem(healText);
        }
        delete healText;
        healText = nullptr;
    }
    if(canteenText){
        if (scene->items().contains(canteenText)) {
            scene->removeItem(canteenText);
        }
        delete canteenText;
        canteenText = nullptr;
    }
    if(teacherOccurText){
        if(scene->items().contains(teacherOccurText)){
            scene->removeItem(teacherOccurText);
        }
        delete teacherOccurText;
        teacherOccurText = nullptr;
    }
    if(newTeacher){
        if(scene->items().contains(newTeacher)){
            scene->removeItem(newTeacher);
        }
        delete newTeacher;
        newTeacher = nullptr;
    }
    // 清理所有敌人、子弹和物品
    for (auto enemy : enemies) delete enemy;
    for (auto bullet : bullets) delete bullet;
    for (auto item : items) delete item;
    //清理建筑
    for (auto building : buildings) delete building;
}

void SurvivorGame::updateFadeEffect(){
    if (isSleeping) {
        // 渐变为黑色
        maskOpacity += 0.05;
        if (maskOpacity >= 1.0) {
            maskOpacity = 1.0;
            fadeTimer->stop();
        }
    } else {
        // 从黑色渐变恢复
        maskOpacity -= 0.05;
        if (maskOpacity <= 0.0) {
            maskOpacity = 0.0;
            fadeTimer->stop();
        }
    }
    blackMask->setOpacity(maskOpacity);
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

    //如果是第三张地图，要添加老师
    if(mapId == 3){
        if(newTeacher){
            if (scene->items().contains(newTeacher)) {
                scene->removeItem(newTeacher);
            }
            delete newTeacher;
            newTeacher = nullptr;
        }
        if(teacherOccurText){
            //qDebug()<<"exists";
            if (scene->items().contains(teacherOccurText)) {
                scene->removeItem(teacherOccurText);
            }
            delete teacherOccurText;
            teacherOccurText = nullptr;
        }
        //qDebug()<<"ok";
        teacherOccurText = new QGraphicsTextItem();
        // 随机生成一个老师子类（6选1）
        int randomVal = QRandomGenerator::global()->bounded(1, 101); // 生成 1~100 的整数（闭区间）
        // 概率分配逻辑：
        if (randomVal <= 10) {
            // Surprise 老师：10% 概率（极低）
            newTeacher = new SurpriseTeacher();
            teacherOccurText->setPlainText("神秘嘉宾！");
        } else if(randomVal <= 20){
            // 没有老师出现
            teacherOccurText->setPlainText("下次再来！");
        }else{
            randomVal = QRandomGenerator::global()->bounded(1, 6);
            switch(randomVal){
            case 1: newTeacher = new ProbabilityTeacher; break;
            case 2: newTeacher = new StructureTeacher; break;
            case 3: newTeacher = new AITeacher; break;
            case 4: newTeacher = new ConvexTeacher; break;
            case 5: newTeacher = new ProgrammingTeacher; break;
            default: newTeacher = new ProgrammingTeacher; break;
            }
            teacherOccurText->setPlainText("老师出现！");
        }
        if (newTeacher) {
            // 1. 计算场景的全局中心坐标
            QPointF sceneCenter(GAME_WIDTH / 2.0, GAME_HEIGHT / 2.0);
            // 2. 计算老师自身的中心偏移（修正锚点，避免左上角对齐导致偏移）
            QPointF teacherOffset = newTeacher->boundingRect().center();
            // 3. 老师最终位置 = 场景中心 - 老师自身中心偏移（完全居中）
            newTeacher->setPos(sceneCenter - teacherOffset);
            newTeacher->setZValue(1);
            newTeacher->show(scene);
        }
        teacherOccurText->setDefaultTextColor(Qt::white);
        teacherOccurText->setFont(QFont("Arial", 16, QFont::Bold));
        QRectF sceneRect = scene->sceneRect();
        QRectF textRect = teacherOccurText->boundingRect();
        qreal x = (sceneRect.width() - textRect.width()) / 2.0;
        teacherOccurText->setPos(x, 30);
        teacherOccurText->setZValue(100);
        scene->addItem(teacherOccurText);
        //qDebug()<<scene->items().contains(teacherOccurText);
    }

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
        if(isSleeping) break;
        keys[0] = true;
        break;
    case Qt::Key_Down:
    case Qt::Key_S:
        if(isSleeping) break;
        keys[1] = true;
        break;
    case Qt::Key_Left:
    case Qt::Key_A:
        if(isSleeping) break;
        keys[2] = true;
        break;
    case Qt::Key_Right:
    case Qt::Key_D:
        if(isSleeping) break;
        keys[3] = true;
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        isEnterPressed = true;
        if(isSleeping){
            sleepTimer->stop();
            healthRecover->stop();
            if(healText){
                if (scene->items().contains(healText)) {
                    scene->removeItem(healText);
                }
                delete healText;
                healText = nullptr;
            }
            isSleeping = false;
            fadeTimer->start();
        } else {
            handleEnterPressed();
        }
        break;
    case Qt::Key_Escape:
        close();
        break;
    }
}

void SurvivorGame::handleEnterPressed(){
    checkPortalInteraction();
    if(currentMapId == 2){
        handleBuildingInteraction();
    } else if(currentMapId == 3){
        learnNewSkill();
    }
}

void SurvivorGame::learnNewSkill(){
    if(newTeacher && !haveLearned){ // 每次进入教室只准学习一次
        QString end = newTeacher->apply_skill(player);
        haveLearned = true;
        QGraphicsTextItem *text = new QGraphicsTextItem(end);
        text->setDefaultTextColor(Qt::green);
        text->setFont(QFont("Arial", 16, QFont::Bold));
        QRectF sceneRect = scene->sceneRect();
        QRectF textRect = text->boundingRect();
        qreal x = (sceneRect.width() - textRect.width()) / 2.0;
        text->setPos(x, 70);
        text->setZValue(100);
        scene->addItem(text);
        QTimer::singleShot(1000, [=]() {
            if (text && scene->items().contains(text)) {
                scene->removeItem(text);
                delete text;
            }
        });
    }
}

void SurvivorGame::handleBuildingInteraction(){
    //第二张地图进入地图
    building *targetBuilding = checkCollisions_buildings();
    if(targetBuilding){
        int targetMapId = targetBuilding->getTeleportTarget();
        isEnterPressed = false;
        if(targetMapId == 3 || targetMapId == 4) {
            //qDebug()<<"currentMapId"<<currentMapId<<" to "<<targetMapId;
            if(targetMapId == 4) {
                shiftToMap(targetMapId);
                createSupermarketInterface();
            } else {
                shiftToMap(targetMapId);
            }
        } else if(targetMapId==6){
            targetBuilding->update(player);
        }   else if(targetMapId == 7){
            if (!isSleeping) { // 未休眠时，触发变黑
                isSleeping = true;
                fadeTimer->start(); // 开始渐变变黑
                sleepTimer->start(MAX_SLEEP_DURATIO); // 启动自动恢复计时
                healthRecover->start();
            }
        } else if(targetMapId == 5 && !isPoisoned){ // 中毒只叠加一次
            // 使用食堂，看是否会中毒
            double randomDouble = QRandomGenerator::global()->generateDouble();
            //qDebug()<<"randomDouble ok";
            float foodGaugeBefore = player->getFoodGauge();
            QString text = Canteen().randomEvent(randomDouble, player);
            float foodGaugeAfter = player->getFoodGauge();
            //qDebug()<<"text ok";
            if(randomDouble < 0.3){
                //qDebug()<<"Poisoned";
                isPoisoned = true;
                if(canteenText){
                    //qDebug()<<"canteenText exists";
                    if (scene->items().contains(canteenText)) {
                        scene->removeItem(canteenText);
                    }
                    delete canteenText;
                    canteenText = nullptr;
                }
                canteenText = new QGraphicsTextItem(text);
                //qDebug()<<"canteenText ok";
                canteenText->setDefaultTextColor(Qt::darkBlue);
                canteenText->setFont(QFont("Arial", 16, QFont::Bold));
                canteenText->setPos(GAME_WIDTH/2 - 30, 100);
                canteenText->setZValue(300);
                scene->addItem(canteenText);
                canteenTextInterval->start();
            } else{
                if(canteenText){
                    //qDebug()<<"canteenText exists";
                    if (scene->items().contains(canteenText)) {
                        scene->removeItem(canteenText);
                    }
                    delete canteenText;
                    canteenText = nullptr;
                }
                canteenText = new QGraphicsTextItem(text + QString::number(foodGaugeBefore) + " to " + QString::number(foodGaugeAfter));
                //qDebug()<<"canteenText ok";
                canteenText->setDefaultTextColor(Qt::darkBlue);
                canteenText->setFont(QFont("Arial", 16, QFont::Bold));
                canteenText->setPos(GAME_WIDTH/2 - 120, 100);
                canteenText->setZValue(300);
                scene->addItem(canteenText);
                canteenTextInterval->start();
            }
        } else if(targetMapId == 8) {
            // 图书馆技能
            QString text = targetBuilding->update(player);

            // 创建图书馆效果文字
            QGraphicsTextItem *libraryText = new QGraphicsTextItem(text);
            libraryText->setDefaultTextColor(Qt::darkBlue);
            libraryText->setFont(QFont("Arial", 16, QFont::Bold));
            libraryText->setPos(GAME_WIDTH/2 - 150, 100);
            libraryText->setZValue(300);
            scene->addItem(libraryText);

            //1秒后移除
            QTimer::singleShot(1000, [=]() {
                if (libraryText && scene->items().contains(libraryText)) {
                    scene->removeItem(libraryText);
                    delete libraryText;
                }
            });

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
    if (event->button() == Qt::LeftButton && currentMapId == 1 && player->getFoodGauge() != 0) {
        // 计算鼠标相对于场景的位置
        QPointF mousePos = view->mapToScene(event->pos());
        // 玩家射击
        Bullet *bullet = player->shoot(mousePos);
        if (bullet) {
            bullets.push_back(bullet);
            scene->addItem(bullet);
        }
    }
    if (inSupermarketInterface && event->button() == Qt::LeftButton) {
        QPointF scenePos = view->mapToScene(event->pos());
        handleSupermarketButtonClick(scenePos);
        return;
    }
}

void SurvivorGame::createSupermarketInterface()
{
    inSupermarketInterface = true;

    // 创建食物按钮
    foodButton = new QGraphicsRectItem(0, 0, 100, 80);
    foodButton->setPos(GAME_WIDTH/2 - 120, GAME_HEIGHT/2 - 50);
    foodButton->setBrush(Qt::lightGray);
    foodButton->setPen(QPen(Qt::black));
    foodButton->setZValue(0);
    scene->addItem(foodButton);

    foodText = new QGraphicsTextItem("食物\n-10金币\n+15饱食度");
    foodText->setDefaultTextColor(Qt::black);
    foodText->setFont(QFont("Arial", 12));
    foodText->setPos(GAME_WIDTH/2 - 115, GAME_HEIGHT/2 - 45);
    foodText->setZValue(1);
    scene->addItem(foodText);

    // 创建子弹按钮
    bulletButton = new QGraphicsRectItem(0, 0, 100, 80);
    bulletButton->setPos(GAME_WIDTH/2 + 20, GAME_HEIGHT/2 - 50);
    bulletButton->setBrush(Qt::lightGray);
    bulletButton->setPen(QPen(Qt::black));
    bulletButton->setZValue(0);
    scene->addItem(bulletButton);

    bulletText = new QGraphicsTextItem("子弹\n-20金币\n+15子弹");
    bulletText->setDefaultTextColor(Qt::black);
    bulletText->setFont(QFont("Arial", 12));
    bulletText->setPos(GAME_WIDTH/2 + 25, GAME_HEIGHT/2 - 45);
    bulletText->setZValue(1);
    scene->addItem(bulletText);
}

// 移除超市购买界面
void SurvivorGame::removeSupermarketInterface()
{
    inSupermarketInterface = false;

    if (foodButton) {
        if (scene->items().contains(foodButton)) {
            scene->removeItem(foodButton);
        }
        delete foodButton;
        foodButton = nullptr;
    }

    if (foodText) {
        if (scene->items().contains(foodText)) {
            scene->removeItem(foodText);
        }
        delete foodText;
        foodText = nullptr;
    }

    if (bulletButton) {
        if (scene->items().contains(bulletButton)) {
            scene->removeItem(bulletButton);
        }
        delete bulletButton;
        bulletButton = nullptr;
    }

    if (bulletText) {
        if (scene->items().contains(bulletText)) {
            scene->removeItem(bulletText);
        }
        delete bulletText;
        bulletText = nullptr;
    }
}

// 处理超市按钮点击
void SurvivorGame::handleSupermarketButtonClick(QPointF clickPos)
{
    Supermarket supermarket;

    // 检查点击了哪个按钮
    if (foodButton && foodButton->contains(foodButton->mapFromScene(clickPos))) {
        if (supermarket.buyFood(player)) {
            // 购买成功提示
            QGraphicsTextItem *successText = new QGraphicsTextItem("购买食物成功！");
            successText->setDefaultTextColor(Qt::green);
            successText->setFont(QFont("Arial", 14, QFont::Bold));
            successText->setPos(GAME_WIDTH/2 - 60, GAME_HEIGHT/2 + 50);
            successText->setZValue(201);
            scene->addItem(successText);

            // 1秒后移除成功提示
            QTimer::singleShot(1000, [successText, this]() {
                if (successText && scene->items().contains(successText)) {
                    scene->removeItem(successText);
                    delete successText;
                }
            });
        } else {
            // 购买失败提示
            QGraphicsTextItem *failText = new QGraphicsTextItem("金币不足！");
            failText->setDefaultTextColor(Qt::red);
            failText->setFont(QFont("Arial", 14, QFont::Bold));
            failText->setPos(GAME_WIDTH/2 - 40, GAME_HEIGHT/2 + 50);
            failText->setZValue(201);
            scene->addItem(failText);

            // 1秒后移除失败提示
            QTimer::singleShot(1000, [failText, this]() {
                if (failText && scene->items().contains(failText)) {
                    scene->removeItem(failText);
                    delete failText;
                }
            });
        }
    }
    else if (bulletButton && bulletButton->contains(bulletButton->mapFromScene(clickPos))) {
        if (supermarket.buyBullet(player)) {
            // 购买成功提示
            QGraphicsTextItem *successText = new QGraphicsTextItem("购买子弹成功！");
            successText->setDefaultTextColor(Qt::green);
            successText->setFont(QFont("Arial", 14, QFont::Bold));
            successText->setPos(GAME_WIDTH/2 - 60, GAME_HEIGHT/2 + 50);
            successText->setZValue(201);
            scene->addItem(successText);

            // 1秒后移除成功提示
            QTimer::singleShot(1000, [successText, this]() {
                if (successText && scene->items().contains(successText)) {
                    scene->removeItem(successText);
                    delete successText;
                }
            });
        } else {
            // 购买失败提示
            QGraphicsTextItem *failText = new QGraphicsTextItem("金币不足！");
            failText->setDefaultTextColor(Qt::red);
            failText->setFont(QFont("Arial", 14, QFont::Bold));
            failText->setPos(GAME_WIDTH/2 - 40, GAME_HEIGHT/2 + 50);
            failText->setZValue(201);
            scene->addItem(failText);

            // 1秒后移除失败提示
            QTimer::singleShot(1000, [failText, this]() {
                if (failText && scene->items().contains(failText)) {
                    scene->removeItem(failText);
                    delete failText;
                }
            });
        }
    }

    // 更新HUD显示
    drawHUD();
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
    updateCamera();
    hud->hp= player->getHealth();
    hud->ammo= player->getAmmo();
    hud->wave= wave;
    hud->score= score;
    hud->money= player->getMoney();
    hud->foodGauge= player->getFoodGauge();
    hud->mapId= currentMapId;
    hud->update();

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
    // 删除旧的 mapHint
    if (mapHint) {
        if (scene->items().contains(mapHint)) {
            scene->removeItem(mapHint);
        }
        delete mapHint;
        mapHint = nullptr;
    }

    // 删除旧的 HUD 文本（ZValue 10-20）
    QList<QGraphicsItem*> allItems = scene->items();
    for (QGraphicsItem* item : allItems) {
        if (QGraphicsTextItem *textItem = dynamic_cast<QGraphicsTextItem*>(item)) {
            if (textItem->scene() == scene && textItem->zValue() >= 10 && textItem->zValue() <= 20) {
                scene->removeItem(textItem);
                delete textItem;
            }
        }
    }

    // 只绘制与地图相关的提示，不绘制任何 HUD（HUD 已交给 HUDWidget）
    if (currentMapId == 1) {
        //mapHint = new QGraphicsTextItem("这是第一张地图\n移动到底部传送门按Enter进入第二张地图");
        //mapHint->setDefaultTextColor(Qt::white);
        //mapHint->setFont(QFont("Arial", 16, QFont::Bold));
        //mapHint->setPos(GAME_WIDTH / 2 - mapHint->boundingRect().width() / 2, 20);
        //mapHint->setZValue(100);
        //scene->addItem(mapHint);
    }
    else if (currentMapId == 2) {
        //mapHint = new QGraphicsTextItem("这是第二张地图\n移动到底部传送门按Enter返回第一张地图");
        //mapHint->setDefaultTextColor(Qt::white);
        //mapHint->setFont(QFont("Arial", 16, QFont::Bold));
        //mapHint->setPos(GAME_WIDTH / 2 - mapHint->boundingRect().width() / 2, 20);
        //mapHint->setZValue(100);
        //scene->addItem(mapHint);
    }
    else if (currentMapId == 4) {
        // 不再绘制金币 HUD！因为已由 HUDWidget 绘制
        // 留空
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
    foodGaugeInterval->stop();
    foodGaugeIntervalPoisoned->stop();
    intervalBetweenPoinsoned->stop();
    fadeTimer->stop();
    sleepTimer->stop();
    healthRecover->stop();
    canteenTextInterval->stop();

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
    if (distance < TELEPORT_INTERACTION_RADIUS && !isSleeping) {
        // 如果按下了Enter键，切换地图
        if (isEnterPressed) {
            if((currentMapId == 1 || (currentMapId == 2 && targetMapId == 1)) && teleportInterval->isActive()) return ;
            if(currentMapId == 1 || (currentMapId == 2 && targetMapId == 1)) {
                teleportInterval->start(TELEPORT_INTERVAL);
                player->setRotation(0);
            }
            if(currentMapId == 2 && targetMapId == 1 && isPoisoned){
                foodGaugeInterval->stop();
                foodGaugeIntervalPoisoned->start();
                intervalBetweenPoinsoned->start();
                //qDebug()<<"foodGaugeInterval: "<<foodGaugeInterval->isActive();
                //qDebug()<<"foodGaugePoisoned: "<<foodGaugeIntervalPoisoned->isActive();
                //qDebug()<<"intervalBetweenPoinsoned: "<<intervalBetweenPoinsoned->isActive();
            }
            if(currentMapId == 3){
                if(newTeacher){
                    if(scene->items().contains(newTeacher)){
                        scene->removeItem(newTeacher);
                    }
                    delete newTeacher;
                    newTeacher = nullptr;
                }
                if(teacherOccurText){
                    if(scene->items().contains(teacherOccurText)){
                        //qDebug()<<"scene contains";
                        scene->removeItem(teacherOccurText);
                        //qDebug()<<"remove";
                    }
                    //qDebug()<<"remove ok";
                    delete teacherOccurText;
                    //qDebug()<<"delete ok";
                    teacherOccurText = nullptr;
                }
                haveLearned = false;
            }
            isEnterPressed = false;
            if (currentMapId == 4 && inSupermarketInterface) {
                removeSupermarketInterface();
            }
            //qDebug()<<"currentMapId"<<currentMapId<<" to "<<targetMapId;
            //qDebug()<<"QTimer is "<<teleportInterval->isActive();
            shiftToMap(targetMapId);
        }
    }
}

void SurvivorGame::updateCamera()
{
    if (!player) return;

    QPointF playerCenter = player->pos() + player->boundingRect().center();
    QPointF mouseScenePos = view->mapToScene(view->mapFromGlobal(QCursor::pos()));

    QPointF mouseVec = (mouseScenePos - playerCenter) * mouseInfluence;

    QPointF targetPos = playerCenter + mouseVec;

    QRectF sceneRect = scene->sceneRect();
    QRectF viewRect = view->viewport()->rect();

    float halfWidth  = viewRect.width()  * 0.5;
    float halfHeight = viewRect.height() * 0.5;

    float minX = sceneRect.left()   + halfWidth;
    float maxX = sceneRect.right()  - halfWidth;
    float minY = sceneRect.top()    + halfHeight;
    float maxY = sceneRect.bottom() - halfHeight;

    auto edgeReduce = [&](float value, float minVal, float maxVal) {
        float distToEdge = std::min(value - minVal, maxVal - value);
        float maxDist = halfWidth;
        float t = distToEdge / maxDist;
        t = qBound(0.0f, t, 1.0f);    // 使用 Qt 的 clamp
        return t;
    };

    float reduceX = edgeReduce(playerCenter.x(), minX, maxX);
    float reduceY = edgeReduce(playerCenter.y(), minY, maxY);

    targetPos.setX(playerCenter.x() + mouseVec.x() * reduceX);
    targetPos.setY(playerCenter.y() + mouseVec.y() * reduceY);

    // Qt clamp
    targetPos.setX(qBound(minX, targetPos.x(), maxX));
    targetPos.setY(qBound(minY, targetPos.y(), maxY));

    cameraOffset = cameraOffset * (1 - cameraSmooth) + targetPos * cameraSmooth;

    view->centerOn(cameraOffset);
}


