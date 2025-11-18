#ifndef MENUWINDOW_H
#define MENUWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include "SurvivorGame.h"

class MenuWindow : public QMainWindow
{
    Q_OBJECT
public:
    MenuWindow(QWidget *parent = nullptr);
    ~MenuWindow();
private:
    // 开始新游戏
    void onStartNewGame();
    // 读取存档
    void onLoadGame();
    // 退出游戏
    void onExitGame();
    SurvivorGame *game;
    QString save_path;
    void initUI();
};

#endif // MENUWINDOW_H
