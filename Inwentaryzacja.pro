QT       += core gui sql widgets

CONFIG += c++17

# Uncomment, aby wymusić kompilację bez przestarzałych API
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
    itemList.cpp \
    main.cpp \
    mainwindow.cpp \
    models.cpp \
    photoitem.cpp \
    status.cpp \
    storage.cpp \
    types.cpp \
    vendors.cpp

HEADERS += \
    itemList.h \
    mainwindow.h \
    models.h \
    photoitem.h \
    status.h \
    storage.h \
    types.h \
    vendors.h

FORMS += \
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
    README.md \
    createTable.sql \
    database_example_record.sql
