
set(MY_TEST_TARGET tst_updatechecker)

set(MY_TEST_SOURCES
    # ${CMAKE_CURRENT_SOURCE_DIR}/src/core/updatechecker_p.h
    )

add_executable(${MY_TEST_TARGET} WIN32
    ${CMAKE_CURRENT_SOURCE_DIR}/test/core/updatechecker/tst_updatechecker.cpp
    ${MY_TEST_SOURCES}
    )

add_test(NAME ${MY_TEST_TARGET} COMMAND ${MY_TEST_TARGET})

# Qt
qt5_use_modules(${MY_TEST_TARGET} Core Test )
