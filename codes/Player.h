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
    void takeFoodGauge(float decrease);
    void addHealth(int amount);
    void addAmmo(int amount);
    void addDamage(int addition);
    void addFoodGuage(float increase);
    void resumeCoins(int num); // 消耗金币
    void addCoins(int num); // 增加金币
    // Getter methods
    int getCoinns() const {return coins;};
    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
    int getAmmo() const { return ammo; }
    int getDamage()const {return damage;}
    float getFoodGauge()const {return foodGauge;}
    bool isDead() const { return health <= 0; }
    
private:
    int health;
    int maxHealth;
    int ammo;
    int speed;
    int damage;
    int fireRate;
    int lastShootTime;
    float foodGauge;
    int coins;
    
    void rotateToMouse(const QPointF &targetPos);
    void loadSprite();
};

#endif // PLAYER_H
