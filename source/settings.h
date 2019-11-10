#pragma once

#include "imgui.h"
#include "inipp/inipp.h"

class AppState;

//Settings saved to .ini file, and settings window rendering
class Settings
{
private:
    std::string filename;
    std::string midi_out_device;
    std::string midi_in_device;
    bool send_velocity;

public:
    Settings(const std::string& filename);
    void save_to_disk();

    void layout_settings_window(AppState& app_state);

    const bool get_send_velocity() const { return send_velocity; }
    void set_send_velocity(bool v) { send_velocity = v; }
    const std::string& get_midi_out_name() const { return midi_out_device; }
    void set_midi_out_name(std::string v) { midi_out_device = v; }
    const std::string& get_midi_in_name() const { return midi_in_device; }
    void set_midi_in_name(std::string v) { midi_in_device = v; }
};
