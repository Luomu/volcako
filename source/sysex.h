#pragma once

typedef unsigned char u8;

//Sysex offset names for one voice message
//(32 voice sysex packs multiple values in one field)
enum SysexFields
{
    SX_STATUS,
    SX_YAMAHA_ID,
    SX_MIDI_CHANNEL,
    SX_FORMAT_NUMBER,
    SX_BYTE_COUNT_MSB,
    SX_BYTE_COUNT_LSB,

    SX_OP6_EGR1,
    SX_OP6_EGR2,
    SX_OP6_EGR3,
    SX_OP6_EGR4,

    SX_OP6_EGL1,
    SX_OP6_EGL2,
    SX_OP6_EGL3,
    SX_OP6_EGL4,

    SX_OP6_KBD_BREAKPOINT,
    SX_OP6_KBD_SCALE_LEFT_DEPTH,
    SX_OP6_KBD_SCALE_RIGHT_DEPTH,
    SX_OP6_KBD_SCALE_LEFT_CURVE,
    SX_OP6_KBD_SCALE_RIGHT_CURVE,
    SX_OP6_KBD_RATE_SCALING,

    SX_OP6_MOD_SENS_AMPLITUDE,
    SX_OP6_VELOCITY_SENSITIVITY,
    SX_OP6_OUTPUT_LEVEL,
    SX_OP6_OSC_MODE,
    SX_OP6_FREQ_COARSE,
    SX_OP6_FREQ_FINE,
    SX_OP6_DETUNE,

    SX_OP5_EGR1,
    SX_OP5_EGR2,
    SX_OP5_EGR3,
    SX_OP5_EGR4,

    SX_OP5_EGL1,
    SX_OP5_EGL2,
    SX_OP5_EGL3,
    SX_OP5_EGL4,

    SX_OP5_KBD_BREAKPOINT,
    SX_OP5_KBD_SCALE_LEFT_DEPTH,
    SX_OP5_KBD_SCALE_RIGHT_DEPTH,
    SX_OP5_KBD_SCALE_LEFT_CURVE,
    SX_OP5_KBD_SCALE_RIGHT_CURVE,
    SX_OP5_KBD_RATE_SCALING,

    SX_OP5_MOD_SENS_AMPLITUDE,
    SX_OP5_VELOCITY_SENSITIVITY,
    SX_OP5_OUTPUT_LEVEL,
    SX_OP5_OSC_MODE,
    SX_OP5_FREQ_COARSE,
    SX_OP5_FREQ_FINE,
    SX_OP5_DETUNE,

    SX_OP4_EGR1,
    SX_OP4_EGR2,
    SX_OP4_EGR3,
    SX_OP4_EGR4,

    SX_OP4_EGL1,
    SX_OP4_EGL2,
    SX_OP4_EGL3,
    SX_OP4_EGL4,

    SX_OP4_KBD_BREAKPOINT,
    SX_OP4_KBD_SCALE_LEFT_DEPTH,
    SX_OP4_KBD_SCALE_RIGHT_DEPTH,
    SX_OP4_KBD_SCALE_LEFT_CURVE,
    SX_OP4_KBD_SCALE_RIGHT_CURVE,
    SX_OP4_KBD_RATE_SCALING,

    SX_OP4_MOD_SENS_AMPLITUDE,
    SX_OP4_VELOCITY_SENSITIVITY,
    SX_OP4_OUTPUT_LEVEL,
    SX_OP4_OSC_MODE,
    SX_OP4_FREQ_COARSE,
    SX_OP4_FREQ_FINE,
    SX_OP4_DETUNE,

    SX_OP3_EGR1,
    SX_OP3_EGR2,
    SX_OP3_EGR3,
    SX_OP3_EGR4,

    SX_OP3_EGL1,
    SX_OP3_EGL2,
    SX_OP3_EGL3,
    SX_OP3_EGL4,

    SX_OP3_KBD_BREAKPOINT,
    SX_OP3_KBD_SCALE_LEFT_DEPTH,
    SX_OP3_KBD_SCALE_RIGHT_DEPTH,
    SX_OP3_KBD_SCALE_LEFT_CURVE,
    SX_OP3_KBD_SCALE_RIGHT_CURVE,
    SX_OP3_KBD_RATE_SCALING,

    SX_OP3_MOD_SENS_AMPLITUDE,
    SX_OP3_VELOCITY_SENSITIVITY,
    SX_OP3_OUTPUT_LEVEL,
    SX_OP3_OSC_MODE,
    SX_OP3_FREQ_COARSE,
    SX_OP3_FREQ_FINE,
    SX_OP3_DETUNE,

    SX_OP2_EGR1,
    SX_OP2_EGR2,
    SX_OP2_EGR3,
    SX_OP2_EGR4,

    SX_OP2_EGL1,
    SX_OP2_EGL2,
    SX_OP2_EGL3,
    SX_OP2_EGL4,

    SX_OP2_KBD_BREAKPOINT,
    SX_OP2_KBD_SCALE_LEFT_DEPTH,
    SX_OP2_KBD_SCALE_RIGHT_DEPTH,
    SX_OP2_KBD_SCALE_LEFT_CURVE,
    SX_OP2_KBD_SCALE_RIGHT_CURVE,
    SX_OP2_KBD_RATE_SCALING,

    SX_OP2_MOD_SENS_AMPLITUDE,
    SX_OP2_VELOCITY_SENSITIVITY,
    SX_OP2_OUTPUT_LEVEL,
    SX_OP2_OSC_MODE,
    SX_OP2_FREQ_COARSE,
    SX_OP2_FREQ_FINE,
    SX_OP2_DETUNE,

    SX_OP1_EGR1,
    SX_OP1_EGR2,
    SX_OP1_EGR3,
    SX_OP1_EGR4,

    SX_OP1_EGL1,
    SX_OP1_EGL2,
    SX_OP1_EGL3,
    SX_OP1_EGL4,

    SX_OP1_KBD_BREAKPOINT,
    SX_OP1_KBD_SCALE_LEFT_DEPTH,
    SX_OP1_KBD_SCALE_RIGHT_DEPTH,
    SX_OP1_KBD_SCALE_LEFT_CURVE,
    SX_OP1_KBD_SCALE_RIGHT_CURVE,
    SX_OP1_KBD_RATE_SCALING,

    SX_OP1_MOD_SENS_AMPLITUDE,
    SX_OP1_VELOCITY_SENSITIVITY,
    SX_OP1_OUTPUT_LEVEL,
    SX_OP1_OSC_MODE,
    SX_OP1_FREQ_COARSE,
    SX_OP1_FREQ_FINE,
    SX_OP1_DETUNE,

    SX_PITCH_EG_R1,
    SX_PITCH_EG_R2,
    SX_PITCH_EG_R3,
    SX_PITCH_EG_R4,
    SX_PITCH_EG_L1,
    SX_PITCH_EG_L2,
    SX_PITCH_EG_L3,
    SX_PITCH_EG_L4,

    SX_ALGORITHM,
    SX_FEEDBACK,
    SX_OSC_SYNC,
    SX_LFO_SPEED,
    SX_LFO_DELAY,
    SX_LFO_PMD,
    SX_LFO_AMD,
    SX_LFO_SYNC,
    SX_LFO_WAVE,
    SX_MOD_SENS_PITCH,
    SX_TRANSPOSE,
    SX_NAME1,
    SX_NAME2,
    SX_NAME3,
    SX_NAME4,
    SX_NAME5,
    SX_NAME6,
    SX_NAME7,
    SX_NAME8,
    SX_NAME9,
    SX_NAME10,
    SX_OPERATOR_ON, //b5-b0 operator 1-6 on/off bit

    SX_END
};
