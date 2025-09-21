#pragma once

enum class PAPER
{
    eA4,
    e10_15
};

struct Paper
{
    double W{210};
    double H{297};
};

struct CardSizes
{
    double Width{};
    double Height{};
    double Margin{};
    double Bleed{};
};

Paper GetPaper(PAPER p);

double GetTotalCardWidth(CardSizes const& s);
double GetTotalCardHeight(CardSizes const& s);


constexpr CardSizes MPCFillCard{.Width = 63, .Height = 88, .Margin = 1, .Bleed = 3.048};
constexpr CardSizes HalfSizeCard{.Width = 88/2, .Height = 63, .Margin = 1, .Bleed = 3.048};

constexpr Paper pA4{ .W = 210, .H = 297};
constexpr Paper p10x15{ .W= 100, .H= 150}; 