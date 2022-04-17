/*
 * Licensed under MIT. See LICENSE.txt for details.
 */
#pragma once
#include "imgui.h"
#include <string>

typedef unsigned char u8;

// Operator and pitch envelope class
struct Envelope
{
    u8 r1 = 99;
    u8 r2 = 99;
    u8 r3 = 99;
    u8 r4 = 99;

    u8 l1 = 99;
    u8 l2 = 99;
    u8 l3 = 99;
    u8 l4 = 0;

    Envelope(u8 r1, u8 r2, u8 r3, u8 r4, u8 l1, u8 l2, u8 l3, u8 l4)
        : r1(r1)
        , r2(r2)
        , r3(r3)
        , r4(r4)
        , l1(l1)
        , l2(l2)
        , l3(l3)
        , l4(l4)
    {
    }

    static void draw_envelope(const char* label, const Envelope& envelope, ImVec2 frame_size);
    // make a clipboard-appropriate string:
    //"env={r1,r2,r3,r4,l1,l2,l3,l4}"
    std::string to_string() const;
    // set values from a clipboard string
    void from_string(const std::string& string);
};
