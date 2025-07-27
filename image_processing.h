#pragma once
#include <filesystem>
#include <thread>
#include "utility.h"
#include "configuration.h"

void CropImages(Configuration const &conf, std::vector<path> const &images, path const &output_folder);

void AddBleed(Configuration const& conf, path const& input, path const& output);