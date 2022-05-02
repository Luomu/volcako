/*
 * Licensed under MIT. See LICENSE.txt for details.
 */
#pragma once

#include "algorithm.h"
#include "common.h"
#include "synthstate.h"

#include <rtmidi/RtMidi.h>
#include <vector>

struct GLFWwindow;

/** Holds various variables of the application state */
class AppState
{
public:
    bool show_options          = false;
    bool show_bulk_dump_window = false;
    bool patch_dirty           = false;
    bool quit_requested        = false;
    bool send_velocity         = false;
    bool dx7_compat_mode       = false;
    bool midi_settings_dirty   = false;

    RtMidiOut              midi_out = RtMidiOut(RtMidi::WINDOWS_MM);
    RtMidiIn               midi_in  = RtMidiIn(RtMidi::WINDOWS_MM);
    std::vector<Algorithm> algorithms;

    std::vector<std::string> midi_out_port_names;
    std::vector<std::string> midi_in_port_names;

    // patch library
    int                     current_patch = 0;
    std::vector<SynthState> patches;

    std::vector<std::string> error_messages;

    GLFWwindow* window;

    SynthState& get_current_patch()
    {
        return patches.at(current_patch);
    }

    AppState(GLFWwindow* window)
        : window(window)
    {
        patches.resize(32);
    }

    void refresh_available_midi_ports()
    {
        midi_out_port_names.clear();
        midi_out_port_names.push_back(MIDI_PORT_NONE);
        for (unsigned int i = 0; i < midi_out.getPortCount(); i++)
        {
            midi_out_port_names.push_back(midi_out.getPortName(i));
        }

        midi_in_port_names.clear();
        midi_in_port_names.push_back(MIDI_PORT_NONE);
        for (unsigned int i = 0; i < midi_in.getPortCount(); i++)
        {
            midi_in_port_names.push_back(midi_in.getPortName(i));
        }
    }

    void push_error_message(const std::string& error)
    {
        error_messages.push_back(error);
    }

    void pop_error_message()
    {
        error_messages.pop_back();
    }
};
