#include "library.h"
#include "Player.h"
#include "define.h"
#include <QRandomGenerator>

Library::Library()
{
    setPos(335, 686);
}

QString Library::update(Player *player)
{
    QString end = "";
    end = applyeffect(player);
    return end;
}

QString Library::applyeffect(Player *player)
{
    if (QRandomGenerator::global()->bounded(100) < 50) {
        return "吸取知识力量！能力提升！";
    } else {
        return "在图书馆静心学习";
    }
}
