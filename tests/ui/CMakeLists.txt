# Requires gtest, otherwise fail
find_package(GTest 1.10 REQUIRED NO_MODULE)

# Creation of executable for this specific test
add_executable(projectMSDL-ui-test
    MainMenuUITest.cpp
)

# Call required libraries; testcore, gtest
target_link_libraries(projectMSDL-ui-test
    PRIVATE
    projectMSDL_testcorelib
    ProjectMSDL-GUI
    GTest::gtest_main
)

# Add our test to the executable
add_test(NAME projectMSDL-ui-test COMMAND projectMSDL-ui-test)