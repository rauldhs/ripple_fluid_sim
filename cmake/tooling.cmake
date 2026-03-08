find_program(CLANG_TIDY_EXE clang-tidy)
if(CLANG_TIDY_EXE)
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

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
