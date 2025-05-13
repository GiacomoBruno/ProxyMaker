#include <iostream>
#include "PDFWriter/PDFWriter.h"
#include "PDFWriter/PDFPage.h"
#include "PDFWriter/PageContentContext.h"

#include <filesystem>
#include <vector>

using namespace PDFHummus;

constexpr double CARD_W = 2.49;     // inch
constexpr double CARD_H = 3.48;     // inch
constexpr double BLEED = 0.03;      // inch
constexpr double MULTIPLIER = 72;   // dpi
constexpr double PAPER_W_A6 = 4.1;  // inch
constexpr double PAPER_H_A6 = 5.8;  // inch
constexpr double PAPER_W_A4 = 8.26666667;  // inch
constexpr double PAPER_H_A4 = 11.6933333; // inch

enum PAPER
{
    A4,
    A6
};

std::vector<std::filesystem::path> LoadImages()
{
    std::vector<std::filesystem::path> res{};
    for (auto p : std::filesystem::directory_iterator{".\\images\\crop\\"})
    {
        if (std::filesystem::is_regular_file(p))
            res.push_back(p);
    }
    return res;
}

auto draw_cross(PageContentContext *cxt, double center_x, double center_y)
{
    AbstractContentContext::GraphicOptions gOptions{};
    gOptions.colorValue = 0x00FF00;
    gOptions.drawingType = AbstractContentContext::EDrawingType::eFill;

    auto botx = center_x - (CARD_W * MULTIPLIER) / 2. - 1;
    auto boty = center_y - (CARD_H * MULTIPLIER) / 2. - 1;
    auto botx1 = center_x + (CARD_W * MULTIPLIER) / 2.;
    auto boty1 = center_y + (CARD_H * MULTIPLIER) / 2.;
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

auto GeneratePDF(std::vector<std::filesystem::path> const &images, char *filename, bool crosses, PAPER paperSize)
{
    EStatusCode status = eSuccess;
    PDFWriter pdfWriter;

    status = pdfWriter.StartPDF(filename, ePDFVersion13, LogConfiguration(true, true, ".\\proxy_maker.log"));
    if (status != eSuccess)
        return status;

    AbstractContentContext::ImageOptions options{};
    options.boundingBoxHeight = (CARD_H + BLEED * 2) * MULTIPLIER;
    options.boundingBoxWidth = (CARD_W + BLEED * 2) * MULTIPLIER;
    options.transformationMethod = AbstractContentContext::EImageTransformation::eFit;
    options.fitPolicy = AbstractContentContext::EFitPolicy::eAlways;

    int idx = 0;
    PDFPage *page{nullptr};
    PageContentContext *cxt{nullptr};
    auto offset_x = 30;
    auto offset_y = 50;

    auto pH = (paperSize == A6 ? PAPER_W_A6 : PAPER_H_A4) * MULTIPLIER - offset_y;
    auto pW = (paperSize == A6 ? PAPER_H_A6 : PAPER_W_A4) * MULTIPLIER - offset_x;

    auto cards_per_column = (int)floor(pW / ((CARD_W + BLEED * 2.) * MULTIPLIER));
    auto cards_per_row = (int)floor(pH / ((CARD_H + BLEED * 2.) * MULTIPLIER));
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
            page->SetMediaBox(PDFRectangle(0, 0, pW + offset_x, pH + offset_y));
            cxt = pdfWriter.StartPageContentContext(page);
        }

        double center_x = pW / cards_per_column * (1 + 2 * col) / 2. + (offset_x/2.);
        double center_y = pH / cards_per_row * (1 + 2 * row) / 2.  + (offset_y/2.);

        double x = center_x - ((CARD_W + BLEED * 2.) * MULTIPLIER / 2.);
        double y = center_y - ((CARD_H + BLEED * 2.) * MULTIPLIER / 2.);

        cxt->DrawImage(x, y, im.string(), options);
        if (crosses)
            draw_cross(cxt, center_x, center_y);

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
    GeneratePDF(LoadImages(), argv[1], argv[2] == std::string("c"), argv[3] == std::string("A4") ? A4 : A6);
}