#ifndef WF_PANEL_POPOVER_HPP
#define WF_PANEL_POPOVER_HPP

#include <config.hpp>
#include <gtkmm/menubutton.h>
#include <gtkmm/popover.h>

/**
 * A button which shows a popover on click. It adjusts the popup position
 * automatically based on panel position (valid values are "top" and "bottom")
 */
class WayfireMenuButton : public Gtk::MenuButton
{
    bool interactive = true;
    wf_option panel_position;
    wf_option_callback panel_position_changed;

    /* Make the menu button active on its AutohideWindow */
    void set_active_on_window();

  public:
    Gtk::Popover m_popover;

    WayfireMenuButton(wf_option panel_position);
    virtual ~WayfireMenuButton();

    /**
     * Set whether the popup should grab input focus when opened
     * By default, the menu button interacts with the keyboard.
     */
    void set_keyboard_interactive(bool interactive = true);

    /** @return Whether the menu button interacts with the keyboard */
    bool get_keyboard_interactive() const;

    /**
     * Grab the keyboard focus.
     * Also sets the popover to keyboard interactive.
     *
     * NOTE: this works only if the popover was already opened.
     */
    void grab_focus();
};

#endif /* end of include guard: WF_PANEL_POPOVER_HPP */