cmake_minimum_required(VERSION 3.16)
project(MusicWidgetCpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# GTKmm 3.0 및 GIOMM 2.4 패키지를 찾습니다.
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)
pkg_check_modules(GIOMM REQUIRED giomm-2.4) # GIOMM 추가

# include_directories와 link_libraries 설정
include_directories(${GTKMM_INCLUDE_DIRS} ${GIOMM_INCLUDE_DIRS}) # GIOMM include 디렉토리 추가
link_libraries(${GTKMM_LIBRARIES} ${GIOMM_LIBRARIES}) # GIOMM 라이브러리 추가

# 소스 파일 목록
set(SOURCES
    main.cpp
    MusicWidget.cpp
)

# 실행 파일 생성
add_executable(MusicWidgetCpp ${SOURCES})

# style.css 파일을 빌드 디렉토리로 복사
# 프로젝트 루트 디렉토리에 style.css가 있다고 가정합니다.
file(COPY style.css DESTINATION ${CMAKE_BINARY_DIR})
