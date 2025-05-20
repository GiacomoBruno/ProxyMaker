#include "pdf_generation.h"
#include "PDFWriter/PDFWriter.h"
#include "PDFWriter/PDFPage.h"
#include "PDFWriter/PageContentContext.h"
#include <iostream>
#include <thread>
#include <vector>
#include "BS_thread_pool.hpp"

#define SyncOut std::osyncstream{std::cout}

void DrawCross(Configuration const &conf, PageContentContext *cxt, double center_x, double center_y)
{
    AbstractContentContext::GraphicOptions gOptions{};
    gOptions.colorValue = 0x00FF00;
    gOptions.drawingType = AbstractContentContext::EDrawingType::eFill;

    auto x = center_x - (conf.GetCardW()/2.);
    auto y = center_y - (conf.GetCardH()/2.);
    auto x1 = x + conf.GetCardW();
    auto y1 = y + conf.GetCardH();

    auto t = conf.GetCrossThickness();
    auto l = conf.GetCrossLength();

    auto botx = x - t;
    auto boty = y - t;
   
    cxt->DrawRectangle(botx - (l/2.), boty, l, t, gOptions);
    cxt->DrawRectangle(botx, boty - (l/2.), t, l, gOptions);

    cxt->DrawRectangle(botx - (l/2.), y1, l, t, gOptions);
    cxt->DrawRectangle(botx, y1 - (l/2.), t, l, gOptions);

    cxt->DrawRectangle(x1 - (l/2.), boty, l, t, gOptions);
    cxt->DrawRectangle(x1, boty - (l/2.), t, l, gOptions);

    cxt->DrawRectangle(x1 - (l/2.), y1, l, t, gOptions);
    cxt->DrawRectangle(x1, y1 - (l/2.), t, l, gOptions);

}

void GeneratePage(Configuration const &conf, PageConfiguration const& pConf, int idx, std::vector<std::filesystem::path> const& images)
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

void GeneratePDF(Configuration const &conf, std::vector<std::filesystem::path> const &images, std::string const &filename)
{
    std::cout << "Generating PDF: " << filename << std::endl;
    int idx = 0;
    PageConfiguration pageConf{conf};
    std::vector<std::filesystem::path> batch{};
    BS::thread_pool pool{12};
    for(int i = 0; i < images.size(); i++)
    {
        batch.push_back(images[i]);

        if(((i+1)%pageConf.CardsPerPage == 0 && i > 0) || (i == images.size()-1))
        {
            std::ignore = pool.submit_task([&conf, &pageConf, batch, idx](){ GeneratePage(conf, pageConf, idx, batch);});
            idx++;
            batch.clear();
        }

    }
    
    pool.wait();

    PDFWriter pdfWriter;
    pdfWriter.StartPDF(filename, ePDFVersion13);
    for(int i = 0; i < idx; i++)
    {
        pdfWriter.AppendPDFPagesFromPDF(".\\files\\"+ std::to_string(i)+".pdf",PDFPageRange());
        std::filesystem::remove(".\\files\\"+ std::to_string(i)+".pdf");
    }
    pdfWriter.EndPDF(); //unite all generated pdfs
    std::cout << "PDF generation complete!" << std::endl;
}
