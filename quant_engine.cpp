#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <curl/curl.h>
#include "ta-lib/include/ta_libc.h"

std::mutex output_mutex;
struct MemoryStruct { char *memory; size_t size; };

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    char *ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) return 0;
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

std::vector<double> parse_json(const std::string& json, const std::string& key) {
    std::vector<double> res;
    std::string skey = "\"" + key + "\":[";
    size_t p = json.find(skey);
    if (p == std::string::npos) return res;
    size_t e = json.find("]", p);
    std::stringstream ss(json.substr(p + skey.length(), e - (p + skey.length())));
    std::string v;
    while (std::getline(ss, v, ',')) { if (v != "null" && !v.empty()) res.push_back(std::stod(v)); }
    return res;
}

void process(const std::string& ticker) {
    CURL* curl = curl_easy_init();
    if(!curl) return;
    struct MemoryStruct chunk = { (char*)malloc(1), 0 };
    std::string url = "https://query1.finance.yahoo.com/v8/finance/chart/" + ticker + "?interval=1d&range=3mo";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");

    if(curl_easy_perform(curl) == CURLE_OK && chunk.size > 500) {
        std::string s(chunk.memory);
        auto close = parse_json(s, "close");
        auto high = parse_json(s, "high");
        auto low = parse_json(s, "low");

        if (close.size() >= 30) {
            int b, n;
            double rsi, atr;
            std::vector<double> buf(close.size());
            
            TA_RSI(0, close.size()-1, close.data(), 14, &b, &n, buf.data()); rsi = buf[n-1];
            TA_ATR(0, close.size()-1, high.data(), low.data(), close.data(), 14, &b, &n, buf.data()); atr = buf[n-1];

            std::string posture = (rsi > 55) ? "BULLISH" : (rsi < 45 ? "BEARISH" : "NEUTRAL");

            std::lock_guard<std::mutex> lock(output_mutex);
            std::cout << "| " << std::left << std::setw(6) << ticker 
                      << " | LAST: $" << std::right << std::setw(10) << std::fixed << std::setprecision(2) << close.back()
                      << " | ATR: " << std::setw(6) << atr 
                      << " | RSI: " << std::setw(4) << rsi << " |" << std::endl;

            std::ofstream audit("stargate_audit.csv", std::ios::app);
            audit << ticker << "," << rsi << "," << atr << "," << close.back() << ",2026-03-23\n";
        }
    }
    if(chunk.memory) free(chunk.memory);
    curl_easy_cleanup(curl);
}

int main() {
    // Watchlist deployed with 3 Major Indices + High Alpha Targets
    std::vector<std::string> watch = {"^GSPC", "^DJI", "^RUT", "^IXIC", "IBM", "NVDA", "CVX", "JPM"};
    
    std::ofstream audit("stargate_audit.csv");
    audit << "Ticker,RSI,ATR,Price,Timestamp\n";
    audit.close();

    TA_Initialize();
    curl_global_init(CURL_GLOBAL_ALL);
    std::cout << "\nSTARGATE V3 | MACRO INDEX & SECTOR DEPLOYMENT" << std::endl;
    std::cout << "------------------------------------------------------------" << std::endl;
    std::vector<std::thread> th;
    for(auto& t : watch) th.emplace_back(process, t);
    for(auto& t : th) t.join();
    std::cout << "------------------------------------------------------------" << std::endl;
    curl_global_cleanup();
    TA_Shutdown(); 
    return 0;
}
