cmake_minimum_required(VERSION 3.4.1)

include_directories(include)

aux_source_directory(src SOURCE_FILES)

add_library(yoghurtutils
            STATIC
            ${SOURCE_FILES})

if (ANDROID)
    target_link_libraries(yoghurtutils log)
elseif (WIN32)
    target_link_libraries(yoghurtutils pthread)
endif ()

add_subdirectory(tests)