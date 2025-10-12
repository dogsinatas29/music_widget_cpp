// SpectrumWidget.cpp

#include "SpectrumWidget.h"
#include <cmath>
#include <iostream>

SpectrumWidget::SpectrumWidget()
{
    // CAVA와 비슷한 기본값으로 50개의 바(bar)를 가정합니다.
    m_spectrum_data.resize(50, 0.0);
    // 렌더링 주기를 빠르게 하기 위해 적절한 최소 크기를 설정할 수 있습니다.
    set_size_request(200, 100);
}

SpectrumWidget::~SpectrumWidget() {}

void SpectrumWidget::update_spectrum_data(const std::vector<double>& spectrum_data)
{
    Glib::Mutex::Lock lock(m_data_mutex);
    m_spectrum_data = spectrum_data;
    // 데이터를 업데이트했으니 GTK+에 위젯을 다시 그리도록 요청합니다.
    queue_draw();
}

bool SpectrumWidget::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    Glib::Mutex::Lock lock(m_data_mutex);

    // 1. 위젯 크기 가져오기
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();
    const size_t num_bars = m_spectrum_data.size();
    
    if (num_bars == 0) return true;

    // 2. 배경 지우기 (투명도나 배경 색상 설정)
    cr->set_source_rgba(0.0, 0.0, 0.0, 0.0); // 투명한 배경
    cr->paint();

    // 3. 드로잉 스타일 설정
    cr->set_line_width(0); // 바(Bar) 사이에 선을 그리지 않음
    cr->set_source_rgb(0.0, 1.0, 0.0); // 초록색 바

    // 4. 스펙트럼 바 그리기
    const double bar_spacing = 2.0;
    const double bar_width = (static_cast<double>(width) / num_bars) - bar_spacing;

    for (size_t i = 0; i < num_bars; ++i)
    {
        double normalized_height = m_spectrum_data[i] * height;
        
        // 너무 커지지 않도록 최대 높이 제한
        if (normalized_height > height) normalized_height = height;

        double x = i * (bar_width + bar_spacing);
        double y = height - normalized_height; // 바닥부터 위로 그림

        // 바 그리기: (x, y)를 시작점으로 하는 사각형
        cr->rectangle(x, y, bar_width, normalized_height);
        cr->fill();
    }

    return true;
}
