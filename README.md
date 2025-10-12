# Music Widget C++

GTKmm3 기반의 데스크탑 음악 플레이어 위젯입니다. D-Bus를 통해 MPRIS를 지원하는 미디어 플레이어(Rhythmbox, Spotify, VLC 등)를 제어하고 현재 재생 정보를 표시합니다.
현재는 리듬박스(Rhythmbox)에서만 정상 작동합니다.
## 기능

- 앨범 아트워크 표시
- 트랙 제목 및 아티스트 정보 표시
- 재생/일시정지, 이전 곡, 다음 곡 컨트롤
- 음악 재생 진행률 표시
- 창 투명도 조절
- 창 테두리 없음
- `Ctrl` + 드래그 앤 드롭으로 위치 변경
- 모든 가상 데스크톱에 고정 표시 ( Xorg에서만 작동 )

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
sudo apt install build-essential pkg-config cmake libgtkmm-3.0-dev libglibmm-2.4-dev
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
    cmake --build .
    ```

3.  **실행:**
    `build` 디렉토리 안에서 다음 명령어를 실행합니다. (디버그 메시지를 `output.log`로 리디렉션)
    ```bash
    ./MusicWidgetCpp > output.log 2>&1 &
    ```
    또는 터미널에서 직접 실행하여 디버그 메시지를 확인하려면:
    ```bash
    ./MusicWidgetCpp
    ```

## 사용법

- 위젯을 이동하려면 `Ctrl` 키를 누른 상태에서 마우스 왼쪽 버튼으로 드래그하세요.
- 하단의 슬라이더를 조절하여 위젯의 투명도를 변경할 수 있습니다.

## 알려진 버그 
- 위젯에서 최초 정보를 받아온 후 두번째 정보 ( 앨범 아트, 타이틀, 가수 등 )을 갱신하지 않습니다. ( 수정 완료 )
- 리듬 박스를 먼저 실행하지 않고 위젯을 실행하면  unknown error domain 'rb_shell_player_error': throwing generic Glib::Error exception
에러가 나옵니다. 실행에는 지장이 없습니다. 
    - 지연 연결 및 재시도: 위젯이 실행될 때 Rhythmbox 서비스가 없더라도 에러를 발생시키지 않고 조용히 대기합니다. 주기적으로 DBus 서비스를 확인하거나 Rhythmbox 서비스가 활성화되는 이벤트를 수신한 후 성공적으로 연결되면 위젯을 활성화합니다.

- 웨이랜드에서 가상 화면 이동시에 Sticky가 불가능한 문제. Xorg로는 가능함. 
- 리듬 박스 이외의 다른 미디어 플레이어에서 작동하지 않음. 

## 완료된 작업
- 가상 화면을 이동해도 항상 보이게 작업
- 위젯 사이즈를 바꾸더라도 위젯의 UI를 일정 사이즈로 유지하도록 수정
    - 버튼 사이즈
    - 앨범 아트 사이즈는 위젯 사이즈에 맞게 조절
    - 앨범 정보 ( 타이틀, 가수 ) 출력 위치 및 자간, 행간 고정

## 예정 기능
- CAVA를 차일드로 하는 스펙트럼 효과 추가
- 위젯 자체에 있는 투명도 조절 기능을 옵션메뉴에서 설정하도록 변경
