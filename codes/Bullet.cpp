#include "Bullet.h"
#include "define.h"
#include <QGraphicsScene>
#include <QPainter>

//try

Bullet::Bullet(int damage, QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent), 
      damage(damage), 
      speed(BULLET_SPEED), 
      dirX(0), 
      dirY(0), 
      removeMe(false),
      lifetime(0)
{
    loadSprite();
    setTransformOriginPoint(boundingRect().center());
}

Bullet::~Bullet()
{
}

void Bullet::loadSprite()
{
    // 尝试加载图片文件
    QPixmap pixmap(BULLET_IMAGE_PATH);
    
    // 如果图片加载失败，使用默认绘制的图形
    if (pixmap.isNull()) {
        pixmap = QPixmap(BULLET_WIDTH, BULLET_HEIGHT);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setBrush(BULLET_COLOR);
        painter.drawRect(0, 0, BULLET_WIDTH, BULLET_HEIGHT);
    }
    
    setPixmap(pixmap);
}

void Bullet::setDirection(qreal dx, qreal dy)
{
    dirX = dx;
    dirY = dy;
}

void Bullet::update()
{
    // 移动子弹
    setPos(pos() + QPointF(dirX * speed, dirY * speed));
    
    // 增加生命周期
    lifetime++;
    
    // 检查是否超出屏幕或生命周期过长
    if (pos().x() < -100 || pos().x() > GAME_WIDTH + 100 ||
        pos().y() < -100 || pos().y() > GAME_HEIGHT + 100 ||
        lifetime > BULLET_LIFETIME) {
        removeMe = true;
    }
}
