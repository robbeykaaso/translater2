set(USEDPST ON)

if (${USEDPST} STREQUAL "ON")
    #set(AWSSDK_DIR "D:/awss3c++/aws-sdk-cpp/buildtest4/install/lib/cmake/AWSSDK")
    set(my_Sources
        ${my_Sources}
        "plugin/dpst/dpstTrainHandler.cpp"
        "plugin/dpst/dpstAnnoHandler.h"
        "plugin/dpst/dpstAnnoHandler.cpp"
        "plugin/dpst/ssdp.h"
        "plugin/dpst/ssdp.cpp"
        "plugin/dpst/interface.cpp"
        )
endif()
