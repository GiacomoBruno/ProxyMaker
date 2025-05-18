#include "image_processing.h"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#define SyncOut std::osyncstream{std::cout}

void CropImages(Configuration const &conf, std::vector<std::filesystem::path> const &images, std::filesystem::path const &output_folder)
{
    std::vector<std::thread> threads{};
    auto out = std::filesystem::absolute(output_folder);
    for (auto const &p : images)
    {
        threads.push_back(std::thread{
            [conf, p, out]()
            {
                cv::Mat img = cv::imread(p.string());
                auto w = img.size().width;
                auto h = img.size().height;
                
                auto imageDPI = std::min(w / conf.GetFullBleedW(false), h / conf.GetFullBleedH(false));
                auto toCrop = std::min(conf.GetFullBleedW(false) - conf.GetCardW(false), conf.GetFullBleedH(false) - conf.GetCardH(false));
                toCrop -= (conf.GetBleed(false) * 2);
                toCrop /= 2.;
                toCrop *= imageDPI;

                cv::Mat cropped_image = img(cv::Range(toCrop, h - toCrop), cv::Range(toCrop, w - toCrop));
                auto cropped = out / p.filename();

                cv::imwrite(cropped.string(), cropped_image);
            }});
    }

    for (auto &&t : threads)
    {
        t.join();
    }
}


void ResizeScryfallImages(Configuration const& conf)
{
    if(std::filesystem::exists(".\\files\\images\\scryfall_expanded"))
        std::filesystem::remove_all(".\\files\\images\\scryfall_expanded");

    std::filesystem::create_directory(".\\files\\images\\scryfall_expanded");

    auto images = LoadImages(".\\files\\images\\scryfall_upscaled\\");
    for(auto image : images)
    {
        auto i = cv::imread(image.string());
        auto dpi = std::min(i.size().width / conf.GetCardW(false), i.size().height / conf.GetCardH(false));
        int pixels_to_add = (int)std::floor(std::min(conf.GetFullBleedW(false) - conf.GetCardW(false), conf.GetFullBleedH(false) - conf.GetCardH(false)) * dpi);
        pixels_to_add += pixels_to_add % 2;
        auto new_w = i.size().width + pixels_to_add;
        auto new_h = i.size().height + pixels_to_add;
        cv::Mat img = cv::Mat(new_h,new_w, CV_8UC3, cv::Scalar(0,0,0)); 
        
        auto dest = img(cv::Rect(pixels_to_add /2, pixels_to_add/2, i.size().width, i.size().height));
        i.copyTo(dest);
        auto top_to_copy = i(cv::Rect(0,0, i.size().width, 1));
        auto bot_to_copy = i(cv::Rect(0,i.size().height - 1, i.size().width, 1));
        for(int n = 0; n < pixels_to_add/2; n++)
        {
            auto top_dest = img(cv::Rect(pixels_to_add/2, n, i.size().width, 1));
            auto bot_dest = img(cv::Rect(pixels_to_add/2, img.size().height - (n+1), i.size().width, 1));

            top_to_copy.copyTo(top_dest);
            bot_to_copy.copyTo(bot_dest);
        }

        auto side1_to_copy = img(cv::Rect(pixels_to_add/2,0,1, img.size().height));
        auto side2_to_copy = img(cv::Rect(img.size().width - pixels_to_add/2-1,0,1, img.size().height));

        for(int n = 0; n < pixels_to_add/2; n++)
        {
            auto side1_dest = img(cv::Rect(n, 0, 1, img.size().height));
            auto side2_dest = img(cv::Rect(img.size().width-(n+1), 0, 1, img.size().height));

            side1_to_copy.copyTo(side1_dest);
            side2_to_copy.copyTo(side2_dest);
        }
        
        cv::imwrite(".\\files\\images\\scryfall_expanded\\" + image.filename().string(),img);
    }

    CropImages(conf, LoadImages(".\\files\\images\\scryfall_expanded\\"), ".\\files\\images\\crop\\");
}