cmake_minimum_required(VERSION 3.20)
project(Inwentaryzacja VERSION 1.2.4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Wymuszenie automatycznych reguł Qt (MOC, UIC, RCC)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
# Podpowiedz UIC, gdzie są formularze
set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/forms
)


# ================================
# Wyszukiwanie Qt6 komponentów
# ================================
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql LinguistTools)

# ================================
# Pliki źródłowe
# ================================
file(GLOB_RECURSE SOURCES
    src/*.cpp
)

file(GLOB_RECURSE HEADERS
    include/*.h
)

file(GLOB FORMS
    forms/*.ui
)

file(GLOB TRANSLATIONS
    translations/*.ts
)

# ================================
# Translacje (TS -> QM)
# ================================
qt6_create_translation(QM_FILES
    ${CMAKE_SOURCE_DIR}
    ${TRANSLATIONS}
)

# ================================
# Zasoby (.qrc z fontami, stylami, obrazami)
# ================================
qt6_add_resources(RESOURCES resources.qrc)

# ================================
# Tworzenie binarki z automatycznym UIC/RCC/MOC
# ================================
qt6_add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
    ${RESOURCES}
)

# ================================
# Dodatkowe katalogi nagłówkowe (m.in. dla .ui)
# ================================
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/forms
)

# ================================
# Linkowanie bibliotek Qt
# ================================
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
)
