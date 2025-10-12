#include "CavaWidget.h"
#include <cairomm/context.h>
#include <giomm/resource.h>
#include <glibmm/main.h>
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdexcept>
#include <fstream>
#include <sstream>

const std::string FIFO_PATH = "/tmp/cava.fifo";

CavaWidget::CavaWidget() : m_running(true), m_bar_width(5), m_spacing(2) {
    set_app_paintable(true);
    set_has_window(false); // Use set_has_window(false) for transparency
    // auto screen = Gdk::Screen::get_default(); // Removed
    // if (screen) { // Removed
    //     set_visual(screen->get_rgba_visual()); // Removed
    // }
    if (mkfifo(FIFO_PATH.c_str(), 0666) == -1) {
        if (errno != EEXIST) {
            throw std::runtime_error("Failed to create FIFO");
        }
    }

    m_dispatcher.connect(sigc::mem_fun(*this, &CavaWidget::on_cava_data_received));

    m_cava_thread = std::thread(&CavaWidget::cava_thread_func, this);
}

CavaWidget::~CavaWidget() {
    m_running = false;
    // Unblock the read call
    int fd = open(FIFO_PATH.c_str(), O_WRONLY | O_NONBLOCK);
    if (fd != -1) {
        close(fd);
    }
    
    if (m_cava_thread.joinable()) {
        m_cava_thread.join();
    }
    unlink(FIFO_PATH.c_str());
}

void CavaWidget::on_cava_data_received() {
    std::string data;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        data = m_cava_data;
    }
    parse_cava_output(data);
    queue_draw();
}

void CavaWidget::cava_thread_func() {
    std::cout << "[CavaThread] CAVA thread started." << std::endl;
    // Path to user's CAVA config
    std::string user_config_path = std::string(getenv("HOME")) + "/.config/cava/config";
    std::ifstream user_config_file(user_config_path);
    std::stringstream config_stream;

    if (user_config_file.is_open()) {
        std::cout << "[CavaThread] User CAVA config found: " << user_config_path << std::endl;
        config_stream << user_config_file.rdbuf();
        user_config_file.close();
    } else {
        std::cout << "[CavaThread] User CAVA config not found, using default." << std::endl;
        // Fallback to a default config if user's config doesn't exist
        config_stream << "[general]\nbars = 10\nframerate = 60\n[input]\nmethod = pulse\nsource = auto\n";
    }

    std::string config_content = config_stream.str();

    // Ensure the output section is configured for the widget
    std::string required_output_section = "[output]\nmethod = raw\nraw_target = " + FIFO_PATH + "\ndata_format = ascii\nascii_max_range = 15\n";
    size_t output_pos = config_content.find("[output]");
    if (output_pos != std::string::npos) {
        std::cout << "[CavaThread] Found [output] section in config." << std::endl;
        size_t next_section_pos = config_content.find("\n[", output_pos + 1);
        if (next_section_pos != std::string::npos) {
            config_content.replace(output_pos, next_section_pos - output_pos, required_output_section);
        } else {
            config_content.replace(output_pos, config_content.length() - output_pos, required_output_section);
        }
    } else {
        std::cout << "[CavaThread] [output] section not found, appending." << std::endl;
        config_content += "\n" + required_output_section;
    }

    // Write the final config to a temporary file
    std::string temp_config_path = "/tmp/music_widget_cava.config";
    std::ofstream temp_config_file(temp_config_path);
    if (!temp_config_file) {
        std::cerr << "[CavaThread] ERROR: Failed to create temporary CAVA config file: " << temp_config_path << std::endl;
        return;
    }
    temp_config_file << config_content;
    temp_config_file.close();
    std::cout << "[CavaThread] Temporary CAVA config written to: " << temp_config_path << std::endl;

    // Start CAVA with the temporary config file
    std::string command = "cava -p " + temp_config_path;
    std::cout << "[CavaThread] Starting CAVA with command: " << command << std::endl;
    FILE* cava_process = popen(command.c_str(), "r");
    if (!cava_process) {
        std::cerr << "[CavaThread] ERROR: Failed to start CAVA process." << std::endl;
        return;
    }
    std::cout << "[CavaThread] CAVA process started." << std::endl;

    // Open FIFO for reading
    std::cout << "[CavaThread] Opening FIFO for reading: " << FIFO_PATH << std::endl;
    int fifo_fd = open(FIFO_PATH.c_str(), O_RDONLY);
    if (fifo_fd == -1) {
        std::cerr << "[CavaThread] ERROR: Failed to open FIFO for reading: " << FIFO_PATH << ", errno: " << errno << std::endl;
        pclose(cava_process);
        return;
    }
    std::cout << "[CavaThread] FIFO opened for reading." << std::endl;

    char buffer[256];
    while (m_running) {
        ssize_t len = read(fifo_fd, buffer, sizeof(buffer) - 1);
        if (len > 0) {
            buffer[len] = '\0';
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_cava_data = buffer;
            }
            m_dispatcher.emit();
        } else if (len == 0) { // EOF
             usleep(10000); // Avoid busy-looping
        } else { // Error
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                std::cerr << "[CavaThread] ERROR: Failed to read from FIFO: " << FIFO_PATH << ", errno: " << errno << std::endl;
                break;
            }
        }
    }

    std::cout << "[CavaThread] Closing FIFO and CAVA process." << std::endl;
    close(fifo_fd);
    pclose(cava_process);
    std::cout << "[CavaThread] CAVA thread finished." << std::endl;
}

void CavaWidget::parse_cava_output(const std::string& cava_output) {
    m_bars.clear();
    std::string bar_value;
    for (char c : cava_output) {
        if (c == ';') {
            if (!bar_value.empty()) {
                m_bars.push_back(std::stoi(bar_value));
                bar_value.clear();
            }
        } else if (isdigit(c)) {
            bar_value += c;
        }
    }
}

bool CavaWidget::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
    int width = get_allocated_width();
    int height = get_allocated_height();

    // cr->set_source_rgba(0.0, 0.0, 0.0, 0.0); // Removed background clearing
    // cr->set_operator(Cairo::OPERATOR_SOURCE); // Removed
    // cr->paint(); // Removed
    cr->set_operator(Cairo::OPERATOR_OVER);

    cr->set_source_rgb(0.11, 0.72, 0.33); // Spotify Green

    if (m_bars.empty()) return true;

    int num_bars = m_bars.size();
    double total_bar_width = num_bars * m_bar_width;
    double total_spacing = (num_bars - 1) * m_spacing;
    double total_width = total_bar_width + total_spacing;
    
    double start_x = (width - total_width) / 2.0;

    for (int i = 0; i < num_bars; ++i) {
        double bar_height = (m_bars[i] / 15.0) * height;
        if (bar_height < 1.0) bar_height = 1.0; // Draw at least 1px
        
        double x = start_x + i * (m_bar_width + m_spacing);
        double y = height - bar_height;
        
        cr->rectangle(x, y, m_bar_width, bar_height);
        cr->fill();
    }

    return true;
}

void CavaWidget::on_size_allocate(Gtk::Allocation& allocation) {
    Gtk::DrawingArea::on_size_allocate(allocation);
    // Recalculate bar width and spacing if needed
    if (!m_bars.empty()) {
        int num_bars = m_bars.size();
        m_bar_width = (allocation.get_width() - (num_bars -1) * m_spacing) / num_bars;
        if (m_bar_width < 1) m_bar_width = 1;
    }
}
