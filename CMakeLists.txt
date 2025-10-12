cmake_minimum_required(VERSION 3.10)
project(MusicWidgetCpp CXX)

# Find GTKMM library and dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0 gio-2.0 glibmm-2.4)

# Define source files
# ----------------------------------------------------
# SpectrumWidget.cpp 와 SettingsManager.cpp 추가됨
# ----------------------------------------------------
set(SOURCE_FILES
    main.cpp
    MusicWidget.cpp
    SettingsManager.cpp
    SpectrumWidget.cpp
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${GTKMM_LIBRARIES}
)

# Include directories for headers
target_include_directories(${PROJECT_NAME} PUBLIC
    ${GTKMM_INCLUDE_DIRS}
    ${GLIBMM_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set compile flags
target_compile_options(${PROJECT_NAME} PUBLIC
    -std=c++17
    ${GTKMM_CFLAGS}
)

