#include "paper.h"

using enum PAPER;

double Paper::GetPaperW(double ppu, bool doPPU) const
{
    return W * (doPPU ? ppu : 1);
}

double Paper::GetPaperH(double ppu, bool doPPU) const
{
    return H * (doPPU ? ppu : 1);
}