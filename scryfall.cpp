#include "scryfall.h"
#include "utility.h"
#include "configuration.h"
#include <format>
#include <fstream>
#include <sstream>
#include <iostream>
#include <thread>


void ReadList(std::filesystem::path const& list, std::vector<std::string>& output)
{
    std::string line;
    std::ifstream infile{list};
    while (std::getline(infile, line))
    {
        output.push_back(line);        
    }
    return;
}


void DownloadCard(std::string card_name, std::filesystem::path const& output_dir)
{
    if(std::filesystem::exists(".\\tempfile"))
        std::filesystem::remove(".\\tempfile");
        
    std::replace(card_name.begin(), card_name.end(), ' ', '+');
    if(std::filesystem::exists((output_dir/card_name).string()+".png"))
        return;
    
    
    RunCommand("curl -Ls -w %{url_effective} \"https://scryfall.com/search?q=" + std::format("{}\" --output .\\tmpfile", card_name));

    std::ifstream page{".\\tmpfile"};
    std::stringstream buffer;
    buffer << page.rdbuf();
    std::string page_content = buffer.str();

    auto pos = page_content.find("Download PNG", page_content.size()/2);
    auto href = page_content.rfind("href=", pos); //get href=
    auto png_link = page_content.substr(href+6, page_content.find("\"", href+6) - (href+6));
    RunCommand("curl -Ls -w %{url_effective} \"" + png_link + "\" --output " + (output_dir/card_name).string()+".png");
}

void DownloadList(std::filesystem::path const& list)
{
    std::vector<std::string> cards{};
    if(!std::filesystem::exists(list))
        return;
        
    ReadList(list, cards);

    for(auto& card : cards)
    {
        DownloadCard(card, SCRYFALL_INPUT_FOLDER);
        std::this_thread::sleep_for(std::chrono::milliseconds((std::rand() % 500) + 500));
    }
}