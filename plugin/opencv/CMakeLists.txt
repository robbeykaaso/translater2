set(USEOPENCV ON)

if (${USEOPENCV} STREQUAL "ON")
    find_package(OpenCV REQUIRED)
    add_definitions(-DUSEOPENCV)
    message("opencv_dir: ${OpenCV_DIR}")
    set(my_Libs
        ${my_Libs}
        ${OpenCV_LIBS})
    set(my_Sources
        ${my_Sources}
        "plugin/opencv/util.h"
        "plugin/opencv/util.cpp"
        "plugin/opencv/imageEx.cpp")
    include_directories(${OpenCV_INCLUDE_DIRS})
endif()
