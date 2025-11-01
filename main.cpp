#include <gtkmm/application.h>
#include "MusicWidget.h"
#include "SettingsManager.h" 
#include <iostream>
#include <exception>
#include <streambuf> 

// Gdk::WindowTypeHint를 사용하기 위해 gdkmm 헤더를 포함합니다.
#include <gdkmm/window.h> 

const int DEFAULT_WIDTH = 400;

int main(int argc, char* argv[])
{
    // 표준 스트림 동기화 끄기 및 버퍼링 비활성화
    std::ios_base::sync_with_stdio(false);
    std::cout.rdbuf()->pubsetbuf(0, 0);
    std::cerr.rdbuf()->pubsetbuf(0, 0);

    std::cout << "[Main] Application starting." << std::endl;
    
    auto app = Gtk::Application::create("org.dogsinatas.musicwidget");

    if (!app) {
        std::cerr << "[Main] ERROR: Failed to create Gtk::Application." << std::endl;
        return 1;
    }
    std::cout << "[Main] Gtk::Application created." << std::endl;

    // SettingsManager 인스턴스 생성 (설정 파일 경로 명시)
    SettingsManager settingsManager("/home/dogsinatas/c_project/music_widget_cpp/build/music_widget_state.conf");
    std::cout << "[Main] SettingsManager created." << std::endl;

    app->signal_activate().connect([&app, &settingsManager]() {
        std::cout << "[Main] Application activated, creating MusicWidget." << std::endl;
          try {
            // 설정 파일에서 창 상태 불러오기
            WindowState initialState = settingsManager.load_state();
            std::cout << "[Main] Initial WindowState loaded." << std::endl;
            
            // 앨범 아트 크기 계산 (60x60으로 고정)
            int album_art_size = 60;
            // 위젯의 전체 높이 계산: 앨범 아트 높이 + 컨트롤 및 프로그레스 바를 위한 최소 여백
            int calculated_height = 200; // 200px로 고정

            // 로드된 크기가 기본값과 다르거나 유효하지 않으면 기본값으로 재설정
            if (initialState.width != DEFAULT_WIDTH || initialState.height != calculated_height) {
                initialState.width = DEFAULT_WIDTH;
                initialState.height = calculated_height;
                std::cout << "[Main] Window size reset to default: " 
                          << DEFAULT_WIDTH << "x" << calculated_height << std::endl;
            }
            
            // 불러온 상태와 settingsManager를 전달하여 MusicWidget 생성
            MusicWidget* widget = new MusicWidget(initialState, settingsManager);
            std::cout << "[Main] MusicWidget created." << std::endl;
            
            widget->set_title("Music Widget Controller"); 
            widget->set_type_hint(Gdk::WINDOW_TYPE_HINT_DOCK);
            widget->set_keep_above(true);   
            widget->set_skip_taskbar_hint(true); 
            
            // 초기 설정된 크기 적용
            widget->set_size_request(initialState.width, initialState.height); // set_size_request로 강제 설정
            
            app->add_window(*widget);
            std::cout << "[Main] MusicWidget added to application." << std::endl;
            
            widget->show_all();
            std::cout << "[Main] MusicWidget show_all() called." << std::endl;

        } catch (const Glib::Error& ex) {
            std::cerr << "[Main] ERROR: Failed to create or show MusicWidget (Glib::Error): " << ex.what() << std::endl;
            app->quit();
        } catch (const std::exception& ex) {
            std::cerr << "[Main] ERROR: Unhandled exception during MusicWidget creation: " << ex.what() << std::endl;
            app->quit();
        }
    });

    int result = app->run(argc, argv);
    std::cout << "[Main] Application finished with code: " << result << std::endl;
    return result;
}
