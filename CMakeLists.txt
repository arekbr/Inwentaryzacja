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
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets LinguistTools)

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
)
