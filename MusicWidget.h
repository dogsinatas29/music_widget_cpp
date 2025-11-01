#ifndef MUSIC_WIDGET_H
#define MUSIC_WIDGET_H

#include "WindowState.h"
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

    Gtk::VBox m_AlbumArtVBox;
    Gtk::Overlay m_AlbumArtOverlay;
    
    Gtk::HBox m_ControlHBox;
    Gtk::HBox m_AlbumArtControlBox;

    Gtk::Image m_AlbumArt;
    Gtk::Label m_TrackLabel;
    Gtk::Label m_ArtistLabel;

    Gtk::Button m_PrevButton;
    Gtk::Button m_PlayPauseButton;
    Gtk::Button m_NextButton;

    Gtk::ProgressBar m_ProgressBar;

    // ===================================
    // State, DBus & Timer Members
    // ===================================
    bool m_is_dragging;
    int m_drag_start_x, m_drag_start_y; // 드래그 상태 변수
    Glib::ustring m_current_player_bus_name;

    // D-Bus
    guint m_name_watch_id;
    Glib::RefPtr<Gio::DBus::Connection> m_dbus_connection;
    Glib::RefPtr<Gio::DBus::Proxy> m_player_proxy;
    Glib::RefPtr<Gio::DBus::Proxy> m_properties_proxy;

    // Timer Connections
    sigc::connection m_timer_connection;
    // 타입을 sigc::connection으로 변경하여 컴파일 오류 해결
    sigc::connection m_stick_timer_connection;
    
    // ===================================
    // Internal Methods
    // ===================================
    void init_dbus();
    void find_and_update_player();
    void update_player_status();
    void call_player_method(const Glib::ustring& method_name);
    
    void on_properties_changed(
        const Glib::ustring& sender_name,
        const Glib::ustring& signal_name,
        const Glib::VariantContainerBase& parameters);

    void on_name_appeared(const Glib::RefPtr<Gio::DBus::Connection>& connection, const Glib::ustring& name, const Glib::ustring& name_owner);
    void on_name_vanished(const Glib::RefPtr<Gio::DBus::Connection>& connection, const Glib::ustring& name);
    
    // Button Handlers
    void on_prev_clicked();
    void on_play_pause_clicked();
    void on_next_clicked();
    
    // Timer Handlers
    bool update_progress();
    bool on_stick_timer();
    void on_hide_event();
};

#endif // MUSIC_WIDGET_H

