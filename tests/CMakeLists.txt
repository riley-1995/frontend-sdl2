# ======
# Test-only core library
# (Eventually this should be moved into src/CMakeLists.txt to avoid
# duplicating source lists and dependancies.)
# ======

# Ensure OpenGL is found on all platforms; prefer imported targets if present.
find_package(OpenGL REQUIRED)

if (TARGET OpenGL::GL)
    set(_OPENGL_TARGET OpenGL::GL)
elseif (TARGET OpenGL::OpenGL)
    # On some CMake / Apple setups the imported target may be named OpenGL::OpenGL
    set(_OPENGL_TARGET OpenGL::OpenGL)
elseif(DEFINED OPENGL_LIBRARIES)
    # Fallback to legacy variables provided by find_package(OpenGL)
    set(_OPENGL_FALLBACK_LIBS ${OPENGL_LIBRARIES})
else()
    message(FATAL_ERROR "OpenGL not found. Install platform OpenGL dev packages (e.g. libgl1-mesa-dev on Linux).")
endif()

# Build a core library from production sources to test on
add_library(projectMSDL_testcorelib
    # Add .cpp files as needed for testing
    # Examples:
    # ${PROJECT_SOURCE_DIR}/src/SDLRenderingWindow.cpp
    # ${PROJECT_SOURCE_DIR}/src/RenderLoop.cpp
    # ${PROJECT_SOURCE_DIR}/src/ProjectMWrapper.cpp
    # For now, we'll include a simple file just so this thing works. It can be removed if unneeded later.
    ${PROJECT_SOURCE_DIR}/src/FPSLimiter.cpp
    ${PROJECT_SOURCE_DIR}/src/ProjectMSDLApplication.cpp
    ${PROJECT_SOURCE_DIR}/src/RenderLoop.cpp
    ${PROJECT_SOURCE_DIR}/src/ProjectMWrapper.cpp
    ${PROJECT_SOURCE_DIR}/src/SDLRenderingWindow.cpp
    ${PROJECT_SOURCE_DIR}/src/AudioCapture.cpp
)

# Ensure Audio req's are found on all platforms
if (WIN32)
    target_sources(projectMSDL_testcorelib PRIVATE
        ${PROJECT_SOURCE_DIR}/src/AudioCaptureImpl_WASAPI.cpp
    )
    target_compile_definitions(projectMSDL_testcorelib PRIVATE
        AUDIO_IMPL_HEADER="AudioCaptureImpl_WASAPI.h"
        USE_GLEW
    )
else()
    target_sources(projectMSDL_testcorelib PRIVATE
        ${PROJECT_SOURCE_DIR}/src/AudioCaptureImpl_SDL.cpp
    )
    target_compile_definitions(projectMSDL_testcorelib PRIVATE
        AUDIO_IMPL_HEADER="AudioCaptureImpl_SDL.h"
    )
endif()

# Headers from the production source tree
target_include_directories(projectMSDL_testcorelib
    PUBLIC
    ${PROJECT_SOURCE_DIR}/src
)

# Compile definitions that are commonly referenced
target_compile_definitions(projectMSDL_testcorelib
    PRIVATE
    PROJECTMSDL_CONFIG_LOCATION="${DEFAULT_CONFIG_PATH}"
    PROJECTMSDL_VERSION="${PROJECT_VERSION}"
)

# Linker set (add to this only when the linker asks)
target_link_libraries(projectMSDL_testcorelib
    PRIVATE
    SDL2::SDL2
    Poco::Util
    Poco::Foundation
    ProjectMSDL-GUI
    ProjectMSDL-Notifications
    libprojectM::playlist
)

# Link OpenGL (prefer imported target; fallback to OPENGL_LIBRARIES)
if (DEFINED _OPENGL_TARGET)
    target_link_libraries(projectMSDL_testcorelib PRIVATE ${_OPENGL_TARGET})
elseif (DEFINED _OPENGL_FALLBACK_LIBS)
    target_link_libraries(projectMSDL_testcorelib PRIVATE ${_OPENGL_FALLBACK_LIBS})
endif()


# ======
# Test-only core library WITHOUT RenderLoop
# ======

# Build a core library from production sources to test on
add_library(projectMSDL_testcorelib_norenderloop
    # Add .cpp files as needed for testing
    # Examples:
    # ${PROJECT_SOURCE_DIR}/src/SDLRenderingWindow.cpp
    # ${PROJECT_SOURCE_DIR}/src/RenderLoop.cpp
    # ${PROJECT_SOURCE_DIR}/src/ProjectMWrapper.cpp
    # For now, we'll include a simple file just so this thing works. It can be removed if unneeded later.
    ${PROJECT_SOURCE_DIR}/src/FPSLimiter.cpp
    ${PROJECT_SOURCE_DIR}/src/ProjectMSDLApplication.cpp
    ${PROJECT_SOURCE_DIR}/src/ProjectMWrapper.cpp
    ${PROJECT_SOURCE_DIR}/src/SDLRenderingWindow.cpp
    ${PROJECT_SOURCE_DIR}/src/AudioCapture.cpp
)

if (WIN32)
    target_sources(projectMSDL_testcorelib_norenderloop PRIVATE
        ${PROJECT_SOURCE_DIR}/src/AudioCaptureImpl_WASAPI.cpp
    )
    target_compile_definitions(projectMSDL_testcorelib_norenderloop PRIVATE
        AUDIO_IMPL_HEADER="AudioCaptureImpl_WASAPI.h"
        USE_GLEW
    )
else()
    target_sources(projectMSDL_testcorelib_norenderloop PRIVATE
        ${PROJECT_SOURCE_DIR}/src/AudioCaptureImpl_SDL.cpp
    )
    target_compile_definitions(projectMSDL_testcorelib_norenderloop PRIVATE
        AUDIO_IMPL_HEADER="AudioCaptureImpl_SDL.h"
    )
endif()

# Headers from the production source tree
target_include_directories(projectMSDL_testcorelib_norenderloop
    PUBLIC
    ${PROJECT_SOURCE_DIR}/src
)

# Compile definitions that are commonly referenced
target_compile_definitions(projectMSDL_testcorelib_norenderloop
    PRIVATE
    PROJECTMSDL_CONFIG_LOCATION="${DEFAULT_CONFIG_PATH}"
    PROJECTMSDL_VERSION="${PROJECT_VERSION}"
)

# Linker set (add to this only when the linker asks)
target_link_libraries(projectMSDL_testcorelib_norenderloop
    PRIVATE
    SDL2::SDL2
    Poco::Util
    Poco::Foundation
    ProjectMSDL-GUI
    ProjectMSDL-Notifications
    libprojectM::playlist
)

if (DEFINED _OPENGL_TARGET)
    target_link_libraries(projectMSDL_testcorelib_norenderloop PRIVATE ${_OPENGL_TARGET})
elseif (DEFINED _OPENGL_FALLBACK_LIBS)
    target_link_libraries(projectMSDL_testcorelib_norenderloop PRIVATE ${_OPENGL_FALLBACK_LIBS})
endif()

# ======
# Actual test subdirs 
# Add to this when defining more testing directories
# (To be kept in this file after integration of above code into src/CMakeLists.txt)
# ======
add_subdirectory(renderer)
add_subdirectory(audio)
add_subdirectory(cli)
add_subdirectory(ui)