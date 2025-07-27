#include "configuration.h"
#include "magic_enum.hpp"

#include <filesystem>

using enum PAPER;

constexpr double InchToMM(double inch) { return inch * 25.4; } 
   
CardSizes const& Configuration::GetCardSize() const
{
    return CardSize;
}

double Configuration::GetPPI() const { return PPI; }

bool Configuration::GetDrawCross() const
{
    return DrawCross;
}

path Configuration::GetOutputFile() const
{
    return OutputFile;
}

path Configuration::GetWorkDir() const
{
    return WorkFolder;
}
path Configuration::GetDir(path const& d) const
{
    return WorkFolder.native() + d.native();
}

void Configuration::SetWorkDir(path const& d )
{
    WorkFolder = d;
}

void prepare_directories(path const& work_folder)
{
    if(!std::filesystem::exists(work_folder.native() + FILES_FOLDER))
        std::filesystem::create_directory(work_folder.native() +  FILES_FOLDER);

    if(!std::filesystem::exists(work_folder.native() +  IMAGE_FOLDER))
        std::filesystem::create_directory(work_folder.native() +  IMAGE_FOLDER);

    if(!std::filesystem::exists(work_folder.native() + CROP_FOLDER))
        std::filesystem::create_directory(work_folder.native() +  CROP_FOLDER);

    if(!std::filesystem::exists(work_folder.native() +  SCRYFALL_FOLDER))
        std::filesystem::create_directory(work_folder.native() +  SCRYFALL_FOLDER);
}

Configuration prepare_configuration(char const* work_folder)
{
    Configuration conf{};
    std::cout << work_folder << std::endl;
    conf.SetWorkDir(work_folder);


    prepare_directories(work_folder);
    return conf;
}


double GetCardW(Configuration const& c)
{
    return c.GetCardSize().Width;
}
double GetCardH(Configuration const& c)
{
    return c.GetCardSize().Height;
}
double GetFullCardW(Configuration const& c)
{
    return GetTotalCardWidth(c.GetCardSize());
}
double GetFullCardH(Configuration const& c)
{
    return GetTotalCardHeight(c.GetCardSize());
}
double GetCardBleed(Configuration const& c)
{
    return c.GetCardSize().Bleed;
}

PAPER Configuration::GetPaperType() const
{
    return PaperType;
}

double GetMarginCardW(Configuration const& c)
{
    auto cs = c.GetCardSize();
    return cs.Width + cs.Margin + cs.Margin;
}

double GetMarginCardH(Configuration const& c)
{
    auto cs = c.GetCardSize();
    return cs.Height + cs.Margin + cs.Margin;
}

double GetMargin(Configuration const& c)
{
    auto cs = c.GetCardSize();
    return cs.Margin;
}