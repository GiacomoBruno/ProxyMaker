#pragma once
#include <string>
#include <filesystem>
#include "configuration.h"

void ReadList(std::filesystem::path const& list, std::vector<std::string>& output);

void DownloadCard(std::string card_name, std::filesystem::path const& output_dir);

void DownloadList(std::filesystem::path const& list);