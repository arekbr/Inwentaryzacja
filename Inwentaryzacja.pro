##################################################
## Inwentaryzacja.pro — Qt/QMake project with
## fully-in-.pro, multiplatform deployment
##################################################

QT       += core gui sql widgets

CONFIG  += c++17 \
           sdk_no_version_check \
           lrelease \
           embed_translations
CONFIG  -= static

VERSION = 1.1.8
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

QMAKE_TARGET_COMPANY     = Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
QMAKE_TARGET_PRODUCT     = Inwentaryzacja
QMAKE_TARGET_DESCRIPTION = Program do inwentaryzacji retro komputerów

win32:RC_ICONS = images/icon.ico
macx: ICON     = images/ikona_mac.icns

# (optional) set a custom Linux icon
unix:!macx {
    QMAKE_POST_LINK += $$quote($$PWD/set_icon.sh $$OUT_PWD/Inwentaryzacja $$PWD/images/icon.png)
}

QMAKE_RPATHDIR += $$[QT_INSTALL_PLUGINS]
QMAKE_LIBDIR   += $$[QT_INSTALL_PLUGINS]

INCLUDEPATH += include
DEPENDPATH  += include

SOURCES += \
    src/DatabaseConfigDialog.cpp \
    src/ItemFilterProxyModel.cpp \
    src/fullscreenphotoviewer.cpp \
    src/itemList.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/models.cpp \
    src/photoitem.cpp \
    src/status.cpp \
    src/storage.cpp \
    src/types.cpp \
    src/utils.cpp \
    src/vendors.cpp

HEADERS += \
    include/DatabaseConfigDialog.h \
    include/ItemFilterProxyModel.h \
    include/fullscreenphotoviewer.h \
    include/itemList.h \
    include/mainwindow.h \
    include/models.h \
    include/photoitem.h \
    include/status.h \
    include/storage.h \
    include/types.h \
    include/utils.h \
    include/vendors.h

FORMS += \
    forms/DatabaseConfigDialog.ui \
    forms/itemList.ui \
    forms/mainwindow.ui \
    forms/models.ui \
    forms/status.ui \
    forms/storage.ui \
    forms/types.ui \
    forms/vendors.ui

TRANSLATIONS += \
    translations/Inwentaryzacja_pl_PL.ts

DISTFILES += \
    images/ikonawin.ico \
    mysql_dll/libcrypto-3-x64.dll \
    mysql_dll/libmysql.dll \
    mysql_dll/libssl-3-x64.dll \
    qt_installer.ps1 \
    renumeracja.sh \
    CHANGELOG.md \
    Create_MySQL.sql \
    README.md \
    createTable.sql \
    database_example_record.sql \
    fix_qt_mysql_mac.sh \
    images/256ikona.png \
    images/icon.ico \
    images/icon.png \
    images/ikona.ico \
    images/ikona256.ico \
    images/ikona_mac.icns \
    merge_win.ps1 \
    migrate_to_uuid.sql \
    prepare_release_windows.bat \
    set_icon.sh \
    sqldrivers/mysql_native_password.dll \
    sqldrivers/qsqlmysql.dll

RESOURCES += \
    images/icon.qrc


##################################################
## Deployment — only on Release builds
##################################################

DEPLOY_DIR = $$PWD/deploy
TARGET     = Inwentaryzacja

# — Windows Release deploy
release:win32 {
    DESTDIR = $$PWD/gotowa
    QMAKE_POST_LINK =  windeployqt $$shell_path($$DESTDIR/$${TARGET}.exe) && \
                        copy /Y c:\\Users\\Arek\\projektyQT\\Inwentaryzacja\\mysql_dll\\*.dll c:\\Users\\Arek\\projektyQT\\Inwentaryzacja\\gotowa\\ && \
                        copy /Y c:\\Users\\Arek\\projektyQT\\Inwentaryzacja\\sqldrivers\\*.dll c:\\Users\\Arek\\projektyQT\\Inwentaryzacja\\gotowa\\sqldrivers\\
}

# — Linux Release deploy
release:unix:!macx {
    QMAKE_POST_LINK += $$quote( \
        rm -rf "$${DEPLOY_DIR}" && \
        mkdir -p "$${DEPLOY_DIR}" && \
        linuxdeployqt "$${OUT_PWD}/$${TARGET}" \
            -appimage \
            -qmldir=$$PWD \
            -bundle-non-qt-libs \
            -executable="$${OUT_PWD}/$${TARGET}" \
            -qmake=$$[QMAKE_QMAKE] \
            -no-translations \
            -no-strip \
            -output-dir="$${DEPLOY_DIR}" \
    )
}

# — macOS Release deploy
release:macx {
    QMAKE_POST_LINK += $$quote( \
        rm -rf "$${DEPLOY_DIR}" && \
        mkdir -p "$${DEPLOY_DIR}" && \
        cp -R "$${OUT_PWD}/$${TARGET}.app" "$${DEPLOY_DIR}/" && \
        rm -f "$${DEPLOY_DIR}/$${TARGET}.dmg" && \
        macdeployqt "$${DEPLOY_DIR}/$${TARGET}.app" -dmg -verbose=2 \
    )
}
