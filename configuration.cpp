#include "configuration.h"
#include "json.h"
#include "magic_enum.h"


using enum PAPER;
using enum UNIT_OF_MEASURE;

double Paper::GetPaperW(UNIT_OF_MEASURE unit, double ppu, bool doPPU) const
{
    switch (unit)
    {
    case MM:
        return WidthMM * (doPPU ? ppu : 1);
    case INCH:
        return WidthINCH * (doPPU ? ppu : 1);
    }
    return 0;
}

double Paper::GetPaperH(UNIT_OF_MEASURE unit, double ppu, bool doPPU) const
{
    switch (unit)
    {
    case MM:
        return HeightMM * (doPPU ? ppu : 1);
    case INCH:
        return HeightINCH * (doPPU ? ppu : 1);
    }
    return 0;
}

double Configuration::GetCardW(bool doPPU) const
{
    switch (Unit)
    {
    case MM:
        return CardWidthMM * (doPPU ? PPM : 1);
    case INCH:
        return CardWidthINCH * (doPPU ? PPI : 1);
    }
    return 0.;
}
double Configuration::GetCardH(bool doPPU) const
{
    switch (Unit)
    {
    case MM:
        return CardHeightMM * (doPPU ? PPM : 1);
    case INCH:
        return CardHeightINCH * (doPPU ? PPI : 1);
    }
    return 0.;
}

double Configuration::GetCardWithBleedW() const
{
    switch (Unit)
    {
    case MM:
        return (CardWidthMM + BleedMM * 2) * PPM;
    case INCH:
        return (CardWidthINCH + BleedINCH * 2) * PPI;
    }
    return 0.;
}
double Configuration::GetCardWithBleedH() const
{
    switch (Unit)
    {
    case MM:
        return (CardHeightMM + BleedMM * 2) * PPM;
    case INCH:
        return (CardHeightINCH + BleedINCH * 2) * PPI;
    }
    return 0.;
}

double Configuration::GetPaperW(bool doPPU) const
{
    switch (PaperType)
    {
    case A4:
        return A4_Paper.GetPaperW(Unit, GetPPU(), doPPU);
    case A5:
        return A5_Paper.GetPaperW(Unit, GetPPU(), doPPU);
    case A6:
        return A6_Paper.GetPaperW(Unit, GetPPU(), doPPU);
    case _10_15:
        return _10_15_Paper.GetPaperW(Unit, GetPPU(), doPPU);
    case _13_18:
        return _13_18_Paper.GetPaperW(Unit, GetPPU(), doPPU);
    }
    return 0.;
}

double Configuration::GetPaperH(bool doPPU) const
{
    switch (PaperType)
    {
    case A4:
        return A4_Paper.GetPaperH(Unit, GetPPU(), doPPU);
    case A5:
        return A5_Paper.GetPaperH(Unit, GetPPU(), doPPU);
    case A6:
        return A6_Paper.GetPaperH(Unit, GetPPU(), doPPU);
    case _10_15:
        return _10_15_Paper.GetPaperH(Unit, GetPPU(), doPPU);
    case _13_18:
        return _13_18_Paper.GetPaperH(Unit, GetPPU(), doPPU);
    }
    return 0.;
}

double Configuration::GetBleed(bool doPPU) const
{
    switch (Unit)
    {
    case MM:
        return BleedMM * (doPPU ? PPM : 1);
    case INCH:
        return BleedINCH * (doPPU ? PPI : 1);
    }
    return 0.;
}

double Configuration::GetFullBleed(bool doPPU) const
{
    switch (Unit)
    {
    case MM:
        return FullBleedMM * (doPPU ? PPM : 1);
    case INCH:
        return FullBleedINCH * (doPPU ? PPI : 1);
    }
    return 0.;
}

double Configuration::GetPPU() const
{
    switch (Unit)
    {
    case MM:
        return PPM;
    case INCH:
        return PPI;
    }
    return 0.;
}

int Configuration::GetVerticalOffset() const { return VerticalOffsetPX; }
int Configuration::GetHorizontalOffset() const { return HorizontalOffsetPX; }

bool Configuration::GetDrawCross() const
{
    return DrawCross;
}

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
        Unit = *magic_enum::enum_cast<UNIT_OF_MEASURE>(json["Unit"].as<std::string>("MM"));
        PaperType = *magic_enum::enum_cast<PAPER>(json["paper_type"].as<std::string>("A4"));
        CardWidthMM = json["card_w_mm"].as<double>(CardWidthMM);
        CardWidthINCH = json["card_w_inch"].as<double>(CardWidthINCH);
        CardHeightMM = json["card_h_mm"].as<double>(CardHeightMM);
        CardHeightINCH = json["card_h_inch"].as<double>(CardHeightINCH);
        BleedMM = json["bleed_mm"].as<double>(BleedMM);
        BleedINCH = json["bleed_inch"].as<double>(BleedINCH);
        FullBleedMM = json["full_bleed_mm"].as<double>(FullBleedMM);
        FullBleedINCH = json["full_bleed_inch"].as<double>(FullBleedINCH);
        VerticalOffsetPX = json["vertical_offset"].as<int>(VerticalOffsetPX);
        HorizontalOffsetPX = json["horizontal_offset"].as<int>(HorizontalOffsetPX);
        DrawCross = json["draw_cross"].as<bool>(DrawCross);
        PPI = json["ppi"].as<double>(PPI);
        PPM = json["ppm"].as<double>(PPM);
        OutputFile = json["output_file"].as<std::string>(OutputFile);
    }
}

void Configuration::SaveConfiguration(std::filesystem::path const &confFile)
{
    std::ofstream file{confFile};
    RSJresource json{"{\"Unit\": MM }"};
    json["Unit"] = std::string(magic_enum::enum_name(Unit));
    json["paper_type"] = std::string(magic_enum::enum_name(PaperType));
    json["card_w_mm"] = CardWidthMM;
    json["card_w_inch"] = CardWidthINCH;
    json["card_h_mm"] = CardHeightMM;
    json["card_h_inch"] = CardHeightINCH;
    json["bleed_mm"] = BleedMM;
    json["bleed_inch"] = BleedINCH;
    json["full_bleed_mm"] = FullBleedMM;
    json["full_bleed_inch"] = FullBleedINCH;
    json["vertical_offset"] = VerticalOffsetPX;
    json["horizontal_offset"] = HorizontalOffsetPX;
    json["draw_cross"] = DrawCross;
    json["ppi"] = PPI;
    json["ppm"] = PPM;
    json["output_file"] = OutputFile;
    file << json.as_str();
    file.close();
}