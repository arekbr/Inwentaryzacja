cmake_minimum_required(VERSION 3.20)
project(Inwentaryzacja LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Ścieżka do vcpkg toolchain
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
else()
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Znajdź Qt6
if(DEFINED ENV{QT_DIR})
    set(Qt6_DIR "$ENV{QT_DIR}/lib/cmake/Qt6")
endif()
find_package(Qt6 COMPONENTS Core Gui Widgets Sql REQUIRED)
if(NOT Qt6_FOUND)
    message(FATAL_ERROR "Qt6 not found. Ensure QT_DIR is set or Qt is installed by bootstrap script.")
endif()

# Znajdź OpenSSL i MySQL Connector/C
find_package(OpenSSL REQUIRED)
find_library(MYSQL_LIBRARY NAMES mysqlclient libmysql mysql-connector-c)
if(NOT MYSQL_LIBRARY)
    message(FATAL_ERROR "MySQL Connector/C not found. Ensure vcpkg installed mysql-connector-c.")
endif()

# Źródła projektu (na podstawie Inwentaryzacja.pro)
set(SOURCES
    src/DatabaseConfigDialog.cpp
    src/ItemFilterProxyModel.cpp
    src/fullscreenphotoviewer.cpp
    src/itemList.cpp
    src/main.cpp
    src/mainwindow.cpp
    src/models.cpp
    src/photoitem.cpp
    src/status.cpp
    src/storage.cpp
    src/types.cpp
    src/utils.cpp
    src/vendors.cpp
    # Dodaj inne .cpp
)

set(HEADERS
    include/DatabaseConfigDialog.h
    include/ItemFilterProxyModel.h
    include/fullscreenphotoviewer.h
    include/itemList.h
    include/mainwindow.h
    include/models.h
    include/photoitem.h
    include/status.h
    include/storage.h
    include/types.h
    include/utils.h
    include/vendors.h
    # Dodaj inne .h
)

set(FORMS
    forms/DatabaseConfigDialog.ui
    forms/itemList.ui
    forms/mainwindow.ui
    forms/models.ui
    forms/status.ui
    forms/storage.ui
    forms/types.ui
    forms/vendors.ui
    # Dodaj inne .ui
)

set(RESOURCES
    resources/resources.qrc
    # Dodaj inne .qrc
)

# Tworzenie aplikacji
qt_add_executable(Inwentaryzacja
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
    ${RESOURCES}
)

# Linkowanie bibliotek
target_link_libraries(Inwentaryzacja PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    OpenSSL::SSL
    OpenSSL::Crypto
    ${MYSQL_LIBRARY}
)

# Katalog instalacyjny
set(INSTALL_DIR "${CMAKE_BINARY_DIR}/install")
install(TARGETS Inwentaryzacja DESTINATION "${INSTALL_DIR}/bin")

# Kopiowanie pluginów Qt i bibliotek
if(WIN32)
    # Windows: Użyj windeployqt
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${Qt6_DIR}/../../bin")
    if(WINDEPLOYQT_EXECUTABLE)
        install(CODE "
            execute_process(
                COMMAND \"${WINDEPLOYQT_EXECUTABLE}\" --no-compiler-runtime --debug
                \"${INSTALL_DIR}/bin/Inwentaryzacja.exe\"
            )
        ")
    endif()
    # Kopiowanie mysql_native_password.dll i libmysql.dll
    get_filename_component(MYSQL_LIB_DIR "${MYSQL_LIBRARY}" DIRECTORY)
    install(FILES "${MYSQL_LIB_DIR}/libmysql.dll" DESTINATION "${INSTALL_DIR}/bin")
    install(FILES "${MYSQL_LIB_DIR}/mysql_native_password.dll" DESTINATION "${INSTALL_DIR}/bin" OPTIONAL)
elseif(APPLE)
    # macOS: Tworzenie .app i patchowanie
    set_target_properties(Inwentaryzacja PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Inwentaryzacja"
    )
    install(CODE "
        file(MAKE_DIRECTORY \"${INSTALL_DIR}/Inwentaryzacja.app/Contents/Frameworks\")
        file(MAKE_DIRECTORY \"${INSTALL_DIR}/Inwentaryzacja.app/Contents/PlugIns\")
        file(MAKE_DIRECTORY \"${INSTALL_DIR}/Inwentaryzacja.app/Contents/Resources\")
        # Kopiowanie pluginów Qt
        file(COPY \"${Qt6_DIR}/../../../plugins/sqldrivers\" DESTINATION \"${INSTALL_DIR}/Inwentaryzacja.app/Contents/PlugIns\")
        file(COPY \"${Qt6_DIR}/../../../plugins/imageformats\" DESTINATION \"${INSTALL_DIR}/Inwentaryzacja.app/Contents/PlugIns\")
        file(COPY \"${Qt6_DIR}/../../../plugins/iconengines\" DESTINATION \"${INSTALL_DIR}/Inwentaryzacja.app/Contents/PlugIns\")
        file(COPY \"${Qt6_DIR}/../../../plugins/platforms\" DESTINATION \"${INSTALL_DIR}/Inwentaryzacja.app/Contents/PlugIns\")
        file(COPY \"${Qt6_DIR}/../../../plugins/styles\" DESTINATION \"${INSTALL_DIR}/Inwentaryzacja.app/Contents/PlugIns\")
        file(COPY \"${Qt6_DIR}/../../../plugins/platforminputcontexts\" DESTINATION \"${INSTALL_DIR}/Inwentaryzacja.app/Contents/PlugIns\")
        # Kopiowanie bibliotek
        file(COPY \"${MYSQL_LIBRARY}\" DESTINATION \"${INSTALL_DIR}/Inwentaryzacja.app/Contents/Frameworks\")
        file(COPY \"${OPENSSL_SSL_LIBRARY}\" DESTINATION \"${INSTALL_DIR}/Inwentaryzacja.app/Contents/Frameworks\")
        file(COPY \"${OPENSSL_CRYPTO_LIBRARY}\" DESTINATION \"${INSTALL_DIR}/Inwentaryzacja.app/Contents/Frameworks\")
        # Patchowanie bibliotek
        execute_process(
            COMMAND install_name_tool -change @rpath/libmysqlclient.21.dylib @executable_path/../Frameworks/libmysqlclient.21.dylib
            \"${INSTALL_DIR}/Inwentaryzacja.app/Contents/PlugIns/sqldrivers/libqsqlmysql.dylib\"
        )
        execute_process(
            COMMAND install_name_tool -change @loader_path/../lib/libssl.1.1.dylib @executable_path/../Frameworks/libssl.1.1.dylib
            \"${INSTALL_DIR}/Inwentaryzacja.app/Contents/Frameworks/libmysqlclient.21.dylib\"
        )
        execute_process(
            COMMAND install_name_tool -change @loader_path/../lib/libcrypto.1.1.dylib @executable_path/../Frameworks/libcrypto.1.1.dylib
            \"${INSTALL_DIR}/Inwentaryzacja.app/Contents/Frameworks/libmysqlclient.21.dylib\"
        )
        # Dodanie qt.conf
        file(WRITE \"${INSTALL_DIR}/Inwentaryzacja.app/Contents/Resources/qt.conf\"
            \"[Paths]\\nPlugins = ../PlugIns\"
        )
    ")
else()
    # Linux: Kopiowanie bibliotek
    install(FILES "${MYSQL_LIBRARY}" DESTINATION "${INSTALL_DIR}/lib")
    install(FILES "${OPENSSL_SSL_LIBRARY}" DESTINATION "${INSTALL_DIR}/lib")
    install(FILES "${OPENSSL_CRYPTO_LIBRARY}" DESTINATION "${INSTALL_DIR}/lib")
    install(DIRECTORY "${Qt6_DIR}/../../../plugins/sqldrivers" DESTINATION "${INSTALL_DIR}/plugins")
    install(DIRECTORY "${Qt6_DIR}/../../../plugins/imageformats" DESTINATION "${INSTALL_DIR}/plugins")
    install(DIRECTORY "${Qt6_DIR}/../../../plugins/iconengines" DESTINATION "${INSTALL_DIR}/plugins")
    install(DIRECTORY "${Qt6_DIR}/../../../plugins/platforms" DESTINATION "${INSTALL_DIR}/plugins")
    install(DIRECTORY "${Qt6_DIR}/../../../plugins/styles" DESTINATION "${INSTALL_DIR}/plugins")
    install(DIRECTORY "${Qt6_DIR}/../../../plugins/platforminputcontexts" DESTINATION "${INSTALL_DIR}/plugins")
endif()