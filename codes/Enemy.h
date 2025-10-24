#ifndef ENEMY_H
#define ENEMY_H

#include <QGraphicsPixmapItem>
#include <QPointF>

class Player;

class Enemy : public QGraphicsPixmapItem
{
public:
    Enemy(Player *target, QGraphicsItem *parent = nullptr);
    ~Enemy();
    
    void update();
    void takeDamage(int damage);
    
    // Getter methods
    int getHealth() const { return health; }
    int getDamage() const { return damage; }
    bool isDead() const { return health <= 0; }
    
private:
    Player *target;
    int health;
    int damage;
    int speed;
    
    void loadSprite();
    void moveTowardsTarget();
    void dropItem();
};

#endif // ENEMY_H