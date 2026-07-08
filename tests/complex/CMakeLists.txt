cmake_minimum_required(VERSION 3.10)

project(xnode_complex_tests)

FILE(GLOB FILES
    ../../include/*.h
    xobject_demo/*.cpp
    xobject_demo/*.h
    *.cpp
    *.hpp
    *.h
)

if(DYNAMIC_BUILD)
    list(FILTER FILES EXCLUDE REGEX ".*/xobject_demo/.*")
    list(FILTER FILES EXCLUDE REGEX ".*/xnode_test\\.cpp$")
    list(FILTER FILES EXCLUDE REGEX ".*/xvalue_test\\.cpp$")
endif()

include_directories(
        ../../include
)

if(MSVC)
    add_compile_options(/bigobj)
endif()

add_executable(${PROJECT_NAME}
               ${FILES}
)

if(WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE /IGNORE:4099)
endif()

target_link_libraries(${PROJECT_NAME}
                        GTest::gtest_main
                        xnode
                        xbase
)

source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${FILES})

