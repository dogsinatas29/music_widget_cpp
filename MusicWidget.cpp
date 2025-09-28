#include "MusicWidget.h"
#include <iostream>
#include <glibmm/main.h>
#include <string>
#include <vector>
#include <map>
#include <cstdio> // snprintf

// DBus related headers
#include <giomm/dbusconnection.h>
#include <giomm/dbusproxy.h>
#include <glibmm/variant.h>
#include <glibmm/varianttype.h>
#include <gio/gio.h>

// Constants
const int DEFAULT_WIDTH = 400;
const int DEFAULT_HEIGHT = 200;
const double DEFAULT_OPACITY = 0.8;

MusicWidget::MusicWidget()
    : m_MainBox(Gtk::Orientation::ORIENTATION_VERTICAL, 5),
      m_TopHBox(Gtk::Orientation::ORIENTATION_HORIZONTAL, 10),
      m_PlayerInfoControlsVBox(Gtk::Orientation::ORIENTATION_VERTICAL, 5),
      m_ControlHBox(Gtk::Orientation::ORIENTATION_HORIZONTAL, 5),
      m_AlbumArt("media-optical", Gtk::ICON_SIZE_DIALOG),
      m_TrackLabel("Test Track Label"),
      m_ArtistLabel("Test Artist Label"),
      m_PrevButton(),
      m_PlayPauseButton(),
      m_NextButton(),
      m_ProgressBar(),
      m_OpacityScale(Gtk::Orientation::ORIENTATION_HORIZONTAL),
      m_is_dragging(false)
{
    set_default_size(DEFAULT_WIDTH, DEFAULT_HEIGHT);

    set_decorated(false);
    set_keep_above(true);
    stick();

    set_app_paintable(true);
    auto screen = Gdk::Screen::get_default();
    set_opacity(DEFAULT_OPACITY);

    add(m_MainBox);
    m_MainBox.set_name("music-widget");
    m_MainBox.set_size_request(DEFAULT_WIDTH, DEFAULT_HEIGHT);

    m_MainBox.pack_start(m_TopHBox, true, true, 0);
    m_TopHBox.set_size_request(-1, 120);

    m_TopHBox.pack_start(m_AlbumArt, true, true, 10);
    m_AlbumArt.set_size_request(100, 100);

    m_TopHBox.pack_start(m_PlayerInfoControlsVBox, true, true, 0);
    m_PlayerInfoControlsVBox.set_size_request(200, -1);

    m_PlayerInfoControlsVBox.pack_start(m_TrackLabel, true, true, 0);
    m_TrackLabel.set_halign(Gtk::ALIGN_START);
    m_TrackLabel.set_size_request(-1, 25);

    m_PlayerInfoControlsVBox.pack_start(m_ArtistLabel, true, true, 0);
    m_ArtistLabel.set_halign(Gtk::ALIGN_START);
    m_ArtistLabel.set_size_request(-1, 25);

    m_PlayerInfoControlsVBox.pack_start(m_ControlHBox, true, true, 0);
    m_ControlHBox.set_size_request(-1, 50);

    m_PrevButton.set_image_from_icon_name("media-skip-backward", Gtk::ICON_SIZE_BUTTON);
    m_PlayPauseButton.set_image_from_icon_name("media-playback-start", Gtk::ICON_SIZE_BUTTON);
    m_NextButton.set_image_from_icon_name("media-skip-forward", Gtk::ICON_SIZE_BUTTON);
    m_PrevButton.set_size_request(60, 40);
    m_PlayPauseButton.set_size_request(60, 40);
    m_NextButton.set_size_request(60, 40);

    m_ControlHBox.pack_start(m_PrevButton, true, true, 0);
    m_ControlHBox.pack_start(m_PlayPauseButton, true, true, 0);
    m_ControlHBox.pack_start(m_NextButton, true, true, 0);

    m_ProgressBar.set_fraction(0.0);
    m_ProgressBar.set_show_text(true);
    m_ProgressBar.set_size_request(-1, 5);
    m_MainBox.pack_end(m_ProgressBar, false, true, 5);

    m_OpacityScale.set_range(0.1, 1.0);
    m_OpacityScale.set_increments(0.05, 0.1);
    m_OpacityScale.set_value(DEFAULT_OPACITY);
    m_OpacityScale.signal_value_changed().connect(sigc::mem_fun(*this, &MusicWidget::on_opacity_scale_changed));
    m_MainBox.pack_end(m_OpacityScale, false, true, 5);

    m_PrevButton.signal_clicked().connect(sigc::mem_fun(*this, &MusicWidget::on_prev_clicked));
    m_PlayPauseButton.signal_clicked().connect(sigc::mem_fun(*this, &MusicWidget::on_play_pause_clicked));
    m_NextButton.signal_clicked().connect(sigc::mem_fun(*this, &MusicWidget::on_next_clicked));

    add_events(Gdk::BUTTON_PRESS_MASK);
    signal_button_press_event().connect(sigc::mem_fun(*this, &MusicWidget::on_button_press_event));

    try {
        init_dbus();
    } catch (const std::exception& ex) {
        std::cerr << "DBus initialization error: " << ex.what() << std::endl;
    }

    auto css_provider = Gtk::CssProvider::create();
    try {
        css_provider->load_from_path("style.css");
        Gtk::StyleContext::add_provider_for_screen(screen, css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    } catch (const Glib::Error& ex) {
        std::cerr << "Error loading CSS: " << ex.what() << std::endl;
    }

    show_all_children();
}

MusicWidget::~MusicWidget() {
    if (m_timer_connection.connected()) {
        m_timer_connection.disconnect();
    }
    Gio::DBus::unwatch_name(m_name_watch_id);
}

bool MusicWidget::on_button_press_event(GdkEventButton* event) {
    if (event->type == GDK_BUTTON_PRESS && event->button == 1 && (event->state & GDK_CONTROL_MASK)) {
        begin_move_drag(event->button, event->x_root, event->y_root, event->time);
        return true;
    }
    return false;
}

void MusicWidget::on_opacity_scale_changed() {
    set_opacity(m_OpacityScale.get_value());
}

void MusicWidget::init_dbus() {
    m_name_watch_id = Gio::DBus::watch_name(
        Gio::DBus::BUS_TYPE_SESSION,
        "org.mpris.MediaPlayer2.rhythmbox", // This could be more generic
        sigc::mem_fun(*this, &MusicWidget::on_name_appeared),
        sigc::mem_fun(*this, &MusicWidget::on_name_vanished)
    );
}

void MusicWidget::update_player_status() {
    if (!m_player_proxy || !m_properties_proxy) {
        m_TrackLabel.set_text("No Player Active");
        m_ArtistLabel.set_text("");
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

        if (!art_url_str.empty() && art_url_str.substr(0, 7) == "file://") {
            try {
                auto pixbuf = Gdk::Pixbuf::create_from_file(art_url_str.substr(7));
                m_AlbumArt.set(pixbuf->scale_simple(100, 100, Gdk::INTERP_BILINEAR));
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
    if (!m_player_proxy) return;
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
    m_current_player_bus_name = "org.mpris.MediaPlayer2.rhythmbox";
    try {
        m_player_proxy = Gio::DBus::Proxy::create_sync(m_dbus_connection, m_current_player_bus_name, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player");
        m_properties_proxy = Gio::DBus::Proxy::create_sync(m_dbus_connection, m_current_player_bus_name, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties");
                                                                                  
  	// Connect to the PropertiesChanged signal
  	m_properties_proxy->signal_properties_changed().connect(sigc::mem_fun(*this,&MusicWidget::on_properties_changed));
    
        // Connect to the PropertiesChanged signal
        m_properties_proxy->signal_properties_changed().connect(sigc::mem_fun(*this, &MusicWidget::on_properties_changed));

        update_player_status();
    } catch (const Glib::Error& ex) {
        m_player_proxy.reset();
        m_properties_proxy.reset();
        update_player_status();
    }
}

void MusicWidget::on_name_appeared(const Glib::RefPtr<Gio::DBus::Connection>& connection, const Glib::ustring& name, const Glib::ustring& name_owner) {
    m_dbus_connection = connection;
    find_and_update_player();
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
        auto position_variant_container = m_properties_proxy->call_sync("Get", Glib::Variant<std::tuple<Glib::ustring, Glib::ustring>>::create(std::make_tuple("org.mpris.MediaPlayer2.Player", "Position")));
        Glib::Variant<Glib::Variant<gint64>> position_variant;
        position_variant_container.get_child(position_variant, 0);
        gint64 position = position_variant.get().get();

        auto metadata_variant_container = m_properties_proxy->call_sync("Get", Glib::Variant<std::tuple<Glib::ustring, Glib::ustring>>::create(std::make_tuple("org.mpris.MediaPlayer2.Player", "Metadata")));
        Glib::Variant<Glib::Variant<std::map<Glib::ustring, Glib::VariantBase>>> metadata_variant;
        metadata_variant_container.get_child(metadata_variant, 0);
        std::map<Glib::ustring, Glib::VariantBase> metadata_map = metadata_variant.get().get();

        gint64 length = 0;
        if (metadata_map.count("mpris:length")) {
            length = Glib::VariantBase::cast_dynamic<Glib::Variant<gint64>>(metadata_map["mpris:length"]).get();
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
