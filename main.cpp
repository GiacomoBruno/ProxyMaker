#include "configuration.h"
#include "image_processing.h"
#include "pdf_generation.h"
#include "scryfall.h"

int main(int argc, char **argv)
{
    auto conf = prepare_configuration(argc == 2 ? argv[1] : ".");
    
    PrepareScryfallCards(conf);
    CropImages(conf, LoadImages(conf.GetDir(IMAGE_FOLDER)), conf.GetDir(CROP_FOLDER));
    auto images = LoadImages(conf.GetDir(CROP_FOLDER));
    GeneratePDF(conf, images, conf.GetDir(conf.GetOutputFile()));
    conf.SaveConfiguration();
}