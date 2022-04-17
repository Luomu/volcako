/*
 * Licensed under MIT. See LICENSE.txt for details.
 */
#pragma once
#include <vector>

/** Data for the graphical representation of an algorithm */
struct Algorithm
{
    // Operators, carriers are always on bottom line,
    // rest are modulators
    struct OperatorNode
    {
        int   id         = 0;
        bool  is_carrier = false;
        float pos_x      = 0;
        float pos_y      = 0;

        std::vector<int> modulators;

        OperatorNode& add_modulator(int id_)
        {
            modulators.push_back(id_);
            return *this;
        }
    };

    // Connecting lines
    struct Line
    {
        float from_x = 0;
        float from_y = 0;
        float to_x   = 0;
        float to_y   = 0;
    };

    std::vector<OperatorNode> operators;
    std::vector<Line>         lines;

    Algorithm()
    {
        operators.resize(6);
    }

    static std::vector<Algorithm> build_algorithms();
};
