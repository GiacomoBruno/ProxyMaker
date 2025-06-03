#include "image_processing.h"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <format>
#include <optional>
#include "BS_thread_pool.hpp"

#define SyncOut \
    std::osyncstream { std::cout }

void CropImages(Configuration const &conf, std::vector<std::filesystem::path> const &images, std::filesystem::path const &output_folder)
{
    std::vector<std::thread> threads{};
    auto out = std::filesystem::absolute(output_folder);
    BS::thread_pool pool{12};
    for (auto const &p : images)
    {
        std::ignore = pool.submit_task(
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
            });
    }

    pool.wait();
}

bool Upscale(Configuration const &conf, std::filesystem::path const &input, std::filesystem::path const &output)
{
    auto command = std::format("{}realesrgan-ncnn-vulkan -i {} -o {} -s 2 -m .{} -n {}",
                                conf.WF.Get(BIN_FOLDER).string(),
                                std::filesystem::absolute(input).string(), std::filesystem::absolute(output).string(), MODELS_FOLDER, "realesr-animevideov3");

    return RunCommand(command);
}

void FillEmptySpaces(std::filesystem::path const &input)
{
    for (auto image : LoadImages(input))
    {
        auto i = cv::imread(image.string());
        uint8_t *pixelPtr = (uint8_t *)i.data;
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
            cv::imwrite((input / image.filename()).string(), i);
        }
    }
}

void ResizeScryfallImages(Configuration const &conf)
{
    auto scryfall_folder = conf.WF.Get(SCRYFALL_INPUT_FOLDER);
    auto scryfall_upscaled_folder = conf.WF.Get(SCRYFALL_UPSCALED_FOLDER);
    auto scryfall_bleeded_folder = conf.WF.Get(SCRYFALL_BLEEDED_FOLDER);
    FillEmptySpaces(scryfall_folder);

    if (std::filesystem::exists(scryfall_upscaled_folder))
        std::filesystem::remove_all(scryfall_upscaled_folder);
    std::filesystem::create_directory(scryfall_upscaled_folder);

    if (!Upscale(conf, scryfall_folder, scryfall_upscaled_folder))
        return;

    if (std::filesystem::exists(scryfall_bleeded_folder))
        std::filesystem::remove_all(scryfall_bleeded_folder);

    std::filesystem::create_directory(scryfall_bleeded_folder);

    auto images = LoadImages(scryfall_upscaled_folder);
    BS::thread_pool pool{12};
    for (auto image : images)
    {
        std::ignore = pool.submit_task([image, &conf, &scryfall_bleeded_folder]()
                                       {
            auto i = cv::imread(image.string());
            auto dpi = std::min(i.size().width / conf.GetCardW(false), i.size().height / conf.GetCardH(false));
            int pixels_to_add = (int)std::floor(std::min(conf.GetFullBleedW(false) - conf.GetCardW(false), conf.GetFullBleedH(false) - conf.GetCardH(false)) * dpi);
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

            cv::imwrite(scryfall_bleeded_folder.string() + image.filename().string(), img); });
    }

    pool.wait();

    CropImages(conf, LoadImages(scryfall_bleeded_folder), CROP_FOLDER);
}