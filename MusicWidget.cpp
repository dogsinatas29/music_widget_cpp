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
      m_is_dragging(false)
{
    // 스펙트럼 시뮬레이션을 위한 랜덤 시드 초기화
    std::srand(std::time(0));

    // 이전 상태로 창 위치 설정
    move(state.x, state.y);

    set_decorated(false);
    set_resizable(false); // 크기 조절 비활성화
    set_keep_above(true);
    set_skip_taskbar_hint(true);
    set_skip_pager_hint(true);

    set_app_paintable(true);
    auto screen = Gdk::Screen::get_default();
    set_opacity(DEFAULT_OPACITY);

    set_default_size(state.width, state.height); // 초기 창 크기 설정

    add(m_MainBox);
    m_MainBox.set_name("music-widget");

    // m_TopHBox에 상단 패딩 3px 적용
    m_MainBox.pack_start(m_TopHBox, false, false, 3);
    
    // 앨범 아트 크기 설정 (main.cpp에서 계산된 initialState.width를 기반으로 함)
    // 최소 너비 200px 가정 시, 앨범 아트는 너비의 절반인 100px이 됩니다.
    m_AlbumArt.set_size_request(60, 60); // 앨범 아트 크기 60x60으로 고정
    m_AlbumArtOverlay.add(m_AlbumArt);

    m_ControlHBox.set_valign(Gtk::ALIGN_CENTER);
    m_ControlHBox.set_halign(Gtk::ALIGN_CENTER);

    // 1. m_AlbumArtVBox 설정
    m_AlbumArtVBox.set_orientation(Gtk::ORIENTATION_VERTICAL);
    m_AlbumArtVBox.set_halign(Gtk::ALIGN_START);

    // 2. 앨범 아트를 m_AlbumArtVBox에 추가 (상단 패딩 3px)
    m_AlbumArtVBox.pack_start(m_AlbumArtOverlay, false, false, 3);

    // 3. m_AlbumArtControlBox 설정
    m_AlbumArtControlBox.set_halign(Gtk::ALIGN_CENTER);
    m_AlbumArtControlBox.pack_start(m_ControlHBox, false, false, 0);

    // 4. m_AlbumArtControlBox (컨트롤 버튼)를 m_AlbumArtVBox에 추가 (간격 0)
    m_AlbumArtVBox.pack_start(m_AlbumArtControlBox, false, false, 0);

    // 5. m_AlbumArtVBox 전체를 m_TopHBox (수평 컨테이너)에 추가 (좌측 패딩 3px)
    m_TopHBox.pack_start(m_AlbumArtVBox, false, false, 3);

    // ========================================================================
    // 정보 섹션
    // ========================================================================
    m_InfoVBox.set_orientation(Gtk::Orientation::ORIENTATION_VERTICAL);
    m_InfoVBox.set_size_request(250, -1); // 가로 250px로 고정, 세로는 유동적
    // m_InfoVBox를 m_TopHBox에 추가 (좌측 패딩 3px)
    m_TopHBox.pack_start(m_InfoVBox, false, false, 3); // 정보 섹션 (오른쪽) - 수평 확장 허용 안함

    m_TrackLabel.set_halign(Gtk::ALIGN_START);
    m_ArtistLabel.set_halign(Gtk::ALIGN_START);
    m_InfoVBox.pack_start(m_TrackLabel, false, false, 0);
    m_InfoVBox.pack_start(m_ArtistLabel, false, false, 0);
    
    // 프로그레스 바를 아티스트 이름 바로 아래에 추가 (간격 0)
    m_ProgressBar.set_fraction(0.0);
    m_ProgressBar.set_show_text(true);
    m_ProgressBar.set_size_request(-1, 5);
    m_InfoVBox.pack_start(m_ProgressBar, false, false, 0);

    m_PrevButton.set_image_from_icon_name("media-skip-backward", Gtk::ICON_SIZE_BUTTON);
    m_PlayPauseButton.set_image_from_icon_name("media-playback-start", Gtk::ICON_SIZE_BUTTON);
    m_NextButton.set_image_from_icon_name("media-skip-forward", Gtk::ICON_SIZE_BUTTON);

    m_PrevButton.get_style_context()->add_class("control-button");
    m_PlayPauseButton.get_style_context()->add_class("control-button");
    m_NextButton.get_style_context()->add_class("control-button");

    m_ControlHBox.pack_start(m_PrevButton, false, false, 0);
    m_ControlHBox.pack_start(m_PlayPauseButton, false, false, 0);
    m_ControlHBox.pack_start(m_NextButton, false, false, 0);

    m_PrevButton.signal_clicked().connect(sigc::mem_fun(*this, &MusicWidget::on_prev_clicked));
    m_PlayPauseButton.signal_clicked().connect(sigc::mem_fun(*this, &MusicWidget::on_play_pause_clicked));
    m_NextButton.signal_clicked().connect(sigc::mem_fun(*this, &MusicWidget::on_next_clicked));

    add_events(Gdk::BUTTON_PRESS_MASK | Gdk::VISIBILITY_NOTIFY_MASK);
    signal_button_press_event().connect(sigc::mem_fun(*this, &MusicWidget::on_button_press_event));
    signal_map_event().connect(sigc::mem_fun(*this, &MusicWidget::on_map_event));
    signal_visibility_notify_event().connect(sigc::mem_fun(*this, &MusicWidget::on_visibility_notify_event));
    signal_hide().connect(sigc::mem_fun(*this, &MusicWidget::on_hide_event));

    try {
        init_dbus();
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
    
}

MusicWidget::~MusicWidget() {
    m_settings_manager.save_state(get_window_state());

    if (m_timer_connection.connected()) {
        m_timer_connection.disconnect();
    }
    if (m_stick_timer_connection.connected()) {
        m_stick_timer_connection.disconnect();
    }
    // Gio::DBus::unwatch_name은 Gio::DBus::watch_name을 통해 반환된 ID를 사용해야 합니다.
    Gio::DBus::unwatch_name(m_name_watch_id);
}

// ========================================================================
// 멤버 함수 구현 (이전 오류 해결을 위해 필수)
// ========================================================================

bool MusicWidget::on_map_event(GdkEventAny* event) {
    stick();
    return false;
}

bool MusicWidget::on_button_press_event(GdkEventButton* event) {
    // Ctrl + 클릭 시 이동 시작
    if (event->type == GDK_BUTTON_PRESS && event->button == 1 && (event->state & GDK_CONTROL_MASK)) {
        begin_move_drag(event->button, event->x_root, event->y_root, event->time);
        return true;
    }
    // 일반 클릭 이벤트 무시 (이동이 목적이므로)
    return false;
}

void MusicWidget::init_dbus() {
    m_name_watch_id = Gio::DBus::watch_name(
        Gio::DBus::BUS_TYPE_SESSION,
        "org.mpris.MediaPlayer2.rhythmbox", // Rhythmbox 대신 범용 이름 사용 가능
        sigc::mem_fun(*this, &MusicWidget::on_name_appeared),
        sigc::mem_fun(*this, &MusicWidget::on_name_vanished)
    );
}

void MusicWidget::update_player_status() {
    if (!m_player_proxy || !m_properties_proxy) {
        m_TrackLabel.set_text("No Player Active");
        m_ArtistLabel.set_text("Ctrl + Click to Move");
        // 앨범 아트 크기 조절
        int album_art_size = std::max(100, (get_width() > 0 ? get_width() : DEFAULT_WIDTH) / 2);
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
        std::map<Glib::ustring, Glib::VariantBase> properties_map = properties_map_variant.get();

        Glib::ustring title = "Unknown Track";
        Glib::ustring artist = "Unknown Artist";
        Glib::ustring playback_status = "Stopped";
        Glib::ustring art_url_str;

        if (properties_map.count("Metadata")) {
            auto metadata_map_variant = Glib::VariantBase::cast_dynamic<Glib::Variant<std::map<Glib::ustring, Glib::VariantBase>>>(properties_map["Metadata"]);
            std::map<Glib::ustring, Glib::VariantBase> metadata_map = metadata_map_variant.get();

            if (metadata_map.count("xesam:title")) {
                title = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(metadata_map["xesam:title"]).get();
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
                art_url_str = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(metadata_map["mpris:artUrl"]).get();
            }
        }
        
        if (properties_map.count("PlaybackStatus")) {
            playback_status = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(properties_map["PlaybackStatus"]).get();
        }

        m_TrackLabel.set_text(title);
        m_ArtistLabel.set_text(artist);

        // 앨범 아트 로딩
        if (!art_url_str.empty() && art_url_str.substr(0, 7) == "file://") {
            try {
                // substr(7)은 "file://" 부분을 제거하여 로컬 파일 경로만 남깁니다.
                auto pixbuf = Gdk::Pixbuf::create_from_file(art_url_str.substr(7));
                // 앨범 아트 크기 조절 (60x60으로 고정)
                m_AlbumArt.set(pixbuf->scale_simple(60, 60, Gdk::INTERP_BILINEAR));
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
    // 임시로 Rhythmbox로 고정. 실제로는 `org.mpris.MediaPlayer2`를 소유한 모든 이름 조회 필요.
    m_current_player_bus_name = "org.mpris.MediaPlayer2.rhythmbox"; 
    try {
        // DBus::Proxy::create_sync는 Glib::RefPtr<Gio::DBus::Proxy>를 반환
        m_player_proxy = Gio::DBus::Proxy::create_sync(m_dbus_connection, m_current_player_bus_name, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player");
        m_properties_proxy = Gio::DBus::Proxy::create_sync(m_dbus_connection, m_current_player_bus_name, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties");
                                                        
        m_properties_proxy->signal_signal().connect(sigc::mem_fun(*this, &MusicWidget::on_properties_changed));
    
        update_player_status();
    } catch (const Glib::Error& ex) {
        m_player_proxy.reset();
        m_properties_proxy.reset();
        update_player_status();
    }
}

void MusicWidget::on_properties_changed(
    const Glib::ustring& sender_name,
    const Glib::ustring& signal_name,
    const Glib::VariantContainerBase& parameters)
{
    // PropertiesChanged 시그널이 발생하면 상태 업데이트
    if (signal_name == "PropertiesChanged") {
        update_player_status();
    }
}

void MusicWidget::on_name_appeared(const Glib::RefPtr<Gio::DBus::Connection>& connection, const Glib::ustring& name, const Glib::ustring& name_owner) {
    m_dbus_connection = connection;
    find_and_update_player();
    // 1초마다 진행률을 업데이트하는 타이머를 연결합니다.
    if (!m_timer_connection.connected()) {
        m_timer_connection = Glib::signal_timeout().connect(sigc::mem_fun(*this, &MusicWidget::update_progress), 1000);
    }
}

void MusicWidget::on_name_vanished(const Glib::RefPtr<Gio::DBus::Connection>& connection, const Glib::ustring& name) {
    if (name == m_current_player_bus_name) {
        m_player_proxy.reset();
        m_properties_proxy.reset();
        m_current_player_bus_name = "";
        if (m_timer_connection.connected()) {
            m_timer_connection.disconnect();
        }
        update_player_status();
    }
}

bool MusicWidget::update_progress() {
    if (!m_properties_proxy) return false;

    try {
        // Get Position
        auto position_variant_container = m_properties_proxy->call_sync("Get", Glib::Variant<std::tuple<Glib::ustring, Glib::ustring>>::create(std::make_tuple("org.mpris.MediaPlayer2.Player", "Position")));
        Glib::Variant<Glib::Variant<gint64>> position_variant;
        position_variant_container.get_child(position_variant, 0);
        gint64 position = position_variant.get().get(); // microseconds

        // Get Metadata (to get Length)
        auto metadata_variant_container = m_properties_proxy->call_sync("Get", Glib::Variant<std::tuple<Glib::ustring, Glib::ustring>>::create(std::make_tuple("org.mpris.MediaPlayer2.Player", "Metadata")));
        Glib::Variant<Glib::Variant<std::map<Glib::ustring, Glib::VariantBase>>> metadata_variant;
        metadata_variant_container.get_child(metadata_variant, 0);
        std::map<Glib::ustring, Glib::VariantBase> metadata_map = metadata_variant.get().get();

        gint64 length = 0;
        if (metadata_map.count("mpris:length")) {
            length = Glib::VariantBase::cast_dynamic<Glib::Variant<gint64>>(metadata_map["mpris:length"]).get(); // microseconds
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
    } catch (const Glib::Error& ex) {
        m_ProgressBar.set_fraction(0.0);
        m_ProgressBar.set_text("N/A");
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
    std::cout << "[Debug] on_size_allocate called. New size: " 
              << allocation.get_width() << "x" << allocation.get_height() << std::endl;
}

