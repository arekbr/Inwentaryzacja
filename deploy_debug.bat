@echo off
setlocal

set QT_BIN=C:\Qt\6.9.0\mingw_64\bin
set QT_PLUGINS=C:\Qt\6.9.0\mingw_64\plugins
set PROJECT_BUILD_DIR=%~dp0build\Desktop_Qt_6_9_0_MinGW_64_bit-Debug\debug

echo [DEPLOY] WykonujÄ™ deploy debugowego builda...

if exist "%PROJECT_BUILD_DIR%\Inwentaryzacja.exe" (
    echo [DEPLOY] Uruchamiam windeployqt...
    "%QT_BIN%\windeployqt.exe" --no-compiler-runtime --debug "%PROJECT_BUILD_DIR%\Inwentaryzacja.exe"

    echo [DEPLOY] KopiujÄ™ libmysql.dll
    copy /Y "%~dp0mysql_dll\libmysql.dll" "%PROJECT_BUILD_DIR%\libmysql.dll"

    echo [DEPLOY] KopiujÄ™ platform plugin qwindows.dll
    if not exist "%PROJECT_BUILD_DIR%\platforms" mkdir  mkdir "%PROJECT_BUILD_DIR%\platforms"
    copy /Y "%QT_PLUGINS%\platforms\qwindows.dll" "%PROJECT_BUILD_DIR%\platforms\qwindows.dll"

    echo [DEPLOY] Gotowe!
) else (
    echo [DEPLOY] UWAGA: Plik Inwentaryzacja.exe nie istnieje!
)

pause