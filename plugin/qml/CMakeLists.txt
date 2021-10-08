set(USEQML ON)

if (${USEQML} STREQUAL "ON")
    set(my_Sources
        ${my_Sources}
        "plugin/qml/qml.qrc"
        "plugin/qml/main.cpp"
        )
endif()
