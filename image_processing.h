#pragma once
#include <filesystem>
#include <thread>
#include "utility.h"
#include "configuration.h"

void CropImages(Configuration const &conf, std::vector<std::filesystem::path> const &images, std::filesystem::path const &output_folder);

void ResizeScryfallImages(Configuration const& conf);