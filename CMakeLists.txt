cmake_minimum_required(VERSION 3.10)
project(MusicWidgetCpp CXX)

# Find GTKMM library and dependencies
find_package(PkgConfig REQUIRED)

# ====================================================================
# ✨ 핵심 수정 1: giomm-2.4를 별도로 찾아 변수에 저장
# GTKmm은 giomm을 자동으로 가져오지만, 명시적으로 선언하는 것이 좋습니다.
pkg_check_modules(GTKMM_DEPS REQUIRED gtkmm-3.0 gio-2.0 glibmm-2.4)
pkg_check_modules(GIOMM REQUIRED giomm-2.4)
# ====================================================================

# Define source files
set(SOURCE_FILES
    main.cpp
    MusicWidget.cpp
    SettingsManager.cpp
    SpectrumWidget.cpp
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Link libraries
# ====================================================================
# ✨ 핵심 수정 2: GIOMM_LIBRARIES 변수 추가
# DBus 기능을 위해 giomm 라이브러리를 명시적으로 링크합니다.
target_link_libraries(${PROJECT_NAME}
    ${GTKMM_DEPS_LIBRARIES} # 또는 ${GTKMM_LIBRARIES}
    ${GIOMM_LIBRARIES}      # giomm-2.4 링크
)
# ====================================================================

# Include directories for headers
target_include_directories(${PROJECT_NAME} PUBLIC
    ${GTKMM_DEPS_INCLUDE_DIRS} # 또는 ${GTKMM_INCLUDE_DIRS}
    ${GIOMM_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set compile flags
target_compile_options(${PROJECT_NAME} PUBLIC
    -std=c++17
    ${GTKMM_DEPS_CFLAGS} # 또는 ${GTKMM_CFLAGS}
)
