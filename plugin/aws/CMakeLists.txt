set(USEAWS OFF)

if (${USEAWS} STREQUAL "ON")
    #set(AWSSDK_DIR "D:/awss3c++/aws-sdk-cpp/buildtest4/install/lib/cmake/AWSSDK")
    message("use aws")
    find_package(AWSSDK REQUIRED COMPONENTS core s3)
    add_definitions(-DUSEAWS)
    set(my_Sources
        ${my_Sources}
        "plugin/aws/aws_s3.h"
        "plugin/aws/aws_s3.cpp"
        "plugin/aws/awsStorage.h"
        "plugin/aws/awsStorage.cpp"
        "plugin/aws/service.cpp"
        #"plugin/aws/test_storage.cpp"
        )
    set(my_Libs
        ${my_Libs}
        ${AWSSDK_LINK_LIBRARIES})
    include_directories(${AWSSDK_INCLUDE_DIR})
    #message("xxx: ${AWSSDK_INCLUDE_DIR}")
endif()
