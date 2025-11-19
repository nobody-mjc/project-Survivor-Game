#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsPixmapItem>
#include <QPointF>
#include<fstream>
#include<QFile>
#include<QTextStream>
#include "Bullet.h"

class Player : public QGraphicsPixmapItem
{
public:
    Player(QGraphicsItem *parent = nullptr);
    ~Player();
    int f_shotgun;//子弹是否散射
    void updateMovement(bool keys[]);
    Bullet* shoot(const QPointF &targetPos,qreal fluctuation);
    void takeDamage(int damage);
    void takeFoodGauge(float decrease);
    void addHealth(int amount);
    void addAmmo(int amount);
    void addDamage(int addition);
    void addFoodGuage(float increase);
    void addMoney(float increase);
    // Getter methods
    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
    int getAmmo() const { return ammo; }
    int getDamage()const {return damage;}
    float getFoodGauge()const {return foodGauge;}
    float getMoney()const {return money;}
    bool isDead() const { return health <= 0; }
    void setMoney(float money) { this->money = money; }
    void setFoodGauge(float foodGauge) { this->foodGauge = foodGauge; }
    void setAmmo(int ammo) { this->ammo = ammo; }

    //老师的技能
    void add_crit_rate(float increase);
    void add_MaxHealth(int increase);

    //存档
    void save(std::string saving_name,int mapId);
    int read_saving(std::string saving_path);
private:
    bool facingRight = true;   // true = 面向右，false = 面向左
    int health;
    int maxHealth;
    int ammo;
    int speed;
    int damage;
    int fireRate;
    int lastShootTime;
    int time_for_hunger;
    float foodGauge;
    float money;
    float crit,crit_rate;

    void rotateToMouse(const QPointF &targetPos);
    void loadSprite();
};

#endif // PLAYER_H
