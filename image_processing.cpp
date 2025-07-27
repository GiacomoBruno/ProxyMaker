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

void CropImages(Configuration const &conf, std::vector<path> const &images, path const &output_folder)
{
    std::vector<std::thread> threads{};
    auto out = std::filesystem::absolute(output_folder);
    BS::thread_pool pool{12};
    for (auto const &p : images)
    {
        std::ignore = pool.submit_task(
            [conf, p, out]()
            {
                cv::Mat img = cv::imread(p);
                auto w = img.size().width;
                auto h = img.size().height;

                auto imageDPI = std::min(w / GetFullCardW(conf), h / GetFullCardH(conf));
                auto toCrop = std::min(GetFullCardW(conf) - GetCardW(conf), GetFullCardH(conf) - GetCardH(conf));
                toCrop -= (GetCardBleed(conf) * 2);
                toCrop /= 2.;
                toCrop *= imageDPI;

                cv::Mat cropped_image = img(cv::Range(toCrop, h - toCrop), cv::Range(toCrop, w - toCrop));
                auto cropped = out / std::filesystem::path(p).filename();
                cv::imwrite(cropped.string(), cropped_image);
            });
    }

    pool.wait();
}

bool Upscale(Configuration const &conf, path const &input, path const &output)
{
    auto command = std::format("{}{} -i {} -o {} -s 2 -m {} -n {}",
                                conf.GetDir(BIN_FOLDER).string(), REALESRGAN,
                                std::filesystem::absolute(input).string(), std::filesystem::absolute(output).string(), conf.GetDir(MODELS_FOLDER).string(), "realesr-animevideov3");

    return RunCommand(command, true);
}

void FillEmptySpaces(path const &input)
{
    for (auto image : LoadImages(input))
    {
        auto i = cv::imread(image);
        uint8_t *pixelPtr = (uint8_t *)i.data;
        if(pixelPtr == nullptr)
        {
            std::cout << "failed to read image: " << image << std::endl;
            return;
        }

        int pos = 20;
        if (pixelPtr[0] == pixelPtr[1] && pixelPtr[1] == pixelPtr[2] && pixelPtr[1] == 255)
        {
            auto channels = i.channels();

            auto idx = pos * channels * (i.cols + 1);
            auto idx2 = i.cols * channels * (i.rows - pos) + (pos * channels);
            auto top = cv::Mat(32, 32, CV_8UC3, cv::Vec3b(pixelPtr[idx], pixelPtr[idx + 1], pixelPtr[idx + 2]));
            auto bot = cv::Mat(32, 32, CV_8UC3, cv::Vec3b(pixelPtr[idx2], pixelPtr[idx2 + 1], pixelPtr[idx2 + 2]));

            auto d1 = i(cv::Rect(0, 0, 32, 32));
            auto d2 = i(cv::Rect(i.cols - 32, 0, 32, 32));
            auto d3 = i(cv::Rect(0, i.rows - 32, 32, 32));
            auto d4 = i(cv::Rect(i.cols - 32, i.rows - 32, 32, 32));

            top.copyTo(d1);
            top.copyTo(d2);

            bot.copyTo(d3);
            bot.copyTo(d4);
            cv::imwrite(image, i);
        }
    }
}

void AddBleed(Configuration const& conf, path const& input, path const& output)
{
    BS::thread_pool pool{12};
    for (auto image : LoadImages(input))
    {
        std::ignore = pool.submit_task([image, &conf, &output]()
                                       {

            if(std::filesystem::exists(output.string() + std::filesystem::path(image).filename().string())) return;
            
            auto i = cv::imread(image);

            //todo add inch to mm conversions.
            auto dpi = std::min(i.size().width / GetCardW(conf), i.size().height / GetCardH(conf));
            int pixels_to_add = (int)std::floor(std::min(GetFullCardW(conf) - GetCardW(conf), GetFullCardH(conf) - GetCardH(conf)) * dpi);
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

            cv::imwrite(output.string() + std::filesystem::path(image).filename().string(), img); });
    }

    pool.wait();
}
