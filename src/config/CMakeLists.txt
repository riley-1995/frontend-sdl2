add_library(ProjectMSDL-Config STATIC
        ConfigMigration.cpp
        ConfigMigration.h
        )

target_include_directories(ProjectMSDL-Config
        PUBLIC
        "${CMAKE_SOURCE_DIR}/src"
        )

target_link_libraries(ProjectMSDL-Config
        PUBLIC
        Poco::Util
        )
