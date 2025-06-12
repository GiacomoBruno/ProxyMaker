#pragma once

#include "PDFWriter/PageContentContext.h"
#include "paper.h"
#include "utility.h"

#include <math.h>
#include <string>
#include <iostream>
#include <unordered_map>
//everything in inches becasue MTG is U.S. based (sigh.)

#define FILES_FOLDER "/files/"
#define BIN_FOLDER "/bin/"
#define IMAGE_FOLDER FILES_FOLDER "images/"
#define CROP_FOLDER IMAGE_FOLDER "crop/"
#define SCRYFALL_FOLDER IMAGE_FOLDER "scryfall/"
#define SCRYFALL_INPUT_FOLDER SCRYFALL_FOLDER "input/"
#define SCRYFALL_UPSCALED_FOLDER SCRYFALL_FOLDER "upscaled/"
#define SCRYFALL_BLED_FOLDER SCRYFALL_FOLDER "bled/"
#define MODELS_FOLDER FILES_FOLDER "models/"

#define CONFIG_FILE "config.json"
#define SCRYFALL_FILE "card_list.txt"
#define TMP_FILE "tmpfile.tmp"



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
    std::string WorkFolder{};
    
public:
    void LoadConfiguration();
    void SaveConfiguration();

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
    path GetWorkDir() const;
    path GetDir(path const&) const;
    void SetWorkDir(path const&);

    std::unordered_map<path, int> PrintList{};
};

Configuration prepare_configuration(char const* work_folder);



struct PageConfiguration
{
    PageConfiguration(Configuration const& conf);

    int CardsPerPage{};
    int Rows{};
    int Cols{};
    double PW{};
    double PH{};
    AbstractContentContext::ImageOptions Options{};
};
