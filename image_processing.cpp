#include "image_processing.h"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <format>
#include <optional>
#include "BS_thread_pool.hpp"

#define SyncOut \
    std::osyncstream { std::cout }

#ifdef __linux__ 
#define REALESRGAN "realesrgan_ubuntu"
#elif _WIN32
#define REALESRGAN "realesrgan_win.exe"
#elif __APPLE__
#define REALESRGAN "realesrgan_mac"
#else
#define REALESRGAN ""
#endif

double ColorShift(cv::Point3_<uchar> c1, cv::Point3_<uchar> c2)
{   
    
    auto res = c1.x - c2.x + c1.y - c2.y + c1.z - c2.z;
    return res < 0 ? -res : res;
}

void CropImages(Configuration const &conf, std::vector<path> const &images, path const &output_folder, bool fullart)
{
    std::vector<std::thread> threads{12};
    auto out = std::filesystem::absolute(output_folder);
    BS::thread_pool pool{1};
    for (auto const &p : images)
    {
        std::ignore = pool.submit_task(
            [&conf, p, out, fullart]()
            {
                cv::Mat img = cv::imread(p.string());
                auto w = img.size().width;
                auto h = img.size().height;

                auto imageDPI = std::min(w / GetPrintSize(GetFullCardW(conf), 1), h / GetPrintSize(GetFullCardH(conf), 1));
                auto toCrop = GetCardBleed(conf);
                toCrop -= (GetMargin(conf));
                if(fullart) toCrop -= 0.4; //for some reason full art images from mpcfill have a bigger bleed it seems.
                toCrop = GetPrintSize(toCrop, imageDPI);

                double toCropSide = toCrop;
                double toCropTop = toCrop;

                using color = cv::Point3_<uchar>;
                if(!fullart){
                    auto mid_h = img.size().height / 2;
                    auto mid_w = img.size().width / 2;

                    auto side_border_start = GetPrintSize(GetCardBleed(conf), imageDPI);
                    auto side_border_color = img.at<color>(mid_h, side_border_start);
                    color nextSideColor{};
                    auto top_border_start = GetPrintSize(GetCardBleed(conf), imageDPI);
                    auto top_border_color = img.at<color>(top_border_start, mid_w);
                    color nextTopColor{};

                    int i1 = 1;
                    int i2 = 1;

                    bool side_not_end = true;
                    bool top_not_end = true;

                    while(side_not_end || top_not_end)
                    {
                        if(side_not_end)
                        {
                            if(ColorShift(nextSideColor = img.at<color>(mid_h, side_border_start + i1), side_border_color) < 10)
                            {
                                i1++;
                            }
                            else{
                                side_not_end = false;
                            }
                        }
                        if(top_not_end)
                        {
                            if(ColorShift(nextSideColor = img.at<color>(top_border_start + i2, mid_w), top_border_color) < 10)
                            {
                                i2++;
                            }
                            else{
                                top_not_end = false;
                            }
                        }
                    }


                    double top_border = (i2 / imageDPI) * 25.4;
                    double side_border = (i1 / imageDPI) * 25.4;

                    double border_in_mm = top_border < side_border ? top_border : side_border;
                    if(top_border < 1.9)
                        border_in_mm = side_border; //for legend crown case

                    if(side_border < 2.5) //border is either 2.5mm (modern card), or 3.5mm (old cards)
                    {
                        auto dif = 2.5 - side_border;
                        toCropSide = GetPrintSize(GetCardBleed(conf) - dif - GetMargin(conf), imageDPI);
                    }
                    if(top_border < 2.6 && top_border > 2.0)
                    {
                        auto dif = 2.6 - top_border;
                        toCropTop = GetPrintSize(GetCardBleed(conf) - dif - GetMargin(conf), imageDPI);
                    }
                    else if(top_border < 2.0)
                    {
                        auto dif = 2.6 - side_border;
                        toCropTop = GetPrintSize(GetCardBleed(conf) - dif - GetMargin(conf), imageDPI);
                    }

                    if(side_border > 3.1) //border is either 2.5mm (modern card), or 3.5mm (old cards)
                    {
                        auto dif = 3.1 - side_border;
                        toCropSide = GetPrintSize(GetCardBleed(conf) - dif - GetMargin(conf), imageDPI);
                    }
                    if(top_border > 3.5)
                    {
                        auto dif = 3.5 - top_border;
                        toCropTop = GetPrintSize(GetCardBleed(conf) - dif - GetMargin(conf), imageDPI);
                    }
                }

                if(toCropTop < 0)
                    toCropTop = toCrop;
                if(toCropSide < 0)
                    toCropSide = toCrop;
                cv::Mat cropped_image = img(cv::Range(toCropTop, h - toCropTop), cv::Range(toCropSide, w - toCropSide));
                auto cropped = out / std::filesystem::path(p).filename();
                cv::imwrite(cropped.string(), cropped_image);
                conf.AddGeneratedFile(cropped);
            });
    }

    pool.wait();
}

void PadImages(Configuration const& conf, path const& input, path const& output)
{
    BS::thread_pool pool{12};
    for (auto image : LoadImages(input, true))
    {
        std::ignore = pool.submit_task([image, &conf, &output]()
                                       {

            if(std::filesystem::exists(output.string() + std::filesystem::path(image).filename().string())) return;
            
            auto i = cv::imread(image.string());

            //todo add inch to mm conversions.
            auto dpi = std::min(i.size().width / GetCardW(conf), i.size().height / GetCardH(conf));
            int pixels_to_add = (int)std::floor(std::min(GetFullCardW(conf) - GetCardW(conf) - (GetMargin(conf)*2), GetFullCardH(conf) - GetCardH(conf) - (GetMargin(conf)*2)) * dpi);
            pixels_to_add += pixels_to_add % 2;
            auto new_w = i.size().width + pixels_to_add;
            auto new_h = i.size().height + pixels_to_add;
            cv::Mat img = cv::Mat(new_h, new_w, CV_8UC3, cv::Scalar(0, 0, 0));

            auto dest = img(cv::Rect(pixels_to_add / 2, pixels_to_add / 2, i.size().width, i.size().height));
            i.copyTo(dest);
            auto top_to_copy = i(cv::Rect(0, 0, i.size().width, 1));
            auto bot_to_copy = i(cv::Rect(0, i.size().height - 1, i.size().width, 1));
            for (int n = 0; n < pixels_to_add / 2; n++)
            {
                auto top_dest = img(cv::Rect(pixels_to_add / 2, n, i.size().width, 1));
                auto bot_dest = img(cv::Rect(pixels_to_add / 2, img.size().height - (n + 1), i.size().width, 1));

                top_to_copy.copyTo(top_dest);
                bot_to_copy.copyTo(bot_dest);
            }

            auto side1_to_copy = img(cv::Rect(pixels_to_add / 2, 0, 1, img.size().height));
            auto side2_to_copy = img(cv::Rect(img.size().width - pixels_to_add / 2 - 1, 0, 1, img.size().height));

            for (int n = 0; n < pixels_to_add / 2; n++)
            {
                auto side1_dest = img(cv::Rect(n, 0, 1, img.size().height));
                auto side2_dest = img(cv::Rect(img.size().width - (n + 1), 0, 1, img.size().height));

                side1_to_copy.copyTo(side1_dest);
                side2_to_copy.copyTo(side2_dest);
            }
            auto outpath = output.string() + std::filesystem::path(image).filename().string();
            cv::imwrite(outpath, img);
            conf.AddGeneratedFile(outpath);

        });
    }

    pool.wait();
}