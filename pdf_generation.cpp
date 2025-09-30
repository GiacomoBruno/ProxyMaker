#include "pdf_generation.h"
#include "PDFWriter/PDFWriter.h"
#include "PDFWriter/PDFPage.h"
#include "PDFWriter/PageContentContext.h"
#include "PDFWriter/PDFModifiedPage.h"
#include <iostream>
#include <thread>
#include <vector>
#include <filesystem>
#include "BS_thread_pool.hpp"

#define SyncOut \
    std::osyncstream { std::cout }

void DrawCross(Configuration const &conf, PageContentContext *cxt, double _x, double _y)
{
    AbstractContentContext::GraphicOptions gOptions{};
    gOptions.colorValue = 0xFF0000;
    gOptions.drawingType = AbstractContentContext::EDrawingType::eFill;

    int margin = GetPrintSize(GetMargin(conf), conf.GetPPI());
    auto hMargin = margin / 2;
    double oMargin = GetPrintSize(GetMargin(conf), conf.GetPPI());
    auto totcardw = GetPrintSize(GetMarginCardW(conf), conf.GetPPI());
    auto totcardh = GetPrintSize(GetMarginCardH(conf), conf.GetPPI());

    int space = floor(totcardw);
    int fitsize = space / 13;
    int iterations = (space / fitsize) / 2 + ((space / fitsize) % 2 ? 1 : 0);

    for (int i = 0; i < iterations; i++)
    {
        if (!(i % 2))
        {
            auto x1 = _x + (fitsize * i);
            auto y1 = _y;
            auto x2 = _x + totcardw - (fitsize * i) - fitsize;
            auto y2 = _y + totcardh - oMargin;

            cxt->DrawRectangle(x1, y1, fitsize, oMargin, gOptions);
            cxt->DrawRectangle(x2, y1, fitsize, oMargin, gOptions);
            cxt->DrawRectangle(x1, y2, fitsize, oMargin, gOptions);
            cxt->DrawRectangle(x2, y2, fitsize, oMargin, gOptions);
        }
    }

    space = floor(totcardh);
    fitsize = space / 13;

    iterations = (space / fitsize) / 2 + ((space / fitsize) % 2 ? 1 : 0);

    for (int i = 0; i < iterations; i++)
    {
        if (!(i % 2))
        {
            auto x1 = _x;
            auto y1 = _y + (fitsize * i);
            auto x2 = _x + totcardw - oMargin;
            auto y2 = _y + totcardh - (fitsize * i) - fitsize;

            cxt->DrawRectangle(x1, y1, oMargin, fitsize, gOptions);
            cxt->DrawRectangle(x2, y1, oMargin, fitsize, gOptions);
            cxt->DrawRectangle(x1, y2, oMargin, fitsize, gOptions);
            cxt->DrawRectangle(x2, y2, oMargin, fitsize, gOptions);
        }
    }

    gOptions.colorValue = 0x00FF00;
    cxt->DrawRectangle(_x, _y, totcardw, margin, gOptions);
    cxt->DrawRectangle(_x, _y, margin, totcardh, gOptions);
    cxt->DrawRectangle(_x + totcardw - margin, _y, margin, totcardh, gOptions);
    cxt->DrawRectangle(_x, _y + totcardh - margin, totcardw, margin, gOptions);

    // for(int i = 0; (i*dSize)+dSize <= totcardw; i++)
    //{
    //     auto x1 = _x + hMargin + (i * dSize);
    //     auto y1 = _y + hMargin;
    //     auto y2 = _y + totcardh - margin;
    //     if(!(i % 2)){
    //         cxt->DrawRectangle(x1, y1, dSize, hMargin, gOptions);
    //         cxt->DrawRectangle(x1, y2, dSize, hMargin, gOptions);
    //
    //    }
    //}
    //
    // dSize = floor(totcardh / 31);
    //
    // for(int i = 0; (i*dSize)+dSize <= totcardh; i++)
    //{
    //    auto x1 = _x + hMargin;
    //    auto x2 = _x + totcardw - margin;
    //    auto y1 = _y + hMargin + (i * dSize);
    //
    //    if(!(i % 2)){
    //        cxt->DrawRectangle(x1, y1, hMargin, dSize, gOptions);
    //        cxt->DrawRectangle(x2, y1, hMargin, dSize, gOptions);
    //
    //    }
    //}

    // cxt->DrawRectangle(_x, _y, margin, margin, gOptions);
    // cxt->DrawRectangle(_x + cW, _y, margin, margin, gOptions);
    // cxt->DrawRectangle(_x, _y + cH, margin, margin, gOptions);
    // cxt->DrawRectangle(_x + cW, _y + cH, margin, margin, gOptions);

    // cxt->DrawRectangle(x, y, cW - width, width, gOptions);      //_____
    // cxt->DrawRectangle(x, y + cH - width, cW, width, gOptions); //-------

    // cxt->DrawRectangle(x, y, width, cH, gOptions);
    // cxt->DrawRectangle(x + cW - width, y, width, cH, gOptions);
}

std::tuple<int, int, int> GetPageStats(Configuration const &conf, Paper &p)
{
    auto cW = GetMarginCardW(conf);
    auto cH = GetMarginCardH(conf);

    int cpr = floor(p.W / cW);
    int cpc = floor(p.H / cH);

    int cprl = floor(p.H / cW);
    int cpcl = floor(p.W / cH);

    if ((cpr * cpc) < (cprl * cpcl))
    {
        auto tmp = p.W;
        p.W = p.H;
        p.H = tmp;
    }

    cpr = floor(p.W / cW);
    cpc = floor(p.H / cH);

    int cpp = cpr * cpc;
    return std::make_tuple(cpr, cpc, cpp);
}

void GenerateSilhouettePage(Configuration const &conf, int idx, std::vector<Card> const &cards)
{
    Paper p = GetPaper(conf.GetPaperType());
    auto [cpr, cpc, cpp] = GetPageStats(conf, p);

    double xs[8] =
        {
            17.0, 107.0,
            17.0, 107.0,
            17.0, 107.0,
            17.0, 107.0};

    double ys[8]{
        22.0, 22.0,
        86.0, 86.0,
        150.0, 150.0,
        214.0, 214.0};

    PDFWriter pdfWriter;
    pdfWriter.StartPDF(conf.GetDir(FILES_FOLDER + tostr(idx) + STR_PREFIX(".pdf")).string(), ePDFVersion17, LogConfiguration(false, nullptr));
    std::cout << std::format("PRINT SIZE: {} by {} pixels", GetPrintSize(p.W, conf.GetPPI()), GetPrintSize(p.H, conf.GetPPI())) << std::endl;

    auto page = new PDFPage();
    page->SetMediaBox(PDFRectangle(0, 0, GetPrintSize(p.W, conf.GetPPI()), GetPrintSize(p.H, conf.GetPPI())));
    auto context = pdfWriter.StartPageContentContext(page);

    // for simplicity at the moment all Cards have the same size.
    // later we can change that.
    AbstractContentContext::ImageOptions Options;
    Options.boundingBoxWidth = GetPrintSize(GetMarginCardH(conf), conf.GetPPI());
    Options.boundingBoxHeight = GetPrintSize(GetMarginCardW(conf), conf.GetPPI());
    Options.transformationMethod = AbstractContentContext::EImageTransformation::eFit;
    Options.fitPolicy = AbstractContentContext::EFitPolicy::eAlways;
    int i = 0;

    for (auto &im : cards)
    {
        // print stuff
        auto row = ((i % cpp) / cpr);
        auto col = ((i % cpp) % cpr);

        double x = GetPrintSize(xs[i] - GetMargin(conf), conf.GetPPI());
        double y = GetPrintSize(ys[i] - GetMargin(conf), conf.GetPPI());

        context->DrawImage(x, y, im.ImageFile.string(), Options);
        i++;
    }

    Options.boundingBoxWidth = GetPrintSize(p.W, conf.GetPPI());
    Options.boundingBoxHeight = GetPrintSize(p.H, conf.GetPPI());
    Options.transformationMethod = AbstractContentContext::EImageTransformation::eFit;
    context->DrawImage(0,0, conf.GetDir(TEMPLATE).string(), Options);

    pdfWriter.EndPageContentContext(context);
    pdfWriter.WritePageAndRelease(page);
    pdfWriter.EndPDF();
}

void GeneratePage(Configuration const &conf, int idx, std::vector<Card> const &cards)
{
    Paper p = GetPaper(conf.GetPaperType());
    auto [cpr, cpc, cpp] = GetPageStats(conf, p);

    PDFWriter pdfWriter;
    pdfWriter.StartPDF(conf.GetDir(FILES_FOLDER + tostr(idx) + STR_PREFIX(".pdf")).string(), ePDFVersion17, LogConfiguration(false, nullptr));
    auto page = new PDFPage();
    page->SetMediaBox(PDFRectangle(0, 0, GetPrintSize(p.W, conf.GetPPI()), GetPrintSize(p.H, conf.GetPPI())));

    std::cout << std::format("PRINT SIZE: {} by {} pixels", GetPrintSize(p.W, conf.GetPPI()), GetPrintSize(p.H, conf.GetPPI())) << std::endl;
    auto context = pdfWriter.StartPageContentContext(page);

    // for simplicity at the moment all Cards have the same size.
    // later we can change that.
    AbstractContentContext::ImageOptions Options;
    Options.boundingBoxHeight = GetPrintSize(GetMarginCardH(conf), conf.GetPPI());
    Options.boundingBoxWidth = GetPrintSize(GetMarginCardW(conf), conf.GetPPI());
    Options.transformationMethod = AbstractContentContext::EImageTransformation::eFit;
    Options.fitPolicy = AbstractContentContext::EFitPolicy::eAlways;

    auto mCardW = GetPrintSize(GetMarginCardW(conf), conf.GetPPI());
    auto mCardH = GetPrintSize(GetMarginCardH(conf), conf.GetPPI());

    uint16_t spacing = conf.GetSpacing();

    int xOffset = (GetPrintSize(p.W, conf.GetPPI()) / 2.) - (cpr * mCardW / 2.);
    int yOffset = (GetPrintSize(p.H, conf.GetPPI()) / 2.) - (cpc * mCardH / 2.);

    int i = 0;
    auto mid_col = cpc / 2;
    auto mid_row = cpr / 2;
    for (auto &im : cards)
    {
        // print stuff
        auto row = ((i % cpp) / cpr);
        auto col = ((i % cpp) % cpr);

        int x = (col * mCardW) + xOffset + (spacing * (col - mid_col));
        int y = (row * mCardH) + yOffset + (spacing * (row - mid_row));

        context->DrawImage(x, y, im.ImageFile.string(), Options);
        DrawCross(conf, context, x, y);
        i++;
    }

    pdfWriter.EndPageContentContext(context);
    pdfWriter.WritePageAndRelease(page);
    pdfWriter.EndPDF();
}

void GeneratePDF(Configuration const &conf, std::vector<Card> &images, std::filesystem::path const &filename)
{
    std::cout << "Generating PDF: " << filename.string() << std::endl;
    int idx = 0;
    std::vector<Card> batch{};
    BS::thread_pool pool{12};
    std::vector<Card> toAdd{};

    for (int i = 0; i < images.size(); i++)
    {
        auto inPrintList = conf.PrintList.find(images[i].ImageFile.filename().string());

        if (inPrintList != conf.PrintList.end() && inPrintList->second > 1)
        {
            for (int j = 0; j < inPrintList->second - 1; j++)
                toAdd.push_back(images[i]);
        }
    }

    Paper p = GetPaper(conf.GetPaperType());
    auto [cpr, cpc, cpp] = GetPageStats(conf, p);
    images.insert(images.end(), toAdd.begin(), toAdd.end());

    for (int i = 0; i < images.size(); i++)
    {
        batch.push_back(images[i]);

        if (((i + 1) % cpp == 0 && i > 0) || (i == images.size() - 1))
        {
            std::ignore = pool.submit_task([&conf, batch, idx]()
                                           { 
                                            if(conf.GetUseSilhouetteTemplate())
                                                GenerateSilhouettePage(conf, idx, batch);
                                            else 
                                                GeneratePage(conf, idx, batch); });
            idx++;
            batch.clear();
        }
    }

    pool.wait();

    PDFWriter pdfWriter;
    pdfWriter.StartPDF(filename.string(), ePDFVersion13);
    for (int i = 0; i < idx; i++)
    {
        pdfWriter.AppendPDFPagesFromPDF(conf.GetDir(FILES_FOLDER + tostr(i) + STR_PREFIX(".pdf")).string(), PDFPageRange());
        std::filesystem::remove(conf.GetDir(FILES_FOLDER + tostr(i) + STR_PREFIX(".pdf")));
    }
    pdfWriter.EndPDF(); // unite all generated pdfs
    std::cout << "PDF generation complete!" << std::endl;
}