
set(MY_TEST_TARGET tst_resourceitem)

set(MY_TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/fileutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/mask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/resourceitem.cpp
    )

add_executable(${MY_TEST_TARGET} WIN32
    ${CMAKE_CURRENT_SOURCE_DIR}/test/core/resourceitem/tst_resourceitem.cpp
    ${MY_TEST_SOURCES}
    )

add_test(NAME ${MY_TEST_TARGET} COMMAND ${MY_TEST_TARGET})

# Qt
qt5_use_modules(${MY_TEST_TARGET} Core Test )
