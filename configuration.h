#pragma once
#include <string>
#include <filesystem>


enum class PAPER
{
    A4,
    A5,
    A6,
    _10_15,
    _13_18,
};


enum class UNIT_OF_MEASURE {
    MM, INCH
};


struct Paper
{
    const double WidthMM{210};
    const double HeightMM{297};
    const double WidthINCH{8.3};
    const double HeightINCH{11.7};

    double GetPaperW(UNIT_OF_MEASURE, double ppu, bool doPPU = true) const;
    double GetPaperH(UNIT_OF_MEASURE, double ppu, bool doPPU = true) const;

};

constexpr Paper A4_Paper{.WidthMM{210.}, .HeightMM{297.}, .WidthINCH{8.3},.HeightINCH{11.7}};
constexpr Paper A5_Paper{.WidthMM{148.}, .HeightMM{210.}, .WidthINCH{5.8},.HeightINCH{8.3}};
constexpr Paper A6_Paper{.WidthMM{105.}, .HeightMM{148.}, .WidthINCH{4.1},.HeightINCH{5.8}};
constexpr Paper _10_15_Paper{.WidthMM{102.}, .HeightMM{152.}, .WidthINCH{4.},.HeightINCH{6.}};
constexpr Paper _13_18_Paper{.WidthMM{127.}, .HeightMM{177.8}, .WidthINCH{5.},.HeightINCH{7.}};

struct Configuration
{
private:
    UNIT_OF_MEASURE Unit{UNIT_OF_MEASURE::MM};
    PAPER PaperType{PAPER::A4};
    double CardWidthMM{63.};
    double CardWidthINCH{2.49};
    double CardHeightMM{88.};
    double CardHeightINCH{3.48};
    double BleedMM{0.762};
    double BleedINCH{0.03};
    double FullBleedMM{2.921};
    double FullBleedINCH{0.115};
    int VerticalOffsetPX{50};
    int HorizontalOffsetPX{30};
    bool DrawCross{true};
    double PPI{72.};
    double PPM{11.8095238};
    std::string OutputFile{".\\files\\output.pdf"};
    
public:
    void LoadConfiguration(std::filesystem::path const& confFile);
    void SaveConfiguration(std::filesystem::path const& confFile);

    double GetCardW(bool ppu = true) const;
    double GetCardH(bool ppu = true) const;
    double GetCardWithBleedW() const;
    double GetCardWithBleedH() const;
    double GetPaperW(bool ppu = true) const;
    double GetPaperH(bool ppu = true) const;
    double GetBleed(bool ppu = true) const;
    double GetFullBleed(bool ppu = true) const;
    double GetPPU() const;
    int GetVerticalOffset() const;
    int GetHorizontalOffset() const;
    bool GetDrawCross() const;
    std::string GetOutputFile() const;
};

