#include "configuration.h"
#include "image_processing.h"
#include "pdf_generation.h"

int main(int argc, char **argv)
{
    auto conf = prepare_configuration(argc == 2 ? argv[1] : ".");
    PrepareDirectory(conf.GetDir(CROP_FOLDER), true);

    PrintConfiguration(conf);
    
    UpdateConfiguration(conf);

    PadImages(conf, conf.GetDir(UNPADDED_IMAGE_INPUT), conf.GetDir(IMAGE_INPUT));
    CropImages(conf, LoadImages(conf.GetDir(IMAGE_INPUT), true, true), conf.GetDir(CROP_FOLDER));
    CropImages(conf, LoadImages(conf.GetDir(IMAGE_FULL_INPUT), true, true), conf.GetDir(CROP_FOLDER), true);
    auto images = PrepCards(LoadImages(conf.GetDir(CROP_FOLDER)));
    GeneratePDF(conf, images, conf.GetDir(conf.GetOutputFile()));
    conf.Cleanup();
}