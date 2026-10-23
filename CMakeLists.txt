cmake_minimum_required(VERSION 3.20)

# ================================
# Odczyt wersji z pliku
# ================================
file(READ "version.txt" PROJECT_VERSION_FROM_FILE)
string(STRIP "${PROJECT_VERSION_FROM_FILE}" PROJECT_VERSION_FROM_FILE)

project(Inwentaryzacja VERSION ${PROJECT_VERSION_FROM_FILE} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ================================
# Definicja wersji dla kompilatora
# ================================
add_compile_definitions(APP_VERSION="${PROJECT_VERSION}")

# ================================
# Automatyczne MOC/UIC/RCC
# ================================
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/forms
)

# ================================
# Qt6 (bez Sql na razie)
# ================================
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets LinguistTools Network Test)
find_package(ZLIB REQUIRED)

# ================================
# Specjalna obsługa Sql (przed ładowaniem aliasów pluginów)
# ================================
if(APPLE)
    set(Qt6Sql_NO_CREATE_TARGETS TRUE)
    set(Qt6Sql_NO_PLUGIN_INCLUDES TRUE)
endif()

find_package(Qt6Sql REQUIRED)

# ================================
# Usuwanie błędnych aliasów Qt6::QODBCDriverPlugin (macOS + brak .dylib)
# ================================
if(APPLE)
    if(TARGET Qt6::QODBCDriverPlugin)
        get_target_property(_odbc_path Qt6::QODBCDriverPlugin IMPORTED_LOCATION)
        if(NOT EXISTS "${_odbc_path}")
            message(WARNING "⚠️  Qt6::QODBCDriverPlugin wskazuje na brakujący plik: ${_odbc_path}, usuwam...")
            unset_target_properties(Qt6::QODBCDriverPlugin PROPERTIES IMPORTED_LOCATION)
        endif()
    endif()
endif()

# ================================
# QtKeychain — klucz API w systemowym sejfie (Pęk kluczy macOS, Menedżer
# poświadczeń Windows, Secret Service Linux) zamiast jawnego tekstu w ini.
# Budowany statycznie: nic dodatkowego do pakowania w .app / instalatorze.
# ================================
include(FetchContent)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
FetchContent_Declare(qtkeychain
    URL      https://github.com/frankosterfeld/qtkeychain/archive/refs/tags/0.17.0.tar.gz
    URL_HASH SHA256=3b85c3929034b0a99da777130c34d99f006fcd3a9d56564159399a33fee0e504
)
# Funkcja = osobny zasięg zmiennych: ustawienia poniżej nie przeciekają do reszty projektu.
# AUTOMOC wyłączony, bo QtKeychain sam woła qt_wrap_cpp (podwójny moc = zdublowane symbole).
function(inwentaryzacja_dodaj_qtkeychain)
    set(CMAKE_AUTOMOC OFF)
    set(CMAKE_AUTOUIC OFF)
    set(CMAKE_AUTORCC OFF)
    set(BUILD_SHARED_LIBS OFF)
    set(BUILD_TESTING OFF)
    set(BUILD_TRANSLATIONS OFF)
    FetchContent_MakeAvailable(qtkeychain)
endfunction()
inwentaryzacja_dodaj_qtkeychain()
# Biblioteka statyczna: bez tego nagłówek eksportu na Windows deklaruje __declspec(dllimport).
target_compile_definitions(qt6keychain PUBLIC QT6KEYCHAIN_STATIC_DEFINE)

# ================================
# Pliki źródłowe
# ================================
file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS include/*.h)
file(GLOB FORMS forms/*.ui)
file(GLOB TRANSLATIONS translations/*.ts)

# ================================
# Translacje (TS → QM)
# ================================
qt6_create_translation(QM_FILES
    ${CMAKE_SOURCE_DIR}
    ${TRANSLATIONS}
)

# ================================
# Zasoby (.qrc)
# ================================
qt6_add_resources(RESOURCES resources.qrc)

# ================================
# Tworzenie binarki
# ================================
qt6_add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
    ${RESOURCES}
)

enable_testing()

qt6_add_executable(${PROJECT_NAME}Tests
    tests/repository_tests.cpp
    include/DatabaseBackupService.h
    include/DatabaseMigration.h
    include/ItemFilterProxyModel.h
    include/ItemFormValidator.h
    include/itemList.h
    include/mainwindow.h
    include/photoitem.h
    include/PreviewDialog.h
    include/AiEnrichmentService.h
    include/DatabaseConfigDialog.h
    include/EnrichPreviewDialog.h
    include/SecretStore.h
    include/fullscreenphotoviewer.h
    include/PacmanOverlay.h
    include/PacmanAnimationModel.h
    include/types.h
    include/vendors.h
    include/models.h
    include/status.h
    include/storage.h
    src/DatabaseBackupService.cpp
    src/ItemRepository.cpp
    src/DictionaryRepository.cpp
    src/ItemFormValidator.cpp
    src/PhotoService.cpp
    src/DatabaseMigration.cpp
    src/ItemFilterProxyModel.cpp
    src/DatabaseSchemaUtils.cpp
    src/itemList.cpp
    src/mainwindow.cpp
    src/photoitem.cpp
    src/PreviewDialog.cpp
    src/AiEnrichmentService.cpp
    src/DatabaseConfigDialog.cpp
    src/EnrichPreviewDialog.cpp
    src/SecretStore.cpp
    src/fullscreenphotoviewer.cpp
    src/PacmanOverlay.cpp
    src/PacmanAnimationModel.cpp
    src/types.cpp
    src/vendors.cpp
    src/models.cpp
    src/status.cpp
    src/storage.cpp
    src/utils.cpp
)

# ================================
# Include directories
# ================================
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/forms
)

# ================================
# Linkowanie Qt
# ================================
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::Network  # v1.5: AiEnrichmentService → Anthropic API
    qt6keychain   # klucz API w systemowym sejfie
    ZLIB::ZLIB
)

target_include_directories(${PROJECT_NAME}Tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/forms
)

target_link_libraries(${PROJECT_NAME}Tests PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::Network  # v1.5: AiEnrichmentService glob-included w testy
    qt6keychain
    Qt6::Test
    ZLIB::ZLIB
)

add_test(NAME repository_tests COMMAND ${PROJECT_NAME}Tests)
set_tests_properties(repository_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
if(APPLE)
    # Sejf macOS oddaje wynik przez główną kolejkę GCD, której offscreen nie opróżnia —
    # testy sejfu idą osobno na platformie cocoa (w offscreen same się pomijają).
    add_test(NAME keychain_tests COMMAND ${PROJECT_NAME}Tests
        aiApiKey_storesReadsAndDeletesInKeychain
        aiApiKey_migratesLegacyIniKeyToKeychain
        aiApiKey_storeRemovesLegacyIniKey
        mysqlPassword_configDialogMigratesIniPasswordToKeychain
        mysqlPassword_configDialogStoresChangedPasswordInKeychain)
    # Zablokowany pęk kluczy / brak sesji GUI = czekanie bez końca — niech CTest to przerwie.
    set_tests_properties(keychain_tests PROPERTIES TIMEOUT 60)
endif()
