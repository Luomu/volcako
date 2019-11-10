#include "sysex.h"
#include "common.h"

//Sysex conversion functions

const size_t SYSEX_CHECKSUM_START = 6;

//In a couple of places data is encoded in '14 bit' value,
//using two u8's, 7 bits each
void to_14bit(unsigned short in, u8& msb, u8& lsb)
{
    lsb = in & 0x007f;
    msb = (in >> 7) & 0x007f;
}

unsigned short from_14bit(u8 msb, u8 lsb)
{
    return (msb << 7) + lsb;
}

//http://www.2writers.com/eddie/TutSysEx.htm
u8 calculate_checksum(const u8* data, int length)
{
    u8 sum = 0;
    for (int i = 0; i < length; i++)
    {
        sum += data[i];
        if (sum >= 128)
            sum = sum - 128;
    }
    return 128 - sum;
}

//Send one patch to midi out
u8 SynthState::send(RtMidiOut &midi_out)
{
    static unsigned char m[SYSEX_MESSAGE_LENGTH] = {};

    //common
    m[SX_STATUS] = 0xf0;
    m[SX_YAMAHA_ID] = 0x43;
    m[SX_MIDI_CHANNEL] = 0x00;
    m[SX_FORMAT_NUMBER] = 0x00;
    m[SX_BYTE_COUNT_MSB] = 0x01;
    m[SX_BYTE_COUNT_LSB] = 0x1b;

    //size is 155, it does not include header, sx_operator_on or sx_end

    op1.to_message(m);
    op2.to_message(m);
    op3.to_message(m);
    op4.to_message(m);
    op5.to_message(m);
    op6.to_message(m);

    m[SX_PITCH_EG_R1] = pitch_env.r1;
    m[SX_PITCH_EG_R2] = pitch_env.r2;
    m[SX_PITCH_EG_R3] = pitch_env.r3;
    m[SX_PITCH_EG_R4] = pitch_env.r4;

    m[SX_PITCH_EG_L1] = pitch_env.l1;
    m[SX_PITCH_EG_L2] = pitch_env.l2;
    m[SX_PITCH_EG_L3] = pitch_env.l3;
    m[SX_PITCH_EG_L4] = pitch_env.l4;

    m[SX_ALGORITHM] = algorithm - 1;
    m[SX_FEEDBACK] = feedback;
    m[SX_OSC_SYNC] = osc_key_sync ? 1 : 0;
    m[SX_LFO_SPEED] = lfo_speed;
    m[SX_LFO_DELAY] = lfo_delay;
    m[SX_LFO_PMD] = lfo_pitch_mod_depth;
    m[SX_LFO_AMD] = lfo_amp_mod_depth;
    m[SX_LFO_SYNC] = lfo_key_sync ? 1 : 0;
    m[SX_LFO_WAVE] = lfo_wave;
    m[SX_MOD_SENS_PITCH] = mod_sense_pitch;
    m[SX_TRANSPOSE] = transpose;

    char spaced_name[10] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };
    memcpy(spaced_name, patch_name, strlen(patch_name));

    m[SX_NAME1] = spaced_name[0];
    m[SX_NAME2] = spaced_name[1];
    m[SX_NAME3] = spaced_name[2];
    m[SX_NAME4] = spaced_name[3];
    m[SX_NAME5] = spaced_name[4];
    m[SX_NAME6] = spaced_name[5];
    m[SX_NAME7] = spaced_name[6];
    m[SX_NAME8] = spaced_name[7];
    m[SX_NAME9] = spaced_name[8];
    m[SX_NAME10] = spaced_name[9];

    //all on = 0x3f;
    const unsigned char ops_on =
        (op1.is_on ? (1 << 0) : 0)
        | (op2.is_on ? (1 << 1) : 0)
        | (op3.is_on ? (1 << 2) : 0)
        | (op4.is_on ? (1 << 3) : 0)
        | (op5.is_on ? (1 << 4) : 0)
        | (op6.is_on ? (1 << 5) : 0);

    m[SX_OPERATOR_ON] = ops_on; //checksum on dx7

    //data
    m[SX_END] = 0xf7;

    u8 checksum = calculate_checksum(&m[SYSEX_CHECKSUM_START], from_14bit(m[SX_BYTE_COUNT_MSB], m[SX_BYTE_COUNT_LSB]));

    midi_out.sendMessage(m, SYSEX_MESSAGE_LENGTH);

    return checksum;
}

//Send 32 voices
u8 SynthState::send_bank(RtMidiOut& midi_out, const std::vector<SynthState>& patches)
{
    std::vector<u8> m(SYSEX_BANK_SIZE, 0);

    assert(patches.size() == 32);
    for (unsigned int i = 0; i < patches.size(); i++)
    {
        patches[i].to_sysex_bank(&m[0], i);
    }
    midi_out.sendMessage(&m[0], m.size());

    u8 checksum = 0;
    return checksum;
}

void Operator::to_message(unsigned char* msg) const
{
    //op6 is first
    //21 params per operator
    size_t offset = (6 - op_number) * 21;

#define SET(param, value) msg[param + offset] = value;
    SET(SX_OP6_EGR1, envelope.r1);
    SET(SX_OP6_EGR2, envelope.r2);
    SET(SX_OP6_EGR3, envelope.r3);
    SET(SX_OP6_EGR4, envelope.r4);
    SET(SX_OP6_EGL1, envelope.l1);
    SET(SX_OP6_EGL2, envelope.l2);
    SET(SX_OP6_EGL3, envelope.l3);
    SET(SX_OP6_EGL4, envelope.l4);

    SET(SX_OP6_KBD_BREAKPOINT, level_scale_break_point);
    SET(SX_OP6_KBD_SCALE_LEFT_DEPTH, level_scale_left_depth);
    SET(SX_OP6_KBD_SCALE_RIGHT_DEPTH, level_scale_right_depth);
    SET(SX_OP6_KBD_SCALE_LEFT_CURVE, level_scale_left_curve);
    SET(SX_OP6_KBD_SCALE_RIGHT_CURVE, level_scale_right_curve);
    SET(SX_OP6_KBD_RATE_SCALING, osc_rate_scale);

    SET(SX_OP6_MOD_SENS_AMPLITUDE, amp_mod_sense);
    SET(SX_OP6_VELOCITY_SENSITIVITY, key_velocity_sense);

    SET(SX_OP6_OUTPUT_LEVEL, output_level);
    SET(SX_OP6_OSC_MODE, osc_mode);

    SET(SX_OP6_FREQ_COARSE, freq_coarse);
    SET(SX_OP6_FREQ_FINE, freq_fine);

    SET(SX_OP6_DETUNE, detune);
#undef SET
}

void Operator::to_message_32(u8* msg, size_t& offset) const
{
    //Write 17 bytes for each Operator
    const size_t offs_start = offset;

    msg[offset++] = envelope.r1;
    msg[offset++] = envelope.r2;
    msg[offset++] = envelope.r3;
    msg[offset++] = envelope.r4;

    msg[offset++] = envelope.l1;
    msg[offset++] = envelope.l2;
    msg[offset++] = envelope.l3;
    msg[offset++] = envelope.l4;

    msg[offset++] = level_scale_break_point;
    msg[offset++] = level_scale_left_depth;
    msg[offset++] = level_scale_right_depth;

    //These are packed in one byte, 2 bits each
    const u8 curves = level_scale_left_curve | (level_scale_right_curve << 2);
    msg[offset++] = curves;

    //3 bits and 4 bits
    const u8 ratescale_detune = osc_rate_scale | (detune << 3);
    msg[offset++] = ratescale_detune;

    //2 and 3
    const u8 kvs_msa = amp_mod_sense | (key_velocity_sense << 2);
    msg[offset++] = kvs_msa;

    msg[offset++] = output_level;

    //1 and 5
    const u8 freqc_oscmode = osc_mode | (freq_coarse << 1);
    msg[offset++] = freqc_oscmode;

    msg[offset++] = freq_fine;

    assert(offset - offs_start == 17);
}

void Operator::from_message(const u8* msg, size_t& offset)
{
    //not read
    is_on = true;

    //Read 17 bytes for each Operator
    const size_t offs_start = offset;

    envelope.r1 = msg[offset++];
    envelope.r2 = msg[offset++];
    envelope.r3 = msg[offset++];
    envelope.r4 = msg[offset++];

    envelope.l1 = msg[offset++];
    envelope.l2 = msg[offset++];
    envelope.l3 = msg[offset++];
    envelope.l4 = msg[offset++];

    level_scale_break_point = msg[offset++];
    level_scale_left_depth = msg[offset++];
    level_scale_right_depth = msg[offset++];

    //These are packed in one byte, 2 bits each
    const u8 curves = msg[offset++];
    level_scale_left_curve = curves & 0x3;
    level_scale_right_curve = (curves >> 2) & 0x3;

    //3 bits and 4 bits
    const u8 ratescale_detune = msg[offset++];
    osc_rate_scale = ratescale_detune & 0x7;
    detune = (ratescale_detune >> 3) & 0xf;

    //2 and 3
    const u8 kvs_msa = msg[offset++];
    amp_mod_sense = kvs_msa & 0x3;
    key_velocity_sense = (kvs_msa >> 2) & 0x7;

    output_level = msg[offset++];

    //1 and 5
    const u8 freqc_oscmode = msg[offset++];
    osc_mode = freqc_oscmode & 0x1;
    freq_coarse = (freqc_oscmode >> 1) & 0x1f;

    freq_fine = msg[offset++];

    assert(offset - offs_start == 17);
}

//Read one patch from 32 voice sysex message
void SynthState::from_sysex(const u8* m, int voice_index)
{
    const u8 status = m[SX_STATUS];
    const u8 ymh_id = m[SX_YAMAHA_ID];
    const u8 channel = m[SX_MIDI_CHANNEL];
    const u8 fmt_number = m[SX_FORMAT_NUMBER];
    const u8 len_msb = m[SX_BYTE_COUNT_MSB];
    const u8 len_lsb = m[SX_BYTE_COUNT_LSB];
    //u8 checksum = m[SX_OPERATOR_ON];

    const unsigned short data_length = from_14bit(len_msb, len_lsb);

    //32voice format only!
    assert(data_length == 4096);
    if (data_length != 4096)
    {
        std::cout << "Data length error" << std::endl;
        return;
    }

    size_t offset = SX_OP6_EGR1 + voice_index * 128;
    const size_t offset_start = offset;
    op6.from_message(m, offset);
    op5.from_message(m, offset);
    op4.from_message(m, offset);
    op3.from_message(m, offset);
    op2.from_message(m, offset);
    op1.from_message(m, offset);

    assert(offset - offset_start == 102);

    pitch_env.r1 = m[offset++];
    pitch_env.r2 = m[offset++];
    pitch_env.r3 = m[offset++];
    pitch_env.r4 = m[offset++];

    pitch_env.l1 = m[offset++];
    pitch_env.l2 = m[offset++];
    pitch_env.l3 = m[offset++];
    pitch_env.l4 = m[offset++];

    algorithm = m[offset++] + 1;

    //3 bits and 1 bit
    const u8 feedback_keysync = m[offset++];
    feedback = feedback_keysync & 0x7;
    osc_key_sync = (feedback_keysync >> 3) & 0x1;

    lfo_speed = m[offset++]; //not sent to volca
    lfo_delay = m[offset++];
    lfo_pitch_mod_depth = m[offset++]; //not sent to volca
    lfo_amp_mod_depth = m[offset++];

    //1 bit, 4 bits, 2 bits
    const u8 msp_lfowave_lfosync = m[offset++];
    lfo_key_sync = msp_lfowave_lfosync & 0x1;
    lfo_wave = (msp_lfowave_lfosync >> 1) & 0xf;
    mod_sense_pitch = (msp_lfowave_lfosync >> 4) && 0x3;

    transpose = m[offset++];

    assert(offset - offset_start == 118);

    char spaced_name[10] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };

    //offset = SX_OP6_EGR1 + 118;
    spaced_name[0] = m[offset++];
    spaced_name[1] = m[offset++];
    spaced_name[2] = m[offset++];
    spaced_name[3] = m[offset++];
    spaced_name[4] = m[offset++];
    spaced_name[5] = m[offset++];
    spaced_name[6] = m[offset++];
    spaced_name[7] = m[offset++];
    spaced_name[8] = m[offset++];
    spaced_name[9] = m[offset++];

    memcpy(patch_name, spaced_name, 10);
    patch_name[10] = '\0';

    assert(offset - offset_start == 128);

    //u8 calculated_checksum = calculate_checksum(&m[SYSEX_CHECKSUM_START], data_length);
    //printf("%d %d\n", checksum, calculated_checksum);
    //std::cout << data_length << std::endl;
}

//Convert this patch to it's offset in bulk send message
void SynthState::to_sysex_bank(u8* m, int voice_index) const
{
    if (voice_index == 0)
    {
        m[SX_STATUS] = 0xf0; //sysex start
        m[SX_YAMAHA_ID] = 0x43;
        m[SX_MIDI_CHANNEL] = 0x00;
        m[SX_FORMAT_NUMBER] = 0x09; //32voices
        m[SX_BYTE_COUNT_MSB] = 0x20;
        m[SX_BYTE_COUNT_LSB] = 0x00;

        //m[4102] = checksum
        m[4103] = 0xf7; //sysex end
    }

    size_t offset = SX_OP6_EGR1 + voice_index * 128;
    const size_t offset_start = offset;

    op6.to_message_32(m, offset);
    op5.to_message_32(m, offset);
    op4.to_message_32(m, offset);
    op3.to_message_32(m, offset);
    op2.to_message_32(m, offset);
    op1.to_message_32(m, offset);

    assert(offset - offset_start == 102);

    m[offset++] = pitch_env.r1;
    m[offset++] = pitch_env.r2;
    m[offset++] = pitch_env.r3;
    m[offset++] = pitch_env.r4;

    m[offset++] = pitch_env.l1;
    m[offset++] = pitch_env.l2;
    m[offset++] = pitch_env.l3;
    m[offset++] = pitch_env.l4;

    m[offset++] = algorithm - 1;

    //3 bits and 1 bit
    const u8 feedback_keysync = feedback | (osc_key_sync << 3);
    m[offset++] = feedback_keysync;

    m[offset++] = lfo_speed; //not sent to volca
    m[offset++] = lfo_delay;
    m[offset++] = lfo_pitch_mod_depth; //not sent to volca
    m[offset++] = lfo_amp_mod_depth;

    //1 bit, 4 bits, 2 bits
    const u8 msp_lfowave_lfosync = (lfo_key_sync ? 1 : 0) | (lfo_wave << 1) | (mod_sense_pitch << 4);
    m[offset++] = msp_lfowave_lfosync;

    m[offset++] = transpose;

    assert(offset - offset_start == 118);

    //offset = SX_OP6_EGR1 + 118;
    char spaced_name[10] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };
    size_t namelen = strlen(patch_name);
    memcpy(spaced_name, patch_name, namelen <= 10 ? namelen : 10);

    m[offset++] = spaced_name[0];
    m[offset++] = spaced_name[1];
    m[offset++] = spaced_name[2];
    m[offset++] = spaced_name[3];
    m[offset++] = spaced_name[4];
    m[offset++] = spaced_name[5];
    m[offset++] = spaced_name[6];
    m[offset++] = spaced_name[7];
    m[offset++] = spaced_name[8];
    m[offset++] = spaced_name[9];

    assert(offset - offset_start == 128);

    if (voice_index == 31)
    {
        const u8 calculated_checksum = calculate_checksum(&m[SX_OP6_EGR1], 4096);
        m[4102] = calculated_checksum;
    }
}
