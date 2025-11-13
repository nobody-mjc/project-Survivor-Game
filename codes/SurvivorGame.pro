QT += core gui multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SurvivorGame
TEMPLATE = app

# 源文件
SOURCES += \
    building.cpp \
    canteen.cpp \
    classroom.cpp \
    define.cpp \
    hostel.cpp \
    main.cpp \
    SurvivorGame.cpp \
    Player.cpp \
    Enemy.cpp \
    Bullet.cpp \
    Item.cpp \
    Map.cpp \
    playground.cpp \
    supermarket.cpp \
    teacher.cpp
# 头文件
HEADERS += \
    SurvivorGame.h \
    Player.h \
    Enemy.h \
    Bullet.h \
    Item.h \
    Map.h \
    building.h \
    canteen.h \
    classroom.h \
    define.h \
    hostel.h \
    playground.h \
    supermarket.h \
    teacher.h

# 资源文件
# 如果需要添加图片等资源，可以创建.qrc文件并在这里引用
# RESOURCES += resources.qrc

# 编译器选项
QMAKE_CXXFLAGS += -std=c++17

# 确保中文显示正常
QMAKE_CXXFLAGS += -DUNICODE -D_UNICODE

RESOURCES += \
    res.qrc
