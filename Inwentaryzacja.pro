QT       += core gui sql widgets

CONFIG += c++17

VERSION = 1.1.0
QMAKE_TARGET_COMPANY = Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
QMAKE_TARGET_PRODUCT = Inwentaryzacja
QMAKE_TARGET_DESCRIPTION = Program do inwentaryzacji retro komputerów

# Ikona aplikacji
win32:RC_ICONS = images/icon.ico
macx:ICON = images/icon.icns

# Uncomment, aby wymusić kompilację bez przestarzałych API
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
unix:!macx:QMAKE_POST_LINK += $$quote($$PWD/set_icon.sh $$OUT_PWD/Inwentaryzacja $$PWD/images/icon.png)

# QMAKE_POST_LINK += $$quote(/home/arekbr/projektyQT/Inwentaryzacja/set_icon.sh $$OUT_PWD/Inwentaryzacja /home/arekbr/projektyQT/Inwentaryzacja/images/icon.png)

SOURCES += \
    DatabaseConfigDialog.cpp \
    fullscreenphotoviewer.cpp \
    itemList.cpp \
    main.cpp \
    mainwindow.cpp \
    models.cpp \
    photoitem.cpp \
    status.cpp \
    storage.cpp \
    types.cpp \
    utils.cpp \
    vendors.cpp

HEADERS += \
    DatabaseConfigDialog.h \
    fullscreenphotoviewer.h \
    itemList.h \
    mainwindow.h \
    models.h \
    photoitem.h \
    status.h \
    storage.h \
    types.h \
    utils.h \
    vendors.h

FORMS += \
    DatabaseConfigDialog.ui \
    itemList.ui \
    mainwindow.ui \
    models.ui \
    status.ui \
    storage.ui \
    types.ui \
    vendors.ui

TRANSLATIONS += \
    Inwentaryzacja_pl_PL.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    CHANGELOG.md \
    Create_MySQL.sql \
    README.md \
    createTable.sql \
    database_example_record.sql \
    images/256ikona.png \
    images/icon.ico \
    images/icon.png \
    images/ikona.ico \
    images/ikona256.ico \
    images/ikona_mac.icns \
    migrate_to_uuid.sql \
    set_icon.sh

RESOURCES += \
    images/icon.qrc
