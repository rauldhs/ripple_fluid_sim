cmake_minimum_required(VERSION 4.2)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

project(fluid_sim VERSION 1.0 LANGUAGES C CXX)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
    -Wall -Wextra -Wpedantic
    -Wshadow -Wconversion -Wsign-conversion
    -Wnull-dereference -Wdouble-promotion
    -Wformat=2 -Wcast-qual -Wold-style-cast
)

# ----------------------------------------------------
# clang-tidy (Google checks)
# ----------------------------------------------------
find_program(CLANG_TIDY_EXE clang-tidy)
if(CLANG_TIDY_EXE)
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

# ----------------------------------------------------
# clang-format "format" target
# ----------------------------------------------------
find_program(CLANG_FORMAT_EXE clang-format)
if(CLANG_FORMAT_EXE)
  file(GLOB_RECURSE FORMAT_FILES
        "${CMAKE_SOURCE_DIR}/src/*.[ch]pp"
        "${CMAKE_SOURCE_DIR}/src/*.[ch]"
    )
  add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXE} -i ${FORMAT_FILES}
        COMMENT "Running clang-format"
    )
endif()

# ----------------------------------------------------
# Auto-generate Google style configs if missing
# ----------------------------------------------------
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/.clang-format)
  file(WRITE ${CMAKE_SOURCE_DIR}/.clang-format
"BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 120
")
endif()

if(NOT EXISTS ${CMAKE_SOURCE_DIR}/.clang-tidy)
  file(WRITE ${CMAKE_SOURCE_DIR}/.clang-tidy
"Checks: '-*,google-*'
WarningsAsErrors: ''
HeaderFilterRegex: 'src'
")
endif()

set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)
add_subdirectory(external/glm)

add_library(glad STATIC external/glad/src/gl.c)
target_include_directories(glad PUBLIC external/glad/include external/stb)

file(GLOB_RECURSE SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
add_executable(circles_stress_test
    tests/circles_stress_test.cpp
    src/circle.cpp
)
target_link_libraries(circles_stress_test PRIVATE glfw glad glm::glm)
target_include_directories(circles_stress_test
    PRIVATE src external/glad/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw glad glm::glm)
target_include_directories(${PROJECT_NAME}
    PRIVATE src external/glad/include
)
