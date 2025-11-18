#include "menuwindow.h"
#include "define.h"
#include <QVBoxLayout>
#include <QFont>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

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
    setWindowTitle("南苏物语");

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
    if(!save_path.isEmpty()){
        game=new SurvivorGame(save_path);
        game->show();
        this->hide();
    }
}

void MenuWindow::onStartNewGame(){
    game=new SurvivorGame();
    game->show();
    this->hide();
}

void MenuWindow::onLoadGame(){
    // 打开文件选择对话框
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("选择存档文件"),
        QDir::currentPath(),
        tr("所有文件 (*);;存档文件 (*.sav)")
        );

    // 如果用户选择了文件
    if (!filePath.isEmpty()) {
        // 显示选择的文件路径（实际应用中可以使用这个路径来加载游戏存档）
        QMessageBox::information(this, tr("文件已选择"), tr("选择的文件路径: ") + filePath);
        save_path=filePath;
        // 在这里可以添加加载存档的逻辑，使用获取到的filePath
        // 例如：loadGameFromFile(filePath);
    }
}
void MenuWindow::onExitGame(){
    close();
}
