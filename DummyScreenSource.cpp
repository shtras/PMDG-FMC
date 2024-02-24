#include "DummyScreenSource.h"

#include <Windows.h>
#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <spdlog/spdlog.h>

#include <string>

namespace FMC_Server
{
void DummyScreenSource::Start(std::function<void(const char*)> f)
{
    spdlog::info("Starting dummy screen source");
    constexpr int CDU_ROWS = 14;
    constexpr int CDU_COLUMNS = 24;
    running_ = true;
    while (running_) {
        rapidjson::Document d;
        d.SetObject();
        auto& a = d.GetAllocator();
        for (int y = 0; y < CDU_ROWS; ++y) {
            rapidjson::Value row(rapidjson::kObjectType);
            for (int x = 0; x < CDU_COLUMNS; ++x) {
                char text = 'A' + (x + y) % 26;
                if (x % 5 == 0 || y % 7 == 0) {
                    //text = ' ';
                }
                rapidjson::Value key(std::to_string(x), a);
                rapidjson::Value value(std::string(1, text), a);
                rapidjson::Value cell(rapidjson::kObjectType);
                cell.AddMember("text", value, a);

                if (smallRows_.count(y) > 0) {
                    cell.AddMember("small", true, a);
                }
                if (reverseRows_.count(y) > 0) {
                    cell.AddMember("reverse", true, a);
                }
                if (unusedRows_.count(y) > 0) {
                    cell.AddMember("unused", true, a);
                }

                if (colorRows_.count(y) > 0) {
                    cell.AddMember("color", colors_[colorRows_[y]], a);
                }

                row.AddMember(key, cell, a);
            }
            rapidjson::Value colKey(std::to_string(y), a);
            d.AddMember(colKey, row, a);
        }

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        d.Accept(writer);

        f(buffer.GetString());

        Sleep(1000);
    }
    stoppingSem_.release();
}

void DummyScreenSource::Invalidate()
{
}

void DummyScreenSource::ButtonClick(std::string btn)
{
    if (btn.size() == 2 && (btn[0] == 'L' || btn[0] == 'R')) {
        int rowId = (btn[1] - '0') * 2;
        if (btn[0] == 'L') {
            if (smallRows_.count(rowId) > 0) {
                smallRows_.erase(rowId);
                reverseRows_.insert(rowId);
            } else if (reverseRows_.count(rowId) > 0) {
                reverseRows_.erase(rowId);
                unusedRows_.insert(rowId);
            } else {
                unusedRows_.erase(rowId);
                smallRows_.insert(rowId);
            }
        } else {
            if (colorRows_.count(rowId) > 0) {
                int currColor = colorRows_.at(rowId);
                ++currColor;
                if (currColor >= colors_.size()) {
                    colorRows_.erase(rowId);
                } else {
                    colorRows_[rowId] = currColor;
                }
                spdlog::info("{} {} {}", currColor, colors_.size(), colors_[colorRows_[rowId]]);
            } else {
                colorRows_[rowId] = 1;
            }
        }
    }
}
} // namespace MFC_Server
