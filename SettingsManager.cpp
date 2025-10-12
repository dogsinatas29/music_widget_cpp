#include "SettingsManager.h"
#include <fstream>
#include <sstream>
#include <glibmm.h> // Glib::get_user_config_dir() 사용을 위해 추가
#include <sys/stat.h>   // mkdir 사용을 위해 추가
#include <cerrno>     // errno 사용을 위해 추가

// 설정 파일의 절대 경로를 반환하는 헬퍼 함수
std::string get_settings_file_path() {
    std::string config_dir = Glib::build_filename(Glib::get_user_config_dir(), "music_widget_cpp");
    // 설정 디렉토리가 없으면 생성
    if (mkdir(config_dir.c_str(), 0755) == 0) {
        std::cout << "[SettingsManager] Config directory created: " << config_dir << std::endl;
    } else if (errno != EEXIST) {
        std::cerr << "[SettingsManager] Error creating config directory: " << config_dir << ", errno: " << errno << std::endl;
    }
    return Glib::build_filename(config_dir, "widget_settings.txt");
}

SettingsManager::SettingsManager() {
    // 생성자 구현
}

void SettingsManager::save_state(const WindowState& state) {
    std::string settings_path = get_settings_file_path();
    std::ofstream ofs(settings_path);
    if (ofs.is_open()) {
        ofs << "x=" << state.x << "\n";
        ofs << "y=" << state.y << "\n";
        ofs << "width=" << state.width << "\n";
        ofs << "height=" << state.height << "\n";
        ofs.close();
        std::cout << "[SettingsManager] Window state saved to: " << settings_path << std::endl;
    } else {
        std::cerr << "[SettingsManager] Error saving settings to: " << settings_path << std::endl;
    }
}

WindowState SettingsManager::load_state() {
    WindowState state;
    std::string settings_path = get_settings_file_path();
    std::ifstream ifs(settings_path);
    std::string line;

    if (ifs.is_open()) {
        while (std::getline(ifs, line)) {
            std::stringstream ss(line);
            std::string key;
            std::string value_str;
            if (std::getline(ss, key, '=') && std::getline(ss, value_str)) {
                try {
                    int value = std::stoi(value_str);
                    if (key == "x") state.x = value;
                    else if (key == "y") state.y = value;
                    else if (key == "width") state.width = value;
                    else if (key == "height") state.height = value;
                } catch (...) {}
            }
        }
        ifs.close();
        std::cout << "[SettingsManager] Window state loaded from: " << settings_path 
                  << ", x=" << state.x << ", y=" << state.y 
                  << ", width=" << state.width << ", height=" << state.height << std::endl;
    } else {
        std::cerr << "[SettingsManager] Settings file not found at: " << settings_path 
                  << ", using default state (x=" << state.x << ", y=" << state.y 
                  << ", width=" << state.width << ", height=" << state.height << ")." << std::endl;
    }
    return state;
}

