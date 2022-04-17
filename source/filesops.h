#pragma once
#include "common.h"
#include "imgui_internal.h"
#include "inipp/inipp.h"
#include "sysex.h"
#include <fstream>

#pragma warning(push)
#ifdef _MSC_VER
#pragma warning(disable : 4996) // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif

// Saving and loading patch data to/from .txt and .syx
namespace Fileops
{
    void add_line(ImGuiTextBuffer& buf, const char* name, int value)
    {
        buf.appendf("%s=%d\n", name, value);
    }

    void write_operator(ImGuiTextBuffer& buf, const Operator& op)
    {
#define SET(field) add_line(buf, #field, op.field);
        buf.appendf("[operator-%d]\n", op.op_number);
        add_line(buf, "enabled", op.is_on ? 1 : 0);
        buf.appendf("eg_rate=%d,%d,%d,%d\n", op.envelope.r1, op.envelope.r2, op.envelope.r3, op.envelope.r4);
        buf.appendf("eg_level=%d,%d,%d,%d\n", op.envelope.l1, op.envelope.l2, op.envelope.l3, op.envelope.l4);
        SET(level_scale_break_point);
        SET(level_scale_left_depth);
        SET(level_scale_right_depth);
        SET(level_scale_left_curve);
        SET(level_scale_right_curve);
        SET(osc_rate_scale);
        SET(amp_mod_sense);
        SET(key_velocity_sense);
        SET(output_level);
        SET(osc_mode);
        SET(freq_coarse);
        SET(freq_fine);
        SET(detune);

        buf.append("\n");
#undef SET
    }

    void write_preset(ImGuiTextBuffer& buf, SynthState& synth)
    {
        buf.append("[patch]\n");
        buf.appendf("name=%s\n", synth.patch_name);
        add_line(buf, "algorithm", synth.algorithm);
        buf.appendf("pitch_rate=%d,%d,%d,%d\n", synth.pitch_env.r1, synth.pitch_env.r2, synth.pitch_env.r3, synth.pitch_env.r4);
        buf.appendf("pitch_level=%d,%d,%d,%d\n", synth.pitch_env.l1, synth.pitch_env.l2, synth.pitch_env.l3, synth.pitch_env.l4);
        add_line(buf, "feedback", synth.feedback);
        add_line(buf, "osc_key_sync", synth.osc_key_sync ? 1 : 0);
        add_line(buf, "lfo_delay", synth.lfo_delay);
        add_line(buf, "lfo_amp_mod_depth", synth.lfo_amp_mod_depth);
        add_line(buf, "lfo_key_sync", synth.lfo_key_sync ? 1 : 0);
        add_line(buf, "lfo_wave", synth.lfo_wave);
        add_line(buf, "mod_sense_pitch", synth.mod_sense_pitch);
        add_line(buf, "transpose", synth.transpose);

        buf.append("\n");

        write_operator(buf, synth.op1);
        write_operator(buf, synth.op2);
        write_operator(buf, synth.op3);
        write_operator(buf, synth.op4);
        write_operator(buf, synth.op5);
        write_operator(buf, synth.op6);
    }

    const char* save_preset_to_memory(size_t* out_size, SynthState& synth)
    {
        static ImGuiTextBuffer buf;
        buf.Buf.resize(0);
        buf.Buf.push_back(0);

        write_preset(buf, synth);

        if (out_size)
            *out_size = (size_t)buf.size();
        return buf.c_str();
    }

    void save_to_disk(const char* ini_filename, SynthState& synth)
    {
        if (!ini_filename)
            return;

        size_t      ini_data_size = 0;
        const char* ini_data      = save_preset_to_memory(&ini_data_size, synth);
        FILE*       f             = ImFileOpen(ini_filename, "wt");
        if (!f)
            return;
        fwrite(ini_data, sizeof(char), ini_data_size, f);
        fclose(f);
    }

    // number gets converted wrong if read as u8 straight away
    void extract_u8(inipp::Ini<char>::Section& sec, const char* name, u8& output)
    {
        int temp = -1;
        inipp::extract(sec[name], temp);
        if (temp < 0 || temp > 99)
        {
            std::cerr << "unexpected value: " << name << " " << temp << std::endl;
            temp = 0;
        }
        output = static_cast<u8>(temp);
    }

    void extract_bool(inipp::Ini<char>::Section& sec, const char* name, bool& output)
    {
        int temp = -1;
        inipp::extract(sec[name], temp);
        if (temp < 0 || temp > 1)
        {
            std::cerr << "unexpected value: " << name << " " << temp << std::endl;
            temp = 0;
        }
        output = temp > 0 ? true : false;
    }

    void extract_env(inipp::Ini<char>::Section& sec, const char* name, u8& x, u8& y, u8& z, u8& w)
    {
        std::string temp;
        inipp::extract(sec[name], temp);

        // VS scanf doesn't seem to support %hhu properly (C6328), so read to temp ints
        unsigned int a, b, c, d = 0;
        if (sscanf(temp.c_str(), "%u,%u,%u,%u", &a, &b, &c, &d) != 4)
        {
            std::cerr << "unexpected value: " << name << " " << temp << std::endl;
            x = a;
            y = b;
            z = c;
            w = d;
        }
    }

    void load_common(inipp::Ini<char>::Section& sec, SynthState& synth)
    {
#define GET_U8(key) extract_u8(sec, #key, synth.key);
        std::string patch_name;
        inipp::extract(sec["name"], patch_name);
        snprintf(synth.patch_name, 11, "%s", patch_name.c_str());
        extract_env(sec, "pitch_rate", synth.pitch_env.r1, synth.pitch_env.r2, synth.pitch_env.r3, synth.pitch_env.r4);
        extract_env(sec, "pitch_level", synth.pitch_env.l1, synth.pitch_env.l2, synth.pitch_env.l3, synth.pitch_env.l4);
        GET_U8(algorithm);
        GET_U8(feedback);
        extract_bool(sec, "osc_key_sync", synth.osc_key_sync);
        GET_U8(lfo_delay);
        GET_U8(lfo_amp_mod_depth);
        extract_bool(sec, "lfo_key_sync", synth.lfo_key_sync);
        GET_U8(lfo_wave);
        GET_U8(mod_sense_pitch);
        GET_U8(transpose);
#undef GET_U8
    }

    void load_operator(inipp::Ini<char>::Section& sec, Operator& op)
    {
#define GET_U8(key) extract_u8(sec, #key, op.key);
        extract_bool(sec, "enabled", op.is_on);
        extract_env(sec, "eg_rate", op.envelope.r1, op.envelope.r2, op.envelope.r3, op.envelope.r4);
        extract_env(sec, "eg_level", op.envelope.l1, op.envelope.l2, op.envelope.l3, op.envelope.l4);
        GET_U8(level_scale_break_point);
        GET_U8(level_scale_left_depth);
        GET_U8(level_scale_right_depth);
        GET_U8(level_scale_left_curve);
        GET_U8(level_scale_right_curve);
        GET_U8(osc_rate_scale);
        GET_U8(amp_mod_sense);
        GET_U8(key_velocity_sense);
        GET_U8(output_level);
        GET_U8(osc_mode);
        GET_U8(freq_coarse);
        GET_U8(freq_fine);
        GET_U8(detune);
#undef GET_U8
    }

    void load_from_disk(const char* filename, SynthState& synth)
    {
        inipp::Ini<char> ini;
        std::ifstream    is(filename);
        if (is.fail())
        {
            return;
        }
        ini.parse(is);

        load_common(ini.sections["patch"], synth);

        load_operator(ini.sections["operator-1"], synth.op1);
        load_operator(ini.sections["operator-2"], synth.op2);
        load_operator(ini.sections["operator-3"], synth.op3);
        load_operator(ini.sections["operator-4"], synth.op4);
        load_operator(ini.sections["operator-5"], synth.op5);
        load_operator(ini.sections["operator-6"], synth.op6);
    }

    std::string load_from_syx_file(const char* filename, std::vector<SynthState>& patches)
    {
        std::ifstream is(filename, std::ios::binary);
        if (is.fail())
        {
            return "Failed to read file";
        }
        is.seekg(0, std::ios::end);
        const size_t file_size_in_byte = static_cast<size_t>(is.tellg());

        if (file_size_in_byte != SYSEX_BANK_SIZE)
        {
            return "Unsupported file size, expected 4104 bytes";
        }

        std::vector<char> data; // used to store text data
        data.resize(file_size_in_byte);
        is.seekg(0, std::ios::beg);
        is.read(&data[0], file_size_in_byte);

        patches.resize(32);
        for (int i = 0; i < 32; i++)
        {
            patches[i].from_sysex(reinterpret_cast<u8*>(&data[0]), i);
        }

        return "";
    }

    std::string save_to_syx_file(const std::string& filename, const std::vector<SynthState>& patches)
    {
        std::vector<u8> buffer(SYSEX_BANK_SIZE, 0);

        std::ofstream os(filename, std::ios::out | std::ios::binary);
        if (os.fail())
        {
            return "Could not open save file";
        }

        assert(patches.size() == 32);
        if (patches.size() != 32)
        {
            return "Can only save 32 voice bank";
        }

        for (unsigned int i = 0; i < patches.size(); i++)
        {
            patches[i].to_sysex_bank(&buffer[0], i);
        }

        os.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());

        return "";
    }

    static bool file_exists(char const* const filename)
    {
        FILE* lIn;
        if (!filename || !strlen(filename))
        {
            return false;
        }
        lIn = fopen(filename, "r");
        if (!lIn)
        {
            return false;
        }
        fclose(lIn);
        return true;
    }
}

#pragma warning(pop)
