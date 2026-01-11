# 프로젝트 현재 상태 분석

## 1. 개요
이 프로젝트는 GTK3(GTKmm)를 사용한 C++ 기반 음악 플레이어 위젯입니다. 현재 기본적인 음악 제어 및 정보 표시 기능이 구현되어 있습니다.

## 2. 구현 상태 상세
### ✅ 구현 완료된 기능
- **기본 UI**: 앨범 아트(60x60), 재생/정지, 이전/다음 곡 버튼.
- **창 관리**: 
    - Always on Top (항상 위)
    - Sticky (모든 작업 공간에서 보임)
    - Dock 모드 (작업 표시줄 제외)
- **설정 저장**: `SettingsManager`를 통해 창의 위치(x, y)를 `music_widget_state.conf` 파일에 저장하고 로드함.
- **드래그 이동**: `Ctrl + 드래그`를 통한 위치 이동 지원.

### 🚧 진행 중 또는 미흡한 부분
- **시각화 기능 (Cava/Spectrum)**: `CavaWidget` 및 `SpectrumWidget` 소스 파일은 존재하나, `CMakeLists.txt`에 포함되지 않아 실제 빌드 및 사용이 되지 않고 있음.
- **크기 조절**: `README.md`에는 크기 조절이 가능하다고 되어 있으나, 소스 코드상으로는 400x200으로 고정되어 있음.
- **하드코딩**: `main.cpp`에 특정 사용자 디렉토리 경로가 하드코딩되어 있어 환경 이식성이 낮음.

## 3. 기술 부채 및 개선 제안
1. **경로 유연화**: `main.cpp`의 하드코딩된 경로를 실행 파일 경로 기준 또는 `XDG_CONFIG_HOME` 기준으로 변경 필요.
2. **시각화 통합**: 준비된 `CavaWidget`과 `SpectrumWidget`을 빌드 시스템에 추가하고 위젯에 통합.
3. **빌드 정리**: 프로젝트 루트의 빌드 잔여물(`CMakeFiles`, `CMakeCache.txt` 등) 정리 필요.

## 4. 파일 구조
- `main.cpp`: 진입점 및 윈도우 초기화.
- `MusicWidget.cpp/h`: 메인 UI 및 로직.
- `SettingsManager.cpp/h`: 설정 파일 입출력.
- `WindowState.h`: 창 상태 데이터 구조.
- `style.css`: UI 스타일링.
- *(미사용)* `CavaWidget.cpp/h`, `SpectrumWidget.cpp/h`
