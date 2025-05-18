#include "utility.h"

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
    if(!std::filesystem::exists(folder)) return {};
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