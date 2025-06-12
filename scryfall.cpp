#include "scryfall.h"
#include "utility.h"
#include "configuration.h"
#include "image_processing.h"
#include <format>
#include <fstream>
#include <sstream>
#include <iostream>
#include <thread>

struct card_info
{
    std::string card_name{};
    std::string safe_card_name{};
    std::string set{};
    int collector_number{};
    int amount{1};
    std::string png_link{};
    
    std::string get_link() const;
};

std::string card_info::get_link() const
{
    if(set.empty())
    {
        return "https://scryfall.com/search?q=" + card_name;
    }
    else
    {
        return std::format("https://scryfall.com/card/{}/{}/{}", set, collector_number, card_name);
    }
}


std::vector<card_info> ReadList(std::filesystem::path const &list)
{
    std::vector<card_info> output{};
    std::string line;
    std::ifstream infile{list};
    while (std::getline(infile, line))
    {
        card_info card{};
        size_t amount_end = line.find_first_of(' ');
        size_t card_name_pos = amount_end + 1;
        try{
        if(line.npos != line.find('('))
        {
            size_t card_name_end = line.find_first_of('(', card_name_pos) - 1;
            size_t set_pos = card_name_end+2;
            size_t set_end = line.find_first_of(')', set_pos);
            size_t collector_pos = set_end + 2;
            card.amount = std::stoi(line.substr(0,amount_end));
            card.card_name = line.substr(card_name_pos, card_name_end - card_name_pos);
            size_t double_card_start = card.card_name.find_first_of('/');
            if(double_card_start != card.card_name.npos)
            {
                card.card_name = card.card_name.substr(0,double_card_start - 1);
            }
            card.set = line.substr(set_pos, set_end- set_pos);
            card.collector_number = std::stoi(line.substr(collector_pos));

            std::replace(card.card_name.begin(), card.card_name.end(), ' ', '-');

        }
        else
        {
            card.amount = std::stoi(line.substr(0,amount_end));
            card.card_name = line.substr(card_name_pos);

            std::replace(card.card_name.begin(), card.card_name.end(), ' ', '+');
        }

        card.safe_card_name = card.card_name;
        std::replace(card.safe_card_name.begin(), card.safe_card_name.end(), '/', '+');

        output.push_back(card);
        }
        catch(...)
        {
            std::cout << "failed to read line: " << line << std::endl;
        }
    }
    return output;
}

void GetPngLink(card_info& card)
{
    auto tmp_file = (std::filesystem::current_path() / TMP_FILE).string();
    std::filesystem::remove(tmp_file);

    RunCommand("curl -Ls -w %{url_effective} \"" + std::format("{}\" --output \"{}\"", card.get_link(), tmp_file));

    std::ifstream page{tmp_file};
    std::stringstream buffer;
    buffer << page.rdbuf();
    std::string page_content = buffer.str();



    if(page_content.empty()) return; 

    auto pos = page_content.find("Download PNG", page_content.size() / 2);
    auto href = page_content.rfind("href=", pos); // get href=
    auto png_link = page_content.substr(href + 6, page_content.find("\"", href + 6) - (href + 6));
    if(png_link[0] ==  '/') return;

    card.png_link = png_link;
}

bool DownloadCard(card_info& card, std::filesystem::path const &output_dir)
{
    if (std::filesystem::exists((output_dir.string() + card.safe_card_name) + ".png"))
        return false;

    GetPngLink(card);

    if(card.png_link.empty()) return false;
    RunCommand(std::format("curl \"{}\" --output \"{}.png\"", card.png_link, output_dir.string() + card.safe_card_name));

    return true;
}

void CleanUpScryfallFolders(Configuration const &conf, std::vector<card_info> &cards)
{
    auto scryfall_folder = conf.GetDir(SCRYFALL_INPUT_FOLDER);
    auto scryfall_upscaled_folder = conf.GetDir(SCRYFALL_UPSCALED_FOLDER);
    auto scryfall_bled_folder = conf.GetDir(SCRYFALL_BLED_FOLDER);

    PrepareDirectory(scryfall_folder, false);
    PrepareDirectory(scryfall_upscaled_folder, false);
    PrepareDirectory(scryfall_bled_folder, false);

    std::vector<std::string> to_remove{};
    std::vector<std::string> card_names{};

    for(auto& c: cards)
    card_names.push_back(c.safe_card_name + ".png");

    for (auto &p : std::filesystem::directory_iterator{scryfall_bled_folder})
    {
        if (p.is_regular_file())
        {
            auto filename = p.path().filename().string();

            auto find = std::find(card_names.begin(), card_names.end(), filename);
            if (find == card_names.end())
            {
                to_remove.push_back(filename);
            }
        }
    }

    auto rem = [](std::string const &file)
    {
        if (std::filesystem::exists(file))
            std::filesystem::remove(file);
    };

    for (auto &r : to_remove)
    {
        rem(scryfall_folder + r);
        rem(scryfall_upscaled_folder + r);
        rem(scryfall_bled_folder + r);
    }

    for(auto iter = cards.begin(); iter != cards.end(); )
    {
        if(std::filesystem::exists(scryfall_bled_folder + iter->safe_card_name + ".png"))
        {
            std::filesystem::remove(scryfall_folder + iter->safe_card_name + ".png");
            iter = cards.erase(iter);
        }
        else
        {
            iter++;
        }

    }
}

void DownloadList(Configuration &conf)
{
    auto list = conf.GetDir(FILES_FOLDER SCRYFALL_FILE);

    if (!std::filesystem::exists(list))
        return;

    auto cards = ReadList(list);

    for(auto& card: cards)
    {
        conf.PrintList[card.safe_card_name+".png"] = card.amount;
    }

    CleanUpScryfallFolders(conf, cards);

    for (auto &card : cards)
    {
        if(DownloadCard(card, conf.GetDir(SCRYFALL_INPUT_FOLDER)))
            std::this_thread::sleep_for(std::chrono::milliseconds((std::rand() % 500) + 500));
    }
}

void PrepareScryfallImages(Configuration const &conf)
{
    auto scryfall_folder = conf.GetDir(SCRYFALL_INPUT_FOLDER);
    auto scryfall_upscaled_folder = conf.GetDir(SCRYFALL_UPSCALED_FOLDER);
    auto scryfall_bled_folder = conf.GetDir(SCRYFALL_BLED_FOLDER);

    FillEmptySpaces(scryfall_folder);

    if (!Upscale(conf, scryfall_folder, scryfall_upscaled_folder))
        return;

    AddBleed(conf, scryfall_upscaled_folder, scryfall_bled_folder);
    CropImages(conf, LoadImages(scryfall_bled_folder), conf.GetDir(CROP_FOLDER));
}

void PrepareScryfallCards(Configuration &conf)
{
    DownloadList(conf);
    PrepareScryfallImages(conf);
}