#include "paper.h"


double GetTotalCardWidth(CardSizes const& s)
{
    return s.Width + s.Bleed + s.Bleed;
}
double GetTotalCardHeight(CardSizes const& s)
{
    return s.Height + s.Bleed + s.Bleed;
}

Paper GetPaper(PAPER p)
{
    switch(p)
    {
        case PAPER::eA4:
            return pA4;
        case PAPER::e10_15:
            return p10x15;
    }
    return {};
}
