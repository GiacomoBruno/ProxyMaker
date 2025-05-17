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
#include <syncstream>
#include <format>

using namespace PDFHummus;
#define SyncOut std::osyncstream{std::cout}

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

                auto mW = w / conf.GetFullBleedW(false);
                auto mH = h / conf.GetFullBleedH(false);
                auto cW = std::round(((conf.GetFullBleedW(false) - conf.GetCardW(false)) / 2.) * mW);
                auto cH = std::round(((conf.GetFullBleedH(false) - conf.GetCardH(false)) / 2.) * mH);
                cW = std::round(cW - (conf.GetBleed(false)*mW));
                cH = std::round(cH - (conf.GetBleed(false)*mH));
                cv::Mat cropped_image = img(cv::Range(cH, h - cH), cv::Range(cW, w - cW));
                auto cropped = out / p.filename();
                cv::imwrite(cropped.string(), cropped_image);
                SyncOut << std::format("Image [{}x{}] DPI: {}x{} CROP [{}x{}] BLEED [{}x{}]", w, h, (int)mW, (int)mH, (int)cW, (int)cH, int(conf.GetBleed(false) * mW), (int)(conf.GetBleed(false) * mH)) << std::endl;

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
    double t = conf.GetBleed(false);
    double l = conf.GetCrossLength();
    auto cW = conf.GetCardW(false) / 2.;
    auto cH = conf.GetCardH(false) / 2.;
    auto botx = center_x - ((cW + t)*conf.GetPPI());
    auto boty = center_y - ((cH + t)*conf.GetPPI());
    auto botx1 = center_x + cW*conf.GetPPI();
    auto boty1 = center_y + cH*conf.GetPPI();

    auto p = (l / 2.);
    t = t * conf.GetPPI();
    cxt->DrawRectangle(botx - p, boty, l, t, gOptions);
    cxt->DrawRectangle(botx, boty - p, t, l, gOptions);

    cxt->DrawRectangle(botx1 - p, boty, l, t, gOptions);
    cxt->DrawRectangle(botx1, boty - p, t, l, gOptions);

    cxt->DrawRectangle(botx1 - p, boty1, l, t, gOptions);
    cxt->DrawRectangle(botx1, boty1 - p, t, l, gOptions);

    cxt->DrawRectangle(botx - p, boty1, l, t, gOptions);
    cxt->DrawRectangle(botx, boty1 - p, t, l, gOptions);
}

auto GeneratePage(Configuration const &conf, PageConfiguration const& pConf, int idx, std::vector<std::filesystem::path> const& images)
{
    PDFWriter pdfWriter;
    pdfWriter.StartPDF(".\\files\\"+std::to_string(idx)+".pdf", ePDFVersion13, LogConfiguration(false, nullptr));

    auto page = new PDFPage();
    page->SetMediaBox(PDFRectangle(0, 0, pConf.PW + conf.GetHorizontalOffset(), pConf.PH + conf.GetVerticalOffset()));

    auto cxt = pdfWriter.StartPageContentContext(page);      

    int i = 0;
    for(auto &im : images)
    {
        //print stuff
        auto row = ((i % pConf.CardsPerPage) / pConf.Cols);
        auto col = ((i % pConf.CardsPerPage) % pConf.Cols);

        double center_x = pConf.PW / pConf.Cols * (1 + 2 * col) / 2. + (conf.GetHorizontalOffset() / 2.);
        double center_y = pConf.PH / pConf.Rows * (1 + 2 * row) / 2. + (conf.GetVerticalOffset() / 2.);

        double x = center_x - (conf.GetCardWithBleedW() / 2.);
        double y = center_y - (conf.GetCardWithBleedH() / 2.);

        cxt->DrawImage(x, y, im.string(), pConf.Options);
        if (conf.GetDrawCross())
            DrawCross(conf, cxt, center_x, center_y);

        i++;
    }
    pdfWriter.EndPageContentContext(cxt);
    pdfWriter.WritePageAndRelease(page);
    pdfWriter.EndPDF();
}

auto GeneratePDF(Configuration const &conf, std::vector<std::filesystem::path> const &images, std::string const &filename)
{
    std::cout << "Generating PDF: " << filename << std::endl;
    int idx = 0;
    PageConfiguration pageConf{conf};
    std::vector<std::thread> threads{};
    std::vector<std::filesystem::path> batch{};

    for(int i = 0; i < images.size(); i++)
    {
        batch.push_back(images[i]);

        if(((i+1)%pageConf.CardsPerPage == 0 && i > 0) || (i == images.size()-1))
        {
            threads.push_back(std::thread([&conf, &pageConf, batch, idx](){ GeneratePage(conf, pageConf, idx, batch);}));
            idx++;
            batch.clear();
        }

    }
    
    for(auto& t: threads)
        t.join();

    PDFWriter pdfWriter;
    pdfWriter.StartPDF(filename, ePDFVersion13);
    for(int i = 0; i < threads.size(); i++)
    {
        pdfWriter.AppendPDFPagesFromPDF(".\\files\\"+ std::to_string(i)+".pdf",PDFPageRange());
        std::filesystem::remove(".\\files\\"+ std::to_string(i)+".pdf");
    }
    pdfWriter.EndPDF(); //unite all generated pdfs
}

int main(int argc, char **argv)
{
    Configuration conf{};
    conf.LoadConfiguration(".\\files\\config.json");
    CropImages(conf, LoadImages(".\\files\\images\\"), ".\\files\\images\\crop\\");
    GeneratePDF(conf, LoadImages(".\\files\\images\\crop\\"), conf.GetOutputFile());
    conf.SaveConfiguration(".\\files\\config.json");
}