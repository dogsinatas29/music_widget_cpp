#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <iostream>
#include <string>
#include "WindowState.h" // WindowState 정의를 가져옴

// MusicWidget의 상태를 저장하기 위한 구조체 (정의 제거)
// struct WindowState {
//     int x = 0;
//     int y = 0;
//     int width = 600;
//     int height = 600;
// };

class SettingsManager
{
public:
    SettingsManager();
    // 윈도우 상태 저장/로드 함수 선언
    void save_state(const WindowState& state);
    WindowState load_state();
};

#endif // SETTINGSMANAGER_H

