#pragma once
#include "rtmidi/RtMidi.h"
#include "envelope.h"

//Some common includes in this file

//message size for one voice
const size_t SYSEX_MESSAGE_LENGTH = 163;
//32 voice bank size
const size_t SYSEX_BANK_SIZE = 4104;

typedef unsigned char u8;

const u8 REALTIME_MESSAGE_START = 0xfa;
const u8 REALTIME_MESSAGE_STOP = 0xfc;

//for ch0
const u8 MIDI_IN_NOTE_ON = 0x90;
const u8 MIDI_IN_NOTE_OFF = 0x80;
const u8 MIDI_OUT_VELOCITY = 0x29;

const ImU32 COLOR_MODULATOR = 0xffff6d00; //blueish
const ImU32 COLOR_CARRIER = 0xff507f00; //greenish

const std::string MIDI_PORT_NONE = "None";

static const char* LFO_WAVE_NAMES[6] = {
    "Triangle",
    "Saw down",
    "Saw up",
    "Square",
    "Sine",
    "S&H"
};

static const char* LEVEL_SCALE_CURVE_NAMES[4] = {
    "-LIN",
    "-EXP",
    "EXP",
    "LIN"
};

//Operator/oscillator class
struct Operator
{
    u8 op_number = 0;

    bool is_on = true;
    bool is_carrier = false;

    Envelope envelope = { 99, 99, 99, 99, 99, 99, 99, 0 };

    u8 level_scale_break_point = 39; //c3
    u8 level_scale_left_depth = 0;
    u8 level_scale_right_depth = 0;
    u8 level_scale_left_curve = 0;
    u8 level_scale_right_curve = 0;

    u8 osc_rate_scale = 0;

    u8 amp_mod_sense = 0;
    u8 key_velocity_sense = 0;

    u8 output_level = 0;
    u8 osc_mode = 0;

    u8 freq_coarse = 1;
    u8 freq_fine = 0;

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

//Patch state
struct SynthState
{
    //sent names are not null terminated, but padded with space
    char patch_name[11] = { 'I', 'N', 'I', 'T', '\0', 0, 0, 0, 0, 0, 0 };

    u8 algorithm = 1;

    Operator op1;
    Operator op2;
    Operator op3;
    Operator op4;
    Operator op5;
    Operator op6;

    Envelope pitch_env = { 99,99,99,99, 50,50,50,50 };

    u8 feedback = 0;
    bool osc_key_sync = true;
    u8 lfo_speed = 35; //not saved to volca
    u8 lfo_delay = 0;
    u8 lfo_pitch_mod_depth = 0; //not saved to volca
    u8 lfo_amp_mod_depth = 0;
    bool lfo_key_sync = true;
    u8 lfo_wave = 0;
    u8 mod_sense_pitch = 3;
    u8 transpose = 24; //0 = C1 48 = C5

    SynthState()
        : op1(1, 99)
        , op2(2)
        , op3(3)
        , op4(4)
        , op5(5)
        , op6(6)
    {
    }

    //send sysex message, returns yamaha checksum (which volca doesn't use)
    u8 send(RtMidiOut& midi_out);
    //bulk send the entire 32 voice bank, returns yamaha checksum
    static u8 send_bank(RtMidiOut& midi_out, const std::vector<SynthState>& patches);
    //initialize this patch from yamaha sysex message (32 voice format only!)
    void from_sysex(const u8* sysex, int voice_index);
    //write a 32 voice sysex bank, header is written when voice_index = 0
    //checksum is written when voice_index = 31
    //data buffer must be 4104 bytes in size!
    void to_sysex_bank(u8* sysex, int voice_index) const;

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
