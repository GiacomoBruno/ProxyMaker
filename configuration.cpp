#include "configuration.h"
#include "magic_enum.hpp"
#include "json.h"

using enum PAPER;

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

void Configuration::LoadConfiguration(std::filesystem::path const &confFile)
{
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

void Configuration::SaveConfiguration(std::filesystem::path const &confFile)
{
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