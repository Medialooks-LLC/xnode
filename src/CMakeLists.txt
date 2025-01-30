cmake_minimum_required(VERSION 3.10)

project(xnode)

file(GLOB_RECURSE FILES
    ../include/*.*
    ./*.h
    ./*.hpp
    ./*.cpp
)

include_directories(
        ../include
)

if(DYNAMIC_BUILD)
    add_library(${PROJECT_NAME}
        SHARED
        ${FILES}
    )
else()
    add_library(${PROJECT_NAME}
        STATIC
        ${FILES}
    )
endif()

if(WIN32)
    set_target_properties(xnode PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

target_link_libraries(${PROJECT_NAME}
                      PUBLIC
                      rapidjson
                      XercesC::XercesC
                      xbase
)

target_include_directories(
        ${PROJECT_NAME}
        PUBLIC
        ../include
        ..
)

source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${FILES})
