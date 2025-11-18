#include "Enemy.h"
#include "Player.h"
#include "Item.h"
#include "define.h"
#include <QGraphicsScene>
#include <QPainter>
#include <QRandomGenerator>
#include <QTimer>
#include <cmath>


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


void Enemy::takeDamage(int damage,std::vector<Item*> &items)
{
    health -= damage;
    if (health <= 0) {
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
