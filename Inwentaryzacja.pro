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

VERSION = 1.2.0
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
    LICENSE \
    images/background.png \
    images/ikonawin.ico \
    images/installericon.ico \
    images/logo.png \
    images/watermark.png \
    merge.sh \
    mysql_dll/libcrypto-3-x64.dll \
    mysql_dll/libmysql.dll \
    mysql_dll/libssl-3-x64.dll \
    qt_full_installer.ps1 \
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
    DESTDIR = $$PWD/deploy
    TARGET_DIR = $$DESTDIR/$${TARGET}
    APPIMAGE_DIR = $$DESTDIR/$${TARGET}.AppDir

    # Katalogi dla pakietów
    DEB_DIR = $$DESTDIR/deb
    RPM_DIR = $$DESTDIR/rpm

    QMAKE_POST_LINK += $$quote( \
        echo 🧹 Czyszczenie starego deployu... && \
        rm -rf "$${DESTDIR}" && \
        mkdir -p "$${TARGET_DIR}" "$${DEB_DIR}" "$${RPM_DIR}" "$${APPIMAGE_DIR}" && \
        echo 📂 Kopiowanie binarki i zasobów... && \
        cp "$${OUT_PWD}/$${TARGET}" "$${TARGET_DIR}/" && \
        cp -r "$${PWD}/images" "$${TARGET_DIR}/" && \
        cp "$${PWD}/LICENSE" "$${TARGET_DIR}/" && \
        cp "$${PWD}/README.md" "$${TARGET_DIR}/" && \
        echo 🚀 Wywołanie linuxdeployqt... && \
        linuxdeployqt "$${TARGET_DIR}/$${TARGET}" -bundle-non-qt-libs -qmake=$$QMAKE_QMAKE && \
        echo 📦 Tworzenie AppImage... && \
        linuxdeployqt "$${TARGET_DIR}/$${TARGET}" -appimage && \
        mv $${TARGET_DIR}/*.AppImage "$${DESTDIR}/$${TARGET}-$${VERSION}.AppImage" && \
        echo 📦 Tworzenie pakietu .deb... && \
        fpm -s dir -t deb \
            -n "$${TARGET}" \
            -v "$${VERSION}" \
            --description "$${QMAKE_TARGET_DESCRIPTION}" \
            --vendor "$${QMAKE_TARGET_COMPANY}" \
            --license "GPL" \
            --prefix /usr \
            -C "$${TARGET_DIR}" \
            -p "$${DEB_DIR}/$${TARGET}_$${VERSION}_amd64.deb" \
            . && \
        echo 📦 Tworzenie pakietu .rpm... && \
        fpm -s dir -t rpm \
            -n "$${TARGET}" \
            -v "$${VERSION}" \
            --description "$${QMAKE_TARGET_DESCRIPTION}" \
            --vendor "$${QMAKE_TARGET_COMPANY}" \
            --license "GPL" \
            --prefix /usr \
            -C "$${TARGET_DIR}" \
            -p "$${RPM_DIR}/$${TARGET}-$${VERSION}.x86_64.rpm" \
            . && \
        echo ✅ Gotowe: $${DESTDIR}/$${TARGET}-$${VERSION}.AppImage, $${DEB_DIR}/$${TARGET}_$${VERSION}_amd64.deb, $${RPM_DIR}/$${TARGET}-$${VERSION}.x86_64.rpm \
    )
}

# — macOS Release deploy
release:macx {
    QMAKE_POST_LINK += $$quote( \
        echo 🧹 Czyszczenie starego deployu... && \
        rm -rf "$${DEPLOY_DIR}" && \
        mkdir -p "$${DEPLOY_DIR}" && \
        echo 📂 Kopiowanie aplikacji... && \
        cp -R "$${OUT_PWD}/$${TARGET}.app" "$${DEPLOY_DIR}/" && \
        echo 🧹 Usuwanie niepotrzebnych wtyczek przed macdeployqt... && \
        rm -f "$$[QT_INSTALL_PLUGINS]/sqldrivers/libqsqlmysql.dylib" && \
        rm -f "$$[QT_INSTALL_PLUGINS]/sqldrivers/libqsqlodbc.dylib" && \
        rm -f "$$[QT_INSTALL_PLUGINS]/sqldrivers/libqsqlpsql.dylib" && \
        echo 🚀 Wywołanie macdeployqt... && \
        macdeployqt "$${DEPLOY_DIR}/$${TARGET}.app" -verbose=1 && \
        echo 💿 Tworzenie DMG... && \
        hdiutil create -volname "$${TARGET}" \
            -srcfolder "$${DEPLOY_DIR}/$${TARGET}.app" \
            -ov -format UDZO "$${DEPLOY_DIR}/$${TARGET}_macOS.dmg" && \
        echo ✅ Gotowe: $${DEPLOY_DIR}/$${TARGET}_macOS.dmg \
    )
}


