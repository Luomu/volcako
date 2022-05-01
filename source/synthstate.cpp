#include "synthstate.h"
#include "fileops.h"

#include <sstream>

std::string SynthState::to_clipboard_string() const
{
    const std::string result = Fileops::save_preset_to_memory(nullptr, *this);
    return result;
}

void SynthState::from_clipboard_string(const std::string& str)
{
    inipp::Ini<char> ini;

    std::stringstream stream(str);
    ini.parse(stream);

    Fileops::load_common(ini.sections["patch"], *this);

    Fileops::load_operator(ini.sections["operator-1"], op1);
    Fileops::load_operator(ini.sections["operator-2"], op2);
    Fileops::load_operator(ini.sections["operator-3"], op3);
    Fileops::load_operator(ini.sections["operator-4"], op4);
    Fileops::load_operator(ini.sections["operator-5"], op5);
    Fileops::load_operator(ini.sections["operator-6"], op6);
}
