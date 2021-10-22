cmake_minimum_required(VERSION 3.5)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#if (CMAKE_BUILD_TYPE STREQUAL Debug)
#set(${CMAKE_BINARY_DIR} install)
if (NOT DEFINED MS)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../build-app-Desktop_Qt_5_13_2_MSVC2017_64bit-${CMAKE_BUILD_TYPE}/plugin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/install/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../build-app-Desktop_Qt_5_13_2_MSVC2017_64bit-${CMAKE_BUILD_TYPE}/plugin)
else()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${MS})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/install/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MS})
endif()

project(NWLan)

#add_definitions(-DEXPORTDLL)

find_package(Qt5 COMPONENTS Widgets Core Quick SerialPort SerialBus WebEngineWidgets WebEngine 3DCore 3DExtras 3DRender 3DInput REQUIRED)
set(REA2_DIR ${CMAKE_CURRENT_LIST_DIR}/../frm/cmake)
find_package(REA2)

set(my_Sources
    #"qml.qrc"
    "interface.cpp"
    "handler.h"
    "handler.cpp"
    "synchronizer.cpp"
    "fileMonitor.cpp"
    "recoverUserState.cpp"
    "customQSGService.cpp"
    )

set(my_Libs
    Qt5::Core
    Qt5::Quick
    Qt5::Widgets
    Qt5::SerialPort
    Qt5::SerialBus
    Qt5::WebEngineWidgets
    Qt5::WebEngine
    Qt5::3DCore
    Qt5::3DExtras
    Qt5::3DRender
    Qt5::3DInput
    ${REA2_LIBS}
    )

file(GLOB PLUGINS ${CMAKE_CURRENT_SOURCE_DIR}/plugin/*)
foreach(plg ${PLUGINS})
    if (EXISTS ${plg}/CMakeLists.txt)
        include(${plg}/CMakeLists.txt)
    endif()
endforeach()

# 根据库文件代码生成动态库
add_library(NWLan SHARED
    ${my_Sources}
    )
# 创建动态库别名

#add_library(hello::library ALIAS NWLan)

target_link_libraries(NWLan PRIVATE
    ${my_Libs})

include_directories(${REA2_INCLUDE_DIRS})

file(GLOB PLUGINS ${CMAKE_CURRENT_SOURCE_DIR}/plugin/*)
foreach(plg ${PLUGINS})
    if (EXISTS ${plg}/CMakeLists2.txt)
        include(${plg}/CMakeLists2.txt)
    endif()
endforeach()

list(LENGTH gui_source count)
math(EXPR count "${count}-1")
foreach(i RANGE ${count})
    list(GET gui_source ${i} src)
    list(GET gui_target ${i} tgt)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${src}
        ${tgt}
        COMMENT "copying gui components.")
endforeach()
