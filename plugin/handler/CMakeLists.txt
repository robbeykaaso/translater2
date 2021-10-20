set(USEHANDLER ON)

if (${USEHANDLER} STREQUAL "ON")
    set(my_Sources
        ${my_Sources}
        "plugin/handler/imageHandler.cpp"
        "plugin/handler/qsgHandler.cpp"
        "plugin/handler/textHandler.cpp"
        "plugin/handler/3DHandler.cpp"
        "plugin/handler/customCameraController.h"
        "plugin/handler/customCameraController.cpp"
        "plugin/handler/mideaHandler.cpp"
        "plugin/handler/echartHandler.cpp"
        "plugin/handler/operatorGraphHandler.cpp"
        "plugin/handler/htmlHandler.cpp"
        "plugin/handler/resourceHandler.cpp"
        "plugin/handler/menuHandler.cpp"
        "plugin/handler/statusHandler.cpp"
        )
endif()
