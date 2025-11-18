#ifndef GAMEEXITDIALOG_H
#define GAMEEXITDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class GameExitDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GameExitDialog(QWidget *parent = nullptr) : QDialog(parent) {
        setWindowTitle("确认退出");
        setFixedSize(300, 150);

        QLabel *label = new QLabel("确定要退出游戏吗？", this);
        label->setAlignment(Qt::AlignCenter);

        QPushButton *exitBtn = new QPushButton("退出游戏", this);

        QVBoxLayout *layout = new QVBoxLayout;
        QHBoxLayout *btnLayout = new QHBoxLayout;

        btnLayout->addWidget(exitBtn);

        layout->addWidget(label);
        layout->addLayout(btnLayout);

        setLayout(layout);

        connect(exitBtn, &QPushButton::clicked, this, &QDialog::accept);
    }
};

#endif // GAMEEXITDIALOG_H
