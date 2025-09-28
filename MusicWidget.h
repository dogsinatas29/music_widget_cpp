#ifndef MUSIC_WIDGET_H
#define MUSIC_WIDGET_H

#include <gtkmm.h>
#include <string>

// DBus 관련 헤더
#include <giomm/dbusconnection.h>
#include <giomm/dbusproxy.h>
// #include <giomm/bus.h> // 이 줄은 제거합니다!

class MusicWidget : public Gtk::Window
{
public:
    MusicWidget();
    virtual ~MusicWidget();

protected:
    bool on_button_press_event(GdkEventButton* event);
    void on_opacity_scale_changed();

    void on_prev_clicked();
    void on_play_pause_clicked();
    void on_next_clicked();

    void init_dbus();
    void update_player_status();
    void call_player_method(const Glib::ustring& method_name);

    // GIOMM 2.66.8의 signal_name_owner_changed 시그널 서명에 맞춤
    // Glib::RefPtr<Gio::DBus::Connection>& connection, const Glib::ustring& name, const Glib::ustring& old_owner, const Glib::ustring& new_owner
    void find_and_update_player();
    void on_name_appeared(const Glib::RefPtr<Gio::DBus::Connection>& connection, const Glib::ustring& name, const Glib::ustring& name_owner);
    void on_name_vanished(const Glib::RefPtr<Gio::DBus::Connection>& connection, const Glib::ustring& name);
    void on_properties_changed(
        const Glib::ustring& interface_name,
        const std::map<Glib::ustring, Glib::VariantBase>& changed_properties,
        const std::vector<Glib::ustring>& invalidated_properties);


    Gtk::Box m_MainBox;
    Gtk::Box m_TopHBox;
    Gtk::Box m_PlayerInfoControlsVBox;
    Gtk::Box m_ControlHBox;

    Gtk::Image m_AlbumArt;
    Gtk::Label m_TrackLabel;
    Gtk::Label m_ArtistLabel;

    Gtk::Button m_PrevButton;
    Gtk::Button m_PlayPauseButton;
    Gtk::Button m_NextButton;

    Gtk::ProgressBar m_ProgressBar;
    Gtk::Scale m_OpacityScale;

    bool m_is_dragging;
    int m_drag_start_x, m_drag_start_y;

    guint m_name_watch_id;
    Glib::RefPtr<Gio::DBus::Connection> m_dbus_connection;
    Glib::ustring m_current_player_bus_name;
    Glib::RefPtr<Gio::DBus::Proxy> m_player_proxy;
    Glib::RefPtr<Gio::DBus::Proxy> m_properties_proxy;
    sigc::connection m_timer_connection;

private:
    bool update_progress();
};

#endif // MUSIC_WIDGET_H
