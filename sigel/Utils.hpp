#pragma once

#include <vector>
#include <fstream>

namespace sigel
{
    inline void status(const std::string &module, const std::string &msg)
    {
        printf("[ %-8s ] > %s\n", module.c_str(), msg.c_str());
    }

    static std::vector<char> readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file!");
        }

        std::vector<char> buffer(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        file.close();

        return buffer;
    }

    static std::string toLower(const std::string& str) {
        std::string lowerStr = str;
        std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
            [](unsigned char c){ return std::tolower(c); });
        return lowerStr;
    }

    static std::string getBaseDir(const std::string &path)
    {
        std::string base_dir = "";
        size_t pos = path.find_last_of("/\\");
        if (pos != std::string::npos) {
            base_dir = path.substr(0, pos + 1);
        }

        return base_dir;
    }
}
