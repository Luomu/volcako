#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "algorithm.h"
#include "appstate.h"
#include "common.h"
#include "filesops.h"
#include "settings.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <rtmidi/RtMidi.h>
#include <stdio.h>
#include <tinyfiledialogs/tinyfiledialogs.h>

#pragma warning(disable : 4996) // crt_secure_no_warnings

#ifndef IMGUI_IMPL_OPENGL_LOADER_GL3W
#define IMGUI_IMPL_OPENGL_LOADER_GL3W
#endif
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

// The included glfw3.lib is from vs2010 era to maximize compatibility, and requires this pragma for newer compilers
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

const float  ALGO_BOX_SIZE = 20.f;
const float  FONT_SIZE     = 16.0f;
const ImVec4 CLEAR_COLOR   = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

#if __APPLE__
// GL 3.2 + GLSL 150
const char* glsl_version = "#version 150";
#else
// GL 3.0 + GLSL 130
const char* glsl_version = "#version 130";
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

struct MidiError
{
    RtMidiError::Type type;
    std::string       errorText;
};

std::vector<MidiError> midi_errors;

void midi_error_callback(RtMidiError::Type type, const std::string& errorText, void*)
{
    if (midi_errors.size() < 1000)
        midi_errors.push_back({ type, errorText });

    std::cout << "MidiError: " << type << " " << errorText << std::endl;
}

void midi_in_callback(double /*delta_time*/, std::vector<unsigned char>* message, void* user_data)
{
    /*unsigned int nBytes = message->size();
    for (unsigned int i = 0; i < nBytes; i++)
    {
        std::cout << "Byte " << i << " = " << (int)message->at(i) << std::endl;
    }*/

    auto app_state = static_cast<AppState*>(user_data);

    unsigned char msg_type = message->at(0);

    // Send the velocity change message when receiving note data
    if (app_state->send_velocity && msg_type == MIDI_IN_NOTE_ON)
    {
        const u8 vel_msg[3] = { 0xb0, MIDI_OUT_VELOCITY, message->at(2) };
        app_state->midi_out.sendMessage(vel_msg, 3);
    }

    app_state->midi_out.sendMessage(message);
}

// Open the midi port matching name
void open_midi_port(const std::string& port_name, RtMidi& midi_interface)
{
    if (midi_interface.isPortOpen())
        midi_interface.closePort();

    if (port_name.empty() || port_name == MIDI_PORT_NONE)
        return;

    int midi_port_count = midi_interface.getPortCount();

    for (int i = 0; i < midi_port_count; i++)
    {
        if (midi_interface.getPortName(i) == port_name)
        {
            midi_interface.openPort(i);
            break;
        }
    }
}

void copy_to_clipboard(const AppState& app, const std::string& text)
{
    glfwSetClipboardString(app.window, text.c_str());
}

std::string get_from_clipboard(const AppState& app)
{
    return glfwGetClipboardString(app.window);
}

// imgui::dragint wrapper for u8 type
bool drag_u8(const char* label, u8& value, int min = 0, int max = 99)
{
    int tempval = value;
    if (ImGui::DragInt(label, &tempval, 0.3f, min, max))
    {
        value = static_cast<u8>(tempval);
        return true;
    }
    return false;
}

// Update the operators' carrier status when algorithm changes
void on_algorithm_changed(const AppState& app, SynthState& synth)
{
    if (synth.algorithm <= app.algorithms.size())
    {
        const Algorithm& algo = app.algorithms[synth.algorithm - 1];
        synth.op1.is_carrier  = algo.operators[0].is_carrier;
        synth.op2.is_carrier  = algo.operators[1].is_carrier;
        synth.op3.is_carrier  = algo.operators[2].is_carrier;
        synth.op4.is_carrier  = algo.operators[3].is_carrier;
        synth.op5.is_carrier  = algo.operators[4].is_carrier;
        synth.op6.is_carrier  = algo.operators[5].is_carrier;
    }
    else
    {
        synth.op1.is_carrier = false;
        synth.op2.is_carrier = false;
        synth.op3.is_carrier = false;
        synth.op4.is_carrier = false;
        synth.op5.is_carrier = false;
        synth.op6.is_carrier = false;
    }
}

void on_active_patch_changed(AppState& app)
{
    on_algorithm_changed(app, app.get_current_patch());
    app.patch_dirty = true;
}

void init_patch(AppState& app)
{
    app.patches[app.current_patch] = SynthState();
    on_algorithm_changed(app, app.get_current_patch());
    app.patch_dirty = true;
}

// Set all 32 patches to init state, named INITXX
void init_all(AppState& app)
{
    init_patch(app);

    for (unsigned int i = 0; i < app.patches.size(); i++)
    {
        app.patches[i] = SynthState();
        ImFormatString(app.patches[i].patch_name, IM_ARRAYSIZE(app.patches[i].patch_name), "INIT%d", i + 1);
    }
}

// Copy pasting envelopes.
// There doesn't seem to be an obvious way to check the context menu on multiple widgets,
// so this must be called after each right-clickable widget.
void envelope_copy_paste_context_menu(const char* id, AppState& app, Envelope& envelope)
{
    if (ImGui::BeginPopupContextItem(id))
    {
        if (ImGui::Selectable("Copy Envelope"))
        {
            copy_to_clipboard(app, envelope.to_string());
        }
        if (ImGui::Selectable("Paste Envelope"))
        {
            const std::string cbstr = get_from_clipboard(app);
            envelope.from_string(cbstr);
        }
        ImGui::EndPopup();
    }
}

// Draw the envelope graph and rate/level input boxes
void layout_envelope(AppState& app, Envelope& envelope)
{
    ImGui::PushItemWidth(250);

    int v[4] = {
        envelope.r1,
        envelope.r2,
        envelope.r3,
        envelope.r4
    };
    if (ImGui::DragInt4("R", v, 0.4f, 0, 99))
    {
        envelope.r1     = v[0];
        envelope.r2     = v[1];
        envelope.r3     = v[2];
        envelope.r4     = v[3];
        app.patch_dirty = true;
    }

    envelope_copy_paste_context_menu("copypaste_R", app, envelope);

    v[0] = envelope.l1;
    v[1] = envelope.l2;
    v[2] = envelope.l3;
    v[3] = envelope.l4;
    if (ImGui::DragInt4("L", v, 0.4f, 0, 99))
    {
        envelope.l1     = v[0];
        envelope.l2     = v[1];
        envelope.l3     = v[2];
        envelope.l4     = v[3];
        app.patch_dirty = true;
    }

    envelope_copy_paste_context_menu("copypaste_L", app, envelope);

    ImGui::PopItemWidth();
}

// Layout all controls for all operators
void layout_operator(Operator& op, AppState& app)
{
    const ImGuiStyle& style = ImGui::GetStyle();
    ImGui::PushID(op.op_number);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + 40.f, pos.y + ImGui::GetTextLineHeight() + style.WindowPadding.y), op.is_carrier ? COLOR_CARRIER : COLOR_MODULATOR);
    ImGui::Text("OP %d", op.op_number);
    ImGui::SameLine(60);
    app.patch_dirty |= (ImGui::Checkbox("On", &op.is_on));

    // OSCN
    ImGui::SameLine();
    bool osc_fixed = op.osc_mode > 0;
    if (ImGui::Checkbox("Fixed", &osc_fixed))
    {
        op.osc_mode     = osc_fixed ? 1 : 0;
        app.patch_dirty = true;
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Frequency mode (fixed/ratio)");

    ImGui::PushItemWidth(150.f);

    int coarse = op.freq_coarse;
    if (ImGui::SliderInt("FreqCoarse", &coarse, 0, 31))
    {
        op.freq_coarse  = static_cast<u8>(coarse);
        app.patch_dirty = true;
    }

    int fine = op.freq_fine;
    if (ImGui::SliderInt("FreqFine", &fine, 0, 99))
    {
        op.freq_fine    = static_cast<u8>(fine);
        app.patch_dirty = true;
    }

    int level = op.output_level;
    if (ImGui::SliderInt("Level", &level, 0, 99))
    {
        op.output_level = static_cast<u8>(level);
        app.patch_dirty = true;
    }

    ImGui::PopItemWidth();

    ImGui::PushItemWidth(80.f);

    // DETU
    int detune = op.detune;
    if (ImGui::SliderInt("Detune", &detune, 0, 14))
    {
        op.detune       = static_cast<u8>(detune);
        app.patch_dirty = true;
    }

    ImGui::PopItemWidth();

    layout_envelope(app, op.envelope);
    Envelope::draw_envelope("envelope", op.envelope, ImVec2(200.f, 75.f));
    envelope_copy_paste_context_menu("copypaste_env", app, op.envelope); // for the envelope graphic itself

    ImGui::PushItemWidth(40.f);

    // ORS
    app.patch_dirty |= drag_u8("Rate scaling", op.osc_rate_scale, 0, 7);

    // AMS
    ImGui::SameLine();
    app.patch_dirty |= drag_u8("Amp mod sense", op.amp_mod_sense, 0, 3);

    // KVS
    app.patch_dirty |= drag_u8("Vel. sense", op.key_velocity_sense, 0, 7);
    ImGui::PopItemWidth();

    ImGui::Text("Level Scaling");
    int                current_bp          = op.level_scale_break_point;
    static const char* break_point_names[] = { "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#" };
    std::ostringstream oss;
    oss << break_point_names[current_bp % 12] << (current_bp + 9) / 12 - 1 << " (%d)";
    if (ImGui::SliderInt("Break Point", &current_bp, 0, 99, oss.str().c_str()))
    {
        op.level_scale_break_point = current_bp;
        app.patch_dirty            = true;
    }

    app.patch_dirty |= drag_u8("L Depth", op.level_scale_left_depth);
    app.patch_dirty |= drag_u8("R Depth", op.level_scale_right_depth);

    ImGui::PushItemWidth(80.f);

    int item_current = op.level_scale_left_curve;
    if (ImGui::Combo("L Curve", &item_current, LEVEL_SCALE_CURVE_NAMES, IM_ARRAYSIZE(LEVEL_SCALE_CURVE_NAMES)))
    {
        op.level_scale_left_curve = item_current;
        app.patch_dirty           = true;
    }

    ImGui::SameLine();

    item_current = op.level_scale_right_curve;
    if (ImGui::Combo("R Curve", &item_current, LEVEL_SCALE_CURVE_NAMES, IM_ARRAYSIZE(LEVEL_SCALE_CURVE_NAMES)))
    {
        op.level_scale_right_curve = item_current;
        app.patch_dirty            = true;
    }

    ImGui::PopItemWidth();

    ImGui::PopID();
}

void draw_algorithm(AppState& app, SynthState& synth);

// Right side pane: pitch envelope, lfo settings, algorithm
void layout_common(SynthState& synth, AppState& app)
{
    ImGui::PushItemWidth(200.f);

    ImGui::PushItemWidth(40.f);
    ImGui::Text("Pitch envelope");
    layout_envelope(app, synth.pitch_env);
    Envelope::draw_envelope("envelope", synth.pitch_env, ImVec2(200.f, 75.f));
    envelope_copy_paste_context_menu("copypaste_env", app, synth.pitch_env); // for the envelope graphic itself
    ImGui::PopItemWidth();

    ImGui::Separator();

    const int fb_min = 0;
    const int fb_max = 7;
    app.patch_dirty |= ImGui::SliderScalar("Feedback", ImGuiDataType_U8, &synth.feedback, &fb_min, &fb_max, "%d");

    app.patch_dirty |= ImGui::Checkbox("OSC Key Sync", &synth.osc_key_sync);

    ImGui::Text("LFO");

    int item_current = synth.lfo_wave;
    if (ImGui::Combo("Wave", &item_current, LFO_WAVE_NAMES, IM_ARRAYSIZE(LFO_WAVE_NAMES)))
    {
        synth.lfo_wave  = item_current;
        app.patch_dirty = true;
    }

    app.patch_dirty |= drag_u8("Delay", synth.lfo_delay);
    app.patch_dirty |= drag_u8("Amp Mod Depth", synth.lfo_amp_mod_depth);
    app.patch_dirty |= ImGui::Checkbox("Key Sync", &synth.lfo_key_sync);
    app.patch_dirty |= drag_u8("Mod Sense Pitch", synth.mod_sense_pitch, 0, 7);
    app.patch_dirty |= drag_u8("Speed", synth.lfo_speed);
    app.patch_dirty |= drag_u8("Pitch Mod Depth", synth.lfo_pitch_mod_depth);

    // Transpose
    {
        static const char* transpose_labels[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        int                current_tp         = synth.transpose;
        std::ostringstream oss;
        oss << transpose_labels[current_tp % 12] << current_tp / 12 + 1 << " (%d)";

        const int transpo_min = 0;
        const int transpo_max = 48;
        app.patch_dirty |= ImGui::SliderScalar("Transpose", ImGuiDataType_U8, &synth.transpose, &transpo_min, &transpo_max, oss.str().c_str());
    }

    ImGui::Separator();
    ImGui::Text("Algorithm");
    const int algo_min = 1;
    const int algo_max = 32;
    if (ImGui::SliderScalar("Algorithm", ImGuiDataType_U8, &synth.algorithm, &algo_min, &algo_max, "%d"))
    {
        app.patch_dirty = true;
        on_algorithm_changed(app, synth);
    }

    ImGui::BeginChild("Algoview", ImVec2(ALGO_BOX_SIZE * 12, ALGO_BOX_SIZE * 10), false);
    {
        draw_algorithm(app, synth);
    }
    ImGui::EndChild();

    ImGui::PopItemWidth();
}

// Main menu bar
void layout_menu_bar(AppState& app, SynthState& synth)
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            const char* file_filter[]     = { "*.txt" };
            const char* file_filter_syx[] = { "*.syx" };

            if (ImGui::MenuItem("Init Patch"))
            {
                init_patch(app);
            }

            // Init all 32 patches
            /*if (ImGui::MenuItem("Init All"))
            {
                init_all(app);
            }*/

            // Load from txt
            if (ImGui::MenuItem("Load preset"))
            {
                const char* load_file_name = tinyfd_openFileDialog("Load preset", "presets/preset.txt", 1, file_filter, "Preset file", 0);
                if (load_file_name != nullptr)
                {
                    Fileops::load_from_disk(load_file_name, synth);
                    on_algorithm_changed(app, synth);
                    app.patch_dirty = true;
                }
            }

            // Save to txt
            if (ImGui::MenuItem("Save preset"))
            {
                std::string patch_name = "presets/preset.txt";
                if (strlen(synth.patch_name) > 0)
                {
                    patch_name = "presets/" + std::string(synth.patch_name) + ".txt";
                }

                const char* save_file_name = tinyfd_saveFileDialog("Save preset", patch_name.c_str(), 1, file_filter, "Preset file");
                if (save_file_name != nullptr)
                {
                    Fileops::save_to_disk(save_file_name, synth);
                }
            }

            // Load sysex bank
            if (ImGui::MenuItem("Load .SYX"))
            {
                const char* load_file_name = tinyfd_openFileDialog("Load .SYX", "presets/preset.syx", 1, file_filter_syx, "Sysex file", 0);
                if (load_file_name != nullptr)
                {
                    app.current_patch = 0;
                    synth             = app.patches[0];

                    const std::string error = Fileops::load_from_syx_file(load_file_name, app.patches);

                    if (error.size() > 0)
                    {
                        app.push_error_message(error);
                    }
                    else
                    {
                        on_algorithm_changed(app, synth);
                        app.patch_dirty = true;
                    }
                }
            }

            // Save sysex bank
            if (ImGui::MenuItem("Save .SYX"))
            {
                const char* save_file_name = tinyfd_saveFileDialog("Save preset", "presets/preset.syx", 1, file_filter_syx, "Sysex file");
                std::string error;
                if (save_file_name != nullptr)
                {
                    error = Fileops::save_to_syx_file(save_file_name, app.patches);
                }

                if (error.size() > 0)
                {
                    app.push_error_message(error);
                }
            }

            // Send&save all 32 patches
            if (ImGui::MenuItem("Send all patches..."))
            {
                app.show_bulk_dump_window = true;
            }
            ImGui::Separator();

            if (ImGui::MenuItem("Options..."))
            {
                app.show_options = true;
            }
            if (ImGui::MenuItem("Quit", "Alt+F4"))
            {
                app.quit_requested = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

// Bottom status bar with midi status and send velocity checkbox
void layout_status_bar(AppState& app_state)
{
    ImGuiContext& g                      = *GImGui;
    const float   height                 = g.NextWindowData.MenuBarOffsetMinVal.y + g.FontBaseSize + g.Style.FramePadding.y + ImGui::GetTextLineHeight();
    g.NextWindowData.MenuBarOffsetMinVal = ImVec2(g.Style.DisplaySafeAreaPadding.x, ImMax(g.Style.DisplaySafeAreaPadding.y - g.Style.FramePadding.y, 0.0f));
    ImGui::SetNextWindowPos(ImVec2(0.0f, g.IO.DisplaySize.y - height));
    ImGui::SetNextWindowSize(ImVec2(g.IO.DisplaySize.x, height));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));
    const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar;
    ImGui::Begin("##StatusBar", NULL, window_flags);
    {
        const bool midi_in_ok  = app_state.midi_in.isPortOpen();
        const bool midi_out_ok = app_state.midi_out.isPortOpen();
        ImGui::Text("Midi in: %s", midi_in_ok ? "OK" : "NO");

        if (ImGui::IsItemClicked(0))
        {
            app_state.show_options = true;
        }

        ImGui::SameLine();
        ImGui::Text("Midi out: %s", midi_out_ok ? "OK" : "NO");

        if (ImGui::IsItemClicked(0))
        {
            app_state.show_options = true;
        }

        ImGui::SameLine();
        ImGui::Checkbox("Send key velocity", &app_state.send_velocity);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Send midi key velocity. Volca's edit mode must be off!");
        }
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
    g.NextWindowData.MenuBarOffsetMinVal = ImVec2(0.0f, 0.0f);
}

// Left pane patch list
void layout_patch_list(AppState& app)
{
    int i = 0;
    for (const SynthState& patch : app.patches)
    {
        char label[128];
        sprintf(label, "%d %s", i, patch.patch_name);
        if (ImGui::Selectable(label, app.current_patch == i))
        {
            if (app.current_patch != i)
            {
                app.current_patch = i;
                on_active_patch_changed(app);
            }
        }
        i++;
    }
}

// Visualize the algorithm: the layout has already been defined
// in the data so here we just interpret it
void draw_algorithm(AppState& app, SynthState& synth)
{
    if (synth.algorithm > app.algorithms.size())
    {
        return;
    }

    const Algorithm& algo = app.algorithms[synth.algorithm - 1];

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    const ImVec2 curpos     = ImGui::GetCursorScreenPos() + ImVec2(0.f, 25.f);
    const float  op_width   = ALGO_BOX_SIZE;
    const ImVec2 label_size = ImGui::CalcTextSize("8");
    const float  pad_h      = (op_width - label_size.x) / 2.f;
    const float  pad_v      = (op_width - label_size.y) / 2.f;
    // Draw operator boxes
    for (int i = 0; i < 6; i++)
    {
        const Algorithm::OperatorNode& op              = algo.operators[i];
        const float                    adjx            = op.pos_x;
        const float                    adjy            = op.pos_y;
        const ImVec2                   pos             = curpos + ImVec2(float(adjx * op_width), float(adjy * op_width));
        ImU32                          op_color        = op.is_carrier ? COLOR_CARRIER : COLOR_MODULATOR;
        const ImU32                    op_border_color = op.is_carrier ? COLOR_CARRIER : COLOR_MODULATOR;
        // Draw disabled operator slightly translucent
        if (!synth.get_operator(i).is_on)
        {
            op_color = 0x44000000 | (op_color & 0x00ffffff);
        }
        draw_list->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + op_width, pos.y + op_width), op_color);
        draw_list->AddRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + op_width, pos.y + op_width), op_border_color);
        static char buf[2];
        ImFormatString(buf, IM_ARRAYSIZE(buf), "%d", i + 1);
        draw_list->AddText(ImVec2(pos.x + pad_h, pos.y + pad_v), IM_COL32(255, 255, 255, 255), buf);
    }

    // Draw lines connecting operators
    for (const Algorithm::Line& line : algo.lines)
    {
        const ImVec2 offset = curpos;
        draw_list->AddLine(offset + ImVec2(line.from_x * op_width, line.from_y * op_width), offset + ImVec2(line.to_x * op_width, line.to_y * op_width), IM_COL32(128, 128, 128, 255));
    }
}

// Initial window setup
GLFWwindow* setup_window()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return nullptr;

        // Decide GL+GLSL versions
#if __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    glfwWindowHint(GLFW_MAXIMIZED, 1);
    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Volcako", NULL, NULL);
    if (window == NULL)
        return nullptr;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    return window;
}

void setup_midi_initial(AppState& app_state, Settings& settings)
{
    app_state.midi_out.setErrorCallback(midi_error_callback);
    app_state.midi_in.setErrorCallback(midi_error_callback);

    app_state.refresh_available_midi_ports();

    app_state.midi_in.setCallback(midi_in_callback, &app_state);

    if (settings.get_midi_in_name().empty() && !app_state.midi_in_port_names.empty())
    {
        settings.set_midi_in_name(*app_state.midi_in_port_names.begin());
    }

    if (settings.get_midi_out_name().empty() && !app_state.midi_out_port_names.empty())
    {
        settings.set_midi_out_name(*app_state.midi_out_port_names.begin());
    }

    open_midi_port(settings.get_midi_out_name(), app_state.midi_out);
    open_midi_port(settings.get_midi_in_name(), app_state.midi_in);
}

void on_midi_ports_changed(AppState& app_state, const Settings& settings)
{
    open_midi_port(settings.get_midi_out_name(), app_state.midi_out);
    open_midi_port(settings.get_midi_in_name(), app_state.midi_in);
}

// Confirmation window for the buld send feature, as it's destructive
void layout_bulk_dump_window(AppState& app_state)
{
    if (!ImGui::IsPopupOpen("Send all patches"))
    {
        ImGui::OpenPopup("Send all patches");
    }

    ImGui::BeginPopupModal("Send all patches");

    ImGui::Text("Send all 32 patches to Volca?");
    ImGui::Text("Existing data will be LOST FOREVER");

    if (ImGui::Button("Yes"))
    {
        SynthState::send_bank(app_state.midi_out, app_state.patches);
        app_state.show_bulk_dump_window = false;
    }

    ImGui::SameLine();
    if (ImGui::Button("No"))
    {
        app_state.show_bulk_dump_window = false;
    }

    ImGui::EndPopup();
}

// Generic error window, shows the last error from message stack
void layout_error_window(AppState& app_state)
{
    if (app_state.error_messages.empty())
    {
        return;
    }

    if (!ImGui::IsPopupOpen("Error"))
    {
        ImGui::OpenPopup("Error");
    }

    const std::string& message = app_state.error_messages.back();

    ImGui::BeginPopupModal("Error");

    ImGui::Text(message.c_str());

    if (ImGui::Button("Close"))
    {
        app_state.pop_error_message();
    }

    ImGui::EndPopup();
}

int main(int, char**)
{
    GLFWwindow* window = setup_window();
    if (window == nullptr)
    {
        fprintf(stderr, "Failed to initialize GLFW Window\n");
        return 1;
    }

    // Initialize OpenGL loader
    const bool err = gl3wInit() != 0;
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& IO = ImGui::GetIO();

    ImGui::StyleColorsClassic();
    IO.IniFilename = NULL; // prevent imgui from saving settings.ini

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Support loading the included font from a couple of places
    ImFont* main_font = nullptr;
    if (Fileops::file_exists("DroidSans.ttf"))
    {
        main_font = IO.Fonts->AddFontFromFileTTF("DroidSans.ttf", FONT_SIZE);
    }
    else
    {
        main_font = IO.Fonts->AddFontFromFileTTF("../misc/fonts/DroidSans.ttf", FONT_SIZE);
    }

    Settings settings("settings.ini");

    AppState app_state(window);
    app_state.send_velocity = settings.get_send_velocity();
    app_state.algorithms    = Algorithm::build_algorithms();

    if (main_font == nullptr)
    {
        app_state.push_error_message("Could not load font DroidSans.ttf, application will not look as intended");
    }

    // First run, show setup dialog
    app_state.show_options = settings.get_midi_in_name().empty() && settings.get_midi_out_name().empty();

    setup_midi_initial(app_state, settings);

    // Load the last edited patch
    {
        SynthState& synth_state = app_state.patches[0];
        Fileops::load_from_disk("lastpreset.txt", synth_state);
        on_algorithm_changed(app_state, synth_state);
    }

    // Main loop
    while (!glfwWindowShouldClose(window) && !app_state.quit_requested)
    {
        glfwPollEvents();

        // support quick escape when debugging
#ifndef NDEBUG
        if (ImGui::IsKeyReleased(GLFW_KEY_ESCAPE))
        {
            app_state.quit_requested = true;
        }
#endif

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const ImGuiContext& g                 = *GImGui;
        const float         status_bar_height = g.FontBaseSize + g.Style.FramePadding.y + ImGui::GetTextLineHeight();

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(IO.DisplaySize.x, IO.DisplaySize.y - status_bar_height), ImGuiCond_Always);
        const ImGuiWindowFlags main_window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;
        ImGui::Begin("MainWindow", 0, main_window_flags);

        layout_status_bar(app_state);
        layout_menu_bar(app_state, app_state.get_current_patch());

        const float left_pane_width       = 210.f;
        const float right_pane_width      = 400.f;
        const float operator_column_width = 300.f;

        if (ImGui::BeginChild("LeftPane", ImVec2(left_pane_width, 0), false, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::BeginChild("PatchList", ImVec2(200, 0), true);
            layout_patch_list(app_state);
            ImGui::EndChild();
        }

        ImGui::EndChild();

        ImGui::SameLine();

        const float max_patch_window_width = IO.DisplaySize.x - left_pane_width - right_pane_width;
        ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(max_patch_window_width, -1));

        // Draw the 6 operator view: we support 3 and 2 column layout depending
        // on the window size
        if (ImGui::BeginChild("Patch", ImVec2(0, 0), true))
        {
            SynthState& synth_state = app_state.get_current_patch();

            ImGui::PushItemWidth(80.f);
            app_state.patch_dirty |= ImGui::InputText("Name", synth_state.patch_name, 10);
            ImGui::PopItemWidth();

            int num_columns = 3;
            if (max_patch_window_width < operator_column_width * 3.f)
            {
                num_columns = 2;
            }

            ImGui::Columns(num_columns, "Operators");
            if (num_columns == 3)
            {
                ImGui::SetColumnWidth(0, operator_column_width);
                ImGui::SetColumnWidth(1, operator_column_width);
                ImGui::SetColumnWidth(2, operator_column_width);

                // operators 1-3
                layout_operator(synth_state.op1, app_state);
                ImGui::NextColumn();
                layout_operator(synth_state.op2, app_state);
                ImGui::NextColumn();
                layout_operator(synth_state.op3, app_state);
                ImGui::Separator();

                // operators 4-6
                ImGui::NextColumn();
                layout_operator(synth_state.op4, app_state);
                ImGui::NextColumn();
                layout_operator(synth_state.op5, app_state);
                ImGui::NextColumn();
                layout_operator(synth_state.op6, app_state);
            }
            else
            {
                ImGui::SetColumnWidth(0, operator_column_width);
                ImGui::SetColumnWidth(1, operator_column_width);

                // op 1-2
                layout_operator(synth_state.op1, app_state);
                ImGui::NextColumn();
                layout_operator(synth_state.op2, app_state);

                ImGui::Separator();

                // op 3-4
                ImGui::NextColumn();
                layout_operator(synth_state.op3, app_state);
                ImGui::NextColumn();
                layout_operator(synth_state.op4, app_state);

                ImGui::Separator();

                // op 5-6
                ImGui::NextColumn();
                layout_operator(synth_state.op5, app_state);
                ImGui::NextColumn();
                layout_operator(synth_state.op6, app_state);
            }

            ImGui::Columns(1);
        }
        ImGui::EndChild();

        ImGui::SameLine();
        if (ImGui::BeginChild("RightPane"), ImVec2(right_pane_width, 0), false, ImGuiWindowFlags_AlwaysAutoResize)
        {
            layout_common(app_state.get_current_patch(), app_state);
        }
        ImGui::EndChild();

        ImGui::End(); // main window

        // Show the error window, options window or bulk send confirmation window
        if (app_state.error_messages.size() > 0)
        {
            layout_error_window(app_state);
        }
        else if (app_state.show_options)
        {
            settings.layout_settings_window(app_state);
        }
        else if (app_state.show_bulk_dump_window)
        {
            layout_bulk_dump_window(app_state);
        }

        // Imgui demo window is handy for development
        /*bool show_demo_window = true;
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);*/

        // Reset midi ports
        if (app_state.midi_settings_dirty)
        {
            on_midi_ports_changed(app_state, settings);
            app_state.midi_settings_dirty = false;
        }

        // Send the current patch
        if (app_state.patch_dirty)
        {
            const int checksum = app_state.get_current_patch().send(app_state.midi_out);
            // std::cout << "checksum " << std::hex << checksum << std::endl;
            app_state.patch_dirty = false;
        }

        // Render current frame
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(CLEAR_COLOR.x, CLEAR_COLOR.y, CLEAR_COLOR.z, CLEAR_COLOR.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Save the last preset to disk on exit
    SynthState& synth_state = app_state.get_current_patch();
    Fileops::save_to_disk("lastpreset.txt", synth_state);

    settings.set_send_velocity(app_state.send_velocity);
    settings.save_to_disk();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
