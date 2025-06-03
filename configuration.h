#pragma once
#include <string>
#include <filesystem>
#include <iostream>
#include "PDFWriter/PageContentContext.h"
#include "paper.h"

//everything in inches becasue MTG is U.S. based (sigh.)

#define FILES_FOLDER "/files/"
#define BIN_FOLDER "/bin/"
#define IMAGE_FOLDER FILES_FOLDER "images/"
#define CROP_FOLDER IMAGE_FOLDER "crop/"
#define SCRYFALL_FOLDER IMAGE_FOLDER "scryfall/"
#define SCRYFALL_INPUT_FOLDER SCRYFALL_FOLDER "input"
#define SCRYFALL_UPSCALED_FOLDER SCRYFALL_FOLDER "upscaled"
#define SCRYFALL_BLEEDED_FOLDER SCRYFALL_FOLDER "bleeded/"
#define CONFIG_FILE "config.json"
#define MODELS_FOLDER FILES_FOLDER "models/"

struct WorkFolder
{
    std::filesystem::path WF;

    std::filesystem::path Get(char const* pth) const { 
        
        return WF.string() + pth; }
};

struct CardMeasures
{
    double SafeW{};
    double SafeH{};
    double AdditionalSafe{};
    double BleedSize{};
    double FullBleedSizeW{};
    double FullBleedSizeH{};

};

constexpr CardMeasures MeasuresMPCFill{.SafeW = 2.48, .SafeH = 3.49, .AdditionalSafe = 0.00, .BleedSize = 0.03, .FullBleedSizeW = 2.74, .FullBleedSizeH = 3.74};

struct Configuration
{
private:
    PAPER PaperType{PAPER::A4};
    CardMeasures CardSize{MeasuresMPCFill};
    int VerticalOffsetPX{0};
    int HorizontalOffsetPX{0};
    bool DrawCross{true};
    double PPI{300.};
    double CrossThickness{0.01};
    double CrossLength{0.1};
    std::string OutputFile{FILES_FOLDER"output.pdf"};
    public: WorkFolder WF;
    
public:
    void LoadConfiguration(std::filesystem::path const& confFile);
    void SaveConfiguration(std::filesystem::path const& confFile);

    double GetCardW(bool ppu = true) const;
    double GetCardH(bool ppu = true) const;
    double GetCardWithBleedW(bool ppu = true) const;
    double GetCardWithBleedH(bool ppu = true) const;
    double GetPaperW(bool ppu = true) const;
    double GetPaperH(bool ppu = true) const;
    double GetBleed(bool ppu = true) const;
    double GetFullBleedW(bool ppu = true) const;
    double GetFullBleedH(bool ppu = true) const; 
    int GetVerticalOffset() const;
    int GetHorizontalOffset() const;
    bool GetDrawCross() const;
    double GetCrossThickness(bool ppu = true) const;
    double GetCrossLength(bool ppu = true) const;
    double GetPPI() const;
    std::string GetOutputFile() const;
};

constexpr double InchToMM(double inch) { return inch * 25.4; } 


struct PageConfiguration
{
    PageConfiguration(Configuration const& conf) {
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

    int CardsPerPage{};
    int Rows{};
    int Cols{};
    double PW{};
    double PH{};
    AbstractContentContext::ImageOptions Options{};
};
