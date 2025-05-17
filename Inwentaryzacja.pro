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

VERSION = 1.2.5
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
    src/DatabaseMigration.cpp \
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
    include/DatabaseMigration.h \
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

# Licencje i skrypty
DISTFILES += \
    CMakeLists.txt \
    LICENSE \
    Makefile \
    Refactor-Inwentaryzacja.ps1 \
    copy_libs_to_project.sh \
    deploy_debug.bat \
    fix_mariadb_mac.sh \
    merge.sh \
    merge_win.ps1 \
    renumeracja.sh \
    scripts_build_debian/1.bootstrap_linux.sh \
    scripts_build_debian/2.build_qt_mysql_plugin_linux.sh \
    scripts_build_debian/3.build_inwentaryzacja_linux.sh \
    scripts_build_debian/4.deploy_linux.sh \
    scripts_build_debian/5.run_inwentaryzacja_linux.sh \
    scripts_build_debian/6.make_deb_linux.sh \
    scripts_build_debian/README.md \
    scripts_build_macos_ventura/1.bootstrap_macos.sh \
    scripts_build_macos_ventura/2.build_qt_mysql_plugin_macos.sh \
    scripts_build_macos_ventura/3.sanitize_qt_sql_plugins_macos.sh \
    scripts_build_macos_ventura/4.build_inwentaryzacja_macos.sh \
    scripts_build_macos_ventura/5.deploy_macos.sh \
    scripts_build_macos_ventura/6.run_inwentaryzacja_macos.sh \
    scripts_build_macos_ventura/7.make_dmg_macos.sh \
    scripts_build_macos_ventura/8.restore_qt_sql_plugins_macos.sh \
    scripts_build_macos_ventura/README.md \
    scripts_build_windows/1.bootstrap_windows.ps1 \
    scripts_build_windows/2.build_qt_mysql_plugin_windows.ps1 \
    scripts_build_windows/3.build_project_windows.ps1 \
    scripts_build_windows/4.deploy_windows.ps1 \
    scripts_build_windows/5.run_inwentaryzacja_windows.ps1 \
    scripts_build_windows/6.make_installer_windows.ps1 \
    scripts_build_windows/README.md \
    set_icon.sh \

# Dokumentacja techniczna (.md)
DISTFILES += \
    documentation/DatabaseConfigDialog_cpp.md \
    documentation/DatabaseConfigDialog_h.md \
    documentation/ItemFilterProxyModel_cpp.md \
    documentation/ItemFilterProxyModel_h.md \
    documentation/fullscreenphotoviewer_cpp.md \
    documentation/fullscreenphotoviewer_h.md \
    documentation/itemList_cpp.md \
    documentation/itemList_h.md \
    documentation/main.md \
    documentation/mainwindow_cpp.md \
    documentation/mainwindow_h.md \
    documentation/models_cpp.md \
    documentation/models_h.md \
    documentation/photoitem_cpp.md \
    documentation/photoitem_h.md \
    documentation/status_cpp.md \
    documentation/status_h.md \
    documentation/storage_cpp.md \
    documentation/storage_h.md \
    documentation/types_cpp.md \
    documentation/types_h.md \
    documentation/utils_cpp.md \
    documentation/utils_h.md \
    documentation/vendors_cpp.md \
    documentation/vendors_h.md

# Czcionki
DISTFILES += \
    fonts/topaz.ttf \
    fonts/zxspectrum.ttf \
    EightBit Atari-Ataripl.ttf


# Obrazy i ikony
DISTFILES += \
    images/256ikona.png \
    images/amiga_about.png \
    images/amiga_clone.png \
    images/amiga_delete.png \
    images/amiga_edit.png \
    images/amiga_end.png \
    images/amiga_new.png \
    images/background.png \
    images/icon.icns \
    images/icon.ico \
    images/icon.png \
    images/ikona.ico \
    images/ikona256.ico \
    images/ikona_mac.icns \
    images/ikonawin.ico \
    images/installericon.ico \
    images/logo.png \
    images/watermark.png

# Style (QSS)
DISTFILES += \
    styles/amiga.qss \
    styles/atari8bit.qss \
    styles/default.qss \
    styles/zxspectrum.qss

# Skrypty SQL i pliki danych
DISTFILES += \
    CHANGELOG.md \
    README.md \
    Create_MySQL.sql \
    createTable.sql \
    database_example_record.sql

RESOURCES += \
    resources.qrc

##################################################
## Deployment — only on Release builds
##################################################

DEPLOY_DIR = $$PWD/deploy
TARGET     = Inwentaryzacja

release:win32 {
    DESTDIR = $$PWD/gotowa
    MYSQL_DLL_DIR = $$PWD/mysql_dll
    SQLDRIVERS_DIR = $$PWD/sqldrivers
    SQLDRIVERS_DEST = $$DESTDIR/sqldrivers

    QMAKE_POST_LINK = \
        windeployqt $$shell_path($$DESTDIR/$${TARGET}.exe) && \
        if not exist $$shell_path($$SQLDRIVERS_DEST) mkdir $$shell_path($$SQLDRIVERS_DEST) && \
        copy /Y $$shell_path($$MYSQL_DLL_DIR\\*.dll) $$shell_path($$DESTDIR\\) && \
        copy /Y $$shell_path($$SQLDRIVERS_DIR\\*.dll) $$shell_path($$SQLDRIVERS_DEST\\)
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
release:macx {
    QMAKE_POST_LINK += $$quote( \
        echo 📦 Kopiowanie bibliotek do mac_lib_sql... && \
        $${PWD}/copy_libs_to_project.sh && \
        echo 🔌 Kopiowanie i patchowanie bibliotek MariaDB do katalogu build... && \
        $${PWD}/fix_mariadb_mac.sh "$${OUT_PWD}/$${TARGET}.app" && \
        echo 🧹 Czyszczenie starego deployu... && \
        rm -rf "$${DEPLOY_DIR}" && \
        mkdir -p "$${DEPLOY_DIR}" && \
        echo 📂 Kopiowanie aplikacji... && \
        cp -R "$${OUT_PWD}/$${TARGET}.app" "$${DEPLOY_DIR}/" && \
        echo 🔌 Kopiowanie i patchowanie bibliotek MariaDB do katalogu deploy... && \
        $${PWD}/fix_mariadb_mac.sh "$${DEPLOY_DIR}/$${TARGET}.app" && \
        echo 🚀 Wywołanie macdeployqt... && \
        ~/Qt/6.9.0/macos/bin/macdeployqt "$${DEPLOY_DIR}/$${TARGET}.app" -verbose=1 -qmldir=$$PWD && \
        echo 💿 Tworzenie DMG... && \
        hdiutil create -volname "$${TARGET}" \
            -srcfolder "$${DEPLOY_DIR}/$${TARGET}.app" \
            -ov -format UDZO "$${DEPLOY_DIR}/$${TARGET}_macOS.dmg" && \
        echo ✅ Gotowe: $${DEPLOY_DIR}/$${TARGET}_macOS.dmg \
    )
}
