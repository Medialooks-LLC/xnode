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

add_library(${PROJECT_NAME}
    STATIC
    ${FILES}
)

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
