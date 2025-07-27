#include "card.h"
#include <iostream>

std::vector<Card> PrepCards(std::vector<std::filesystem::path> const& input)
{
    std::vector<Card> output{};
    
    for(auto& i : input)
    {        
        Card c{};
        c.ImageFile = i;
        output.push_back(c);
    }
    return output;
}