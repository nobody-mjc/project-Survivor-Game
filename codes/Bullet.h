#ifndef BULLET_H
#define BULLET_H

#include <QGraphicsPixmapItem>

class Bullet : public QGraphicsPixmapItem
{
public:
    Bullet(int damage, QGraphicsItem *parent = nullptr);
    ~Bullet();
    
    void update();
    void setDirection(qreal dx, qreal dy);
    
    // Getter methods
    int getDamage() const { return damage; }
    bool shouldRemove() const { return removeMe; }
    
private:
    int damage;
    int speed;
    qreal dirX;
    qreal dirY;
    bool removeMe;
    int lifetime;
    
    void loadSprite();
};

#endif // BULLET_H