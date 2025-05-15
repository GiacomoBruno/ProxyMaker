#include <iostream>
#include <filesystem>
#include <vector>
#include <fstream>
#include <thread>
#include "configuration.h"
#include "PDFWriter/PDFWriter.h"
#include "PDFWriter/PDFPage.h"
#include "PDFWriter/PageContentContext.h"
#include "opencv2/imgcodecs.hpp"
#include "json.h"

/*
TODO:
1. maybe add image downloader
*/

using namespace PDFHummus;

bool IsImageExt(std::filesystem::path const &ext)
{
    return ext == ".jpg" || ext == ".png" || ext == ".jpeg";
}

std::string FromWstring(std::wstring const &wide)
{
    std::string str(wide.length(), 0);
    std::transform(wide.begin(), wide.end(), str.begin(), [](wchar_t c)
                   { 
                    if (c == 8217) return '\'';
                    if (c > 127) return '_';
                    return (char)c; });

    return str;
}

std::vector<std::filesystem::path> LoadImages(std::filesystem::path const &folder)
{
    std::vector<std::filesystem::path> res{};
    for (auto p : std::filesystem::directory_iterator{folder})
    {
        if (std::filesystem::is_regular_file(p) && IsImageExt(p.path().extension()))
        {
            auto sb = p.path().filename().wstring();
            auto new_path = folder / FromWstring(sb);
            std::filesystem::rename(p, new_path);
            res.push_back(new_path);
        }
    }
    return res;
}

auto CropImages(Configuration const &conf, std::vector<std::filesystem::path> const &images, std::filesystem::path const &output_folder)
{
    auto out = std::filesystem::absolute(output_folder);
    if (std::filesystem::exists(out))
        std::filesystem::remove_all(out);
    std::filesystem::create_directory(out);
    std::vector<std::thread> threads{};
    for (auto const &p : images)
    {
        threads.push_back(std::thread{
            [conf, p, out]()
            {
                cv::Mat img = cv::imread(p.string());
                auto w = img.size().width;
                auto h = img.size().height;

                auto m = std::min(w / conf.GetCardW(false), h / conf.GetCardH(false));
                auto c = std::round((conf.GetFullBleed(false) - conf.GetBleed(false)) * m);

                cv::Mat cropped_image = img(cv::Range(c, h - c), cv::Range(c, w - c));
                auto cropped = out / p.filename();
                cv::imwrite(cropped.string(), cropped_image);
            }});
    }

    for (auto &&t : threads)
    {
        t.join();
    }
}

auto DrawCross(Configuration const &conf, PageContentContext *cxt, double center_x, double center_y)
{
    AbstractContentContext::GraphicOptions gOptions{};
    gOptions.colorValue = 0x00FF00;
    gOptions.drawingType = AbstractContentContext::EDrawingType::eFill;

    auto botx = center_x - conf.GetCardW() / 2. - 1;
    auto boty = center_y - conf.GetCardH() / 2. - 1;
    auto botx1 = center_x + conf.GetCardW() / 2.;
    auto boty1 = center_y + conf.GetCardH() / 2.;
    auto w = 20.;
    auto h = 1;

    cxt->DrawRectangle(botx - (w / 2.), boty, w, h, gOptions);
    cxt->DrawRectangle(botx, boty - (w / 2.), h, w, gOptions);
    cxt->DrawRectangle(botx1 - (w / 2.), boty, w, h, gOptions);
    cxt->DrawRectangle(botx1, boty - (w / 2.), h, w, gOptions);
    cxt->DrawRectangle(botx - (w / 2.), boty1, w, h, gOptions);
    cxt->DrawRectangle(botx, boty1 - (w / 2.), h, w, gOptions);
    cxt->DrawRectangle(botx1 - (w / 2.), boty1, w, h, gOptions);
    cxt->DrawRectangle(botx1, boty1 - (w / 2.), h, w, gOptions);
}

auto GeneratePDF(Configuration const &conf, std::vector<std::filesystem::path> const &images, std::string const &filename)
{
    EStatusCode status = eSuccess;
    PDFWriter pdfWriter;
    std::cout << "Generating PDF: " << filename << std::endl;
    status = pdfWriter.StartPDF(filename, ePDFVersion13, LogConfiguration(true, true, ".\\proxy_maker.log"));
    if (status != eSuccess)
        return status;

    AbstractContentContext::ImageOptions options{};
    options.boundingBoxHeight = conf.GetCardH() + conf.GetBleed() * 2;
    options.boundingBoxWidth = conf.GetCardW() + conf.GetBleed() * 2;
    options.transformationMethod = AbstractContentContext::EImageTransformation::eFit;
    options.fitPolicy = AbstractContentContext::EFitPolicy::eAlways;

    int idx = 0;
    PDFPage *page{nullptr};
    PageContentContext *cxt{nullptr};

    auto pH = conf.GetPaperH() - conf.GetVerticalOffset();
    auto pW = conf.GetPaperW() - conf.GetHorizontalOffset();

    {
        auto tot_cards_1 = (int)floor(pW / conf.GetCardWithBleedW()) * (int)floor(pH / conf.GetCardWithBleedH());
        auto tot_cards_2 = (int)floor(pH / conf.GetCardWithBleedW()) * (int)floor(pW / conf.GetCardWithBleedH());
        if (tot_cards_2 > tot_cards_1)
        {
            pH = conf.GetPaperW() - conf.GetVerticalOffset();
            pW = conf.GetPaperH() - conf.GetHorizontalOffset();
        }
    }

    auto cards_per_column = (int)floor(pW / conf.GetCardWithBleedW());
    auto cards_per_row = (int)floor(pH / conf.GetCardWithBleedH());
    auto pbreak = cards_per_column * cards_per_row;
    for (auto &im : images)
    {
        auto row = ((idx % pbreak) / cards_per_column);
        auto col = ((idx % pbreak) % cards_per_column);

        if (col == 0 && row == 0)
        {
            if (cxt != nullptr)
            {
                pdfWriter.EndPageContentContext(cxt);
                pdfWriter.WritePageAndRelease(page);
            }
            page = new PDFPage();
            page->SetMediaBox(PDFRectangle(0, 0, pW + conf.GetHorizontalOffset(), pH + conf.GetVerticalOffset()));
            cxt = pdfWriter.StartPageContentContext(page);
        }

        double center_x = pW / cards_per_column * (1 + 2 * col) / 2. + (conf.GetHorizontalOffset() / 2.);
        double center_y = pH / cards_per_row * (1 + 2 * row) / 2. + (conf.GetVerticalOffset() / 2.);

        double x = center_x - (conf.GetCardWithBleedW() / 2.);
        double y = center_y - (conf.GetCardWithBleedH() / 2.);

        cxt->DrawImage(x, y, im.string(), options);
        if (conf.GetDrawCross())
            DrawCross(conf, cxt, center_x, center_y);

        idx++;
    }

    if (cxt != nullptr)
    {
        pdfWriter.EndPageContentContext(cxt);
        pdfWriter.WritePageAndRelease(page);
    }

    status = pdfWriter.EndPDF();
    if (status != eSuccess)
        return status;

    return status;
}

int main(int argc, char **argv)
{
    Configuration conf{};
    conf.LoadConfiguration(".\\files\\config.json");

    CropImages(conf, LoadImages(".\\files\\images\\"), ".\\files\\images\\crop\\");
    GeneratePDF(conf, LoadImages(".\\files\\images\\crop\\"), conf.GetOutputFile());
    conf.SaveConfiguration(".\\files\\config.json");
}