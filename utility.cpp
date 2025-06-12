#include "utility.h"
#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <filesystem>

bool IsImageExt(path const &ext)
{
    return ext == ".jpg" || ext == ".png" || ext == ".jpeg" || ext == ".tiff";
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

std::vector<path> LoadImages(path const &folder)
{
    std::vector<path> res{};
    if (!std::filesystem::exists(folder))
        return {};
    for (auto p : std::filesystem::directory_iterator{folder})
    {
        if (std::filesystem::is_regular_file(p) && IsImageExt(p.path().extension()))
        {
            #ifdef __WIN32__
            auto sb = p.path().filename().wstring();
            auto new_path = folder + FromWstring(sb);
            std::filesystem::rename(p, new_path);
            res.push_back(new_path);
            #else
            res.push_back(folder + p.path().filename().string());
            #endif
            
        }
    }
    return res;
}

bool RunCommand(std::string const &command, bool quiet)
{
    std::cout << "Running command: " << command << std::endl;
    int result = 0;
    if(quiet)
    {
        #ifdef __WIN32__
        result = system((command + " >nul 2>nul").c_str());
        #else
        result = system((command + " > /dev/null 2> /dev/null").c_str());
        #endif
        std::cout << "\nProcess returned " << result << std::endl;
    }
    else
    {
        char psBuffer[128];
        FILE *pPipe;

        if ((pPipe = popen(command.c_str(), "r")) == NULL)
            return false;

        
        while (fgets(psBuffer, 128, pPipe))
        {
            std::cout << psBuffer;
        }
        

        if (feof(pPipe))
        {
            result = pclose(pPipe);
            std::cout << "\nProcess returned " << result << std::endl;
        }
    }
    return result == 0;
}

void PrepareDirectory(path const& dir, bool clean)
{
    if(clean)
    {
        std::filesystem::remove_all(dir);
        PrepareDirectory(dir, false);
    }
    
    if(!std::filesystem::exists(dir))
    {
        std::filesystem::create_directories(dir);
        return;
    }

    
}