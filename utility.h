#pragma once

#include <vector>
#include <string>
#include <filesystem>

using path = std::string;

bool IsImageExt(path const &ext);

std::string FromWstring(std::wstring const &wide);

std::vector<path> LoadImages(path const &folder);

bool RunCommand(std::string const& command, bool quiet = true);

void PrepareDirectory(path const& dir, bool clean = true);