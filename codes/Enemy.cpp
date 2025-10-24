#include "Enemy.h"
#include "Player.h"
#include "Item.h"
#include "define.h"
#include <QGraphicsScene>
#include <QPainter>
#include <QRandomGenerator>
#include <QTimer>

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
    }
    
    setPixmap(pixmap);
}

void Enemy::update()
{
    if (!target || target->isDead()) return;
    
    moveTowardsTarget();
}

void Enemy::moveTowardsTarget()
{
    // 计算敌人指向玩家的方向
    QLineF line(pos() + boundingRect().center(), target->pos() + target->boundingRect().center());
    setRotation(-line.angle());
    
    // 向玩家移动
    qreal dx = line.unitVector().dx() * speed;
    qreal dy = line.unitVector().dy() * speed;
    setPos(pos() + QPointF(dx, dy));
}

void Enemy::takeDamage(int damage)
{
    health -= damage;
    if (health <= 0) {
        // 死亡时掉落物品
        dropItem();
    } else {
        // 受伤闪烁效果
        setOpacity(0.7);
        QTimer::singleShot(50, [=]() {
            setOpacity(1.0);
        });
    }
}

void Enemy::dropItem()
{
    // 根据定义的概率掉落物品
    if (QRandomGenerator::global()->bounded(100) < ITEM_DROP_CHANCE) {
        Item::ItemType type;
        int rand = QRandomGenerator::global()->bounded(3);
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
            type = Item::HealthPack;
        }
        
        Item *item = new Item(type);
        item->setPos(pos());
        scene()->addItem(item);
    }
}
