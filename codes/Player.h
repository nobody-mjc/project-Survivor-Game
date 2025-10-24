#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsPixmapItem>
#include <QPointF>
#include "Bullet.h"

class Player : public QGraphicsPixmapItem
{
public:
    Player(QGraphicsItem *parent = nullptr);
    ~Player();
    
    void updateMovement(bool keys[]);
    Bullet* shoot(const QPointF &targetPos);
    void takeDamage(int damage);
    void addHealth(int amount);
    void addAmmo(int amount);
    
    // Getter methods
    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
    int getAmmo() const { return ammo; }
    bool isDead() const { return health <= 0; }
    
private:
    int health;
    int maxHealth;
    int ammo;
    int speed;
    int damage;
    int fireRate;
    int lastShootTime;
    
    void rotateToMouse(const QPointF &targetPos);
    void loadSprite();
};

#endif // PLAYER_H