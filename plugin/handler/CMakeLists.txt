set(USEHANDLER ON)

if (${USEHANDLER} STREQUAL "ON")
    set(my_Sources
        ${my_Sources}
        "plugin/handler/imageHandler.cpp"
        "plugin/handler/qsgHandler.cpp"
        "plugin/handler/textHandler.cpp"
        "plugin/handler/resourceHandler.cpp"
        )
endif()
