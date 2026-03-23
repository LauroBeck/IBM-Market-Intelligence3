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
            double rsi, atr, sma5, sma20;
            std::vector<double> buf(close.size());
            
            TA_RSI(0, close.size()-1, close.data(), 14, &b, &n, buf.data()); rsi = buf[n-1];
            TA_ATR(0, close.size()-1, high.data(), low.data(), close.data(), 14, &b, &n, buf.data()); atr = buf[n-1];
            TA_SMA(0, close.size()-1, close.data(), 5, &b, &n, buf.data()); sma5 = buf[n-1];
            TA_SMA(0, close.size()-1, close.data(), 20, &b, &n, buf.data()); sma20 = buf[n-1];

            double sentiment = (rsi * 0.4) + (sma5 > sma20 ? 30 : 0);
            std::string posture = (sentiment > 50) ? "STRONG" : (sentiment > 40 ? "NEUTRAL" : "WEAK");

            std::lock_guard<std::mutex> lock(output_mutex);
            // 1. Terminal Output
            std::cout << "| " << std::left << std::setw(6) << ticker 
                      << " | RSI: " << std::fixed << std::setprecision(1) << rsi 
                      << " | ATR: " << std::setw(5) << atr 
                      << " | " << posture << " |" << std::endl;

            // 2. Audit CSV Output (for Python Heatmap)
            std::ofstream audit("stargate_audit.csv", std::ios::app);
            audit << ticker << "," << rsi << "," << atr << "," << close.back() << ",2026-03-23\n";
        }
    }
    if(chunk.memory) free(chunk.memory);
    curl_easy_cleanup(curl);
}

int main() {
    std::vector<std::string> watch = {"^RUT", "^IXIC", "IBM", "^GSPC", "NVDA"};
    // Prepare CSV with Headers
    std::ofstream audit("stargate_audit.csv");
    audit << "Ticker,RSI,ATR,Price,Timestamp\n";
    audit.close();

    TA_Initialize();
    curl_global_init(CURL_GLOBAL_ALL);
    std::cout << "\nSTARGATE V3 | MAR 23 EOD ENGINE" << std::endl;
    std::vector<std::thread> th;
    for(auto& t : watch) th.emplace_back(process, t);
    for(auto& t : th) t.join();
    curl_global_cleanup();
    TA_Shutdown(); 
    return 0;
}
