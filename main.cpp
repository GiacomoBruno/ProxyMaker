#include <iostream>
#include "configuration.h"
#include "image_processing.h"
#include "pdf_generation.h"

using namespace PDFHummus;


int main(int argc, char **argv)
{
    Configuration conf{};
    conf.LoadConfiguration(".\\files\\config.json");

    auto out = std::filesystem::absolute(".\\files\\images\\crop\\");
    if (std::filesystem::exists(out))
        std::filesystem::remove_all(out);
    std::filesystem::create_directory(out);

    CropImages(conf, LoadImages(".\\files\\images\\"), ".\\files\\images\\crop\\");
    ResizeScryfallImages(conf);
    GeneratePDF(conf, LoadImages(".\\files\\images\\crop\\"), conf.GetOutputFile());
    conf.SaveConfiguration(".\\files\\config.json");
    int wait;
    std::cin >> wait;
}