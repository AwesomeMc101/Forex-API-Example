// Forex Analysis.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define CURL_STATICLIB
#include <iostream>
#include <string>
#include <vector>

#include "cURL/curl.h"

#pragma comment (lib, "cURL/libcurl_a.lib")

#pragma comment (lib, "Normaliz.lib")
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Wldap32.lib")
#pragma comment (lib, "advapi32.lib")
#pragma comment (lib, "crypt32.lib")
#pragma comment(lib, "urlmon.lib")


class spreadData {
public:
    std::vector<std::string> ticker; //USD, GMD, etc
    std::vector<std::string> value; //not doing float conversion for this
};

namespace cURL_Info
{
    static size_t curlWriteData(void* buffer, size_t size, size_t nmemb, void* param)
    {
        std::string& text = *static_cast<std::string*>(param);
        size_t totalsize = size * nmemb;
        text.append(static_cast<char*>(buffer), totalsize);
        return totalsize;
    }
    std::string returnJson(std::string link)
    {
        CURL* curl;
        CURLcode res;
        std::string readBuffer;
        curl = curl_easy_init();
        if (curl) {            
            curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteData);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_perform(curl);
        }
        return readBuffer;   
    }
}

namespace Parser
{
    static void simpleClean(std::string& s)
    {
        s.erase(remove(s.begin(), s.end(), '{'), s.end());
        s.erase(remove(s.begin(), s.end(), '}'), s.end());
        s.erase(remove(s.begin(), s.end(), '\"'), s.end());
        s.erase(remove(s.begin(), s.end(), ','), s.end());
        s.erase(remove(s.begin(), s.end(), '_'), s.end());
    }
    void parse(spreadData& sheet)
    {
        std::string fullJson = cURL_Info::returnJson("https://api.exchangerate.host/latest");

        size_t ratesOffset = fullJson.find("rates");
        if (ratesOffset == std::string::npos) { printf("Error retrieving information.\n");  return; }
        fullJson = fullJson.substr(ratesOffset);
        
        size_t comOffset = fullJson.find(",");
        size_t oldComOffset = ratesOffset;
        while (oldComOffset != std::string::npos)
        {
            comOffset = fullJson.find(",", comOffset+1);
            if (comOffset == std::string::npos)
            {
                break;
            }
            if (oldComOffset == 258) /* I hate magic numbers too, but you need this one or EVERYTHING is gonna break. */
            {
                oldComOffset = comOffset;
                continue;
            }
            std::string sub = fullJson.substr(comOffset, (comOffset - oldComOffset));
            simpleClean(sub);

            char delim = ':';
            std::string build = "";
            int leftChar = 0;
            while (leftChar <= sub.length())
            {
                if (sub[leftChar] == ':')
                {
                    //std::cout << "Pushback_A: " << build << std::endl;
                    sheet.ticker.push_back(build);
                    build.clear();
                }
                else
                {
                    build = build + sub[leftChar];
                }
                leftChar++;
            }
            sheet.value.push_back(build);
            oldComOffset = comOffset;
        }
    }
}

int main()
{
    std::cout << "wow men Forex Analyser made by AwesomeMc101?!? amazing!\nCompares to the EURO, all information is gathered from the source." << std::endl;
 

    spreadData sheet;
    Parser::parse(sheet);

    for (int i = 0; i < sheet.value.size(); i++)
    {
        std::cout << sheet.ticker[i] << ": " << sheet.value[i] << "\n";
    }
}
