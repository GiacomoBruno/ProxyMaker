#pragma once

#include "PDFWriter/PageContentContext.h"
#include "paper.h"
#include "utility.h"

#include <math.h>
#include <string>
#include <iostream>
#include <unordered_map>
#include <mutex>
//everything in inches becasue MTG is U.S. based (sigh.)

#ifdef WIN32
#define STR_PREFIX(s) L##s
#define tostr(i) std::to_wstring(i)
#else 
#define STR_PREFIX 
#define tostr(i) std::to_string(i)
#endif

#define FILES_FOLDER STR_PREFIX("/files/")
#define BIN_FOLDER STR_PREFIX("/bin/")
#define IMAGE_FOLDER FILES_FOLDER STR_PREFIX("images/")
#define IMAGE_INPUT IMAGE_FOLDER STR_PREFIX("input/")
#define IMAGE_FULL_INPUT IMAGE_FOLDER STR_PREFIX("fullart/")
#define UNPADDED_IMAGE_INPUT IMAGE_FOLDER STR_PREFIX("to_pad_input")
#define CROP_FOLDER IMAGE_FOLDER STR_PREFIX("crop/")
#define TMP_FILE STR_PREFIX("tmpfile.tmp")
#define TEMPLATES_FOLDER STR_PREFIX("/templates/")
#define TEMPLATE TEMPLATES_FOLDER STR_PREFIX("a4_template.png")

struct Configuration
{
private:
    PAPER PaperType{PAPER::eA4};
    CardSizes CardSize{MPCFillCard};
    double PPI{72.};
    int16_t Spacing = 0u;
    std::filesystem::path OutputFile{FILES_FOLDER STR_PREFIX("output.pdf")};
    std::filesystem::path WorkFolder{};
    bool UseSilhouetteTemplate {false};
    bool DrawCutLines{true};

    mutable std::vector<std::filesystem::path> GeneratedFiles{};
    mutable std::mutex* m_mutex{new std::mutex};
public:
    CardSizes const& GetCardSize() const;
    PAPER GetPaperType() const;
    double GetPPI() const;
    std::filesystem::path GetOutputFile() const;
    int16_t GetSpacing() const;
    path GetWorkDir() const;
    path GetDir(path const&) const;
    void SetWorkDir(path const&);

    bool GetUseSilhouetteTemplate() const;
    bool GetDrawCutLines() const;
    std::unordered_map<path, int> PrintList{};
    void AddGeneratedFile(std::filesystem::path const&) const;
    void Cleanup() const;

    friend void PrintConfiguration(Configuration const& conf);
    friend void UpdateConfiguration(Configuration& conf);
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


