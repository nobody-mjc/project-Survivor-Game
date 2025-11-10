#ifndef ITEM_H
#define ITEM_H

#include <QGraphicsPixmapItem>

class Player;

class Item : public QGraphicsPixmapItem
{
public:
    enum ItemType {
        HealthPack,
        AmmoPack,
        SpeedBoost
    };

    Item(ItemType type, QGraphicsItem *parent = nullptr);
    ~Item();

    void update();
    void applyEffect(Player *player);

    bool shouldRemove() const { return removeMe; }

private:
    ItemType type;
    bool removeMe;
    int lifetime;
    int life_of_speedbuff,flag_of_speedbuff;

    void loadSprite();
};

#endif // ITEM_H
