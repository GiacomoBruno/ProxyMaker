#include "configuration.h"
#include "image_processing.h"
#include "pdf_generation.h"

int main(int argc, char **argv)
{
    auto conf = prepare_configuration(argc == 2 ? argv[1] : ".");
    PrepareDirectory(conf.GetDir(CROP_FOLDER), true);
    CropImages(conf, LoadImages(conf.GetDir(IMAGE_FOLDER), true), conf.GetDir(CROP_FOLDER));
    auto images = PrepCards(LoadImages(conf.GetDir(CROP_FOLDER)));
    GeneratePDF(conf, images, conf.GetDir(conf.GetOutputFile()));
}