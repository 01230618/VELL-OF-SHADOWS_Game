QT += widgets
QT += multimedia
QT += multimedia multimediawidgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Intro.cpp \
    Level2.cpp \
    Level3.cpp \
    Player.cpp \
    Roleselect.cpp \
    Rule.cpp \
    StructDef.cpp \
    level1.cpp \
    main.cpp \
    Widget.cpp

HEADERS += \
    Intro.h \
    Level2.h \
    Level3.h \
    Player.h \
    Roleselect.h \
    Rule.h \
    StructDef.h \
    Widget.h \
    level1.h

FORMS += \
    Intro.ui \
    Roleselect.ui \
    Rule.ui \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Role1.qrc \
    Role2.qrc \
    Role3.qrc \
    Role4.qrc \
    musics.qrc \
    resource.qrc \
    ui.qrc
