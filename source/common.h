/*
 * Licensed under MIT. See LICENSE.txt for details.
 */
#pragma once

#include "envelope.h"
#include <rtmidi/RtMidi.h>

// Some common includes in this file

// message size for one voice
const size_t SYSEX_MESSAGE_LENGTH = 163;
// 32 voice bank size
const size_t SYSEX_BANK_SIZE = 4104;

typedef unsigned char u8;

const u8 REALTIME_MESSAGE_START = 0xfa;
const u8 REALTIME_MESSAGE_STOP  = 0xfc;

// for ch0
const u8 MIDI_IN_NOTE_ON   = 0x90;
const u8 MIDI_IN_NOTE_OFF  = 0x80;
const u8 MIDI_OUT_VELOCITY = 0x29;

const ImU32 COLOR_MODULATOR = 0xffff6d00; // blueish
const ImU32 COLOR_CARRIER   = 0xff507f00; // greenish

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
