/*
 * Licensed under MIT. See LICENSE.txt for details.
 */
#include "settings.h"
#include "appstate.h"
#include "strings.h"
#include <fstream>

typedef inipp::Ini<char> Ini;

int chosen_in_port  = 0;
int chosen_out_port = 0;

int find_selected_index(const std::string& name, const std::vector<std::string>& port_names)
{
    int i = 0;
    for (auto& it : port_names)
    {
        if (it == name)
        {
            return i;
        }
        i++;
    }
    return 0;
}

Settings::Settings(const std::string& filename)
    : filename(filename)
    , send_velocity(true)
    , dx7_compat_mode(false)
{
    Ini ini;

    std::ifstream is(filename);
    if (!is.fail())
    {
        ini.parse(is);

        Ini::Section& sec = ini.sections["settings"];
        midi_in_device    = sec["midi_in_device"];
        midi_out_device   = sec["midi_out_device"];
        send_velocity     = sec["send_velocity"] == "true";
        dx7_compat_mode   = sec["dx7_compat_mode"] == "true";
    }
}

void Settings::save_to_disk()
{
    Ini ini;

    Ini::Section& sec      = ini.sections["settings"];
    sec["midi_in_device"]  = midi_in_device;
    sec["midi_out_device"] = midi_out_device;
    sec["send_velocity"]   = send_velocity ? "true" : "false";
    sec["dx7_compat_mode"] = dx7_compat_mode ? "true" : "false";

    std::ofstream of;
    of.open(filename);
    ini.generate(of);
    of.close();
}

void Settings::layout_settings_window(AppState& app_state)
{
    if (!ImGui::IsPopupOpen(Strings::TITLE_OPTIONS))
    {
        app_state.refresh_available_midi_ports();

        chosen_in_port  = find_selected_index(midi_in_device, app_state.midi_in_port_names);
        chosen_out_port = find_selected_index(midi_out_device, app_state.midi_out_port_names);

        ImGui::OpenPopup(Strings::TITLE_OPTIONS);
    }

    bool settings_changed = false;

    ImGui::BeginPopupModal(Strings::TITLE_OPTIONS);

    ImGui::Text("Midi Out");
    ImGui::PushID("midiout");
    if (!app_state.midi_out_port_names.empty())
    {
        int btn_idx = 0;
        for (const std::string& port_name : app_state.midi_out_port_names)
        {
            if (ImGui::RadioButton(port_name.c_str(), &chosen_out_port, btn_idx++))
            {
                set_midi_out_name(app_state.midi_out_port_names.at(chosen_out_port));
            }
        }
    }
    else
    {
        ImGui::Text(Strings::NO_AVAILABLE_PORTS);
    }
    ImGui::PopID();

    ImGui::Text("Midi In");
    ImGui::PushID("midiin");
    if (!app_state.midi_in_port_names.empty())
    {
        int btn_idx = 0;
        for (const std::string& port_name : app_state.midi_in_port_names)
        {
            if (ImGui::RadioButton(port_name.c_str(), &chosen_in_port, btn_idx++))
            {
                set_midi_in_name(app_state.midi_in_port_names.at(chosen_in_port));
            }
        }
    }
    else
    {
        ImGui::Text(Strings::NO_AVAILABLE_PORTS);
    }
    ImGui::PopID();

    ImGui::Checkbox(Strings::TITLE_DX7_COMPAT_MODE, &dx7_compat_mode);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip(Strings::TOOLTOP_DX7_COMPAT_MODE);

    if (ImGui::Button(Strings::BUTTON_CLOSE))
    {
        app_state.show_options        = false;
        app_state.midi_settings_dirty = true;
        app_state.dx7_compat_mode     = dx7_compat_mode;
    }

    ImGui::EndPopup();
}
