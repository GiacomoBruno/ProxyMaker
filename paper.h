#pragma once

enum class PAPER
{
    A4,
    A5,
    A6,
    _10_15,
    _13_18,
};

struct Paper
{
    const double W{8.3};
    const double H{11.7};

    double GetPaperW(double ppu, bool doPPU = true) const;
    double GetPaperH(double ppu, bool doPPU = true) const;

};

constexpr Paper A4_Paper{.W{8.3},.H{11.7}};
constexpr Paper A5_Paper{.W{5.8},.H{8.3}};
constexpr Paper A6_Paper{.W{4.1},.H{5.8}};
constexpr Paper _10_15_Paper{.W{3.9370},.H{5.90551}};
constexpr Paper _13_18_Paper{.W{5.},.H{7.}};