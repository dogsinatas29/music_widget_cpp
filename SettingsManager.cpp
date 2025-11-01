#include "SettingsManager.h"
#include <fstream>
#include <iostream>
#include <sstream>

// 기본 생성자 - WindowState.h에 정의된 기본값 사용
SettingsManager::SettingsManager()
    : m_filepath("music_widget_state.conf") // 기본 파일명 설정
{
}

// 명시적 파일명 생성자 (사용자의 의도에 맞게 파일명을 멤버 변수에 저장)
SettingsManager::SettingsManager(const std::string& filename)
    : m_filepath(filename) // <--- 여기서 m_filename을 m_filepath로 수정했습니다.
{
}

void SettingsManager::save_state(const WindowState& state)
{
    try {
        std::ofstream ofs(m_filepath);
        if (ofs.is_open()) {
            ofs << "x=" << state.x << "\n";
            ofs << "y=" << state.y << "\n";
            ofs << "width=" << state.width << "\n";
            ofs << "height=" << state.height << "\n";
            ofs.close();
        } else {
            std::cerr << "Error: Could not open file for writing: " << m_filepath << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error saving window state: " << e.what() << std::endl;
    }
}

WindowState SettingsManager::load_state()
{
    WindowState state; // WindowState.h에 정의된 기본값으로 초기화
    try {
        std::ifstream ifs(m_filepath);
        if (ifs.is_open()) {
            std::string line;
            while (std::getline(ifs, line)) {
                size_t eq_pos = line.find('=');
                if (eq_pos != std::string::npos) {
                    std::string key = line.substr(0, eq_pos);
                    std::string value_str = line.substr(eq_pos + 1);
                    int value = std::stoi(value_str);

                    if (key == "x") state.x = value;
                    else if (key == "y") state.y = value;
                    else if (key == "width") state.width = value;
                    else if (key == "height") state.height = value;
                }
            }
            ifs.close();
        } 
        // 파일이 없으면 기본 상태를 반환합니다. 오류로 처리하지 않습니다.
    } catch (const std::exception& e) {
        std::cerr << "Error loading window state, returning default state: " << e.what() << std::endl;
        // 실패 시에도 기본 상태 (state)가 반환됩니다.
    }
    return state;
}
