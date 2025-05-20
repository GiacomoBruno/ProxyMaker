#include <iostream>
#include "configuration.h"
#include "image_processing.h"
#include "pdf_generation.h"

using namespace PDFHummus;


int wmain(int argc, wchar_t **argv)
{
    Configuration conf{};
    conf.LoadConfiguration(FILES_FOLDER CONFIG_FILE);

    auto out = std::filesystem::absolute(CROP_FOLDER);
    if (std::filesystem::exists(out))
        std::filesystem::remove_all(out);
    std::filesystem::create_directory(out);

    CropImages(conf, LoadImages(IMAGE_FOLDER), CROP_FOLDER);
    ResizeScryfallImages(conf);
    GeneratePDF(conf, LoadImages(CROP_FOLDER), conf.GetOutputFile());
    conf.SaveConfiguration(FILES_FOLDER CONFIG_FILE);
    int wait;
    std::cin >> wait;
}