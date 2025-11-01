#pragma once

#include <string>
// WindowState 구조체의 정의를 가져옵니다.
#include "WindowState.h" 

class SettingsManager
{
public:
    // 기본 생성자 선언 추가 (SettingsManager.cpp와 일치)
    SettingsManager();
    
    // WindowState 로딩을 위해 파일 이름 인수를 받는 생성자입니다.
    SettingsManager(const std::string& filename); 

    void save_state(const WindowState& state);
    WindowState load_state();

private:
    std::string m_filepath;
};

