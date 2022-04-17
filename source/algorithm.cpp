/*
 * Licensed under MIT. See LICENSE.txt for details.
 */
#include "algorithm.h"

// Interprets algorithm layout ascii data, typed in from
// the Volca FM reference card. The data may be "hardcoded", but
// it's not like they will add new algorithms!
class AlgorithmBuilder
{
public:
    std::vector<Algorithm> algorithms;

    void build(const int width, std::vector<int> layout, int feedback_from, int feedback_to)
    {
        // will be used to draw a line connecting all carriers
        float carrier_min = 100.f;
        float carrier_max = -100.f;

        Algorithm algo;
        const int height = int(layout.size()) / width;
        for (int col = 0; col < width; col++)
        {
            const float adjcol  = col * 2.f; // add space between cells
            const float centerx = adjcol + 0.5f;
            for (int row = 0; row < height; row++)
            {
                const int   idx     = width * row + col;
                const int   cell    = layout[idx];
                const float centery = row + 0.5f;
                int         op_idx  = to_op_index(cell);
                if (op_idx >= 0 && op_idx <= 5)
                {
                    Algorithm::OperatorNode& op = algo.operators[op_idx];
                    if (row == height - 1)
                    {
                        op.is_carrier = true;
                    }
                    op.pos_x = float(adjcol);
                    op.pos_y = float(row);

                    if (op_idx + 1 == feedback_to)
                    {
                        if (feedback_to == feedback_from)
                        {
                            algo.lines.push_back({ centerx, centery - 0.5f, centerx, centery - 1.f });               // up
                            algo.lines.push_back({ centerx, centery - 1.0f, centerx + 1.0f, centery - 1.f });        // right
                            algo.lines.push_back({ centerx + 1.0f, centery - 1.f, centerx + 1.0f, centery + 1.0f }); // down
                            algo.lines.push_back({ centerx + 1.0f, centery + 1.0f, centerx, centery + 1.0f });       // left
                        }
                        // we know there are only a couple of exceptional feedback cases
                        else if (feedback_from == 4 && feedback_to == 6)
                        {
                            algo.lines.push_back({ centerx, centery - 0.5f, centerx, centery - 1.f });
                            algo.lines.push_back({ centerx, centery - 1.0f, centerx + 1.0f, centery - 1.f });
                            algo.lines.push_back({ centerx + 1.0f, centery - 1.f, centerx + 1.0f, centery + 4.75f });
                            algo.lines.push_back({ centerx + 1.0f, centery + 4.75f, centerx, centery + 4.75f });
                        }
                        else if (feedback_from == 5 && feedback_to == 6)
                        {
                            algo.lines.push_back({ centerx, centery - 0.5f, centerx, centery - 1.f });
                            algo.lines.push_back({ centerx, centery - 1.0f, centerx + 1.0f, centery - 1.f });
                            algo.lines.push_back({ centerx + 1.0f, centery - 1.f, centerx + 1.0f, centery + 2.75f });
                            algo.lines.push_back({ centerx + 1.0f, centery + 2.75f, centerx, centery + 2.75f });
                        }
                    }

                    // half line downwards
                    if (op.is_carrier)
                    {
                        algo.lines.push_back({ centerx, centery + 0.5f, centerx, centery + 1.f });
                        if (centerx < carrier_min)
                            carrier_min = centerx;
                        if (centerx > carrier_max)
                            carrier_max = centerx;
                    }
                }
                else
                {
                    if (cell == '|')
                    {
                        algo.lines.push_back({ centerx, centery - 0.5f, centerx, centery + 0.5f });
                    }
                    else if (cell == '┘')
                    {
                        algo.lines.push_back({ centerx, centery, centerx, centery - 0.5f });
                        algo.lines.push_back({ centerx - 0.5f, centery, centerx, centery });
                        // extra left
                        algo.lines.push_back({ centerx - 0.5f, centery, centerx - 1.5f, centery });
                    }
                    else if (cell == '└')
                    {
                        algo.lines.push_back({ centerx, centery, centerx, centery - 0.5f });
                        algo.lines.push_back({ centerx, centery, centerx + 0.5f, centery });
                        // extra right
                        algo.lines.push_back({ centerx + 0.5f, centery, centerx + 1.5f, centery });
                    }
                    else if (cell == '┐')
                    {
                        algo.lines.push_back({ centerx - 0.5f, centery, centerx, centery });
                        algo.lines.push_back({ centerx, centery, centerx, centery + 0.5f });
                        // extra left
                        algo.lines.push_back({ centerx - 0.5f, centery, centerx - 1.5f, centery });
                    }
                    else if (cell == '┌')
                    {
                        algo.lines.push_back({ centerx, centery, centerx + 0.5f, centery });
                        algo.lines.push_back({ centerx, centery, centerx, centery + 0.5f });
                        // extra right
                        algo.lines.push_back({ centerx + 0.5f, centery, centerx + 1.5f, centery });
                    }
                    else if (cell == '├')
                    {
                        algo.lines.push_back({ centerx, centery - 0.5f, centerx, centery + 0.5f });
                        algo.lines.push_back({ centerx, centery, centerx + 0.5f, centery });
                    }
                    else if (cell == '┼')
                    {
                        algo.lines.push_back({ centerx - 0.5f, centery, centerx + 0.5f, centery });
                        algo.lines.push_back({ centerx, centery - 0.5f, centerx, centery + 0.5f });
                    }
                }
            }
        }

        algo.lines.push_back({ carrier_min, float(height + 0.5f), carrier_max, float(height + 0.5f) });

        algorithms.push_back(algo);
    }

private:
    int to_op_index(int cell_value)
    {
        if (cell_value == '1') return 0;
        if (cell_value == '2') return 1;
        if (cell_value == '3') return 2;
        if (cell_value == '4') return 3;
        if (cell_value == '5') return 4;
        if (cell_value == '6')
            return 5;
        else
            return -1;
    }
};

std::vector<Algorithm> Algorithm::build_algorithms()
{
    // Defining algorithm layouts
    // Bottom row is always carriers
    // feedback from-to is specified separately
    // clang-format off

    AlgorithmBuilder builder;

    //Algo1
    builder.build(2, {
        ' ', '6',
        ' ', '|',
        ' ', '5',
        ' ', '|',
        '2', '4',
        '|', '|',
        '1', '3', }, 6, 6);

    //Algo 2

    builder.build(2, {
        ' ', '6',
        ' ', '|',
        ' ', '5',
        ' ', '|',
        '2', '4',
        '|', '|',
        '1', '3', }, 2, 2);

    //Algo 3
    builder.build(2, {
        '3', '6',
        '|', '|',
        '2', '5',
        '|', '|',
        '1', '4', }, 6, 6);

    //Algo 4
    builder.build(2, {
        '3', '6',
        '|', '|',
        '2', '5',
        '|', '|',
        '1', '4', }, 4, 6);

    //Algo 5
    builder.build(3, {
        '2', '4', '6',
        '|', '|', '|',
        '1', '3', '5' }, 6, 6);

    //Algo 6
    builder.build(3, {
        '2', '4', '6',
        '|', '|', '|',
        '1', '3', '5' }, 5, 6);

    //Algo 7
    builder.build(3, {
        ' ', ' ', '6',
        ' ', ' ', '|',
        '2', '4', '5',
        '|', '|', '|',
        '1', '3', '┘' }, 6, 6);

    //Algo 8
    builder.build(3, {
        ' ', ' ', '6',
        ' ', ' ', '|',
        '2', '4', '5',
        '|', '|', '|',
        '1', '3', '┘' }, 4, 4);

    //Algo 9
    builder.build(3, {
        ' ', ' ', '6',
        ' ', ' ', '|',
        '2', '4', '5',
        '|', '|', '|',
        '1', '3', '┘' }, 2, 2);

    //Algo 10
    builder.build(3, {
        ' ', ' ', '3',
        ' ', ' ', '|',
        '5', '6', '2',
        '|', '|', '|',
        '└', '4', '1' }, 3, 3);

    //Algo 11
    builder.build(3, {
        ' ', ' ', '3',
        ' ', ' ', '|',
        '5', '6', '2',
        '|', '|', '|',
        '└', '4', '1' }, 6, 6);

    //Algo 12
    builder.build(4, {
        '4', '5', '6', '2',
        '|', '|', '|', '|',
        '└', '3', '┘', '1', }, 2, 2);

    //Algo 13
    builder.build(4, {
        '4', '5', '6', '2',
        '|', '|', '|', '|',
        '└', '3', '┘', '1', }, 6, 6);

    //Algo 14
    builder.build(3, {
        ' ', '6', '5',
        ' ', '|', '|',
        '2', '4', '┘',
        '|', '|', ' ',
        '1', '3', ' ' }, 6, 6);

    //Algo 15
    builder.build(3, {
        ' ', '6', '5',
        ' ', '├', '┘',
        '2', '4', ' ',
        '|', '|', ' ',
        '1', '3', ' ' }, 2, 2);

    //Algo 16
    builder.build(3, {
        ' ', '4', '6',
        ' ', '|', '|',
        '2', '3', '5',
        '|', '|', '|',
        '└', '1', '┘' }, 6, 6);

    //Algo 17
    builder.build(3, {
        ' ', '4', '6',
        ' ', '|', '|',
        '2', '3', '5',
        '|', '|', '|',
        '└', '1', '┘' }, 2, 2);

    //Algo 18
    builder.build(3, {
        ' ', ' ', '6',
        ' ', ' ', '|',
        ' ', ' ', '5',
        ' ', ' ', '|',
        '2', '3', '4',
        '|', '|', '|',
        '└', '1', '┘' }, 3, 3);

    //Algo 19
    builder.build(3, {
        '3', ' ', ' ',
        '|', ' ', ' ',
        '2', '6', ' ',
        '|', '├', '┐',
        '1', '4', '5' }, 6, 6);

    //Algo 20
    builder.build(4, {
        '3', ' ', '5', '6',
        '├', '┐', '|', '|',
        '1', '2', '└', '4' }, 3, 3);

    //Algo 21
    builder.build(4, {
        '3', ' ', '6', ' ',
        '├', '┐', '├', '┐',
        '1', '2', '4', '5' }, 3, 3);

    //Algo 22
    builder.build(4, {
        '2', ' ', '6', ' ',
        '|', '┌', '┼', '┐',
        '1', '3', '4', '5' }, 6, 6);

    //Algo 23
    builder.build(4, {
        ' ', '3', '6', ' ',
        ' ', '|', '├', '┐',
        '1', '2', '4', '5' }, 6, 6);

    //Algo 24
    builder.build(5, {
        ' ', ' ', ' ', '6', ' ',
        ' ', ' ', '┌', '┼', '┐',
        '1', '2', '3', '4', '5' }, 6, 6);

    //Algo 25
    builder.build(5, {
        ' ', ' ', ' ', '6', ' ',
        ' ', ' ', ' ', '├', '┐',
        '1', '2', '3', '4', '5' }, 6, 6);

    //Algo 26
    builder.build(4, {
        ' ', '3', '5', '6',
        ' ', '|', '|', '|',
        '1', '2', '└', '4' }, 6, 6);

    //Algo 27
    builder.build(4, {
        ' ', '3', '5', '6',
        ' ', '|', '|', '|',
        '1', '2', '└', '4' }, 3, 3);

    //Algo 28
    builder.build(3, {
        ' ', '5', ' ',
        ' ', '|', ' ',
        '2', '4', ' ',
        '|', '|', ' ',
        '1', '3', '6' }, 5, 5);

    //Algo 29
    builder.build(4, {
        ' ', ' ', '4', '6',
        ' ', ' ', '|', '|',
        '1', '2', '3', '5' }, 6, 6);

    //Algo 30
    builder.build(4, {
        ' ', ' ', '5', ' ',
        ' ', ' ', '|', ' ',
        ' ', ' ', '4', ' ',
        ' ', ' ', '|', ' ',
        '1', '2', '3', '6' }, 5, 5);

    //Algo 31
    builder.build(5, {
        ' ', ' ', ' ', ' ', '6',
        ' ', ' ', ' ', ' ', '|',
        '1', '2', '3', '4', '5' }, 6, 6);

    //Algo 32
    builder.build(6, {
        '1', '2', '3', '4', '5', '6' }, 6, 6);

    return builder.algorithms;
    // clang-format on
}
