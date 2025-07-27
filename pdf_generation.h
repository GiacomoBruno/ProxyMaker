#pragma once
#include "configuration.h"
#include "card.h"

class PageContentContext; 

void DrawCross(Configuration const &conf, PageContentContext *cxt, double center_x, double center_y);

void GeneratePage(Configuration const &conf, int idx, std::vector<Card> const& cards);

void GeneratePDF(Configuration const &conf, std::vector<Card> &cards, std::filesystem::path const &filename);