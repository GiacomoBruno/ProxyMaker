#pragma once

#include <vector>
#include <string>
#include <filesystem>

using path = std::filesystem::path;

bool IsImageExt(path const &ext);

std::vector<path> LoadImages(path const &folder, bool rename = false);

bool RunCommand(std::string const& command, bool quiet = true);

void PrepareDirectory(path const& dir, bool clean = true);

double GetPrintSize(double size, double dpi);