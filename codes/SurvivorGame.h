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
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QFont>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QPushButton>
#include <QProgressBar>
#include <QGraphicsProxyWidget>

class HUDWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HUDWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setAttribute(Qt::WA_TranslucentBackground);
    }

    // HUD 数据
    int hp = 0;
    int ammo = 0;
    int wave = 0;
    int score = 0;
    int money = 0;
    float foodGauge = 0;
    int mapId = 1;


protected:
    void paintEvent(QPaintEvent *) override {
        if (mapId==2) return ;
        QPainter p(this);
        p.setPen(Qt::white);
        p.setFont(QFont("Arial", 16));

        int x = 10;
        int y = 10;
        int line = 25;

        p.drawText(x, y += line, QString("生命: %1").arg(hp));
        p.drawText(x, y += line, QString("弹药: %1").arg(ammo));
        p.drawText(x, y += line, QString("波次: %1").arg(wave));
        p.drawText(x, y += line, QString("分数: %1").arg(score));
        p.drawText(x, y += line, QString("饱食度: %1").arg(foodGauge));
        p.drawText(x, y += line, QString("金币: %1").arg(money));
    }
};

class SurvivorGame : public QMainWindow
{
    Q_OBJECT

public:
    SurvivorGame(QString save_path="",QWidget *parent = nullptr);
    ~SurvivorGame();
    void playBackgroundMusic();
    void pauseBackgroundMusic();
    void stopBackgroundMusic();
    void setMusicVolume(float volume);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void initGameWithMap(int mapId);
    void checkPortalInteraction();
    void initMap2Buildings();
    void learnNewSkill();
    void closeEvent(QCloseEvent *event) override;
    void sleepInHostel();
    void eatInCanteen();

private slots:
    void updateGame();
    void spawnEnemy();
    void checkCollisions();
    building* checkCollisions_buildings();
    void shiftToMap(int mapId);
    void handleEnterPressed();
    void handleBuildingInteraction();
    void updateFadeEffect();
    // 图书馆槽函数
    void initLibraryInterface(); // 图书馆初始化
    void onLibraryTextbookClicked(int index); // 按钮点击事件
    void updateLibraryCountdown(); // 更新时钟
    void onLibraryCountdownFinished(); // 倒计时结束，发放技能
    void showLibraryTextbooks(); // 显示四个按钮
    void hideLibraryTextbookButtons(); // 隐藏四个按钮
    void hideLibraryReadingElements(); // 隐藏阅读相关元素
    // 南雍楼槽函数
    void updateClassroomCountdown(); // 更新时钟
    void hideClassroomLearningElement(); // 隐藏学习相关元素
    void inClassroomCountdownFinished(); // 倒计时结束，发放老师技能
private:
    HUDWidget *hud = nullptr;
    void updateCamera();
    QPointF cameraOffset;     // 当前相机偏移
    float mouseInfluence = 0.25; // 鼠标影响力度（越大越偏向鼠标）
    float cameraSmooth = 0.15;   // 相机平滑系数
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
    QMediaPlayer *backgroundMusic;
    QAudioOutput *audioOutput;
    void initGame();
    void drawHUD();
    void endGame();
    void createSupermarketInterface();
    void removeSupermarketInterface();
    void handleSupermarketButtonClick(QPointF clickPos);
    void initBackgroundMusic();
    QGraphicsPixmapItem *foodPurchaseImage = nullptr;
    QGraphicsPixmapItem *bulletPurchaseImage = nullptr;
    bool isBuyingFood = false;
    bool isBuyingBullet = false;
    void handleSupermarketEnter();
    void showFoodPurchaseImage();
    void showBulletPurchaseImage();
    void confirmFoodPurchase();
    void confirmBulletPurchase();
    void cancelPurchase();
    void removePurchaseImages();
    QPushButton *libraryTextbookButtons[4]; // 四个按钮
    QGraphicsProxyWidget *libraryButtonProxies[4]; // 按钮代理
    QString textbookButtonTexts[4] = {"", "", "", ""}; // 按钮无文字
    bool isLibraryReading = false; // 是否正在读书
    QTimer *libraryCountdownTimer; // 20s计时
    int libraryRemainingTime = 20; // 读书剩余时间——用于显示显示时间
    QGraphicsProxyWidget *libraryClockProxy; // 钟代理
    QProgressBar *libraryCircularClock; // 美观倒计时时钟
    QMovie *libraryFlipBookGif; // 翻书动画化
    QGraphicsPixmapItem *libraryGifItem; // 动图
    int textbookIndex = -1; // 课本序号，发放技能
    // 南雍楼
    QTimer *classroomCountdownTimer; // 20s计时
    bool isLearning = false; // 是否正在学习
    QGraphicsProxyWidget *classroomClockProxy; // 钟代理
    QProgressBar *classroomCircularClock; // 美观倒计时时钟
    int classroomRemainingTime = 20; // 学习剩余时间——用于显示时间
    int teacherIndex = 0; // 老师编号
    bool hasShifted = false;

    //视频播放组件
    QMediaPlayer* media_player;
    QVideoWidget* widget_player;
};



#endif // SURVIVORGAME_H
