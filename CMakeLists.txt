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
    -g
-O3
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

add_library(imgui STATIC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    external/imgui
    external/imgui/backends
)
target_link_libraries(imgui PUBLIC glfw)

file(GLOB_RECURSE SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE glfw glad glm::glm imgui)
target_include_directories(${PROJECT_NAME}
    PRIVATE src external/glad/include
)

# ----------------------------------------------------
# AddressSanitizer - only for executables
# ----------------------------------------------------
#set(ASAN_FLAGS -fsanitize=address -fno-omit-frame-pointer)
#target_compile_options(${PROJECT_NAME} PRIVATE ${ASAN_FLAGS})
#target_link_options(${PROJECT_NAME} PRIVATE ${ASAN_FLAGS})
