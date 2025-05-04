cmake_minimum_required(VERSION 3.20)
project(Inwentaryzacja LANGUAGES CXX)

# ───────────────────────────────  Ogólne ustawienia  ───────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_SOURCE_DIR}/forms")

# ─────────────  Własny katalog instalacyjny wewnątrz build/  ──────────────
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "" FORCE)

# ───────────────────────────────  Toolchain (vcpkg)  ──────────────────────────────
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
else()
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# ───────────────────────────────  Qt 6  ───────────────────────────────────────────
if(DEFINED ENV{QT_BIN})
    set(Qt6_DIR "$ENV{QT_BIN}/lib/cmake/Qt6")
endif()
find_package(Qt6 COMPONENTS Core Gui Widgets Sql LinguistTools REQUIRED)

# ───────────────────────────────  Biblioteki zewnętrzne  ─────────────────────────
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MARIADB REQUIRED IMPORTED_TARGET libmariadb)

# ───────────────────────────────  Pliki źródłowe  ────────────────────────────────
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS include/*.h)
file(GLOB FORM_FILES CONFIGURE_DEPENDS forms/*.ui)
set(TRANSLATION_FILES translations/Inwentaryzacja_pl_PL.ts)
qt6_add_translations(Inwentaryzacja TS_FILES ${TRANSLATION_FILES} QM_FILES_OUTPUT_VARIABLE QM_FILES)

qt_add_executable(Inwentaryzacja
    ${SOURCE_FILES}
    ${HEADER_FILES}
    ${FORM_FILES}
    resources.qrc
    ${QM_FILES}
)

# includes & linking
get_target_property(QtCore_location Qt6::Core LOCATION)
get_filename_component(Qt_prefix "${QtCore_location}" DIRECTORY)
get_filename_component(Qt_prefix "${Qt_prefix}" DIRECTORY) # lib → <prefix>
set(QT_PLUGINS_DIR "${Qt_prefix}/plugins")
set(QT_LIB_DIR "${Qt_prefix}/lib")

 target_include_directories(Inwentaryzacja PRIVATE ${CMAKE_SOURCE_DIR}/include ${MARIADB_INCLUDE_DIRS})

target_link_libraries(Inwentaryzacja PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql
    OpenSSL::SSL OpenSSL::Crypto
    PkgConfig::MARIADB)

# ───────────────────────────────  Instalacja  ────────────────────────────────────
install(TARGETS Inwentaryzacja RUNTIME DESTINATION bin)

if(UNIX AND NOT APPLE)
    # Qt i inne biblioteki współdzielone
    # Qt libraries
install(DIRECTORY "${QT_LIB_DIR}/" DESTINATION lib FILES_MATCHING PATTERN "libQt6*.so*")
# ICU runtime libraries required by Qt
install(DIRECTORY "${QT_LIB_DIR}/" DESTINATION lib FILES_MATCHING PATTERN "libicu*.so*")
    install(FILES ${OpenSSL_SSL_LIBRARY} ${OpenSSL_CRYPTO_LIBRARY} DESTINATION lib)
    # Kopiuj biblioteki MariaDB tylko gdy ścieżka jest absolutna (vcpkg)
foreach(lib ${MARIADB_LIBRARIES})
    if(IS_ABSOLUTE "${lib}" AND EXISTS "${lib}")
        install(FILES "${lib}" DESTINATION lib)
    endif()
endforeach()

    # pluginy Qt
    install(DIRECTORY "${QT_PLUGINS_DIR}" DESTINATION .)

    # wrapper startowy
    install(CODE [[
        file(WRITE "${CMAKE_INSTALL_PREFIX}/bin/Inwentaryzacja.sh" "#!/usr/bin/env bash\nexport LD_LIBRARY_PATH=${CMAKE_INSTALL_PREFIX}/lib:$LD_LIBRARY_PATH\nexport QT_PLUGIN_PATH=${CMAKE_INSTALL_PREFIX}/plugins\nexec \"${CMAKE_INSTALL_PREFIX}/bin/Inwentaryzacja\" \"$@\"\n")
        execute_process(COMMAND chmod +x "${CMAKE_INSTALL_PREFIX}/bin/Inwentaryzacja.sh")
    ]])
endif()

install(FILES ${QM_FILES} DESTINATION translations)
