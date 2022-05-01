/*
 * Licensed under MIT. See LICENSE.txt for details.
 */
#pragma once

#include "common.h"
#include <imgui_internal.h>
#include <inipp/inipp.h>

struct Operator;
struct SynthState;

// Saving and loading patch data to/from .txt and .syx
namespace Fileops
{
    void                      add_line(ImGuiTextBuffer& buf, const char* name, int value);
    void                      write_operator(ImGuiTextBuffer& buf, const Operator& op);
    void                      write_preset(ImGuiTextBuffer& buf, const SynthState& synth);
    [[nodiscard]] const char* save_preset_to_memory(size_t* out_size, const SynthState& synth);
    void                      save_to_disk(const char* ini_filename, SynthState& synth);
    void                      extract_u8(inipp::Ini<char>::Section& sec, const char* name, u8& output);
    void                      extract_bool(inipp::Ini<char>::Section& sec, const char* name, bool& output);
    void                      extract_env(inipp::Ini<char>::Section& sec, const char* name, u8& x, u8& y, u8& z, u8& w);
    void                      load_common(inipp::Ini<char>::Section& sec, SynthState& synth);
    void                      load_operator(inipp::Ini<char>::Section& sec, Operator& op);
    void                      load_from_disk(const char* filename, SynthState& synth);
    [[nodiscard]] std::string load_from_syx_file(const char* filename, std::vector<SynthState>& patches);
    [[nodiscard]] std::string save_to_syx_file(const std::string& filename, const std::vector<SynthState>& patches);
    [[nodiscard]] bool        file_exists(char const* const filename);
}
