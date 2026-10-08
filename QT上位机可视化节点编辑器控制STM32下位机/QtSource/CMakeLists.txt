cmake_minimum_required(VERSION 3.16)

project(bishe VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets SerialPort)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets SerialPort)

# 使用随项目分发的 QtNodes 源码（third_party/nodeeditor），不再依赖外部已编译库
set(BUILD_SHARED_LIBS OFF CACHE BOOL "构建为静态库" FORCE)  # 静态链接，避免运行时缺少动态库
add_subdirectory(third_party/nodeeditor)

# 源码按职责分三层：core 数据与执行核心、nodes 节点模型、ui 界面与绘制
set(PROJECT_SOURCES
        # 程序入口
        src/main.cpp
        # 界面层：主窗口、画布视图、节点面板、连线绘制
        src/ui/mainwindow.cpp
        src/ui/mainwindow.h
        src/ui/mainwindow.ui
        src/ui/flowview.cpp
        src/ui/flowview.h
        src/ui/nodetreeview.cpp
        src/ui/nodetreeview.h
        src/ui/nodeitemdelegate.cpp
        src/ui/nodeitemdelegate.h
        src/ui/flowconnectionpainter.cpp
        src/ui/flowconnectionpainter.h
        # 核心层：数据类型、端口类型、图模型、串口通信
        src/core/flowdata.h
        src/core/flowporttypes.h
        src/core/flowgraphmodel.cpp
        src/core/flowgraphmodel.h
        src/core/serialportmanager.cpp
        src/core/serialportmanager.h
        # 节点层：各类流程节点与显示 / 输出节点
        src/nodes/startnodemodel.cpp
        src/nodes/startnodemodel.h
        src/nodes/ifnodemodel.cpp
        src/nodes/ifnodemodel.h
        src/nodes/loopnodemodel.cpp
        src/nodes/loopnodemodel.h
        src/nodes/sleepnodemodel.cpp
        src/nodes/sleepnodemodel.h
        src/nodes/outputnodemodel.cpp
        src/nodes/outputnodemodel.h
        src/nodes/displaynodemodel.cpp
        src/nodes/displaynodemodel.h
        src/nodes/devicenodemodel.cpp
        src/nodes/devicenodemodel.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(bishe
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET bishe APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(bishe SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(bishe
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# 把各源码目录加入搜索路径，使跨目录的 #include "xxx.h" 可按文件名直接引用
target_include_directories(bishe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nodes
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
)

target_link_libraries(bishe PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(bishe PRIVATE Qt${QT_VERSION_MAJOR}::SerialPort)
target_link_libraries(bishe PRIVATE QtNodes::QtNodes)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.bishe)
endif()
set_target_properties(bishe PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS bishe
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(bishe)
endif()
