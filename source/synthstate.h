/*
 * Licensed under MIT. See LICENSE.txt for details.
 */
#pragma once

#include "envelope.h"
#include "operator.h"
#include <vector>

class RtMidiOut;

// Patch state
struct SynthState
{
    // sent names are not null terminated, but padded with space
    char patch_name[11] = { 'I', 'N', 'I', 'T', '\0', 0, 0, 0, 0, 0, 0 };

    u8 algorithm = 1;

    Operator op1;
    Operator op2;
    Operator op3;
    Operator op4;
    Operator op5;
    Operator op6;

    Envelope pitch_env = { 99, 99, 99, 99, 50, 50, 50, 50 };

    u8   feedback            = 0;
    bool osc_key_sync        = true;
    u8   lfo_speed           = 35; // not saved to volca
    u8   lfo_delay           = 0;
    u8   lfo_pitch_mod_depth = 0; // not saved to volca
    u8   lfo_amp_mod_depth   = 0;
    bool lfo_key_sync        = true;
    u8   lfo_wave            = 0;
    u8   mod_sense_pitch     = 3;
    u8   transpose           = 24; // 0 = C1 48 = C5

    SynthState()
        : op1(1, 99)
        , op2(2)
        , op3(3)
        , op4(4)
        , op5(5)
        , op6(6)
    {
    }

    // send sysex message, returns yamaha checksum (which volca doesn't use)
    u8 send(RtMidiOut& midi_out);
    // bulk send the entire 32 voice bank, returns yamaha checksum
    static u8 send_bank(RtMidiOut& midi_out, const std::vector<SynthState>& patches);
    // initialize this patch from yamaha sysex message (32 voice format only!)
    void from_sysex(const u8* sysex, int voice_index);
    // write a 32 voice sysex bank, header is written when voice_index = 0
    // checksum is written when voice_index = 31
    // data buffer must be 4104 bytes in size!
    void                      to_sysex_bank(u8* sysex, int voice_index) const;
    [[nodiscard]] std::string to_clipboard_string() const;
    void                      from_clipboard_string(const std::string& str);

    const Operator& get_operator(int idx) const
    {
        switch (idx)
        {
        default:
        case 0: return op1;
        case 1: return op2;
        case 2: return op3;
        case 3: return op4;
        case 4: return op5;
        case 5: return op6;
        }
    }
};
