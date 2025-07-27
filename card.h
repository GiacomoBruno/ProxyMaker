#pragma once
#include "paper.h"
#include <filesystem>

struct Card
{
    std::filesystem::path ImageFile;
    CardSizes WantedSize;
};

std::vector<Card> PrepCards(std::vector<std::filesystem::path> const& input);