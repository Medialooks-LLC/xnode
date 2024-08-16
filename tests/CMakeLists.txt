if(NOT MEDIALOOKS_GTEST_AV_FOLDER)
    add_compile_definitions(MEDIALOOKS_GTEST_AV_FOLDER=\"E:/Media\")
else()
    add_compile_definitions(MEDIALOOKS_GTEST_AV_FOLDER=\"${MEDIALOOKS_GTEST_AV_FOLDER}\")
endif()

if(NOT MEDIALOOKS_GTEST_SIO_FOLDER)
    add_compile_definitions(MEDIALOOKS_GTEST_SIO_FOLDER=\"E:/VT\")
else()
    add_compile_definitions(MEDIALOOKS_GTEST_SIO_FOLDER=\"${MEDIALOOKS_GTEST_SIO_FOLDER}\")
endif()

enable_testing()
include(GoogleTest)

add_subdirectory(complex)
add_subdirectory(unit)