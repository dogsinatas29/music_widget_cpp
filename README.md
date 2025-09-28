# Music Widget C++

GTKmm3 기반의 데스크탑 음악 플레이어 위젯입니다. D-Bus를 통해 MPRIS를 지원하는 미디어 플레이어(Rhythmbox, Spotify, VLC 등)를 제어하고 현재 재생 정보를 표시합니다.

## 기능

- 앨범 아트워크 표시
- 트랙 제목 및 아티스트 정보 표시
- 재생/일시정지, 이전 곡, 다음 곡 컨트롤
- 음악 재생 진행률 표시
- 창 투명도 조절
- 창 테두리 없음
- `Ctrl` + 드래그 앤 드롭으로 위치 변경
- 모든 가상 데스크톱에 고정 표시

## 예정 기능

- CAVA를 이용한 오디오 스펙트럼 시각화

## 의존성

- C++11 이상을 지원하는 컴파일러 (g++, clang)
- CMake
- GTKmm 3.0
- pkg-config

### Ubuntu/Debian 기반 시스템에서 의존성 설치

```bash
sudo apt update
sudo apt install build-essential pkg-config cmake libgtkmm-3.0-dev
```

## 빌드 및 실행

1.  **저장소 클론:**
    ```bash
    git clone https://github.com/dogsinatas29/music_widget_cpp.git
    cd music_widget_cpp
    ```

2.  **빌드:**
    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```

3.  **실행:**
    `build` 디렉토리 안에서 다음 명령어를 실행합니다.
    ```bash
    ./MusicWidgetCpp
    ```

## 사용법

- 위젯을 이동하려면 `Ctrl` 키를 누른 상태에서 마우스 왼쪽 버튼으로 드래그하세요.
- 하단의 슬라이더를 조절하여 위젯의 투명도를 변경할 수 있습니다.
