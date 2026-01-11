#ifndef MUSIC_WIDGET_H
#define MUSIC_WIDGET_H

#include "WindowState.h"
#include "SpectrumWidget.h"
#include <gtkmm.h>
#include <string>
#include <iostream>

#include <giomm/dbusconnection.h>
#include <giomm/dbusproxy.h>
#include <glibmm/refptr.h>
#include <glibmm/main.h> // Glib::signal_timeout() 포함
#include <sigc++/connection.h> // sigc::connection 포함
#include <glibmm/variant.h>


class SettingsManager;

class MusicWidget : public Gtk::Window
{
public:
    MusicWidget(const WindowState& state, SettingsManager& settingsManager);
    virtual ~MusicWidget();

    WindowState get_window_state();

protected:
    // Signal handlers (GTK Overrides)
    virtual bool on_button_press_event(GdkEventButton* event) override;
    virtual bool on_map_event(GdkEventAny* event) override;
    virtual bool on_visibility_notify_event(GdkEventVisibility* event) override;
    virtual void on_size_allocate(Gtk::Allocation& allocation) override;

private:
    SettingsManager& m_settings_manager;

    // ===================================
    // UI Components (VBox/HBox로 명확히 지정)
    // ===================================
    Gtk::VBox m_MainBox;
    Gtk::HBox m_TopHBox;
    Gtk::VBox m_InfoVBox;
    
    Gtk::HBox m_ControlHBox;
    Gtk::Box m_AlbumArtControlBox;
    Gtk::DrawingArea m_AlbumArtArea; // Image 대신 DrawingArea 사용
    Glib::RefPtr<Gdk::Pixbuf> m_CurrentPixbuf; // 원본 이미지 보관용
    Gtk::Label m_TrackLabel;
    Gtk::Label m_ArtistLabel;
    Gtk::Label m_AlbumLabel; // 앨범 라벨 추가
    Gtk::Button m_PrevButton;
    Gtk::Button m_PlayPauseButton;
    Gtk::Button m_NextButton;

    Gtk::ProgressBar m_ProgressBar;
    SpectrumWidget m_SpectrumWidget; // CavaWidget replaced with SpectrumWidget

    // ===================================
    // State, DBus & Timer Members
    // ===================================
    bool m_is_dragging;
    bool m_is_resizing;
    int m_drag_start_x, m_drag_start_y; 
    int m_window_start_x, m_window_start_y;
    int m_window_start_width, m_window_start_height;
    enum ResizeDirection { NONE, BOTTOM_RIGHT, RIGHT, BOTTOM };
    ResizeDirection m_resize_dir;
    Glib::ustring m_current_player_bus_name;

    // D-Bus
    // guint m_name_watch_id; // Removed
    // Glib::RefPtr<Gio::DBus::Connection> m_dbus_connection; // Removed
    Glib::RefPtr<Gio::DBus::Proxy> m_player_proxy;
    Glib::RefPtr<Gio::DBus::Proxy> m_properties_proxy;
    Glib::RefPtr<Gio::DBus::Proxy> m_dbus_proxy; // Added for org.freedesktop.DBus proxy

    // Timer Connections
    sigc::connection m_timer_connection;
    // 타입을 sigc::connection으로 변경하여 컴파일 오류 해결
    sigc::connection m_stick_timer_connection;
    sigc::connection m_player_discovery_connection; // Added for periodic player discovery timer
    
    // ===================================
    // Internal Methods
    // ===================================
    bool m_is_updating_ui = false; // Flag to prevent recursion in on_size_allocate
    void init_dbus();
    void find_and_update_player();
    void update_player_status();
    void call_player_method(const Glib::ustring& method_name);
    
    void on_properties_changed(
        const Glib::ustring& sender_name,
        const Glib::ustring& signal_name,
        const Glib::VariantContainerBase& parameters);

    virtual bool on_button_release_event(GdkEventButton* event) override;
    virtual bool on_motion_notify_event(GdkEventMotion* event) override;
    void update_progress_ui();
    bool on_album_art_draw(const Cairo::RefPtr<Cairo::Context>& cr); // 드로잉 핸들러
    void update_spectrum_simulation();

    // Removed on_name_appeared and on_name_vanished
    
    // Button Handlers
    void on_prev_clicked();
    void on_play_pause_clicked();
    void on_next_clicked();
    
    // Timer Handlers
    bool update_progress();
    bool on_stick_timer();
    bool on_player_discovery_timeout(); // Added for periodic player discovery
    void on_hide_event();
};

#endif // MUSIC_WIDGET_H

