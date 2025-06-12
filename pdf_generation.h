#pragma once
#include "configuration.h"

class PageContentContext; 

void DrawCross(Configuration const &conf, PageContentContext *cxt, double center_x, double center_y);

void GeneratePage(Configuration const &conf, PageConfiguration const& pConf, int idx, std::vector<path> const& images);

void GeneratePDF(Configuration const &conf, std::vector<path> &images, std::string const &filename);