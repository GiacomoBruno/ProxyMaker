#include "pdf_generation.h"
#include "PDFWriter/PDFWriter.h"
#include "PDFWriter/PDFPage.h"
#include "PDFWriter/PageContentContext.h"
#include <iostream>
#include <thread>
#include <vector>
#include <filesystem>
#include "BS_thread_pool.hpp"


#define SyncOut std::osyncstream{std::cout}

void DrawCross(Configuration const &conf, PageContentContext *cxt, double _x, double _y)
{
    AbstractContentContext::GraphicOptions gOptions{};
    gOptions.colorValue = 0x00FF00;
    gOptions.drawingType = AbstractContentContext::EDrawingType::eFill;
 
    auto margin = GetPrintSize(GetMargin(conf), 72);
    auto hMargin = margin /2.;
    auto cW = GetPrintSize(GetCardW(conf), 72) + margin;
    auto cH = GetPrintSize(GetCardH(conf), 72) + margin;
    auto cMW = GetPrintSize(GetMarginCardW(conf), 72) - margin;
    auto cMH = GetPrintSize(GetMarginCardH(conf), 72) - margin;
    auto x = _x + hMargin;
    auto y = _y + hMargin;

    cxt->DrawRectangle(_x      , _y      , margin, margin, gOptions);
    cxt->DrawRectangle(_x + cMW, _y      , margin, margin, gOptions);
    cxt->DrawRectangle(_x      , _y + cMH, margin, margin, gOptions);
    cxt->DrawRectangle(_x + cMW, _y + cMH, margin, margin, gOptions);

    cxt->DrawRectangle(x, y, cW - hMargin, hMargin, gOptions);//_____
    cxt->DrawRectangle(x, y + cH - hMargin, cW, hMargin, gOptions);//-------

    cxt->DrawRectangle(x,      y, hMargin, cH, gOptions);
    cxt->DrawRectangle(x + cW-hMargin, y, hMargin, cH, gOptions);
}

std::tuple<int, int, int> GetPageStats(Configuration const& conf, Paper& p)
{
    auto cW = GetMarginCardW(conf);
    auto cH = GetMarginCardH(conf);

    int cpr = floor(p.W / cW);
    int cpc = floor(p.H / cH);

    int cprl = floor(p.H / cW);
    int cpcl = floor(p.W / cH);

    if((cpr*cpc) < (cprl * cpcl))
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

void GeneratePage(Configuration const &conf, int idx, std::vector<Card> const& cards)
{
    PDFWriter pdfWriter;
    pdfWriter.StartPDF(conf.GetDir(FILES_FOLDER + std::to_string(idx)+".pdf"), ePDFVersion13, LogConfiguration(false, nullptr));

    auto page = new PDFPage();
    Paper p = GetPaper(conf.GetPaperType());

    page->SetMediaBox(PDFRectangle(0, 0, GetPrintSize(p.W, 72), GetPrintSize(p.H, 72)));
    
    auto cxt = pdfWriter.StartPageContentContext(page);

    //for simplicity at the moment all Cards have the same size.
    //later we can change that.
    AbstractContentContext::ImageOptions Options;
    Options.boundingBoxHeight = GetPrintSize(GetMarginCardH(conf), 72);
    Options.boundingBoxWidth = GetPrintSize(GetMarginCardW(conf), 72);
    Options.transformationMethod = AbstractContentContext::EImageTransformation::eFit;
    Options.fitPolicy = AbstractContentContext::EFitPolicy::eAlways;

    auto [cpr,cpc, cpp] = GetPageStats(conf, p);
    auto mCardW = GetPrintSize(GetMarginCardW(conf), 72);
    auto mCardH = GetPrintSize(GetMarginCardH(conf), 72);

    double xOffset = (GetPrintSize(p.W, 72) / 2.) - (cpr * mCardW / 2.);
    double yOffset = (GetPrintSize(p.H, 72) / 2.) - (cpr * mCardH / 2.);

    int i = 0;
    for(auto &im : cards)
    {
        //print stuff
        auto row = ((i % cpp) / cpr);
        auto col = ((i % cpp) % cpr);

        double x = (col * mCardW)+ xOffset;
        double y = (row * mCardH)+ yOffset;

        double center_x = x + (mCardW / 2.);
        double center_y = y + (mCardH / 2.);

        cxt->DrawImage(x, y, im.ImageFile.string(), Options);
        

        i++;
    }

    for(int row = 0; row < cpc; row++)
    {
        for(int col = 0; col < cpr; col++)
        {
            double x = (col * mCardW)+ xOffset;
        double y = (row * mCardH)+ yOffset;

        double center_x = x + (mCardW / 2.);
        double center_y = y + (mCardH / 2.);
        DrawCross(conf, cxt, x, y);
        }
    }
   
    pdfWriter.EndPageContentContext(cxt);
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

    std::cout<<"CardH: " << GetPrintSize(GetCardH(conf), 72) << std::endl;
    std::cout<<"CardW: " << GetPrintSize(GetCardW(conf), 72) << std::endl;
    std::cout<<"CardMH" << GetPrintSize(GetMarginCardH(conf), 72) << std::endl;
    std::cout << "CardMW" << GetPrintSize(GetMarginCardW(conf), 72) << std::endl;

    for(int i = 0; i < images.size(); i++)
    {
        auto inPrintList = conf.PrintList.find(images[i].ImageFile.filename().string());

        if(inPrintList != conf.PrintList.end() && inPrintList->second > 1)
        {
            for(int j = 0; j < inPrintList->second - 1; j++)
                toAdd.push_back(images[i]);
        }
    }

    Paper p = GetPaper(conf.GetPaperType());
    auto [cpr, cpc, cpp] = GetPageStats(conf, p);
    images.insert(images.end(), toAdd.begin(), toAdd.end());

    for(int i = 0; i < images.size(); i++)
    {
        batch.push_back(images[i]);

        if(((i+1)%cpp == 0 && i > 0) || (i == images.size()-1))
        {
            std::ignore = pool.submit_task([&conf, batch, idx](){ GeneratePage(conf, idx, batch);});
            idx++;
            batch.clear();
        }

    }
    
    pool.wait();

    PDFWriter pdfWriter;
    pdfWriter.StartPDF(filename, ePDFVersion13);
    for(int i = 0; i < idx; i++)
    {
        pdfWriter.AppendPDFPagesFromPDF(conf.GetDir(FILES_FOLDER + std::to_string(i)+".pdf"),PDFPageRange());
        std::filesystem::remove(conf.GetDir(FILES_FOLDER+ std::to_string(i)+".pdf"));
    }
    pdfWriter.EndPDF(); //unite all generated pdfs
    std::cout << "PDF generation complete!" << std::endl;
}
