#TODO: check alternatives to compiling src twice (static library or some shit)
file(GLOB_RECURSE SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*main\\.cpp$")

file(GLOB_RECURSE TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(ripple_tests ${SOURCE_FILES} ${TEST_FILES})

target_link_libraries(ripple_tests PRIVATE
    GTest::gtest
    GTest::gtest_main
    glfw glad glm::glm imgui
)
target_include_directories(ripple_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external/glad/include
)

include(GoogleTest)
gtest_discover_tests(ripple_tests)
