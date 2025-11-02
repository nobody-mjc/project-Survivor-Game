#include "Item.h"
#include "Player.h"
#include "define.h"
#include <QGraphicsScene>
#include <QPainter>

Item::Item(ItemType type, QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent), 
      type(type), 
      removeMe(false),
      lifetime(0)
{
    loadSprite();
    setTransformOriginPoint(boundingRect().center());
}

Item::~Item()
{
}

void Item::loadSprite()
{
    // 尝试根据物品类型加载对应的图片文件
    QPixmap pixmap;
    QColor itemColor;
    QString itemText;
    
    switch (type) {
    case HealthPack:
        pixmap = QPixmap(HEALTH_PACK_IMAGE_PATH);
        itemColor = HEALTH_PACK_COLOR;
        itemText = "H";
        break;
    case AmmoPack:
        pixmap = QPixmap(AMMO_PACK_IMAGE_PATH);
        itemColor = AMMO_PACK_COLOR;
        itemText = "A";
        break;
    case SpeedBoost:
        pixmap = QPixmap(SPEED_BOOST_IMAGE_PATH);
        itemColor = SPEED_BOOST_COLOR;
        itemText = "S";
        break;
    default:
        pixmap = QPixmap();
        itemColor = Qt::gray;
        itemText = "?";
    }
    
    // 如果图片加载失败，使用默认绘制的图形
    if (pixmap.isNull()) {
        pixmap = QPixmap(ITEM_SIZE, ITEM_SIZE);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setBrush(itemColor);
        painter.drawEllipse(5, 5, ITEM_SIZE - 10, ITEM_SIZE - 10);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 12, QFont::Bold));
        painter.drawText(5, 5, ITEM_SIZE - 10, ITEM_SIZE - 10, Qt::AlignCenter, itemText);
    }
    
    setPixmap(pixmap);
}

void Item::update()
{
    // 物品闪烁动画
    lifetime+=1000;
    if (lifetime % 10 == 0) {
        setOpacity(opacity() > 0.5 ? 0.5 : 1.0);
    }
    
    // 旋转动画
    setRotation(rotation() + 1);
    
    // 根据定义的生命周期自动消失
    if (lifetime > ITEM_LIFETIME) {
        removeMe = true;
    }
}

void Item::applyEffect(Player *player)
{
    // 应用物品效果给玩家
    switch (type) {
    case HealthPack:
        player->addHealth(HEALTH_PACK_VALUE);
        break;
    case AmmoPack:
        player->addAmmo(AMMO_PACK_VALUE);
        break;
    case SpeedBoost:
        // 这里可以实现速度提升效果
        // 由于Player类中没有直接的速度修改方法，暂时给玩家增加一些生命值
        player->addHealth(SPEED_BOOST_VALUE);
        break;
    }
    
    removeMe = true;
    
    // 播放拾取特效
    setOpacity(0);
}
