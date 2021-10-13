set(USECGAL ON)

if (${USECGAL} STREQUAL "ON")
    find_package(CGAL REQUIRED COMPONENTS Core)
    add_definitions(-DUSECGAL)
    set(my_Sources
        ${my_Sources}
        "plugin/cgal/shape.cpp"
        )
    set(my_Libs
        ${my_Libs}
        CGAL::CGAL CGAL::CGAL_Core)
endif()