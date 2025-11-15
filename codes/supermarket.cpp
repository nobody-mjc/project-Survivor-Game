#include "supermarket.h"


Supermarket::Supermarket()
{
    setPos(275, 205);
}

Supermarket::~Supermarket(){}

QString Supermarket::update (Player *player){}

QString Supermarket::applyeffect(Player *player){}

bool Supermarket::buyFood(Player *player)
{
    if (player->getMoney() >= 10) {
        player->setMoney(player->getMoney() - 10);
        player->setFoodGauge(qMin(100.0f, player->getFoodGauge() + 15));
        return true;
    }
    return false;
}

bool Supermarket::buyBullet(Player *player)
{
    if (player->getMoney() >= 20) {
        player->setMoney(player->getMoney() - 20);
        player->setAmmo(player->getAmmo() + 15);
        return true;
    }
    return false;
}
