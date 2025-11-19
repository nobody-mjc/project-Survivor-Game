#include "Enemy.h"
#include "Player.h"
#include "Item.h"
#include "define.h"
#include <QGraphicsScene>
#include <QPainter>
#include <QRandomGenerator>
#include <QTimer>
#include <cmath>
#include <QMovie>
#include <QObject>


Enemy::Enemy(Player *target, QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent),
    target(target),
    health(INITIAL_ENEMY_HEALTH),
    damage(ENEMY_DAMAGE),
    speed(ENEMY_SPEED)
{
    loadSprite();
    setTransformOriginPoint(boundingRect().center());
}

Enemy::~Enemy()
{
}

void Enemy::loadSprite()
{
    // 尝试加载图片文件
    QPixmap pixmap(ENEMY_IMAGE_PATH);

    // 如果图片加载失败，使用默认绘制的图形
    if (pixmap.isNull()) {
        pixmap = QPixmap(ENEMY_SIZE, ENEMY_SIZE);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setBrush(ENEMY_COLOR);
        painter.drawEllipse(5, 5, ENEMY_SIZE - 10, ENEMY_SIZE - 10);
        painter.setBrush(Qt::black);
        painter.drawEllipse(10, 10, 6, 6); // 眼睛
        painter.drawEllipse(24, 10, 6, 6);
        painter.drawLine(10, 24, 30, 24);  // 嘴
    } else setPixmap(pixmap);
}

void Enemy::update()
{
    if (!target || target->isDead()) return;

    if (collidesWithItem(target)) {
        target->takeDamage(damage);
        return;
    }

    moveTowardsTarget();
}

void Enemy::moveTowardsTarget()
{
    // 计算敌人到玩家的方向
    QLineF line(mapToScene(boundingRect().center()), target->mapToScene(target->boundingRect().center()));
    // 计算目标距离
    qreal dist = line.length();
    // 将速度限制为与目标之间的距离（防止怪物跨越玩家）
    qreal step = (dist < speed) ? dist : speed;  // 如果距离小于速度，就按距离来移动，否则按速度移动
    // 归一化方向
    QPointF dir(line.unitVector().dx(), line.unitVector().dy());   // 获取方向向量并归一化
    setPos(pos() + dir * step);  // 更新位置
}

// 创建一个临时的特效项（继承自QGraphicsPixmapItem，用于显示动图帧）
class DeathEffect : public QGraphicsPixmapItem {
public:
    DeathEffect(QGraphicsScene* scene, const QPointF& pos) {
        scene->addItem(this);
        setPos(pos);
        setZValue(10); // 确保特效在最上层
        setScale(0.2);

        // 加载动图（父对象设为nullptr，手动管理内存）
        movie = new QMovie(":/newdeath.gif");
        if (!movie->isValid()) {
            qWarning() << "死亡动图加载失败！请检查：1.路径是否正确 2.文件是否为有效GIF";
            scene->removeItem(this);
            delete movie; // 手动释放动图
            delete this;
            return;
        }

        // 2. 使用QObject::connect静态函数（无需继承QObject）
        QObject::connect(movie, &QMovie::frameChanged, [=](int frame) {
            setPixmap(movie->currentPixmap()); // 更新当前帧图片

            // 处理最后一帧（兼容帧数获取失败的情况）
            int totalFrames = movie->frameCount();
            if (totalFrames > 0) {
                // 播放到最后一帧，延迟删除
                if (frame == totalFrames - 1) {
                    movie->stop();
                    QTimer::singleShot(50, [=]() {
                        scene->removeItem(this);
                        delete movie; // 释放动图内存
                        delete this;  // 释放特效对象
                    });
                }
            } else {
                // 无法获取帧数时，按预估时长（2秒）删除
                QTimer::singleShot(2000, [=]() {
                    scene->removeItem(this);
                    delete movie;
                    delete this;
                });
            }
        });

        movie->start(); // 开始播放
    }

private:
    QMovie* movie;
};

class BoomEffect : public QGraphicsPixmapItem {
public:
    BoomEffect(QGraphicsScene* scene, const QPointF& pos) {
        scene->addItem(this);
        setPos(pos);
        setZValue(10); // 确保特效在最上层
        setScale(0.02);

        // 加载动图（父对象设为nullptr，手动管理内存）
        movie = new QMovie(":/newboom.gif");
        if (!movie->isValid()) {
            qWarning() << "死亡动图加载失败！请检查：1.路径是否正确 2.文件是否为有效GIF";
            scene->removeItem(this);
            delete movie; // 手动释放动图
            delete this;
            return;
        }

        // 2. 使用QObject::connect静态函数（无需继承QObject）
        QObject::connect(movie, &QMovie::frameChanged, [=](int frame) {
            setPixmap(movie->currentPixmap()); // 更新当前帧图片

            // 处理最后一帧（兼容帧数获取失败的情况）
            int totalFrames = movie->frameCount();
            if (totalFrames > 0) {
                // 播放到最后一帧，延迟删除
                if (frame == totalFrames - 1) {
                    movie->stop();
                    QTimer::singleShot(50, [=]() {
                        scene->removeItem(this);
                        delete movie; // 释放动图内存
                        delete this;  // 释放特效对象
                    });
                }
            } else {
                // 无法获取帧数时，按预估时长（2秒）删除
                QTimer::singleShot(2000, [=]() {
                    scene->removeItem(this);
                    delete movie;
                    delete this;
                });
            }
        });

        movie->start(); // 开始播放
    }

private:
    QMovie* movie;
};


void Enemy::takeDamage(int damage,std::vector<Item*> &items)
{
    health -= damage;
    if(damage>PLAYER_DAMAGE){
        new BoomEffect(this->scene(),this->pos());
    }
    if (health <= 0) {
        // 在敌人当前位置创建死亡特效
        new DeathEffect(this->scene(), this->pos()); // 传入场景和敌人位置
        // 死亡时掉落物品
        dropItem(items);
    } else {
        // 受伤闪烁效果
        /*setOpacity(0.7);
        QTimer::singleShot(50, [=]() {
            if(this){
                setOpacity(1.0);
            }
        });*/
    }
}

void Enemy::dropItem(std::vector<Item*> &items)
{
    // 根据定义的概率掉落物品
    if (QRandomGenerator::global()->bounded(100) < ITEM_DROP_CHANCE) {
        Item::ItemType type;
        int rand = QRandomGenerator::global()->bounded(4);
        switch (rand) {
        case 0:
            type = Item::HealthPack;
            break;
        case 1:
            type = Item::AmmoPack;
            break;
        case 2:
            type = Item::SpeedBoost;
            break;
        default:
            type = Item::MoneyPack;
        }

        Item *item = new Item(type);
        items.push_back(item);
        item->setPos(pos());
        scene()->addItem(item);
    }
}
