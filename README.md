# Music Widget Cpp
       https://github.com/user-attachments/assets/caaebb00-7cbe-47ca-9261-a5bc64d2238f
## 개요
이 프로젝트는 GTK3 기반의 음악 플레이어 위젯입니다. 데스크탑 화면에 현재 재생 중인 음악 정보를 표시하고 제어하는 기능을 제공합니다.

## 주요 기능
1.  **앨범 아트워크 표시:** 현재 재생 중인 음악의 앨범 아트워크를 표시합니다.
2.  **음악 플레이어 제어:** 재생, 일시정지, 다음 곡, 이전 곡 기능을 제어할 수 있습니다.
3.  **항상 화면에 표시:** 가상 화면을 옮기더라도 항상 화면에 표시됩니다.
4.  **위젯 위치 변경:** 위젯은 Ctrl + 클릭 드래그 앤 드롭으로 위치 변경이 가능합니다.

## 기술 스택
*   **언어:** C++11 이상
*   **GUI 라이브러리:** GTKmm (GTK 3의 C++ 래퍼)
*   **컴파일러:** g++ (GCC) 또는 clang (Clang)
*   **빌드 시스템:** CMake

## 빌드 및 실행 방법

### 1. 필수 라이브러리 설치
```bash
sudo apt update
sudo apt install build-essential pkg-config \
                 libgtkmm-3.0-dev libglibmm-2.4-dev \
                 libgirepository1.0-dev libcairo2-dev \
                 libdbus-1-dev cmake
```

### 2. 프로젝트 빌드
프로젝트 루트 디렉토리에서 다음 명령어를 실행합니다.
```bash
mkdir build
cd build
cmake ..
make
```

### 3. 애플리케이션 실행
빌드 디렉토리에서 다음 명령어를 실행합니다.
```bash
./MusicWidgetCpp
```
또는 백그라운드에서 실행하려면:
```bash
./MusicWidgetCpp &
```

## 스크린샷
여기에 위젯의 스크린샷을 추가해 주세요.
예시:
```markdown
<img width="448" height="228" alt="Music Widget Screenshot" src="https://github.com/user-attachments/assets/caaebb00-7cbe-47ca-9261-a5bc64d2238f" /> 
```
