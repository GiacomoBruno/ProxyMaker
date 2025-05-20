#pragma once

#include <vector>
#include <string>
#include <filesystem>


bool IsImageExt(std::filesystem::path const &ext);

std::string FromWstring(std::wstring const &wide);

std::vector<std::filesystem::path> LoadImages(std::filesystem::path const &folder);

bool RunCommand(std::string const& command, bool quiet = false);