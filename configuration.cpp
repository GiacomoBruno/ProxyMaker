#include "configuration.h"
#include "magic_enum.hpp"

#include <filesystem>
#include <format>
#include <iostream>

using enum PAPER;

constexpr double InchToMM(double inch) { return inch * 25.4; }

CardSizes const &Configuration::GetCardSize() const
{
    return CardSize;
}

double Configuration::GetPPI() const { return PPI; }

path Configuration::GetOutputFile() const
{
    return OutputFile;
}

path Configuration::GetWorkDir() const
{
    return WorkFolder;
}
path Configuration::GetDir(path const &d) const
{
    return WorkFolder.native() + d.native();
}

void Configuration::SetWorkDir(path const &d)
{
    WorkFolder = d;
}

void prepare_directories(path const &work_folder)
{
    if (!std::filesystem::exists(work_folder.native() + FILES_FOLDER))
        std::filesystem::create_directory(work_folder.native() + FILES_FOLDER);

    if (!std::filesystem::exists(work_folder.native() + IMAGE_FOLDER))
        std::filesystem::create_directory(work_folder.native() + IMAGE_FOLDER);

    if (!std::filesystem::exists(work_folder.native() + IMAGE_INPUT))
        std::filesystem::create_directory(work_folder.native() + IMAGE_INPUT);

    if (!std::filesystem::exists(work_folder.native() + IMAGE_FULL_INPUT))
        std::filesystem::create_directory(work_folder.native() + IMAGE_FULL_INPUT);

    if (!std::filesystem::exists(work_folder.native() + UNPADDED_IMAGE_INPUT))
        std::filesystem::create_directory(work_folder.native() + UNPADDED_IMAGE_INPUT);

    if (!std::filesystem::exists(work_folder.native() + CROP_FOLDER))
        std::filesystem::create_directory(work_folder.native() + CROP_FOLDER);
}

Configuration prepare_configuration(char const *work_folder)
{
    Configuration conf{};
    conf.SetWorkDir(work_folder);

    prepare_directories(work_folder);
    return conf;
}

double GetCardW(Configuration const &c)
{
    return c.GetCardSize().Width;
}
double GetCardH(Configuration const &c)
{
    return c.GetCardSize().Height;
}
double GetFullCardW(Configuration const &c)
{
    return GetTotalCardWidth(c.GetCardSize());
}
double GetFullCardH(Configuration const &c)
{
    return GetTotalCardHeight(c.GetCardSize());
}
double GetCardBleed(Configuration const &c)
{
    return c.GetCardSize().Bleed;
}

PAPER Configuration::GetPaperType() const
{
    return PaperType;
}

int16_t Configuration::GetSpacing() const
{
    return Spacing;
}

double GetMarginCardW(Configuration const &c)
{
    auto cs = c.GetCardSize();
    return cs.Width + cs.Margin + cs.Margin;
}

double GetMarginCardH(Configuration const &c)
{
    auto cs = c.GetCardSize();
    return cs.Height + cs.Margin + cs.Margin;
}

double GetMargin(Configuration const &c)
{
    auto cs = c.GetCardSize();
    return cs.Margin;
}

void PrintConfiguration(Configuration const &conf)
{
    std::cout << "Current Configuration: \n";
    std::cout << std::format("\tPAPER TYPE: {}\n", magic_enum::enum_name<PAPER>(conf.PaperType));
    std::cout << std::format("\tCARD_SIZE: [{}]x[{}] - MARGIN[{}] - BLEED[{}]\n", conf.CardSize.Width, conf.CardSize.Height, conf.CardSize.Margin, conf.CardSize.Bleed);
    std::cout << std::format("\tPPI: {}\n", conf.PPI);
    std::cout << std::format("\tOUTPUT_FILE: {}\n", conf.OutputFile.string());
    std::cout << std::format("\tSPACING: {}\n", conf.Spacing);
}

void UpdateConfiguration(Configuration &conf)
{
    bool done = false;
    while (!done)
    {
        std::cout << std::format("Customize:\n\t1. PAPER_TYPE, \n\t2. CARD_SIZE, \n\t3. PPI, \n\t4. OUTPUT_FILE, \n\t5. SPACING, \n\t6. DONE\n");
        int input = 0;

        std::cin >> input;

        switch (input)
        {
        case 1:
        {
            std::cout << "PAPER_TYPE [A4(1), 10x15(2)]: ";
            int selection = 0;
            std::cin >> selection;
            switch (selection)
            {
            case 1:
                conf.PaperType = PAPER::eA4;
                break;
            case 2:
                conf.PaperType = PAPER::e10_15;
                break;
            default:
                break;
            }
            break;
        }
        case 2:
        {
            std::cout << "CARD_SIZE [MPC_FILL(1), HALF_SIZE(2)]: ";
            int selection = 0;
            std::cin >> selection;
            switch (selection)
            {
            case 1:
                conf.CardSize = MPCFillCard;
                break;
            case 2:
                conf.CardSize = HalfSizeCard;
                break;
            default:
                break;
            }
            break;
        }
        case 3:
        {
            std::cout << "PPI: ";
            double ppi = 72;
            std::cin >> ppi;
            conf.PPI = ppi;
            break;
        }
        case 4:
        {
            std::cout << "File: ";
            std::string file;
            std::cin >> file;
            conf.OutputFile = file;
            break;
        }
        case 5:
        {
            std::cout << "Spacing: ";
            int spacing = 0;
            std::cin >> spacing;
            conf.Spacing = spacing;
            break;
        }
        case 6:
        {
            done = true;
            break;
        }
        default:
            break;
        }
    }
}

void Configuration::AddGeneratedFile(std::filesystem::path const &f) const
{
    std::unique_lock l{*m_mutex};
    GeneratedFiles.push_back(f);
}
void Configuration::Cleanup() const
{
    {
        std::unique_lock l{*m_mutex};
        for (auto &f : GeneratedFiles)
        {
            std::cout << "attempt to delete: " << f.string() << std::endl;
            if (std::filesystem::exists(f))
                std::filesystem::remove(f);
        }
        GeneratedFiles.clear();
    }
    delete m_mutex;
}