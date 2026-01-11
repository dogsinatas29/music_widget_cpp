#ifndef SPECTRUM_WIDGET_H
#define SPECTRUM_WIDGET_H

#include <gtkmm/drawingarea.h>
#include <vector>
#include <glibmm/thread.h> // Glib::Mutex 및 Glib::Mutex::Lock을 사용하기 위해 추가

enum class VisualizerColor {
    GREEN,
    BLUE,
    PURPLE,
    ORANGE,
    WHITE
};

class SpectrumWidget : public Gtk::DrawingArea
{
public:
    SpectrumWidget();
    virtual ~SpectrumWidget();

    // 오디오 데이터를 위젯에 전달하기 위한 메서드
    void update_spectrum_data(const std::vector<double>& spectrum_data);
    void set_max_drawing_height(int height);
    void set_color_preset(VisualizerColor preset);

protected:
    // 드로잉 이벤트를 처리하는 핵심 가상 함수
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
    virtual bool on_button_press_event(GdkEventButton* event) override;

private:
    std::vector<double> m_spectrum_data;
    Glib::Mutex m_data_mutex; // 데이터 동기화를 위한 뮤텍스
    int m_max_drawing_height; // 스펙트럼이 그려질 수 있는 최대 높이
    VisualizerColor m_color_preset;
};

#endif // SPECTRUM_WIDGET_H
