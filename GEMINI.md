개요
1. GTK3 기반의 음악 플레이어 위젯
2. 데스크탑 화면에 현재 재생중인 음악 표시 및 제어 

필요 기능
1. 엘범 아트워크 표시
2. CAVA와 같은 시각화 기능 추가 
3. 음악 플레이어 제어 (재생, 일시정지, 다음 곡, 이전 곡)
4. 가상 화면을 옮기더라도 항상 화면에 표시
5. 위젯은 드래그 앤 드롭으로 위치 변경 가능
6. 사이즈 조절 옵션 및 투명도 조절 옵션 

기술 스펙   
1. C++ 라이브러리 사용
  언어: C++11 이상
  GTKmm 라이브러리: GTK 3 (또는 GTK 4)의 C++ 래퍼
  컴파일러: g++ (GCC) 또는 clang (Clang)
  빌드 시스템: CMake (현대적인 C++ 프로젝트에 권장) 또는 Meson, Makefile
sudo apt update
sudo apt install build-essential pkg-config \
                 libgtkmm-3.0-dev libglibmm-2.4-dev \
                 libgirepository1.0-dev libcairo2-dev \
                 libdbus-1-dev cmake # 필요한 빌드 도구


2. 지원하는 음악 플레이어 : Audacious, Spotify, Rhythmbox, VLC, Lollypop, Clementine, MPD 등펙 
3. wayland 지원 
상세 
프로젝트 구조 예시 (CMake 사용):

music_widget_cpp/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── MusicWidget.h
│   └── MusicWidget.cpp
└── style.css

