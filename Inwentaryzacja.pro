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
    Refactor-Inwentaryzacja.ps1 \
    deploy_debug.bat \
    fix_mysql_mac_new.sh \
    fonts/zxspectrum.ttf \
    images/amiga_about.png \
    images/amiga_clone.png \
    images/amiga_delete.png \
    images/amiga_edit.png \
    images/amiga_end.png \
    images/amiga_new.png \
    images/background.png \
    images/icon.icns \
    images/ikonawin.ico \
    images/installericon.ico \
    images/logo.png \
    images/watermark.png \
    macos_lib_sql/iconengines/libqsvgicon.dylib \
    macos_lib_sql/imageformats/libqgif.dylib \
    macos_lib_sql/imageformats/libqicns.dylib \
    macos_lib_sql/imageformats/libqico.dylib \
    macos_lib_sql/imageformats/libqjpeg.dylib \
    macos_lib_sql/imageformats/libqmacheif.dylib \
    macos_lib_sql/imageformats/libqmacjp2.dylib \
    macos_lib_sql/imageformats/libqpdf.dylib \
    macos_lib_sql/imageformats/libqtga.dylib \
    macos_lib_sql/imageformats/libqtiff.dylib \
    macos_lib_sql/imageformats/libqwbmp.dylib \
    macos_lib_sql/imageformats/libqwebp.dylib \
    macos_lib_sql/libcrypto.1.1.dylib \
    macos_lib_sql/libmysqlclient.21.dylib \
    macos_lib_sql/libmysqlclient.a \
    macos_lib_sql/libqsqlite.dylib \
    macos_lib_sql/libqsqlmysql.dylib \
    macos_lib_sql/libqsqlodbc.dylib \
    macos_lib_sql/libqsqlpsql.dylib \
    macos_lib_sql/libssl.1.1.dylib \
    macos_lib_sql/platforminputcontexts/libqtvirtualkeyboardplugin.dylib \
    macos_lib_sql/platforms/libqcocoa.dylib \
    macos_lib_sql/sqldrivers/libqsqlite.dylib \
    macos_lib_sql/sqldrivers/libqsqlmysql.dylib \
    macos_lib_sql/sqldrivers/libqsqlodbc.dylib \
    macos_lib_sql/sqldrivers/libqsqlpsql.dylib \
    macos_lib_sql/sqldrivers/libssl.1.1.dylib \
    macos_lib_sql/styles/libqmacstyle.dylib \
    make_deb.sh \
    make_deb.sh \
    make_rpm.sh \
    make_rpm.sh \
    merge.sh \
    merge.sh \
    mysql_dll/libcrypto-3-x64.dll \
    mysql_dll/libmysql.dll \
    mysql_dll/libssl-3-x64.dll \
    prepare_dmg.sh \
    prepare_release.sh \
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
    sqldrivers/qsqlmysql.dll \
    styles/amiga.qss \
    styles/default.qss \
    styles/zxspectrum.qss

RESOURCES += \
        resources.qrc


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


# — Linux Release deploy (for .deb and .rpm packages)
release:unix:!macx {
    QMAKE_POST_LINK += $$quote( \
        echo "🧹 Czyszczenie starego deployu..." && \
        rm -rf "$${DEPLOY_DIR}" && \
        mkdir -p "$${DEPLOY_DIR}/usr/bin" && \
        echo "📂 Kopiowanie pliku binarnego..." && \
        echo $${DEPLOY_DIR} && \
        cp "$${OUT_PWD}/$${TARGET}" "$${DEPLOY_DIR}" && \
        echo "✅ Gotowe: Deploy w katalogu $${DEPLOY_DIR}/usr/bin" \
    )
}





# — macOS Release deploy
#release:macx {
#    QMAKE_POST_LINK += $$quote( \
#        echo 🧹 Czyszczenie starego deployu... && \
#        rm -rf "$${DEPLOY_DIR}" && \
##        mkdir -p "$${DEPLOY_DIR}" && \
#        echo 📂 Kopiowanie aplikacji... && \
#        cp -R "$${OUT_PWD}/$${TARGET}.app" "$${DEPLOY_DIR}/" && \
#        echo 🧹 Usuwanie niepotrzebnych wtyczek przed macdeployqt... && \
#        rm -f "$$[QT_INSTALL_PLUGINS]/sqldrivers/libqsqlmysql.dylib" && \
#        rm -f "$$[QT_INSTALL_PLUGINS]/sqldrivers/libqsqlodbc.dylib" && \
#        rm -f "$$[QT_INSTALL_PLUGINS]/sqldrivers/libqsqlpsql.dylib" && \
#        echo 🚀 Wywołanie macdeployqt... && \
#        macdeployqt "$${DEPLOY_DIR}/$${TARGET}.app" -verbose=1 && \
#        echo 💿 Tworzenie DMG... && \
#        hdiutil create -volname "$${TARGET}" \
#            -srcfolder "$${DEPLOY_DIR}/$${TARGET}.app" \
#            -ov -format UDZO "$${DEPLOY_DIR}/$${TARGET}_macOS.dmg" && \
#        echo ✅ Gotowe: $${DEPLOY_DIR}/$${TARGET}_macOS.dmg \
#    )
#}

# — macOS Release deploy
# — macOS Release deploy
release:macx {
    QMAKE_POST_LINK += $$quote( \
        echo 🔌 Kopiowanie i patchowanie bibliotek MySQL do katalogu build... && \
        $${PWD}/fix_mysql_mac_new.sh "$${OUT_PWD}/$${TARGET}.app" && \
        echo 🧹 Czyszczenie starego deployu... && \
        rm -rf "$${DEPLOY_DIR}" && \
        mkdir -p "$${DEPLOY_DIR}" && \
        echo 📂 Kopiowanie aplikacji... && \
        cp -R "$${OUT_PWD}/$${TARGET}.app" "$${DEPLOY_DIR}/" && \
        echo 🧹 Usuwanie niepotrzebnych wtyczek przed macdeployqt... && \
        rm -f "$$[QT_INSTALL_PLUGINS]/sqldrivers/libqsqlodbc.dylib" && \
        rm -f "$$[QT_INSTALL_PLUGINS]/sqldrivers/libqsqlpsql.dylib" && \
        echo 🔌 Kopiowanie i patchowanie bibliotek MySQL do katalogu deploy... && \
        $${PWD}/fix_mysql_mac_new.sh "$${DEPLOY_DIR}/$${TARGET}.app" && \
        echo 🚀 Wywołanie macdeployqt... && \
        macdeployqt "$${DEPLOY_DIR}/$${TARGET}.app" -verbose=1 && \
        echo 💿 Tworzenie DMG... && \
        hdiutil create -volname "$${TARGET}" \
            -srcfolder "$${DEPLOY_DIR}/$${TARGET}.app" \
            -ov -format UDZO "$${DEPLOY_DIR}/$${TARGET}_macOS.dmg" && \
        echo ✅ Gotowe: $${DEPLOY_DIR}/$${TARGET}_macOS.dmg \
    )
}
