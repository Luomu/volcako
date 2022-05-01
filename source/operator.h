#pragma once

#include "envelope.h"

// Operator/oscillator class
struct Operator
{
    u8 op_number = 0;

    bool is_on      = true;
    bool is_carrier = false;

    Envelope envelope = { 99, 99, 99, 99, 99, 99, 99, 0 };

    u8 level_scale_break_point = 39; // c3
    u8 level_scale_left_depth  = 0;
    u8 level_scale_right_depth = 0;
    u8 level_scale_left_curve  = 0;
    u8 level_scale_right_curve = 0;

    u8 osc_rate_scale = 0;

    u8 amp_mod_sense      = 0;
    u8 key_velocity_sense = 0;

    u8 output_level = 0;
    u8 osc_mode     = 0;

    u8 freq_coarse = 1;
    u8 freq_fine   = 0;

    u8 detune = 7;

    Operator(u8 index, u8 output_level = 0)
        : op_number(index)
        , output_level(output_level)
    {
    }

    void to_message(unsigned char* msg) const;
    void to_message_32(u8* msg, size_t& offset) const;
    void from_message(const u8* msg, size_t& offset);
};
