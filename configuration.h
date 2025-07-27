#pragma once

#include "PDFWriter/PageContentContext.h"
#include "paper.h"
#include "utility.h"

#include <math.h>
#include <string>
#include <iostream>
#include <unordered_map>
//everything in inches becasue MTG is U.S. based (sigh.)

#ifdef __WIN32__
#define STR_PREFIX L
#else 
#define STR_PREFIX 
#endif

#define FILES_FOLDER STR_PREFIX"/files/"
#define BIN_FOLDER STR_PREFIX"/bin/"
#define IMAGE_FOLDER FILES_FOLDER STR_PREFIX"images/"
#define CROP_FOLDER IMAGE_FOLDER STR_PREFIX"crop/"
#define SCRYFALL_FOLDER IMAGE_FOLDER STR_PREFIX"scryfall/"
#define SCRYFALL_INPUT_FOLDER SCRYFALL_FOLDER STR_PREFIX"input/"
#define SCRYFALL_UPSCALED_FOLDER SCRYFALL_FOLDER STR_PREFIX"upscaled/"
#define SCRYFALL_BLED_FOLDER SCRYFALL_FOLDER STR_PREFIX"bled/"
#define MODELS_FOLDER FILES_FOLDER STR_PREFIX"models/"

#define CONFIG_FILE STR_PREFIX"config.json"
#define SCRYFALL_FILE STR_PREFIX"card_list.txt"
#define TMP_FILE STR_PREFIX"tmpfile.tmp"

struct Configuration
{
private:
    PAPER PaperType{PAPER::eA4};
    CardSizes CardSize{MPCFillCard};
    bool DrawCross{true};
    double PPI{72.};
    std::filesystem::path OutputFile{FILES_FOLDER STR_PREFIX"output.pdf"};
    std::filesystem::path WorkFolder{};
    
public:
    CardSizes const& GetCardSize() const;
    PAPER GetPaperType() const;
    bool GetDrawCross() const;
    double GetPPI() const;
    std::filesystem::path GetOutputFile() const;
    path GetWorkDir() const;
    path GetDir(path const&) const;
    void SetWorkDir(path const&);

    std::unordered_map<path, int> PrintList{};
};

Configuration prepare_configuration(char const* work_folder);

double GetCardW(Configuration const& c);
double GetCardH(Configuration const& c);
double GetFullCardW(Configuration const& c);
double GetFullCardH(Configuration const& c);
double GetCardBleed(Configuration const& c);
double GetMarginCardW(Configuration const& c);
double GetMarginCardH(Configuration const& c);
double GetMargin(Configuration const& c);
