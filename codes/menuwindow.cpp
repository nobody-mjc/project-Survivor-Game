#include "menuwindow.h"
#include "define.h"
#include <QVBoxLayout>
#include <QFont>
#include <QPushButton>

MenuWindow::MenuWindow(QWidget *parent) :QMainWindow(parent),game(nullptr)
{
    initUI();
}

MenuWindow::~MenuWindow()
{
}

void MenuWindow::initUI()
{
    setFixedSize(GAME_WIDTH, GAME_HEIGHT);
    setWindowTitle("幸存者游戏");

    QWidget *central=new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setSpacing(30);
    mainLayout->setContentsMargins(150,150,150,150);

    QFont ButtonFont("Arial", 16);

    QPushButton *startBtn=new QPushButton("开始新游戏",this);
    startBtn->setFont(ButtonFont);
    startBtn->setMinimumHeight(60);
    startBtn->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; border-radius: 10px; }"
        "QPushButton:hover { background-color: #45a049; }"
        "QPushButton:pressed { background-color: #3d8b40; }"
        );

    QPushButton *loadBtn = new QPushButton("读取存档", this);
    loadBtn->setFont(ButtonFont);
    loadBtn->setMinimumHeight(60);
    loadBtn->setStyleSheet(
        "QPushButton { background-color: #2196F3; color: white; border-radius: 10px; }"
        "QPushButton:hover { background-color: #0b7dda; }"
        "QPushButton:pressed { background-color: #0a69b7; }"
        );

    QPushButton *exitBtn = new QPushButton("退出游戏", this);
    exitBtn->setFont(ButtonFont);
    exitBtn->setMinimumHeight(60);
    exitBtn->setStyleSheet(
        "QPushButton { background-color: #f44336; color: white; border-radius: 10px; }"
        "QPushButton:hover { background-color: #d32f2f; }"
        "QPushButton:pressed { background-color: #b71c1c; }"
        );

    mainLayout->addWidget(startBtn);
    mainLayout->addWidget(loadBtn);
    mainLayout->addWidget(exitBtn);

    setCentralWidget(central);

    connect(startBtn, &QPushButton::clicked, this, &MenuWindow::onStartNewGame);
    connect(loadBtn,&QPushButton::clicked, this,&MenuWindow::onLoadGame);
    connect(exitBtn,&QPushButton::clicked, this,&MenuWindow::onExitGame);
}

void MenuWindow::onStartNewGame(){
    game=new SurvivorGame();
    game->show();
    this->hide();
}

void MenuWindow::onLoadGame(){}

void MenuWindow::onExitGame(){}
