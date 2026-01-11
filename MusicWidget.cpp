#include "MusicWidget.h"
#include "WindowState.h"
#include "SettingsManager.h"
#include <iostream>
#include <cmath>
#include <cstdlib> // std::rand, std::srand
#include <ctime>   // std::time
#include <glibmm/main.h>
#include <sigc++/bind.h>
#include <gdkmm/pixbuf.h> // Gdk::Pixbuf 사용을 위해 추가
#include <string>
#include <vector>
#include <map>
#include <cstdio>
#include <algorithm> // for std::min

// D-Bus 관련 헤더
#include <giomm/dbusconnection.h>
#include <giomm/dbusproxy.h>
#include <glibmm/variant.h>
#include <glibmm/varianttype.h>
#include <gio/gio.h>

const int DEFAULT_WIDTH = 200;
const double DEFAULT_OPACITY = 0.8;

MusicWidget::MusicWidget(const WindowState& state, SettingsManager& settingsManager)
    : m_settings_manager(settingsManager),
      m_MainBox(Gtk::Orientation::ORIENTATION_VERTICAL, 0),
      m_TopHBox(Gtk::Orientation::ORIENTATION_HORIZONTAL, 0),
      m_ControlHBox(Gtk::Orientation::ORIENTATION_HORIZONTAL, 0),
      m_InfoVBox(Gtk::Orientation::ORIENTATION_VERTICAL, 0),
      m_AlbumArtOverlay(),
      m_AlbumArtVBox(),
      m_AlbumArtControlBox(),
      m_AlbumArt("media-optical", Gtk::ICON_SIZE_DIALOG),
      m_TrackLabel("No Music Playing"),
      m_ArtistLabel("Connects to MPRIS v2 Player"),
      m_PrevButton(),
      m_PlayPauseButton(),
      m_NextButton(),
      m_ProgressBar(),
      m_SpectrumWidget(),
      m_is_dragging(false),
      m_is_resizing(false),
      m_resize_dir(NONE)
{
    std::cerr << "[Debug] MusicWidget constructor called. Initializing..." << std::endl << std::flush;

    // 스펙트럼 시뮬레이션을 위한 랜덤 시드 초기화
    std::srand(std::time(0));

    // 이전 상태로 창 위치 설정
    move(state.x, state.y);

    set_decorated(false);
    set_resizable(true); // 크기 조절 활성화
    set_type_hint(Gdk::WINDOW_TYPE_HINT_UTILITY); // Wayland/X11 위젯 힌트
    set_keep_above(true);
    set_accept_focus(false); 
    set_skip_taskbar_hint(true);
    set_skip_pager_hint(true);

    // 창의 투명도(RGBA) 활성화
    auto screen = get_screen();
    auto visual = screen->get_rgba_visual();
    if (visual) {
        set_visual(visual);
    }
    set_app_paintable(true); // 배경을 직접 제어하기 위해 설정

    set_app_paintable(true);
    auto screen = Gdk::Screen::get_default();
    set_opacity(DEFAULT_OPACITY);

    set_default_size(state.width, state.height); // 초기 창 크기 설정

    add(m_MainBox);
    m_MainBox.set_name("music-widget");
    m_MainBox.set_valign(Gtk::ALIGN_CENTER);
    m_MainBox.set_halign(Gtk::ALIGN_FILL);

    // 1. 최상단 박스 배치 (상부: 정보, 하부: 스펙트럼)
    m_MainBox.pack_start(m_TopHBox, false, false, 10);
    m_MainBox.pack_start(m_SpectrumWidget, true, true, 0);

    // 2. 상단 수평 박스 배치 (좌측: 앨범 아트, 우측: 노래 정보 및 컨트롤)
    m_TopHBox.pack_start(m_AlbumArtVBox, false, false, 0);
    m_TopHBox.pack_start(m_InfoVBox, true, true, 15);

    // 3. 앨범 아트 영역 구성
    m_AlbumArt.set_name("album-art");
    m_AlbumArt.set_size_request(120, 120); 
    m_AlbumArtOverlay.add(m_AlbumArt);
    m_AlbumArtVBox.pack_start(m_AlbumArtOverlay, false, false, 0);

    // 4. 정보 섹션 구성 (곡명, 아티스트, 프로그레스, 컨트롤)
    m_InfoVBox.set_orientation(Gtk::Orientation::ORIENTATION_VERTICAL);
    m_InfoVBox.set_spacing(0);
    m_InfoVBox.pack_start(m_TrackLabel, false, false, 0);
    m_InfoVBox.pack_start(m_ArtistLabel, false, false, 0);
    m_InfoVBox.pack_start(m_ProgressBar, false, false, 10);
    
    // 컨트롤 버튼을 정보 섹션 바로 아래에 배치
    m_ControlHBox.set_spacing(8);
    m_ControlHBox.set_halign(Gtk::ALIGN_START);
    m_InfoVBox.pack_start(m_ControlHBox, false, false, 5);

    // 스펙트럼 시각화 위젯 설정
    m_SpectrumWidget.set_size_request(-1, 50);

    m_PrevButton.set_image_from_icon_name("media-skip-backward-symbolic", Gtk::ICON_SIZE_BUTTON);
    m_PlayPauseButton.set_image_from_icon_name("media-playback-start-symbolic", Gtk::ICON_SIZE_BUTTON);
    m_NextButton.set_image_from_icon_name("media-skip-forward-symbolic", Gtk::ICON_SIZE_BUTTON);

    m_PlayPauseButton.set_name("play-pause-button");
    m_TrackLabel.set_name("track-label");
    m_ArtistLabel.set_name("artist-label");

    m_PrevButton.get_style_context()->add_class("control-button");
    m_PlayPauseButton.get_style_context()->add_class("control-button");
    m_NextButton.get_style_context()->add_class("control-button");

    m_ControlHBox.pack_start(m_PrevButton, false, false, 0);
    m_ControlHBox.pack_start(m_PlayPauseButton, false, false, 0);
    m_ControlHBox.pack_start(m_NextButton, false, false, 0);

    m_PrevButton.signal_clicked().connect(sigc::mem_fun(*this, &MusicWidget::on_prev_clicked));
    m_PlayPauseButton.signal_clicked().connect(sigc::mem_fun(*this, &MusicWidget::on_play_pause_clicked));
    m_NextButton.signal_clicked().connect(sigc::mem_fun(*this, &MusicWidget::on_next_clicked));

    add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK | Gdk::VISIBILITY_NOTIFY_MASK);
    signal_button_press_event().connect(sigc::mem_fun(*this, &MusicWidget::on_button_press_event));
    signal_button_release_event().connect(sigc::mem_fun(*this, &MusicWidget::on_button_release_event));
    signal_motion_notify_event().connect(sigc::mem_fun(*this, &MusicWidget::on_motion_notify_event));
    signal_map_event().connect(sigc::mem_fun(*this, &MusicWidget::on_map_event));
    signal_visibility_notify_event().connect(sigc::mem_fun(*this, &MusicWidget::on_visibility_notify_event));
    signal_hide().connect(sigc::mem_fun(*this, &MusicWidget::on_hide_event));

    try {
        // init_dbus(); // Removed as it's now empty and not needed for periodic discovery
        m_player_discovery_connection = Glib::signal_timeout().connect(sigc::mem_fun(*this, &MusicWidget::on_player_discovery_timeout), 5000); // Start periodic discovery
    } catch (const std::exception& ex) {
        std::cerr << "DBus initialization error: " << ex.what() << std::endl;
    }

    auto css_provider = Gtk::CssProvider::create();
    try {
        css_provider->load_from_path("../style.css");
        Gtk::StyleContext::add_provider_for_screen(screen, css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    } catch (const Glib::Error& ex) {
        std::cerr << "Error loading CSS: " << ex.what() << std::endl;
    }

    show_all_children();

    m_stick_timer_connection = Glib::signal_timeout().connect(sigc::mem_fun(*this, &MusicWidget::on_stick_timer), 250);
    
    // 스펙트럼 시뮬레이션 타이머 (50ms)
    Glib::signal_timeout().connect([this]() {
        update_spectrum_simulation();
        return true;
    }, 50);
    
}

MusicWidget::~MusicWidget() {
    m_settings_manager.save_state(get_window_state());

    if (m_timer_connection.connected()) {
        m_timer_connection.disconnect();
    }
    if (m_stick_timer_connection.connected()) {
        m_stick_timer_connection.disconnect();
    }
    // Disconnect the player discovery timer if it's connected
    if (m_player_discovery_connection.connected()) {
        m_player_discovery_connection.disconnect();
    }
    // Gio::DBus::unwatch_name은 Gio::DBus::watch_name을 통해 반환된 ID를 사용해야 합니다。 // Removed
    // Gio::DBus::unwatch_name(m_name_watch_id); // Removed
}

// ========================================================================
// 멤버 함수 구현 (이전 오류 해결을 위해 필수)
// ========================================================================

bool MusicWidget::on_map_event(GdkEventAny* event) {
    stick();
    return false;
}

bool MusicWidget::on_button_press_event(GdkEventButton* event) {
    if (event->type == GDK_BUTTON_PRESS && event->button == 1) {
        int width, height;
        get_size(width, height);
        
        const int margin = 35; // 리사이즈 감지 영역 확대 (35px)

        // 리사이즈 방향 감지
        if (event->x > width - margin && event->y > height - margin) {
            m_is_resizing = true;
            m_resize_dir = BOTTOM_RIGHT;
        } else if (event->x > width - margin) {
            m_is_resizing = true;
            m_resize_dir = RIGHT;
        } else if (event->y > height - margin) {
            m_is_resizing = true;
            m_resize_dir = BOTTOM;
        } else {
            // 드래그 시작
            m_is_dragging = true;
            m_drag_start_x = event->x_root;
            m_drag_start_y = event->y_root;
            get_position(m_window_start_x, m_window_start_y);
        }

        if (m_is_resizing) {
            GdkWindowEdge edge;
            if (m_resize_dir == BOTTOM_RIGHT) edge = GDK_WINDOW_EDGE_SOUTH_EAST;
            else if (m_resize_dir == RIGHT) edge = GDK_WINDOW_EDGE_EAST;
            else edge = GDK_WINDOW_EDGE_SOUTH;

            begin_resize_drag(edge, event->button, event->x_root, event->y_root, event->time);
            m_is_resizing = false; // OS에 맡겼으므로 플래그 해제
        } else if (m_is_dragging) {
            begin_move_drag(event->button, event->x_root, event->y_root, event->time);
            m_is_dragging = false; 
        }
        return true;
    }
    return false;
}

bool MusicWidget::on_button_release_event(GdkEventButton* event) {
    m_is_dragging = false;
    m_is_resizing = false;
    m_resize_dir = NONE;
    return true;
}

bool MusicWidget::on_motion_notify_event(GdkEventMotion* event) {
    // 이제 OS(컴포지터)가 드래그/리사이즈를 처리하므로 이 함수에서는 아무것도 하지 않아도 됩니다.
    return false;
}

void MusicWidget::update_spectrum_simulation() {
    static std::vector<double> spectrum(30, 0.0);
    for (int i = 0; i < 30; ++i) {
        // 부드러운 움직임을 위해 랜덤값에 가중치 부여
        double target = static_cast<double>(std::rand()) / RAND_MAX;
        spectrum[i] = spectrum[i] * 0.7 + target * 0.3;
    }
    m_SpectrumWidget.update_spectrum_data(spectrum);
}

void MusicWidget::init_dbus() {
    // No longer watching names directly here. Periodic polling will handle discovery.
}

void MusicWidget::update_player_status() {
    if (!m_player_proxy || !m_properties_proxy) {
        m_TrackLabel.set_text("No Player Active");
        m_ArtistLabel.set_text("Drag to Move");
        // 앨범 아트 크기 조절 (창 높이에 맞춰 조절, 최소 60, 최대 150 등 제한 가능)
        int win_height;
        get_size(win_height, win_height);
        int album_art_size = std::max(60, std::min(150, (win_height - 60))); // 하단 CAVA 등을 고려하여 높이 조정
        m_AlbumArt.set_size_request(album_art_size, album_art_size);
        m_AlbumArt.set_from_icon_name("media-optical", Gtk::ICON_SIZE_DIALOG);
        m_PlayPauseButton.set_image_from_icon_name("media-playback-start", Gtk::ICON_SIZE_BUTTON);
        return;
    }

    try {
        auto get_all_args = Glib::Variant<std::tuple<Glib::ustring>>::create(
            std::make_tuple("org.mpris.MediaPlayer2.Player")
        );

        Glib::VariantContainerBase result_container = m_properties_proxy->call_sync(
            "GetAll",
            get_all_args
        );
        
        Glib::Variant<std::map<Glib::ustring, Glib::VariantBase>> properties_map_variant;
        result_container.get_child(properties_map_variant, 0);
        
        if (!properties_map_variant.is_of_type(Glib::VariantType("a{sv}"))) {
             std::cerr << "[MusicWidget] Unexpected properties map type" << std::endl;
             return;
        }
        std::map<Glib::ustring, Glib::VariantBase> properties_map = properties_map_variant.get();

        Glib::ustring title = "Unknown Track";
        Glib::ustring artist = "Unknown Artist";
        Glib::ustring playback_status = "Stopped";
        Glib::ustring art_url_str;

        if (properties_map.count("Metadata")) {
            auto metadata_var = properties_map["Metadata"];
            if (metadata_var.is_of_type(Glib::VariantType("a{sv}"))) {
                auto metadata_map_variant = Glib::VariantBase::cast_dynamic<Glib::Variant<std::map<Glib::ustring, Glib::VariantBase>>>(metadata_var);
                std::map<Glib::ustring, Glib::VariantBase> metadata_map = metadata_map_variant.get();

                if (metadata_map.count("xesam:title")) {
                    auto title_var = metadata_map["xesam:title"];
                    if (title_var.is_of_type(Glib::VariantType("s"))) {
                        title = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(title_var).get();
                    }
                }
                if (metadata_map.count("xesam:artist")) {
                    Glib::VariantBase artist_base = metadata_map["xesam:artist"];
                    if (artist_base.is_of_type(Glib::VariantType("as"))) {
                        auto artists_array = Glib::VariantBase::cast_dynamic<Glib::Variant<std::vector<Glib::ustring>>>(artist_base).get();
                        if (!artists_array.empty()) {
                            artist = artists_array[0];
                            for(size_t i = 1; i < artists_array.size(); ++i) artist += ", " + artists_array[i];
                        }
                    } else if (artist_base.is_of_type(Glib::VariantType("s"))) {
                        artist = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(artist_base).get();
                    }
                }
                if (metadata_map.count("mpris:artUrl")) {
                    auto art_url_var = metadata_map["mpris:artUrl"];
                    if (art_url_var.is_of_type(Glib::VariantType("s"))) {
                        art_url_str = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(art_url_var).get();
                    }
                }
            }
        }
        
        if (properties_map.count("PlaybackStatus")) {
             auto ps_var = properties_map["PlaybackStatus"];
             if (ps_var.is_of_type(Glib::VariantType("s"))) {
                playback_status = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(ps_var).get();
             }
        }

        m_TrackLabel.set_text(title);
        m_ArtistLabel.set_text(artist);

        // 앨범 아트 로딩
        if (!art_url_str.empty() && art_url_str.substr(0, 7) == "file://") {
            try {
                // substr(7)은 "file://" 부분을 제거하여 로컬 파일 경로만 남깁니다.
                auto pixbuf = Gdk::Pixbuf::create_from_file(art_url_str.substr(7));
                int win_height;
                get_size(win_height, win_height);
                int album_art_size = std::max(60, std::min(200, (win_height - 60)));
                m_AlbumArt.set_size_request(album_art_size, album_art_size);
                m_AlbumArt.set(pixbuf->scale_simple(album_art_size, album_art_size, Gdk::INTERP_BILINEAR));
            } catch (const Glib::Error& e) {
                m_AlbumArt.set_from_icon_name("media-optical", Gtk::ICON_SIZE_DIALOG);
            }
        } else {
            m_AlbumArt.set_from_icon_name("media-optical", Gtk::ICON_SIZE_DIALOG);
        }

        if (playback_status == "Playing") {
            m_PlayPauseButton.set_image_from_icon_name("media-playback-pause", Gtk::ICON_SIZE_BUTTON);
        } else {
            m_PlayPauseButton.set_image_from_icon_name("media-playback-start", Gtk::ICON_SIZE_BUTTON);
        }

    } catch (const Glib::Error& ex) {
        std::cerr << "Error updating player status (DBus): " << ex.what() << std::endl;
    }
}

void MusicWidget::call_player_method(const Glib::ustring& method_name) {
    if (!m_player_proxy) {
        std::cerr << "[MusicWidget] ERROR: Player proxy is not valid for method: " << method_name << std::endl;
        return;
    }
    try {
        m_player_proxy->call_sync(method_name, Glib::Variant<std::tuple<>>::create(std::make_tuple()));
    } catch (const Glib::Error& ex) {
        std::cerr << "Error calling DBus method " << method_name << ": " << ex.what() << std::endl;
    }
}

void MusicWidget::on_prev_clicked() { call_player_method("Previous"); }
void MusicWidget::on_play_pause_clicked() { call_player_method("PlayPause"); }
void MusicWidget::on_next_clicked() { call_player_method("Next"); }

void MusicWidget::find_and_update_player() {
    std::cerr << "[Debug] find_and_update_player() called." << std::endl << std::flush;
    std::cout << "[Debug] find_and_update_player() called." << std::endl << std::flush;

    // Reset current player state
    m_player_proxy.reset();
    m_properties_proxy.reset();
    m_current_player_bus_name = "";
    if (m_timer_connection.connected()) {
        m_timer_connection.disconnect();
        std::cerr << "[Debug] Disconnected progress timer." << std::endl << std::flush;
    }

    std::cout << "[Debug] find_and_update_player() called." << std::endl << std::flush;

    try {
        if (!m_dbus_proxy) {
            m_dbus_proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SESSION, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus");
            if (!m_dbus_proxy) return;
        }

        auto result_container = m_dbus_proxy->call_sync("ListNames", Glib::Variant<std::tuple<>>::create(std::make_tuple()));
        Glib::Variant<std::vector<Glib::ustring>> names_variant;
        result_container.get_child(names_variant, 0);
        std::vector<Glib::ustring> names = names_variant.get();

        // 1. 현재 플레이어가 여전히 목록에 있는지 확인
        bool current_still_exists = false;
        if (!m_current_player_bus_name.empty()) {
            for (const auto& name : names) {
                if (name == m_current_player_bus_name) {
                    current_still_exists = true;
                    break;
                }
            }
        }

        // 현재 플레이어가 재생 중이라면 굳이 다른 플레이어를 찾지 않음
        if (current_still_exists && m_player_proxy && m_properties_proxy) {
            try {
                auto val = m_properties_proxy->call_sync("Get", Glib::Variant<std::tuple<Glib::ustring, Glib::ustring>>::create(std::make_tuple("org.mpris.MediaPlayer2.Player", "PlaybackStatus")));
                Glib::Variant<Glib::Variant<Glib::ustring>> status_variant;
                val.get_child(status_variant, 0);
                if (status_variant.get().get() == "Playing") return; // 계속 유지
            } catch (...) {}
        }

        Glib::ustring best_player = "";
        Glib::RefPtr<Gio::DBus::Proxy> best_player_proxy;
        Glib::RefPtr<Gio::DBus::Proxy> best_properties_proxy;
        int best_score = -1;

        for (const auto& name : names) {
            if (name.find("org.mpris.MediaPlayer2") == 0) {
                try {
                    auto p_proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SESSION, name, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player");
                    auto prop_proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SESSION, name, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties");
                    if (!p_proxy || !prop_proxy) continue;

                    prop_proxy->set_default_timeout(300);
                    auto val = prop_proxy->call_sync("Get", Glib::Variant<std::tuple<Glib::ustring, Glib::ustring>>::create(std::make_tuple("org.mpris.MediaPlayer2.Player", "PlaybackStatus")));
                    Glib::Variant<Glib::Variant<Glib::ustring>> status_variant;
                    val.get_child(status_variant, 0);
                    Glib::ustring status = status_variant.get().get();

                    int score = 0;
                    if (status == "Playing") score += 100;
                    
                    // 우선순위: musicube나 spotify 등을 브라우저보다 높게 설정
                    if (name.find("musicube") != Glib::ustring::npos || 
                        name.find("spotify") != Glib::ustring::npos ||
                        name.find("rhythmbox") != Glib::ustring::npos) {
                        score += 50;
                    } else if (name.find("chrome") != Glib::ustring::npos || 
                               name.find("chromium") != Glib::ustring::npos ||
                               name.find("firefox") != Glib::ustring::npos) {
                        score -= 50; // 브라우저는 우선순위 낮춤
                    }

                    if (score > best_score) {
                        best_score = score;
                        best_player = name;
                        best_player_proxy = p_proxy;
                        best_properties_proxy = prop_proxy;
                    }
                } catch (...) { continue; }
            }
        }

        // 새로운 플레이어로 전환이 필요할 때만 업데이트
        if (!best_player.empty() && best_player != m_current_player_bus_name) {
            m_current_player_bus_name = best_player;
            m_player_proxy = best_player_proxy;
            m_properties_proxy = best_properties_proxy;
            if (m_properties_proxy) {
                m_properties_proxy->set_default_timeout(2000);
                m_properties_proxy->signal_signal().connect(sigc::mem_fun(*this, &MusicWidget::on_properties_changed));
            }
            update_player_status();
            if (!m_timer_connection.connected()) {
                m_timer_connection = Glib::signal_timeout().connect(sigc::mem_fun(*this, &MusicWidget::update_progress), 1000);
            }
            std::cout << "[Debug] Switched to better player: " << m_current_player_bus_name << std::endl;
        } else if (best_player.empty()) {
            m_player_proxy.reset();
            m_properties_proxy.reset();
            m_current_player_bus_name = "";
            update_player_status();
        }
    } catch (const Glib::Error& ex) {
        std::cerr << "Error in find_and_update_player: " << ex.what() << std::endl;
    }
    } catch (const Glib::Error& ex) {
        std::cerr << "Error finding and updating player (DBus): " << ex.what() << std::endl << std::flush;
        std::cerr << "Error finding and updating player (DBus): " << ex.what() << std::endl << std::flush;
        m_player_proxy.reset();
        m_properties_proxy.reset();
        m_current_player_bus_name = "";
        if (m_timer_connection.connected()) {
            m_timer_connection.disconnect();
        }
        update_player_status();
    }
}

void MusicWidget::on_properties_changed(
    const Glib::ustring& sender_name,
    const Glib::ustring& signal_name,
    const Glib::VariantContainerBase& parameters)
{
    try {
        // PropertiesChanged 시그널이 발생하면 상태 업데이트
        if (signal_name == "PropertiesChanged") {
            update_player_status();
        }
    } catch (...) {
        std::cerr << "[MusicWidget] Error in on_properties_changed" << std::endl;
    }
}

bool MusicWidget::on_player_discovery_timeout() {
    find_and_update_player();
    return true; // Keep the timer running
}

bool MusicWidget::update_progress() {
    if (!m_properties_proxy) return false;

    try {
        // Get Position
        auto position_variant_container = m_properties_proxy->call_sync("Get", Glib::Variant<std::tuple<Glib::ustring, Glib::ustring>>::create(std::make_tuple("org.mpris.MediaPlayer2.Player", "Position")));
        
        if (position_variant_container.get_n_children() > 0) {
            Glib::Variant<Glib::VariantBase> wrapped_pos;
            position_variant_container.get_child(wrapped_pos, 0);
            auto pos_var = wrapped_pos.get();
            if (pos_var.is_of_type(Glib::VariantType("x"))) {
                gint64 position = Glib::VariantBase::cast_dynamic<Glib::Variant<gint64>>(pos_var).get();

                // Get Metadata (to get Length)
                auto metadata_variant_container = m_properties_proxy->call_sync("Get", Glib::Variant<std::tuple<Glib::ustring, Glib::ustring>>::create(std::make_tuple("org.mpris.MediaPlayer2.Player", "Metadata")));
                if (metadata_variant_container.get_n_children() > 0) {
                    Glib::Variant<Glib::VariantBase> wrapped_meta;
                    metadata_variant_container.get_child(wrapped_meta, 0);
                    auto meta_var = wrapped_meta.get();
                    if (meta_var.is_of_type(Glib::VariantType("a{sv}"))) {
                        auto metadata_map = Glib::VariantBase::cast_dynamic<Glib::Variant<std::map<Glib::ustring, Glib::VariantBase>>>(meta_var).get();

                        gint64 length = 0;
                        if (metadata_map.count("mpris:length")) {
                            auto len_var = metadata_map["mpris:length"];
                            if (len_var.is_of_type(Glib::VariantType("x"))) {
                                length = Glib::VariantBase::cast_dynamic<Glib::Variant<gint64>>(len_var).get();
                            }
                        }

                        if (length > 0) {
                            double fraction = static_cast<double>(position) / static_cast<double>(length);
                            m_ProgressBar.set_fraction(fraction);
                            
                            auto pos_seconds = position / 1000000;
                            auto len_seconds = length / 1000000;
                            char buffer[128];
                            snprintf(buffer, sizeof(buffer), "%02ld:%02ld / %02ld:%02ld", pos_seconds / 60, pos_seconds % 60, len_seconds / 60, len_seconds % 60);
                            m_ProgressBar.set_text(buffer);
                        } else {
                            m_ProgressBar.set_fraction(0.0);
                            m_ProgressBar.set_text("00:00 / 00:00");
                        }
                    }
                }
            }
        }
    } catch (...) {
        m_ProgressBar.set_fraction(0.0);
        m_ProgressBar.set_text("Error");
    }
    return true;
}

WindowState MusicWidget::get_window_state()
{
    WindowState current_state;
    int x, y, width, height;
    
    get_position(x, y);
    get_size(width, height);

    current_state.x = x;
    current_state.y = y;
    current_state.width = width;
    current_state.height = height;
    
    std::cout << "[Debug] get_window_state called. "
              << "Position: (" << x << ", " << y << "), "
              << "Size: (" << width << "x" << height << ")" << std::endl;

    return current_state;
}

bool MusicWidget::on_visibility_notify_event(GdkEventVisibility* event)
{
    // 창이 완전히 가려지면 숨기기 (유니티나 일부 WM에서 작동)
    if (event->state == GDK_VISIBILITY_FULLY_OBSCURED) {
        std::cout << "[Debug] Visibility changed to FULLY_OBSCURED. Hiding window." << std::endl;
        // hide(); // 시작 시 바로 닫히는 문제 해결을 위해 일시적으로 비활성화
    }
    return false;
}

bool MusicWidget::on_stick_timer()
{
    // 250ms마다 창을 '모든 작업 공간에 고정(stick)' 상태로 유지
    stick(); 
    return true;
}

void MusicWidget::on_hide_event()
{
    std::cout << "[Debug] on_hide_event called. Saving window state." << std::endl;
    // 창이 닫히기 직전에 상태 저장
    m_settings_manager.save_state(get_window_state());
}

void MusicWidget::on_size_allocate(Gtk::Allocation& allocation)
{
    // 기본 동작 호출
    Gtk::Window::on_size_allocate(allocation);
    
    if (m_is_updating_ui) return;
    m_is_updating_ui = true;
    
    // 크기가 바뀌면 앨범 아트 등을 다시 그릴 수 있도록 상태 업데이트 호출
    update_player_status();

    m_is_updating_ui = false;

    std::cout << "[Debug] on_size_allocate called. New size: " 
              << allocation.get_width() << "x" << allocation.get_height() << std::endl;
}

