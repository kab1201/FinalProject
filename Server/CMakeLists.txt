cmake_minimum_required(VERSION 3.5)

project(Chat_Server VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#здесь мы создаем переменную CHAT_LIBRARIES, в которой будем хранить
#все подключаемые к проекту библиотеки, в т.ч. и библиотеки Qt
set(CHAT_LIBRARIES)

#здесь мы создаем и заполняем список подключаемых модулей Qt
#если нужно добавить модули, добавьте их в этот список по образцу:
#set(Qt5Modules [модуль 1] [модуль 2] ... [модуль n])
set(Qt6Modules Core Widgets Sql Network)

#здесь мы формируем из списка модулей список библиотек,
#которые требуется подключить, и складываем их в CHAT_LIBRARIES
foreach(QtModule IN LISTS Qt6Modules)
    list(APPEND CHAT_LIBRARIES Qt6::${QtModule})
endforeach(QtModule)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS ${Qt6Modules} LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${Qt6Modules} LinguistTools)

set(TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations/my_ru.ts)

#!!! не забывайте, что при добавлении/удалении файлов  обязательно
#нужно запускать CMake
file(GLOB_RECURSE SRC_FILES sources/*.cpp)
file(GLOB_RECURSE HEADERS_FILES sources/*.h)
file(GLOB_RECURSE RC_FILES sources/*.qrc)
file(GLOB_RECURSE UI_FILES sources/*.ui)
file(GLOB_RECURSE TS_SOURCES "*.cpp" "*.h" "*.ui")

set(PROJECT_SOURCES
        ${SRC_FILES} ${HEADERS_FILES} ${RC_FILES} ${UI_FILES} ${QM_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Chat_Server
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}

        sources/checkboxdelegate.h sources/checkboxdelegate.cpp
        sources/client.h sources/client.cpp
        sources/sha1.h sources/sha1.cpp

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Chat_Server APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(Chat_Server SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Chat_Server
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(Chat_Server PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql Qt6::Network)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Chat_Server)
endif()
set_target_properties(Chat_Server PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Chat_Server
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Chat_Server)
endif()
