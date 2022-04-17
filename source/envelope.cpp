/*
 * Licensed under MIT. See LICENSE.txt for details.
 */
#include "envelope.h"
#include "imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_internal.h"

#pragma warning(disable : 4996) // crt_secure_no_warnings

// Plot the envelope graph
void Envelope::draw_envelope(const char* label, const Envelope& env, ImVec2 frame_size)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext&     g     = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID     id    = window->GetID(label);

    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    if (frame_size.x == 0.0f)
        frame_size.x = ImGui::CalcItemWidth();
    if (frame_size.y == 0.0f)
        frame_size.y = label_size.y + (style.FramePadding.y * 2);

    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
    const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, 0, &frame_bb))
        return;

    ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

    // min = top left corner, +y downwards
    const float EMAX  = 99.f;
    const float l1    = 1.f - env.l1 / EMAX;
    const float l2    = 1.f - env.l2 / EMAX;
    const float l3    = 1.f - env.l3 / EMAX;
    const float l4    = 1.f - env.l4 / EMAX;
    const int   color = ImGui::GetColorU32(ImGuiCol_PlotHistogram);

    const u8    sus_len = 80;
    const float max     = static_cast<float>((99 - env.r1) + (99 - env.r2) + (99 - env.r3) + sus_len + (99 - env.r4));
    const float r1      = (99 - env.r1) / max;
    const float r2      = r1 + (99 - env.r2) / max;
    const float r3      = r2 + (99 - env.r3) / max;
    const float sus     = r3 + sus_len / max;

    window->DrawList->PathLineTo(ImLerp(frame_bb.Min, frame_bb.Max, ImVec2(0.f, l4)));
    window->DrawList->PathLineTo(ImLerp(frame_bb.Min, frame_bb.Max, ImVec2(r1, l1)));
    window->DrawList->PathLineTo(ImLerp(frame_bb.Min, frame_bb.Max, ImVec2(r2, l2)));
    window->DrawList->PathLineTo(ImLerp(frame_bb.Min, frame_bb.Max, ImVec2(r3, l3)));
    window->DrawList->PathLineTo(ImLerp(frame_bb.Min, frame_bb.Max, ImVec2(sus, l3)));
    window->DrawList->PathLineTo(ImLerp(frame_bb.Min, frame_bb.Max, ImVec2(1.f, l4)));

    window->DrawList->PathStroke(color, false, 2.f);
}

// For copypaste support
std::string Envelope::to_string() const
{
    char buf[128];
    ImFormatString(buf, IM_ARRAYSIZE(buf), "env={%d,%d,%d,%d,%d,%d,%d,%d}", r1, r2, r3, r4, l1, l2, l3, l4);
    return buf;
}

// For copypaste support
void Envelope::from_string(const std::string& string)
{
    int v[8] = {};

    if (sscanf(string.c_str(), "env={%d,%d,%d,%d,%d,%d,%d,%d}", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7]) == 8)
    {
        r1 = v[0];
        r2 = v[1];
        r3 = v[2];
        r4 = v[3];

        l1 = v[4];
        l2 = v[5];
        l3 = v[6];
        l4 = v[7];
    }
}
