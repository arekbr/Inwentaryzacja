# Inwentaryzacja Makefile
# Automatyczna kompilacja projektu i zależności na Windows, Linux, macOS (Intel/ARM)

# Wykrywanie systemu
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    OS := linux
    EXT := 
    QMAKE := qmake
    LIB_EXT := .so
    MYSQL_LIB := libmysqlclient.so
    SSL_LIB := libssl.so
    CRYPTO_LIB := libcrypto.so
    SHELL := /bin/bash
endif
ifeq ($(UNAME_S),Darwin)
    OS := macos
    EXT := .app
    QMAKE := qmake
    LIB_EXT := .dylib
    MYSQL_LIB := libmysqlclient.21.dylib
    SSL_LIB := libssl.1.1.dylib
    CRYPTO_LIB := libcrypto.1.1.dylib
    SHELL := /bin/bash
endif
ifneq (,$(findstring MINGW,$(UNAME_S)))
    OS := windows
    EXT := .exe
    QMAKE := qmake.exe
    LIB_EXT := .dll
    MYSQL_LIB := libmysql.dll
    SSL_LIB := libssl-3-x64.dll
    CRYPTO_LIB := libcrypto-3-x64.dll
    SHELL := cmd
endif

# Katalogi projektu
SRC_DIR := src
INCLUDE_DIR := include
FORMS_DIR := forms
BUILD_DIR := build
INSTALL_DIR := $(BUILD_DIR)/install
OPENSSL_DIR := $(BUILD_DIR)/openssl
MYSQL_DIR := $(BUILD_DIR)/mysql-connector
QT_SRC_DIR := $(BUILD_DIR)/qt-src
QT_DIR := $(HOME)/Qt/6.9.0

# Wersje zależności
OPENSSL_VERSION := 3.3.2
MYSQL_VERSION := 8.0.35
QT_VERSION := 6.9.0

# URL do pobierania
OPENSSL_URL := https://github.com/openssl/openssl/releases/download/openssl-$(OPENSSL_VERSION)/openssl-$(OPENSSL_VERSION).tar.gz
MYSQL_URL := https://dev.mysql.com/get/Downloads/Connector-C/mysql-connector-c-$(MYSQL_VERSION)-src.tar.gz
# QT_URL := https://download.qt.io/official_releases/qt/6.9/$(QT_VERSION)/single/qt-everywhere-src-$(QT_VERSION).tar.xz
# Zakładamy, że Qt jest lokalnie, można dodać pobieranie Qt jeśli potrzebne

# Ścieżki Qt
QT_BIN := $(QT_DIR)/bin
QT_PLUGINS := $(QT_DIR)/plugins
QT_SRC_PLUGINS := $(QT_SRC_DIR)/qtbase/src/plugins

# Flagi kompilacji
CXXFLAGS := -std=c++17
LDFLAGS := 

# Pliki źródłowe (zgodne z Inwentaryzacja.pro)
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
HEADERS := $(wildcard $(INCLUDE_DIR)/*.h)
FORMS := $(wildcard $(FORMS_DIR)/*.ui)
PRO_FILE := Inwentaryzacja.pro

# Nazwa aplikacji
APP_NAME := Inwentaryzacja

# Domyślny cel
all: dependencies build install

# Zależności
dependencies: openssl mysql-connector qt-plugins

# Budowanie OpenSSL
openssl:
        @echo "Checking OpenSSL..."
        @if [ ! -d "$(OPENSSL_DIR)" ]; then \
                echo "Downloading OpenSSL $(OPENSSL_VERSION)..."; \
                mkdir -p $(BUILD_DIR); \
                curl -L $(OPENSSL_URL) -o $(BUILD_DIR)/openssl.tar.gz; \
                tar -xzf $(BUILD_DIR)/openssl.tar.gz -C $(BUILD_DIR); \
                mv $(BUILD_DIR)/openssl-$(OPENSSL_VERSION) $(OPENSSL_DIR); \
                cd $(OPENSSL_DIR); \
                if [ "$(OS)" = "windows" ]; then \
                        perl Configure mingw64 --prefix=$(INSTALL_DIR) && make && make install; \
                elif [ "$(OS)" = "linux" ]; then \
                        ./config --prefix=$(INSTALL_DIR) && make && make install; \
                else \
                        ./Configure darwin64-$(shell uname -m)-cc --prefix=$(INSTALL_DIR) && make && make install; \
                fi; \
        else \
                echo "OpenSSL already present."; \
        fi

# Budowanie MySQL Connector/C
mysql-connector: openssl
        @echo "Checking MySQL Connector/C..."
        @if [ ! -d "$(MYSQL_DIR)" ]; then \
                echo "Downloading MySQL Connector/C $(MYSQL_VERSION)..."; \
                mkdir -p $(BUILD_DIR); \
                curl -L $(MYSQL_URL) -o $(BUILD_DIR)/mysql-connector.tar.gz; \
                tar -xzf $(BUILD_DIR)/mysql-connector.tar.gz -C $(BUILD_DIR); \
                mv $(BUILD_DIR)/mysql-connector-c-$(MYSQL_VERSION)-src $(MYSQL_DIR); \
                cd $(MYSQL_DIR); \
                cmake . -DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR) \
                        -DWITH_SSL=$(INSTALL_DIR) \
                        -DCMAKE_BUILD_TYPE=Release && \
                make && make install; \
        else \
                echo "MySQL Connector/C already present."; \
        fi

# Budowanie pluginów Qt (qsqlmysql, imageformats, iconengines, platforms, styles)
qt-plugins:
        @echo "Checking Qt plugins..."
        @if [ ! -d "$(QT_SRC_DIR)" ]; then \
                echo "Qt source required for plugins. Assuming local Qt installation."; \
                echo "Copying prebuilt plugins from $(QT_PLUGINS)..."; \
                mkdir -p $(INSTALL_DIR)/plugins; \
                cp -r $(QT_PLUGINS)/sqldrivers $(INSTALL_DIR)/plugins/; \
                cp -r $(QT_PLUGINS)/imageformats $(INSTALL_DIR)/plugins/; \
                cp -r $(QT_PLUGINS)/iconengines $(INSTALL_DIR)/plugins/; \
                cp -r $(QT_PLUGINS)/platforms $(INSTALL_DIR)/plugins/; \
                cp -r $(QT_PLUGINS)/styles $(INSTALL_DIR)/plugins/; \
                cp -r $(QT_PLUGINS)/platforminputcontexts $(INSTALL_DIR)/plugins/; \
        else \
                echo "Building Qt plugins from source..."; \
                mkdir -p $(BUILD_DIR)/qt-plugins; \
                cd $(BUILD_DIR)/qt-plugins; \
                $(QT_BIN)/$(QMAKE) $(QT_SRC_PLUGINS)/sqldrivers && make && make install; \
                $(QT_BIN)/$(QMAKE) $(QT_SRC_PLUGINS)/imageformats && make && make install; \
                $(QT_BIN)/$(QMAKE) $(QT_SRC_PLUGINS)/iconengines && make && make install; \
                $(QT_BIN)/$(QMAKE) $(QT_SRC_PLUGINS)/platforms && make && make install; \
                $(QT_BIN)/$(QMAKE) $(QT_SRC_PLUGINS)/styles && make && make install; \
                $(QT_BIN)/$(QMAKE) $(QT_SRC_PLUGINS)/platforminputcontexts && make && make install; \
                cp -r $(BUILD_DIR)/qt-plugins/plugins/* $(INSTALL_DIR)/plugins/; \
        fi

# Budowanie projektu
build: dependencies
        @echo "Building $(APP_NAME)..."
        @mkdir -p $(BUILD_DIR)
        @cd $(BUILD_DIR) && $(QT_BIN)/$(QMAKE) ../$(PRO_FILE) && make

# Instalacja (kopiowanie binarek, bibliotek i pluginów)
install: build
        @echo "Installing $(APP_NAME)..."
        @mkdir -p $(INSTALL_DIR)
        @cp $(BUILD_DIR)/$(APP_NAME)$(EXT) $(INSTALL_DIR)/
        @mkdir -p $(INSTALL_DIR)/plugins/sqldrivers
        @mkdir -p $(INSTALL_DIR)/plugins/imageformats
        @mkdir -p $(INSTALL_DIR)/plugins/iconengines
        @mkdir -p $(INSTALL_DIR)/plugins/platforms
        @mkdir -p $(INSTALL_DIR)/plugins/styles
        @mkdir -p $(INSTALL_DIR)/plugins/platforminputcontexts
        @cp $(INSTALL_DIR)/plugins/sqldrivers/*$(LIB_EXT) $(INSTALL_DIR)/plugins/sqldrivers/
        @cp $(INSTALL_DIR)/plugins/imageformats/*$(LIB_EXT) $(INSTALL_DIR)/plugins/imageformats/
        @cp $(INSTALL_DIR)/plugins/iconengines/*$(LIB_EXT) $(INSTALL_DIR)/plugins/iconengines/
        @cp $(INSTALL_DIR)/plugins/platforms/*$(LIB_EXT) $(INSTALL_DIR)/plugins/platforms/
        @cp $(INSTALL_DIR)/plugins/styles/*$(LIB_EXT) $(INSTALL_DIR)/plugins/styles/
        @cp $(INSTALL_DIR)/plugins/platforminputcontexts/*$(LIB_EXT) $(INSTALL_DIR)/plugins/platforminputcontexts/
        @cp $(INSTALL_DIR)/lib/$(MYSQL_LIB) $(INSTALL_DIR)/
        @cp $(INSTALL_DIR)/lib/$(SSL_LIB) $(INSTALL_DIR)/
        @cp $(INSTALL_DIR)/lib/$(CRYPTO_LIB) $(INSTALL_DIR)/
        @if [ "$(OS)" = "macos" ]; then \
                echo "Patching macOS libraries..."; \
                install_name_tool -change @rpath/$(MYSQL_LIB) @executable_path/../$(MYSQL_LIB) $(INSTALL_DIR)/plugins/sqldrivers/libqsqlmysql$(LIB_EXT); \
                install_name_tool -change @loader_path/../lib/$(SSL_LIB) @executable_path/../$(SSL_LIB) $(INSTALL_DIR)/$(MYSQL_LIB); \
                install_name_tool -change @loader_path/../lib/$(CRYPTO_LIB) @executable_path/../$(CRYPTO_LIB) $(INSTALL_DIR)/$(MYSQL_LIB); \
                echo "[Paths]\nPlugins = plugins" > $(INSTALL_DIR)/qt.conf; \
                if [ ! -d "$(INSTALL_DIR)/$(APP_NAME).app" ]; then \
                        mkdir -p "$(INSTALL_DIR)/$(APP_NAME).app/Contents/MacOS"; \
                        mkdir -p "$(INSTALL_DIR)/$(APP_NAME).app/Contents/Frameworks"; \
                        mkdir -p "$(INSTALL_DIR)/$(APP_NAME).app/Contents/PlugIns"; \
                        mkdir -p "$(INSTALL_DIR)/$(APP_NAME).app/Contents/Resources"; \
                        mv $(INSTALL_DIR)/$(APP_NAME) "$(INSTALL_DIR)/$(APP_NAME).app/Contents/MacOS/$(APP_NAME)"; \
                        mv $(INSTALL_DIR)/plugins "$(INSTALL_DIR)/$(APP_NAME).app/Contents/PlugIns/"; \
                        mv $(INSTALL_DIR)/*.dylib "$(INSTALL_DIR)/$(APP_NAME).app/Contents/Frameworks/"; \
                        mv $(INSTALL_DIR)/qt.conf "$(INSTALL_DIR)/$(APP_NAME).app/Contents/Resources/"; \
                fi; \
        fi
        @if [ "$(OS)" = "windows" ]; then \
                echo "Copying Windows DLLs..."; \
                $(QT_BIN)/windeployqt --no-compiler-runtime --debug $(INSTALL_DIR)/$(APP_NAME)$(EXT); \
                cp $(INSTALL_DIR)/lib/mysql_native_password$(LIB_EXT) $(INSTALL_DIR)/; \
        fi
        @echo "Installation complete in $(INSTALL_DIR)"

# Czyszczenie
clean:
        @echo "Cleaning..."
        @rm -rf $(BUILD_DIR)

.PHONY: all dependencies openssl mysql-connector qt-plugins build install clean
