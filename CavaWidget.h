#ifndef CAVA_WIDGET_H
#define CAVA_WIDGET_H

#include <gtkmm/drawingarea.h>
#include <glibmm/dispatcher.h>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>

class CavaWidget : public Gtk::DrawingArea {
public:
    CavaWidget();
    virtual ~CavaWidget();

protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
    void on_size_allocate(Gtk::Allocation& allocation) override;

private:
    void cava_thread_func();
    void parse_cava_output(const std::string& cava_output);
    void on_cava_data_received();

    std::vector<int> m_bars;
    std::thread m_cava_thread;
    std::atomic<bool> m_running;

    int m_bar_width;
    double m_spacing;

    Glib::Dispatcher m_dispatcher;
    std::string m_cava_data;
    std::mutex m_mutex;
};

#endif // CAVA_WIDGET_H
