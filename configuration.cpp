#include "configuration.h"
#include "magic_enum.hpp"
#include "json.h"

#include <filesystem>

using enum PAPER;

constexpr double InchToMM(double inch) { return inch * 25.4; } 

double Configuration::GetCardW(bool doPPU) const
{
    return CardSize.SafeW * (doPPU ? PPI : 1);
}

double Configuration::GetCardH(bool doPPU) const
{
    return CardSize.SafeH * (doPPU ? PPI : 1);
}

double Configuration::GetCardWithBleedW(bool doPPU) const
{
    return (CardSize.SafeW + (CardSize.BleedSize * 2.)) *  (doPPU ? PPI : 1);
}
double Configuration::GetCardWithBleedH(bool doPPU) const
{
    return (CardSize.SafeH + (CardSize.BleedSize * 2.)) *  (doPPU ? PPI : 1);
}

double Configuration::GetPaperW(bool doPPU) const
{
    switch (PaperType)
    {
    case A4:
        return A4_Paper.GetPaperW(PPI, doPPU);
    case A5:
        return A5_Paper.GetPaperW(PPI, doPPU);
    case A6:
        return A6_Paper.GetPaperW(PPI, doPPU);
    case _10_15:
        return _10_15_Paper.GetPaperW(PPI, doPPU);
    case _13_18:
        return _13_18_Paper.GetPaperW(PPI, doPPU);
    }
    return 0.;
}

double Configuration::GetPaperH(bool doPPU) const
{
    switch (PaperType)
    {
    case A4:
        return A4_Paper.GetPaperH(PPI, doPPU);
    case A5:
        return A5_Paper.GetPaperH(PPI, doPPU);
    case A6:
        return A6_Paper.GetPaperH(PPI, doPPU);
    case _10_15:
        return _10_15_Paper.GetPaperH(PPI, doPPU);
    case _13_18:
        return _13_18_Paper.GetPaperH(PPI, doPPU);
    }
    return 0.;
}

double Configuration::GetBleed(bool doPPU) const
{
    return CardSize.BleedSize * (doPPU ? PPI : 1);
}

double Configuration::GetFullBleedW(bool doPPU) const
{
    return CardSize.FullBleedSizeW * (doPPU ? PPI : 1);
}

double Configuration::GetFullBleedH(bool doPPU) const
{
    return CardSize.FullBleedSizeH * (doPPU ? PPI : 1);
}

int Configuration::GetVerticalOffset() const { return VerticalOffsetPX; }
int Configuration::GetHorizontalOffset() const { return HorizontalOffsetPX; }

double Configuration::GetPPI() const { return PPI; }

bool Configuration::GetDrawCross() const
{
    return DrawCross;
}

double Configuration::GetCrossThickness(bool doPPU) const { return CrossThickness * (doPPU ? PPI : 1); }
double Configuration::GetCrossLength(bool doPPU) const { return CrossLength * (doPPU ? PPI : 1); }

std::string Configuration::GetOutputFile() const
{
    return OutputFile;
}

path Configuration::GetWorkDir() const
{
    return WorkFolder;
}
path Configuration::GetDir(path const& d) const
{
    return WorkFolder + d;
}

void Configuration::SetWorkDir(path const& d )
{
    WorkFolder = d;
}

void Configuration::LoadConfiguration()
{
    auto confFile = GetDir(FILES_FOLDER CONFIG_FILE);

    if(!std::filesystem::exists(GetDir(FILES_FOLDER)))
    {
        std::filesystem::create_directory(GetDir(FILES_FOLDER));
        return;
    }

    if (std::filesystem::exists(confFile) && std::filesystem::is_regular_file(confFile))
    {
        std::ifstream file{confFile};
        RSJresource json{file};
        PaperType = *magic_enum::enum_cast<PAPER>(json["paper_type"].as<std::string>("A4"));
        CardSize.SafeW = json["card_safe_w"].as<double>(CardSize.SafeW);
        CardSize.SafeH = json["card_safe_h"].as<double>(CardSize.SafeH);
        CardSize.AdditionalSafe = json["card_additional_safe"].as<double>(CardSize.AdditionalSafe);
        CardSize.BleedSize = json["card_bleed_size"].as<double>(CardSize.BleedSize);
        CardSize.FullBleedSizeW = json["card_full_bleed_size_w"].as<double>(CardSize.FullBleedSizeW);
        CardSize.FullBleedSizeH = json["card_full_bleed_size_H"].as<double>(CardSize.FullBleedSizeH);
        VerticalOffsetPX = json["vertical_offset"].as<int>(VerticalOffsetPX);
        HorizontalOffsetPX = json["horizontal_offset"].as<int>(HorizontalOffsetPX);
        DrawCross = json["draw_cross"].as<bool>(DrawCross);
        PPI = json["ppi"].as<double>(PPI);
        OutputFile = json["output_file"].as<std::string>(OutputFile);
    }
}

void Configuration::SaveConfiguration()
{
    auto confFile = GetDir(FILES_FOLDER CONFIG_FILE);
    std::ofstream file{confFile};
    RSJresource json{"{\"Unit\": MM }"};

    json["paper_type"] = std::string(magic_enum::enum_name(PaperType));
    json["card_safe_w"] = CardSize.SafeW;
    json["card_safe_h"] = CardSize.SafeH;
    json["card_additional_safe"] = CardSize.AdditionalSafe;
    json["card_bleed_size"] = CardSize.BleedSize;
    json["card_full_bleed_size_w"] = CardSize.FullBleedSizeW;
    json["card_full_bleed_size_H"] = CardSize.FullBleedSizeH;
    json["vertical_offset"] = VerticalOffsetPX;
    json["horizontal_offset"] = HorizontalOffsetPX;
    json["draw_cross"] = DrawCross;
    json["ppi"] = PPI;
    json["output_file"] = OutputFile;
    file << json.as_str();
    file.close();
}

PageConfiguration::PageConfiguration(Configuration const& conf) {
        PH = conf.GetPaperH() - conf.GetVerticalOffset();
        PW = conf.GetPaperW() - conf.GetHorizontalOffset();

        {
            auto tot_cards_1 = (int)floor(PW / conf.GetCardWithBleedW()) * (int)floor(PH / conf.GetCardWithBleedH());
            auto tot_cards_2 = (int)floor(PW / conf.GetCardWithBleedH()) * (int)floor(PH / conf.GetCardWithBleedW());
            if (tot_cards_2 > tot_cards_1)
            {
                PH = conf.GetPaperW() - conf.GetHorizontalOffset();
                PW = conf.GetPaperH() - conf.GetVerticalOffset();
            }
        }

        Cols = (int)floor(PW / conf.GetCardW());
        Rows = (int)floor(PH / conf.GetCardH());

        CardsPerPage = Cols * Rows;
        Options.boundingBoxHeight = conf.GetCardWithBleedH();
        Options.boundingBoxWidth = conf.GetCardWithBleedW();
        Options.transformationMethod = AbstractContentContext::EImageTransformation::eFit;
        Options.fitPolicy = AbstractContentContext::EFitPolicy::eAlways;

        std::cout << "PPI:" << (conf.GetPPI()) << std::endl;
        std::cout << "PDF: " << (PW>PH ? "Landscape" : "Portrait") << " orientation" << "[" << Cols << "x" << Rows << "]" << std::endl;
        std::cout << "CARD:" << "[" << conf.GetCardW() << "x" << conf.GetCardH() << "]" << " MM[" << InchToMM(conf.GetCardW(false)) <<"x"<< InchToMM(conf.GetCardH(false)) << "]" << std::endl;
        std::cout << "BBOX: " << "[" << Options.boundingBoxWidth << "x" << Options.boundingBoxHeight << "]" << " MM[" << InchToMM(Options.boundingBoxWidth/conf.GetPPI()) <<"x"<< InchToMM(Options.boundingBoxHeight/conf.GetPPI()) << "]" << std::endl;
    }


void prepare_directories(path const& work_folder)
{
    if(!std::filesystem::exists(work_folder + FILES_FOLDER))
        std::filesystem::create_directory(work_folder +  FILES_FOLDER);

    if(!std::filesystem::exists(work_folder +  IMAGE_FOLDER))
        std::filesystem::create_directory(work_folder +  IMAGE_FOLDER);

    if(!std::filesystem::exists(work_folder + CROP_FOLDER))
        std::filesystem::create_directory(work_folder +  CROP_FOLDER);

    if(!std::filesystem::exists(work_folder +  SCRYFALL_FOLDER))
        std::filesystem::create_directory(work_folder +  SCRYFALL_FOLDER);
}

Configuration prepare_configuration(char const* work_folder)
{
    Configuration conf{};
    std::cout << work_folder << std::endl;
    conf.SetWorkDir(work_folder);
    conf.LoadConfiguration();


    prepare_directories(work_folder);
    return conf;
}