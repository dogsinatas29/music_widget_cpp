#include <gtkmm/application.h>
#include "MusicWidget.h"
#include <iostream> // std::cout, std::cerr을 위해 추가
#include <exception> // std::exception을 위해 추가 (Glib::Error 및 std::exception catch를 위해 필요)

int main(int argc, char* argv[])
{
    std::cout << "Application starting." << std::endl;
    // Gio::ApplicationFlags::APPLICATION_HANDLES_COMMAND_LINE 플래그 제거 (기본 동작으로)
    auto app = Gtk::Application::create("org.dogsinatas.musicwidget");

    if (!app) {
        std::cerr << "ERROR: Failed to create Gtk::Application." << std::endl;
        return 1;
    }

    app->signal_activate().connect([&app]() {
        std::cout << "Application activated, creating MusicWidget." << std::endl;
        try {
            MusicWidget* widget = new MusicWidget();
            app->add_window(*widget);
            // widget->signal_hide().connect([widget]() {
            //    std::cout << "MusicWidget hidden, deleting." << std::endl;
            //    delete widget;
            //});
            widget->show_all();
            std::cout << "MusicWidget show_all() called." << std::endl;
        } catch (const Glib::Error& ex) {
            std::cerr << "ERROR: Failed to create or show MusicWidget (Glib::Error): " << ex.what() << std::endl;
            app->quit(); // 위젯 생성 실패 시 애플리케이션 종료
        } catch (const std::exception& ex) {
            std::cerr << "ERROR: Unhandled exception during MusicWidget creation: " << ex.what() << std::endl;
            app->quit();
        }
    });

    int result = app->run(argc, argv);
    std::cout << "Application finished with code: " << result << std::endl;
    return result;
}
