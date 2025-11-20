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
#include "sportingcenter.h"
#include <QRandomGenerator>
#include <algorithm>
#include <QCursor>
#include <GameExitDialog.h>
#include <QMovie>
const int MAP_WIDTH = 3000;
const int MAP_HEIGHT = 3000;


SurvivorGame::SurvivorGame(QString save_path,QWidget *parent)
    : QMainWindow(parent), score(0), wave(1), currentMapId(1), isEnterPressed(false),sum_of_enemies_this_wave(INITIAL_ENEMIES),sum_of_enemies_now(0), mapHint(nullptr),inSupermarketInterface(false), isBuyingFood(false), isBuyingBullet(false)
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
    tmp = new SportingCenter;
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
    setWindowTitle("南苏物语");
    // === 创建 HUD 覆盖层 ===
    hud = new HUDWidget(this);
    hud->setGeometry(0, 0, GAME_WIDTH, GAME_HEIGHT);
    hud->show();

    // 初始化游戏
    initGame();
    if(!save_path.isEmpty()){//读取存档
        player->read_saving(save_path.toStdString());
    }
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
        healText->setFont(QFont("Microsoft YaHei", 16, QFont::Bold));
        healText->setPos(player->pos().x() - 100, player->pos().y() - 50);
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

    // 初始化图书馆4个无文字按钮
    // 调整原则：按钮x = 课本x + (课本宽度 - 110)/2（水平居中）
    // 按钮y = 课本y + 10（下方间距10px）
    QPointF buttonPositions[4] = {
        {60, 430},
        {165, 430},
        {280, 430},
        {400, 430}
    };
    for (int i = 0; i < 4; i++) {
        libraryTextbookButtons[i] = new QPushButton(textbookButtonTexts[i]);
        libraryTextbookButtons[i]->setFixedSize(80, 38);  // 按钮大小
        libraryTextbookButtons[i]->setStyleSheet(R"(
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                            stop:0 #00CED1, stop:1 #008B8B);
                border-radius: 19px;
                border: 2px solid rgba(255, 255, 255, 0.3);
                color: white;
                font-weight: bold;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                            stop:0 #40E0D0, stop:1 #00CED1);
                border-color: rgba(173, 216, 230, 0.8);
            }
            QPushButton:pressed {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                            stop:0 #008B8B, stop:1 #00695C);
            }
            QPushButton:disabled {
                background: #87CEEB;
                border-color: transparent;
                color: #666;
            }
        )");
        libraryButtonProxies[i] = scene->addWidget(libraryTextbookButtons[i]);
        libraryButtonProxies[i]->setPos(buttonPositions[i]);
        libraryButtonProxies[i]->setZValue(1);
        libraryButtonProxies[i]->hide();

        connect(libraryTextbookButtons[i], &QPushButton::clicked, this, [=]() {
            if (!isLibraryReading) {
                onLibraryTextbookClicked(i);
            }
        });
    }

    // 初始化圆形倒计时时钟
    libraryCircularClock = new QProgressBar();
    libraryCircularClock->setRange(0, 20);
    libraryCircularClock->setValue(20);
    libraryCircularClock->setFixedSize(100, 100);
    // 设置正方形时钟样式
    libraryCircularClock->setStyleSheet(R"(
        QProgressBar {
            background-color: transparent;
            border: 3px solid #00CED1;
            border-radius: 10px;
            padding: 0px;
            margin: 0px;
            text-align: center;
            color: black;
            font-size: 24px;
            font-weight: bold;
            font-family: "Microsoft YaHei";
            qproperty-alignment: AlignCenter;
        }
        QProgressBar::chunk {
            background-color: rgba(0, 206, 209, 100);
            border-radius: 7px;
            margin: 3px;
        }
    )");
    libraryCircularClock->setFont(QFont("Microsoft YaHei", 22, QFont::Bold));
    libraryCircularClock->setAlignment(Qt::AlignCenter);
    libraryCircularClock->setFormat(QString("%1秒").arg(libraryRemainingTime));
    libraryClockProxy = scene->addWidget(libraryCircularClock);
    libraryClockProxy->setPos(530, 150);
    libraryClockProxy->setZValue(200);
    libraryClockProxy->hide();

    // 初始化翻书动图
    libraryFlipBookGif = new QMovie(READING_BOOK);
    libraryGifItem = new QGraphicsPixmapItem();
    scene->addItem(libraryGifItem);
    libraryGifItem->setZValue(150);
    libraryGifItem->setPos(150, 250);
    libraryGifItem->hide();

    // 初始化倒计时计时器
    libraryCountdownTimer = new QTimer(this);
    libraryCountdownTimer->setInterval(1000);
    connect(libraryCountdownTimer, &QTimer::timeout, this, &SurvivorGame::updateLibraryCountdown);
    //初始化视频组件
    media_player=new QMediaPlayer;
    media_player->setSource(QUrl("qrc:/sleeping.mp4"));
    widget_player = new QVideoWidget;
    media_player->setVideoOutput(widget_player);
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
    backgroundMusic->setSource(QUrl(GAME_BGM_1));

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

void SurvivorGame::initLibraryInterface()
{
    //qDebug()<<"initLibraryInterface()";
    // 确保所有元素处于初始状态
    hideLibraryReadingElements();
    //qDebug()<<"hideLibraryReadingElements();";
    showLibraryTextbooks();
    //qDebug()<<"showLibraryTextbooks();";

    if (libraryFlipBookGif) {
        //qDebug()<<"gif ok";
        libraryFlipBookGif->setFileName(READING_BOOK);
    }
}

void SurvivorGame::showLibraryTextbooks()
{
    for (int i = 0; i < 4; i++) {
        libraryButtonProxies[i]->show();
        libraryTextbookButtons[i]->setEnabled(true);
    }
}

void SurvivorGame::hideLibraryTextbookButtons()
{
    for (int i = 0; i < 4; i++) {
        libraryButtonProxies[i]->hide();
        libraryTextbookButtons[i]->setEnabled(false);
    }
}

void SurvivorGame::hideLibraryReadingElements()
{
    //qDebug()<<"enter hideLibraryReadingElements()";
    libraryClockProxy->hide();
    //qDebug()<<"libraryClockProxy->hide();";
    libraryGifItem->hide();
    //qDebug()<<"libraryGifItem->hide();";
    if (libraryFlipBookGif->state() == QMovie::Running) {
        libraryFlipBookGif->stop();
    }
}

void SurvivorGame::onLibraryTextbookClicked(int index)
{
    if(isLibraryReading) return;
    //qDebug()<<isLibraryReading<<"enter book";

    textbookIndex = index;
    isLibraryReading = true;
    //qDebug()<<"index: "<<index;

    // 隐藏按钮和提示
    hideLibraryTextbookButtons();
    //qDebug()<<"hideLibraryTextbookButtons();";

    // 初始化倒计时
    libraryRemainingTime = 20;
    libraryCircularClock->setValue(libraryRemainingTime);
    libraryCircularClock->setFormat(QString("%1秒").arg(libraryRemainingTime));

    // 显示倒计时、动图
    libraryClockProxy->show();
    libraryGifItem->show();
    libraryFlipBookGif->start();

    // 启动计时器（实时更新倒计时）
    connect(libraryFlipBookGif, &QMovie::frameChanged, this, [=]() {
        libraryGifItem->setPixmap(libraryFlipBookGif->currentPixmap());
    });
    libraryCountdownTimer->start();
}

// 更新倒计时
void SurvivorGame::updateLibraryCountdown()
{
    libraryRemainingTime--;
    libraryCircularClock->setValue(libraryRemainingTime);
    libraryCircularClock->setFormat(QString("%1秒").arg(libraryRemainingTime));

    // 倒计时结束
    if (libraryRemainingTime <= 0) {
        libraryCountdownTimer->stop();
        onLibraryCountdownFinished();
    }
}

void SurvivorGame::onLibraryCountdownFinished()
{
    // 隐藏阅读元素
    hideLibraryReadingElements();
    isLibraryReading = false;

    // 重新显示按钮（可再次阅读）
    showLibraryTextbooks();

    // 发放技能
    QString skillText, skillDesc;
    if(textbookIndex == -1) return;
    switch (textbookIndex) {
    case 0: // 概率论期中试卷 → 幸运加持
        skillText = "概率论与数理统计·幸运加持";
        skillDesc = "暴击率+12%（永久）+ 金钱+200 + 弹药+30";
        player->add_crit_rate(0.12);
        player->addMoney(200);
        player->addAmmo(30);
        break;
    case 1: // 凸优化课本 → 精准打击
        skillText = "凸优化·精准打击";
        skillDesc = "攻击力+20（永久）+ 暴击率+8%（永久）";
        player->addDamage(20);
        player->add_crit_rate(0.08);
        break;
    case 2: // 人工智能导论 → 智能瞄准
        skillText = "人工智能导论·智能瞄准";
        skillDesc = "射速+5%（永久）+ 攻击力+15 + 弹药+40";
        player->setFireRate(player->getFireRate() * 1.05);
        player->addDamage(15);
        player->addAmmo(40);
        break;
    case 3: // 数据结构课本 → 结构加固
        skillText = "数据结构与算法基础·结构加固";
        skillDesc = "最大生命值+50（永久）+ 当前生命值+100";
        player->add_MaxHealth(50);
        player->addHealth(100);
        break;
    }
    // 技能学习完毕
    textbookIndex = -1;

    // 技能提示（美观样式）
    QGraphicsTextItem *skillTitle = new QGraphicsTextItem(skillText);
    QGraphicsTextItem *skillDescription = new QGraphicsTextItem(skillDesc);

    // 标题样式
    skillTitle->setFont(QFont("Microsoft YaHei", 24, QFont::Bold));
    skillTitle->setDefaultTextColor(Qt::yellow);
    QGraphicsDropShadowEffect *titleShadow = new QGraphicsDropShadowEffect();
    titleShadow->setColor(QColor(139, 0, 0));
    titleShadow->setBlurRadius(12);
    titleShadow->setOffset(3, 3);
    skillTitle->setGraphicsEffect(titleShadow);
    skillTitle->setPos(GAME_WIDTH/2 - 190, GAME_HEIGHT/2 - 120);
    skillTitle->setZValue(300);

    // 描述样式
    skillDescription->setFont(QFont("Microsoft YaHei", 16, QFont::Medium));
    skillDescription->setDefaultTextColor(Qt::white);
    QGraphicsDropShadowEffect *descShadow = new QGraphicsDropShadowEffect();
    descShadow->setColor(Qt::black);
    descShadow->setBlurRadius(6);
    descShadow->setOffset(2, 2);
    skillDescription->setGraphicsEffect(descShadow);
    skillDescription->setPos(GAME_WIDTH/2 - 190, GAME_HEIGHT/2 - 70);
    skillDescription->setZValue(300);

    // 添加到场景，1秒后移除
    scene->addItem(skillTitle);
    scene->addItem(skillDescription);
    QTimer::singleShot(1000, [=]() {
        if (skillTitle && scene->items().contains(skillTitle)) {
            scene->removeItem(skillTitle);
            delete skillTitle;
        }
        if (skillDescription && scene->items().contains(skillDescription)) {
            scene->removeItem(skillDescription);
            delete skillDescription;
        }
    });
}

SurvivorGame::~SurvivorGame()
{
    // 清理图书馆资源
    if (libraryFlipBookGif && libraryFlipBookGif->state() == QMovie::Running) {
        libraryFlipBookGif->stop();
        delete libraryFlipBookGif;
    }
    delete libraryCountdownTimer;
    for (int i = 0; i < 4; i++) {
        delete libraryTextbookButtons[i];
        delete libraryButtonProxies[i];
    }
    delete libraryCircularClock;
    delete libraryClockProxy;
    if (libraryGifItem && scene->items().contains(libraryGifItem)) {
        scene->removeItem(libraryGifItem);
    }
    delete libraryGifItem;

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
            QPointF additionalOffset(-400, 0);
            // 3. 老师最终位置 = 场景中心 - 老师自身中心偏移（完全居中）
            newTeacher->setPos(sceneCenter - teacherOffset + additionalOffset);
            newTeacher->setZValue(1);
            newTeacher->show(scene);
        }
        teacherOccurText->setDefaultTextColor(Qt::red);
        teacherOccurText->setFont(QFont("Microsoft YaHe", 16, QFont::Bold));
        qreal x = newTeacher->pos().x() + teacherOccurText->boundingRect().x();
        teacherOccurText->setPos(x + 50, 200);
        teacherOccurText->setZValue(100);
        scene->addItem(teacherOccurText);
        //qDebug()<<scene->items().contains(teacherOccurText);
    } else if(mapId == 4){
        createSupermarketInterface();
    } else if(mapId == 8){
        initLibraryInterface();
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
        // 切换音乐源
        stopBackgroundMusic();
        backgroundMusic->setSource(QUrl(GAME_BGM_1));
        playBackgroundMusic();

    } else{
        // 第二张地图：停止敌人生成
        enemySpawnTimer->stop();
        // 切换音乐源
        stopBackgroundMusic();
        if(mapId == 2){
            backgroundMusic->setSource(QUrl(GAME_BGM_2));
        } else if(mapId == 3){
            backgroundMusic->setSource(QUrl(GAME_BGM_3));
        } else if(mapId == 4){
            backgroundMusic->setSource(QUrl(GAME_BGM_4));
        } else if(mapId == 5){
            backgroundMusic->setSource(QUrl(GAME_BGM_5));
            setMusicVolume(1.0);
        } else if(mapId == 6){
            backgroundMusic->setSource(QUrl(GAME_BGM_6));
        } else if(mapId == 7){
            backgroundMusic->setSource(QUrl(GAME_BGM_7));
            setMusicVolume(0.5);
        } else if(mapId == 8){
            backgroundMusic->setSource(QUrl(GAME_BGM_8));
            setMusicVolume(0.5);
        } else if(mapId == 9){
            backgroundMusic->setSource(QUrl(GAME_BGM_9));
        }
        playBackgroundMusic();
    }
    //qDebug()<<"before drawHUD()";

    drawHUD();
    //qDebug()<<"drawHUD succeed";
}

void SurvivorGame::initGameWithMap(int mapId)
{
    // 加载地图一的音乐
    initBackgroundMusic();
    playBackgroundMusic();

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
    wave= 1;
    drawHUD();
}

void SurvivorGame::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
    case Qt::Key_W:
        if(isSleeping || isLibraryReading) break;
        keys[0] = true;
        break;
    case Qt::Key_Down:
    case Qt::Key_S:
        if(isSleeping || isLibraryReading) break;
        keys[1] = true;
        break;
    case Qt::Key_Left:
    case Qt::Key_A:
        if(isSleeping || isLibraryReading) break;
        keys[2] = true;
        break;
    case Qt::Key_Right:
    case Qt::Key_D:
        if(isSleeping || isLibraryReading) break;
        keys[3] = true;
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if(isLibraryReading) break;
        // qDebug()<<player->pos();
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
    {
        if (inSupermarketInterface && (isBuyingFood || isBuyingBullet)) {
            cancelPurchase();
        }else{
            // 显示退出确认对话框
            GameExitDialog dialog(player,currentMapId,this);
            int result = dialog.exec();

            if (result == QDialog::Accepted) {
                // 用户确认退出
                close();
            }
        }
        break;
    }
    }
}

void SurvivorGame::closeEvent(QCloseEvent *event)
{
    // 阻止默认关闭行为
    event->ignore();

    // 显示确认对话框
    GameExitDialog dialog(player,currentMapId,this);
    int result = dialog.exec();

    if (result == QDialog::Accepted) {
        // 用户确认退出，允许关闭
        event->accept();
    }
}

void SurvivorGame::handleEnterPressed(){
    checkPortalInteraction();
    if(currentMapId == 2){
        handleBuildingInteraction();
    } else if(currentMapId == 3){
        learnNewSkill();
    } else if(currentMapId == 4){
        handleSupermarketEnter();
    } else if(currentMapId == 5){
        eatInCanteen();
    } else if(currentMapId == 6){
        //
    } else if(currentMapId == 7){
        sleepInHostel();
    } else if(currentMapId == 8){
        // 图书馆技能
        QString text = Library().update(player);

        // 创建图书馆效果文字
        QGraphicsTextItem *libraryText = new QGraphicsTextItem(text);
        libraryText->setDefaultTextColor(Qt::darkBlue);
        libraryText->setFont(QFont("Microsoft YaHe", 16, QFont::Bold));
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
    } else if(currentMapId == 9){
        // 体育馆技能，即在体育馆里面按enter键之后会发生什么
    }
}

void SurvivorGame::sleepInHostel(){
    if (!isSleeping) { // 未休眠时，触发变黑
        //qDebug()<<"isSleeping = false";
        isSleeping = true;
        fadeTimer->start(); // 开始渐变变黑
        widget_player->show();
        media_player->play();
        sleepTimer->start(MAX_SLEEP_DURATIO); // 启动自动恢复计时
        healthRecover->start();
        //qDebug()<<"isSleeping:"<<isSleeping;
    }
}

void SurvivorGame::eatInCanteen(){
    if(!isPoisoned){ // 中毒只叠加一次
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
            canteenText->setFont(QFont("Microsoft YaHe", 16, QFont::Bold));
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
            canteenText->setFont(QFont("Microsoft YaHe", 16, QFont::Bold));
            canteenText->setPos(GAME_WIDTH/2 - 120, 100);
            canteenText->setZValue(300);
            scene->addItem(canteenText);
            canteenTextInterval->start();
        }
    }
}

void SurvivorGame::learnNewSkill(){
    if(newTeacher && !haveLearned){ // 每次进入教室只准学习一次
        QString end = newTeacher->apply_skill(player);
        haveLearned = true;
        QGraphicsTextItem *text = new QGraphicsTextItem(end);
        QGraphicsTextItem *counter_text = new QGraphicsTextItem("下课倒计时：20");
        int remain=20;
        QTimer *counter = new QTimer();

        text->setDefaultTextColor(Qt::green);
        text->setFont(QFont("Microsoft YaHe", 16, QFont::Bold));
        counter_text->setDefaultTextColor(Qt::white);
        counter_text->setFont(QFont("Microsoft YaHe", 16, QFont::Bold));
        qreal x = newTeacher->pos().x() + text->boundingRect().x();
        text->setPos(x + 100, 250); // 50常数暂定
        text->setZValue(100);
        counter_text->setZValue(100);
        counter_text->setPos(x + 50, 250);
        scene->addItem(text);
        scene->addItem(counter_text);
        counter->start(1000);
        connect(counter,&QTimer::timeout,this,[&](){
            if(remain>=0){
                remain-=1;
                counter_text->setPlainText("下课倒计时："+QString(QString::number(remain)));
            }
            else{
                counter->stop();
            }
        });
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
        shiftToMap(targetMapId);
        if(targetMapId == 6){
            targetBuilding->update(player);
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
        Bullet *bullet = player->shoot(mousePos,qreal(0));
        if (bullet) {
            bullets.push_back(bullet);
            scene->addItem(bullet);
            if(player->f_shotgun){
                float t=10;
                for(int i=1;i<=player->f_shotgun;i++){
                    bullet = player->shoot(mousePos,qreal(t));
                    bullets.push_back(bullet);
                    scene->addItem(bullet);

                    bullet = player->shoot(mousePos,qreal(-t));
                    bullets.push_back(bullet);
                    scene->addItem(bullet);
                    t+=10;
                    qDebug()<<i<<" "<<t<<"\n";
                }
            }
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

    QGraphicsTextItem *hintText = new QGraphicsTextItem("靠近物品后按enter键购买");
    hintText->setDefaultTextColor(Qt::white);
    hintText->setFont(QFont("Arial", 14));
    hintText->setPos(GAME_WIDTH/2 - 200, GAME_HEIGHT/2 - 100);
    hintText->setZValue(100);
    scene->addItem(hintText);

    // 3秒后自动移除提示文字
    QTimer::singleShot(3000, [hintText, this]() {
        if (hintText && scene->items().contains(hintText)) {
            scene->removeItem(hintText);
            delete hintText;
        }
    });
}

// 移除超市购买界面
void SurvivorGame::removeSupermarketInterface()
{
    inSupermarketInterface = false;
    removePurchaseImages();
}
void SurvivorGame::handleSupermarketEnter()
{
    if (inSupermarketInterface) {
        // 如果正在显示购买框，则进行购买确认
        if (isBuyingFood) {
            confirmFoodPurchase();
        } else if (isBuyingBullet) {
            confirmBulletPurchase();
        } else {
            // 否则根据玩家位置显示对应的购买框
            QPointF playerPos = player->pos();

            QRectF bulletArea(GAME_WIDTH * 0.18, GAME_HEIGHT * 0.76, 80, 80);    // 子弹购买区域
            QRectF foodArea(GAME_WIDTH * 0.70, GAME_HEIGHT * 0.73, 80, 80);   // 食物购买区域
            if (bulletArea.contains(playerPos)) {
                showBulletPurchaseImage();
            }
            else if (foodArea.contains(playerPos)) {
                showFoodPurchaseImage();
            }
        }
    }
}
void SurvivorGame::showFoodPurchaseImage()
{
    // 移除其他购买框
    removePurchaseImages();

    // 加载并显示食物购买图片
    foodPurchaseImage = new QGraphicsPixmapItem(QPixmap(SUPERMARKET_FOOD_PATH));
    foodPurchaseImage->setPos(GAME_WIDTH/2 - 120, GAME_HEIGHT/2 - 50);
    foodPurchaseImage->setScale(0.2);
    foodPurchaseImage->setZValue(0);
    scene->addItem(foodPurchaseImage);

    isBuyingFood = true;

}

void SurvivorGame::showBulletPurchaseImage()
{
    // 移除其他购买框
    removePurchaseImages();

    // 加载并显示子弹购买图片
    bulletPurchaseImage = new QGraphicsPixmapItem(QPixmap(SUPERMARKET_AMMO_PATH));
    bulletPurchaseImage->setPos(GAME_WIDTH/2 + 20, GAME_HEIGHT/2 - 50);
    bulletPurchaseImage->setScale(0.4);
    bulletPurchaseImage->setZValue(0);
    scene->addItem(bulletPurchaseImage);

    isBuyingBullet = true;

}
void SurvivorGame::confirmFoodPurchase()
{
    Supermarket supermarket;
    if (supermarket.buyFood(player)) {
        // 购买成功提示
        QGraphicsTextItem *successText = new QGraphicsTextItem("购买食物成功！");
        successText->setDefaultTextColor(Qt::green);
        successText->setFont(QFont("Microsoft YaHe", 16, QFont::Bold));
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
        QGraphicsTextItem *failText = new QGraphicsTextItem("金币不足，购买失败！");
        failText->setDefaultTextColor(Qt::red);
        failText->setFont(QFont("Microsoft YaHe", 16, QFont::Bold));
        failText->setPos(GAME_WIDTH/2 - 80, GAME_HEIGHT/2 + 50);
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

    removePurchaseImages();
    drawHUD();
}

void SurvivorGame::confirmBulletPurchase()
{
    Supermarket supermarket;
    if (supermarket.buyBullet(player)) {
        // 购买成功提示
        QGraphicsTextItem *successText = new QGraphicsTextItem("购买子弹成功！");
        successText->setDefaultTextColor(Qt::green);
        successText->setFont(QFont("Microsoft YaHe", 16, QFont::Bold));
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
        QGraphicsTextItem *failText = new QGraphicsTextItem("金币不足，购买失败！");
        failText->setDefaultTextColor(Qt::red);
        failText->setFont(QFont("Microsoft YaHe", 16, QFont::Bold));
        failText->setPos(GAME_WIDTH/2 - 80, GAME_HEIGHT/2 + 50);
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

    removePurchaseImages();
    drawHUD();
}

void SurvivorGame::cancelPurchase()
{
    removePurchaseImages();
}

void SurvivorGame::removePurchaseImages()
{
    if (foodPurchaseImage) {
        if (scene->items().contains(foodPurchaseImage)) {
            scene->removeItem(foodPurchaseImage);
        }
        delete foodPurchaseImage;
        foodPurchaseImage = nullptr;
    }

    if (bulletPurchaseImage) {
        if (scene->items().contains(bulletPurchaseImage)) {
            scene->removeItem(bulletPurchaseImage);
        }
        delete bulletPurchaseImage;
        bulletPurchaseImage = nullptr;
    }

    isBuyingFood = false;
    isBuyingBullet = false;
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
            successText->setFont(QFont("Microsoft YaHe", 16, QFont::Bold));
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
            failText->setFont(QFont("Microsoft YaHe", 16, QFont::Bold));
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
            successText->setFont(QFont("Microsoft YaHe", 16, QFont::Bold));
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
            failText->setFont(QFont("Microsoft YaHe", 16, QFont::Bold));
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
    // qDebug()<<player->pos()<<"\n";
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
            INITIAL_ENEMY_HEALTH+=10;
            MONSTER_RATE*=1.1;
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
    //是否生成精英怪
    float randomVal = QRandomGenerator::global()->bounded(1, 101);
    if(randomVal/100>MONSTER_RATE+0.66){
        QPixmap pixmap(MONSTER_PATH);
        QPixmap scaledPixmap = pixmap.scaled(
            pixmap.width() * 0.1,  // 宽度放大1.5倍
            pixmap.height() * 0.1, // 高度放大1.5倍
            Qt::KeepAspectRatio,   // 保持宽高比，避免变形
            Qt::SmoothTransformation // 平滑缩放，画质更好
            );
        enemy->setPixmap(scaledPixmap);
        enemy->health*=3;
        enemy->speed+=2;
        enemy->damage+=50;
    }
    scene->addItem(enemy);
    enemies.push_back(enemy);


}


void SurvivorGame::checkCollisions()
{
    // 子弹与敌人碰撞
    int f_b=0;
    auto bulleti=bullets.begin();
    for (auto bullet : bullets) {
        for (auto it = enemies.begin(); it != enemies.end();) {
            if (bullet->collidesWithItem(*it)) {
                (*it)->takeDamage(bullet->getDamage(),items);
                scene->removeItem(bullet);
                bullet->removeMe=1;
                f_b=1;
                break;
            }
            ++it;
        }
        if(f_b)break;
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
    if(player){
        qreal distance;
        for(auto it:buildings){
            distance=qSqrt(qPow(it->pos().x()-player->pos().x(),2)+qPow(it->pos().y()-player->pos().y(),2));
            if(distance<=TELEPORT_INTERACTION_RADIUS){
                return it;
            }
        }
        return nullptr;
    }
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

    // 播放结束音乐
    stopBackgroundMusic();
    backgroundMusic->setSource(QUrl(GAME_END_BGM));
    backgroundMusic->setLoops(QMediaPlayer::Once);
    playBackgroundMusic();
}

void SurvivorGame::checkPortalInteraction()
{
    // 检查玩家是否在传送门附近并且按下了Enter键
    QPointF playerPos = player->pos();
    QPointF portalPos;
    int targetMapId = -1;

    if (currentMapId == 2) {
        portalPos = QPointF(TELEPORT_MAP_2_POS_X, TELEPORT_MAP_2_POS_Y);
        targetMapId = 1;
    } else {
        portalPos = QPointF(TELEPORT_MAP_1_POS_X, TELEPORT_MAP_1_POS_Y);
        targetMapId = 2;
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
            if(currentMapId == 7){
                setMusicVolume(0.3);
            }
            if(currentMapId == 5){
                setMusicVolume(0.3);
            }
            if(currentMapId == 8){
                setMusicVolume(0.3);
                hideLibraryReadingElements();
                hideLibraryTextbookButtons();
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


