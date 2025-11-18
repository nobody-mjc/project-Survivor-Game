#include "GameExitDialog.h"
#include "define.h"
#include "Player.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QApplication>

GameExitDialog::GameExitDialog(Player* player, int currentMapId,QWidget *parent): QDialog(parent),m_player(player), m_mapId(currentMapId) {
    setWindowTitle("确认退出");
    setFixedSize(GAME_WIDTH, GAME_HEIGHT); // 使用游戏窗口大小

    QLabel* background = new QLabel(this);
    QPixmap bgPixmap(MENU_PATH);
    background->setFixedSize(GAME_WIDTH, GAME_HEIGHT);
    bgPixmap = bgPixmap.scaled(GAME_WIDTH, GAME_HEIGHT,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    background->setPixmap(bgPixmap);
    background->lower();

    QWidget *centralWidget = new QWidget(this);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setSpacing(30);
    mainLayout->setContentsMargins(150, 150, 150, 150);

    QLabel *label = new QLabel("确定要退出游戏吗？", centralWidget);
    label->setAlignment(Qt::AlignCenter);
    QFont labelFont("Arial", 20, QFont::Bold);
    label->setFont(labelFont);
    mainLayout->addWidget(label);

    QFont ButtonFont("Arial", 16);

    QPushButton *loadBtn = new QPushButton("设置存档", centralWidget);
    loadBtn->setFont(ButtonFont);
    loadBtn->setMinimumHeight(60);
    loadBtn->setStyleSheet(
        "QPushButton { background-color: #2196F3; color: white; border-radius: 10px; }"
        "QPushButton:hover { background-color: #0b7dda; }"
        "QPushButton:pressed { background-color: #0a69b7; }"
        );

    QPushButton *exitBtn = new QPushButton("退出游戏", centralWidget);
    exitBtn->setFont(ButtonFont);
    exitBtn->setMinimumHeight(60);
    exitBtn->setStyleSheet(
        "QPushButton { background-color: #f44336; color: white; border-radius: 10px; }"
        "QPushButton:hover { background-color: #d32f2f; }"
        "QPushButton:pressed { background-color: #b71c1c; }"
        );

    mainLayout->addWidget(loadBtn);
    mainLayout->addWidget(exitBtn);

    QVBoxLayout *dialogLayout = new QVBoxLayout(this); // 对话框的主布局
    dialogLayout->addWidget(centralWidget); // 放入中央部件
    dialogLayout->setContentsMargins(0, 0, 0, 0); // 去除对话框边缘间距

    connect(loadBtn, &QPushButton::clicked, this, [=]() {
        // 弹出输入框，获取用户输入的存档名称
        bool ok;
        QString inputStyle = R"(
        QInputDialog {
            background-color: #f0f0f0; /* 背景色 */
            border: 2px solid #4a90e2; /* 边框 */
            border-radius: 8px;
        }
        QInputDialog QLabel {
            font: 16px "微软雅黑"; /* 提示文字字体 */
            color: #333333; /* 文字颜色 */
            padding: 10px;
        }
        QInputDialog QLineEdit {
            font: 16px "微软雅黑"; /* 输入框字体 */
            padding: 8px;
            border: 1px solid #ccc;
            border-radius: 4px;
            background-color: white;
        }
        QInputDialog QPushButton {
            font: 14px "微软雅黑";
            padding: 6px 15px;
            border-radius: 4px;
            background-color: #4a90e2;
            color: white;
        }
        QInputDialog QPushButton:hover {
            background-color: #3a80d2;
        }
        )";
        // 应用样式到所有QInputDialog
        qApp->setStyleSheet(inputStyle);
        QString saveName = QInputDialog::getText(
            this,                  // 父窗口
            "输入存档名称",         // 标题
            "请输入存档名称:",      // 提示文本
            QLineEdit::Normal,     // 输入框类型
            "save",               // 默认名称
            &ok                    // 确认标志
        );
        qApp->setStyleSheet("");

        // 如果用户点击了确认且输入不为空
        if (ok && !saveName.isEmpty()) {
            // 调用Player的save函数（转换为std::string）
            m_player->save(saveName.toStdString(), m_mapId);
            // 可添加提示：存档成功
            QMessageBox::information(this, "提示", "存档成功！");
            // 关闭对话框（可选）
            this->accept();
        }
    });
    connect(exitBtn, &QPushButton::clicked, this, &QDialog::accept);
}
