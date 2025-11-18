#ifndef GAMEEXITDIALOG_H
#define GAMEEXITDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QMainWindow>
#include "Player.h"

class GameExitDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GameExitDialog(Player* player, int currentMapId,QWidget *parent = nullptr);
    Player* m_player; // 保存Player指针
    int m_mapId;      // 保存当前地图ID
};

#endif // GAMEEXITDIALOG_H
