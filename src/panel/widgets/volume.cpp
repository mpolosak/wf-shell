#include <iostream>
#include "volume.hpp"
#include "launchers.hpp"
#include "config.hpp"
#include "gtk-utils.hpp"

volume_level
WayfireVolume::get_volume_level(pa_volume_t v)
{
    if (v == 0)
        return MUTE;
    else if (v > 0 && v <= (max_norm / 3))
        return LOW;
    else if (v > (max_norm / 3) && v <= ((max_norm / 3) * 2))
        return MED;
    else if (v > ((max_norm / 3) * 2) && v <= max_norm)
        return HIGH;

    return OOR;
}

void
WayfireVolume::update_icon()
{
    int size = volume_size->as_int() / LAUNCHERS_ICON_SCALE;
    volume_level last, current;

    last = get_volume_level(last_volume);
    current = get_volume_level(current_volume);

    if (last == current)
        return;

    button->set_size_request(0, 0);
    if (current == MUTE)
        main_image.set_from_icon_name("audio-volume-muted", Gtk::ICON_SIZE_MENU);
    else if (current == LOW)
        main_image.set_from_icon_name("audio-volume-low",  Gtk::ICON_SIZE_MENU);
    else if (current == MED)
        main_image.set_from_icon_name("audio-volume-medium", Gtk::ICON_SIZE_MENU);
     else if (current == HIGH)
        main_image.set_from_icon_name("audio-volume-high", Gtk::ICON_SIZE_MENU);
     else
        printf("GVC: Volume out of range\n");
}

void
WayfireVolume::update_volume(int direction)
{
    last_volume = current_volume;
    current_volume += inc * direction;
    if (current_volume > max_norm)
        current_volume = max_norm;
    else if (int32_t(current_volume) < 0)
        current_volume = 0;
    gvc_mixer_stream_set_volume(gvc_stream, current_volume);
    gvc_mixer_stream_push_volume(gvc_stream);
    update_icon();
}

void
WayfireVolume::on_scroll(GdkEventScroll *event)
{
    if (event->direction == GDK_SCROLL_SMOOTH) {
        if (event->delta_y > 0)
            update_volume(-1);
        else if (event->delta_y < 0)
            update_volume(1);
    }
}

static void
default_sink_changed (GvcMixerControl *gvc_control,
                      guint            id,
                      gpointer         user_data)
{
    WayfireVolume *wf_volume = (WayfireVolume *) user_data;

    wf_volume->gvc_stream = gvc_mixer_control_get_default_sink(gvc_control);
    if (!wf_volume->gvc_stream) {
        printf("GVC: Failed to get default sink\n");
        return;
    }
    wf_volume->max_norm = gvc_mixer_control_get_vol_max_norm(gvc_control);
    wf_volume->inc = wf_volume->max_norm / 20;

    wf_volume->current_volume = gvc_mixer_stream_get_volume(wf_volume->gvc_stream);
    wf_volume->update_icon();
}

void
WayfireVolume::init(Gtk::HBox *container, wayfire_config *config)
{
    auto config_section = config->get_section("panel");

    volume_size = config_section->get_option("launcher_size",
        std::to_string(DEFAULT_ICON_SIZE));
    volume_size_changed = [=] () { update_icon(); };
    volume_size->add_updated_handler(&volume_size_changed);

    button = std::unique_ptr<WayfireMenuButton> (new WayfireMenuButton(config));
    button->add(main_image);
    auto style = button->get_style_context();
    style->context_save();
    style->set_state(Gtk::STATE_FLAG_NORMAL & ~Gtk::STATE_FLAG_PRELIGHT);
    button->reset_style();
    button->get_popover()->set_constrain_to(Gtk::POPOVER_CONSTRAINT_NONE);
    button->set_events(Gdk::SMOOTH_SCROLL_MASK);
    button->signal_scroll_event().connect_notify(
        sigc::mem_fun(this, &WayfireVolume::on_scroll));

    update_icon();

    button->property_scale_factor().signal_changed().connect(
        [=] () {update_icon(); });

    gvc_control = gvc_mixer_control_new("Wayfire Volume Control");

    g_signal_connect (gvc_control, "default-sink-changed",
        G_CALLBACK (default_sink_changed), this);

    gvc_mixer_control_open(gvc_control);

    container->pack_start(hbox, false, false);
    hbox.pack_start(*button, false, false);

    hbox.show();
    main_image.show();
    button->show();
}

void
WayfireVolume::focus_lost()
{
    button->set_active(false);
}

WayfireVolume::~WayfireVolume()
{
    volume_size->rem_updated_handler(&volume_size_changed);
}