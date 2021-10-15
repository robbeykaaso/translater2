set(USESPLASH ON)

if (${USESPLASH} STREQUAL "ON")
    set(my_Sources
        ${my_Sources}
        "plugin/splash/qml.qrc"
        "plugin/splash/main.cpp"
        )
endif()
