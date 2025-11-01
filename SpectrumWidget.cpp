#include "SpectrumWidget.h"
#include <cairomm/context.h>
#include <glibmm/main.h>
#include <iostream>
#include <cmath>
#include <algorithm>

SpectrumWidget::SpectrumWidget() 
    : m_spectrum_data(50, 0.0),
      m_max_drawing_height(0) // 초기화
{
    // 초기 50개의 스펙트럼 데이터를 0.0으로 초기화합니다.
}

SpectrumWidget::~SpectrumWidget()
{
}

void SpectrumWidget::update_spectrum_data(const std::vector<double>& data)
{
    m_spectrum_data = data; 
    queue_draw(); // 위젯을 다시 그리도록 요청
}

void SpectrumWidget::set_max_drawing_height(int height)
{
    m_max_drawing_height = height;
    queue_draw(); // 높이 변경 시 다시 그리도록 요청
}

bool SpectrumWidget::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    // 실제 그릴 높이를 m_max_drawing_height와 할당된 높이 중 작은 값으로 제한
    const int draw_height = (m_max_drawing_height > 0) ? std::min(allocation.get_height(), m_max_drawing_height) : allocation.get_height();

    // 배경을 투명하게 설정
    cr->set_source_rgba(0.0, 0.0, 0.0, 0.0); 
    cr->paint();

    if (m_spectrum_data.empty()) {
        return true;
    }

    const int num_bars = m_spectrum_data.size();
    const double bar_spacing = 2.0;
    const double bar_width = (double)(width - (num_bars - 1) * bar_spacing) / num_bars;

    // 스펙트럼 바 그리기
    for (int i = 0; i < num_bars; ++i) {
        double bar_height_ratio = m_spectrum_data[i];
        
        // 데이터는 0.0에서 1.0 사이, 높이는 10% ~ 100% 사용
        double bar_height = std::max(draw_height * 0.1, bar_height_ratio * draw_height * 0.9);
        
        double x = i * (bar_width + bar_spacing);
        double y = draw_height - bar_height;
        
        // 색상 설정: 옅은 보라색에서 밝은 시안색으로 그라데이션
        double color_r = 0.5 + 0.5 * bar_height_ratio; // Red (0.5 to 1.0)
        double color_g = 0.5 + 0.5 * bar_height_ratio; // Green (0.5 to 1.0)
        double color_b = 0.8 + 0.2 * bar_height_ratio; // Blue (0.8 to 1.0)
        cr->set_source_rgb(color_r, color_g, color_b);

        // 막대를 둥근 모서리로 그리기
        double radius = bar_width / 4.0; 
        
        cr->begin_new_sub_path(); // <-- 'new_sub_path'를 'begin_new_sub_path'로 수정했습니다.
        cr->arc(x + bar_width - radius, y + radius, radius, -M_PI / 2.0, 0);
        cr->arc(x + bar_width - radius, y + bar_height - radius, radius, 0, M_PI / 2.0);
        cr->arc(x + radius, y + bar_height - radius, radius, M_PI / 2.0, M_PI);
        cr->arc(x + radius, y + radius, radius, M_PI, 3 * M_PI / 2.0);
        cr->close_path();

        cr->fill();
    }

    return true;
}

