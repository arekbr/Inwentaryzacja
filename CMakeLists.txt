cmake_minimum_required(VERSION 3.25)
project(Inwentaryzacja LANGUAGES CXX)

# ───────────────────────── General settings ────────────────────────────
# Require C++17 and enable AUTOMOC/UITools/RCC in a Qt‑idiomatic way
qt_standard_project_setup()

# Allow the caller to switch between vcpkg and system packages
option(USE_VCPKG "Use vcpkg toolchain" ON)
if(USE_VCPKG AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE
        "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
elseif(USE_VCPKG)
    message(FATAL_ERROR "USE_VCPKG=ON but VCPKG_ROOT not set")
endif()

# Target‑level compile features (portable)
add_library(project_options INTERFACE)
set_property(TARGET project_options PROPERTY
    INTERFACE_COMPILE_FEATURES cxx_std_17)

# ────────────────────────── Qt 6 ───────────────────────────────────────
if(DEFINED ENV{QT_BIN})
    list(PREPEND CMAKE_PREFIX_PATH "$ENV{QT_BIN}")
endif()
find_package(Qt6 6.5 COMPONENTS Core Gui Widgets Sql LinguistTools
             REQUIRED)

# ────────────────────────── External libs ──────────────────────────────
find_package(OpenSSL REQUIRED)
# SQLite3 has an official CMake package since 3.42 / vcpkg port name "sqlite3"
find_package(SQLite3 REQUIRED)
# Prefer official CMake find module (CMake ≥ 3.29) and fall back to pkg‑config
find_package(MariaDB CONFIG QUIET)
if(NOT MariaDB_FOUND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(MariaDB REQUIRED IMPORTED_TARGET libmariadb)
endif()

# ─────────────────────────── Sources ───────────────────────────────────
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS include/*.h)
file(GLOB FORM_FILES      CONFIGURE_DEPENDS forms/*.ui)
set(TRANSLATION_FILES translations/Inwentaryzacja_pl_PL.ts)
qt6_add_translations(Inwentaryzacja TS_FILES ${TRANSLATION_FILES}
                    QM_FILES_OUTPUT_VARIABLE QM_FILES)

qt_add_executable(Inwentaryzacja
    ${SOURCE_FILES} ${HEADER_FILES} ${FORM_FILES} resources.qrc ${QM_FILES})

# Include paths & link targets
#   generator‑expressions ensure correct DLL/.so/.dylib selection per‑config

target_link_libraries(Inwentaryzacja PRIVATE
    project_options
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql
    OpenSSL::SSL OpenSSL::Crypto
    SQLite::SQLite3
    
    # MariaDB can be either imported via pkg‑config or CONFIG
    $<$<BOOL:MariaDB::mariadbclient>:MariaDB::mariadbclient>
    $<$<BOOL:PkgConfig::MariaDB>:PkgConfig::MariaDB>
)

# ────────────────────────── RPATH (Linux) ──────────────────────────────
if(UNIX AND NOT APPLE)
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
endif()

# ────────────────────────── Deployment / install ───────────────────────
include(GNUInstallDirs)
install(TARGETS Inwentaryzacja RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/translations)

# Copy dependent shared libraries in a cross‑platform, generator‑safe way
foreach(_t Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql
           OpenSSL::SSL OpenSSL::Crypto
           SQLite::SQLite3 MariaDB::mariadbclient PkgConfig::MariaDB)
    if(TARGET "${_t}")
        install(FILES $<TARGET_FILE:${_t}> DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endif()
endforeach()

# Platform‑specific deployment helpers
if(APPLE)
    # Build a .app bundle and run macdeployqt automatically (Qt ≥ 6.7)
    set_target_properties(Inwentaryzacja PROPERTIES MACOSX_BUNDLE TRUE)
    find_package(Qt6 COMPONENTS macOSDeployment REQUIRED)
    qt6_macos_deploy(TARGETS Inwentaryzacja VERBOSE)
elseif(WIN32)
    # Copy Qt & runtime DLLs with windeployqt (Qt 6.7+ has CMake wrapper)
    find_package(Qt6 COMPONENTS WindowsDeployment REQUIRED)
    qt6_windeployqt(TARGETS Inwentaryzacja PATHS "${CMAKE_INSTALL_PREFIX}" VERBOSE)
else() # Linux
    # Optional wrapper script with LD_LIBRARY_PATH
    configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/LaunchWrapper.sh.in
                   ${CMAKE_CURRENT_BINARY_DIR}/Inwentaryzacja.sh @ONLY)
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/Inwentaryzacja.sh
            DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
