@echo off
setlocal enabledelayedexpansion

:: KONFIGURACJA
set APP_NAME=Inwentaryzacja.exe
set BUILD_DIR=C:\Users\Arek\projektyQT\Inwentaryzacja\build\Desktop_Qt_6_9_0_MinGW_64_bit-Release\release
set QT_BIN=C:\Qt\6.9.0\mingw_64\bin
set MYSQL_DLL_DIR=C:\Users\Arek\projektyQT\mysql_dlls

:: Wyjściowy katalog Release
set OUTPUT_DIR=%BUILD_DIR%\release_final

:: Plik wynikowy ZIP
set OUTPUT_ZIP=%BUILD_DIR%\Inwentaryzacja_portable.zip

echo.
echo 🛠️ -------------------------------------
echo 🛠️ Przygotowywanie wersji Release...
echo 🛠️ -------------------------------------
echo.

:: 1. Przejdź do katalogu build
cd /d %BUILD_DIR%

:: 2. Usuń śmieci (.cpp, .h, .o itd.)
echo 🧹 Czyszczenie niepotrzebnych plików...
del /q *.cpp *.h *.o *.qrc *.a *.prl 2>nul

:: 3. Uruchom windeployqt
echo 🛠️ Uruchamianie windeployqt...
"%QT_BIN%\windeployqt.exe" --release "%APP_NAME%"

:: 4. Tworzenie katalogu release_final
echo 📦 Tworzenie katalogu %OUTPUT_DIR%...
if exist "%OUTPUT_DIR%" rd /s /q "%OUTPUT_DIR%"
mkdir "%OUTPUT_DIR%"

:: 5. Kopiowanie plików .exe i .dll
echo 📋 Kopiowanie plików .exe i .dll...
copy "%APP_NAME%" "%OUTPUT_DIR%\"
xcopy /y /e /i "platforms" "%OUTPUT_DIR%\platforms\"
xcopy /y /e /i "sqldrivers" "%OUTPUT_DIR%\sqldrivers\"
if exist imageformats xcopy /y /e /i "imageformats" "%OUTPUT_DIR%\imageformats\"
if exist styles xcopy /y /e /i "styles" "%OUTPUT_DIR%\styles\"
if exist iconengines xcopy /y /e /i "iconengines" "%OUTPUT_DIR%\iconengines\"

:: 5b. Krytyczne Qt DLL
echo 🔧 Kopiowanie krytycznych Qt DLL...
copy /y "%BUILD_DIR%\Qt6Core.dll" "%OUTPUT_DIR%\"
copy /y "%BUILD_DIR%\Qt6Gui.dll" "%OUTPUT_DIR%\"
copy /y "%BUILD_DIR%\Qt6Widgets.dll" "%OUTPUT_DIR%\"
copy /y "%BUILD_DIR%\Qt6Sql.dll" "%OUTPUT_DIR%\"
copy /y "%BUILD_DIR%\Qt6Network.dll" "%OUTPUT_DIR%\"
copy /y "%BUILD_DIR%\Qt6Svg.dll" "%OUTPUT_DIR%\" 2>nul

:: 5c. DLL od MinGW
echo 🔧 Kopiowanie DLL MinGW...
copy /y "%QT_BIN%\libgcc_s_seh-1.dll" "%OUTPUT_DIR%\"
copy /y "%QT_BIN%\libstdc++-6.dll" "%OUTPUT_DIR%\"
copy /y "%QT_BIN%\libwinpthread-1.dll" "%OUTPUT_DIR%\"
copy /y "%QT_BIN%\opengl32sw.dll" "%OUTPUT_DIR%\"
copy /y "%QT_BIN%\D3Dcompiler_47.dll" "%OUTPUT_DIR%\"

:: 6. MySQL + SSL
echo 🔒 Dodawanie libmysql.dll i SSL 3...
copy /y "%MYSQL_DLL_DIR%\libmysql.dll" "%OUTPUT_DIR%\"
copy /y "%MYSQL_DLL_DIR%\libssl-3-x64.dll" "%OUTPUT_DIR%\"
copy /y "%MYSQL_DLL_DIR%\libcrypto-3-x64.dll" "%OUTPUT_DIR%\"

:: 7. Dodanie pliku README.txt
echo 📄 Tworzenie pliku README.txt...
echo Inwentaryzacja - wersja portable > "%OUTPUT_DIR%\README.txt"
echo. >> "%OUTPUT_DIR%\README.txt"
echo Instrukcja: >> "%OUTPUT_DIR%\README.txt"
echo - Wypakuj pliki z ZIPa >> "%OUTPUT_DIR%\README.txt"
echo - Uruchom Inwentaryzacja.exe >> "%OUTPUT_DIR%\README.txt"
echo - Miłego użytkowania! :) >> "%OUTPUT_DIR%\README.txt"

:: 8. Tworzenie pliku ZIP
echo 📦 Tworzenie pliku ZIP...
if exist "%OUTPUT_ZIP%" del "%OUTPUT_ZIP%"
powershell Compress-Archive -Path "%OUTPUT_DIR%\*" -DestinationPath "%OUTPUT_ZIP%"

echo.
echo ✅ -------------------------------------
echo ✅ GOTOWE!
echo ✅ Folder: %OUTPUT_DIR%
echo ✅ ZIP: %OUTPUT_ZIP%
echo ✅ -------------------------------------
echo.
pause
