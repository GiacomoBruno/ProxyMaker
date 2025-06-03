#include <iostream>
#include "configuration.h"
#include "image_processing.h"
#include "pdf_generation.h"
#include "scryfall.h"

using namespace PDFHummus;


int main(int argc, char **argv)
{
    WorkFolder wf;
    wf.WF = std::filesystem::absolute(argv[1]);


    Configuration conf{};
    std::cout << "WF: " << wf.WF << std::endl;
    std::cout << "WFa: " << std::filesystem::absolute(wf.WF) << std::endl;
    conf.LoadConfiguration(wf.Get(FILES_FOLDER).string() + CONFIG_FILE);
    conf.WF = wf;
    auto out = wf.Get(CROP_FOLDER);
    std::cout << "RESET CROP_FOLDER: " << out <<std::endl;

    if (std::filesystem::exists(out))
        std::filesystem::remove_all(out);
    std::filesystem::create_directory(out);
    std::cout << "END RESET CROP_FOLDER" <<std::endl;

    DownloadList(wf.Get(FILES_FOLDER).string() + "card_list.txt");
    
    CropImages(conf, LoadImages(wf.Get(IMAGE_FOLDER)), wf.Get(CROP_FOLDER));
    ResizeScryfallImages(conf);
    GeneratePDF(conf, LoadImages(wf.Get(CROP_FOLDER)), wf.Get(conf.GetOutputFile().c_str()));
    conf.SaveConfiguration(wf.Get(FILES_FOLDER).string() + CONFIG_FILE);
    int wait;
    std::cin >> wait;
}