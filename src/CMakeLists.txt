file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h
)

add_executable(ripple ${SOURCE_FILES})

target_link_libraries(ripple PRIVATE glfw glad glm::glm imgui)
target_include_directories(ripple PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/external/glad/include
)

target_compile_options(ripple PRIVATE
    -Wall -Wextra -Wpedantic
    -Wshadow -Wconversion -Wsign-conversion
    -Wnull-dereference -Wdouble-promotion
    -Wformat=2 -Wcast-qual -Wold-style-cast
    -g -O3
)

# Uncomment to enable ASan:
# target_compile_options(ripple PRIVATE -fsanitize=address -fno-omit-frame-pointer)
# target_link_options(ripple    PRIVATE -fsanitize=address -fno-omit-frame-pointer)
