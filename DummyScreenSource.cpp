#include "DummyScreenSource.h"

#include <Windows.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

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
        nlohmann::json j;
        for (int y = 0; y < CDU_ROWS; ++y) {
            auto yStr = std::to_string(y);
            for (int x = 0; x < CDU_COLUMNS; ++x) {
                auto xStr = std::to_string(x);
                char text = 'A' + (x + y) % 26;
                if (x % 5 == 0 || y % 7 == 0) {
                    //text = ' ';
                }
                j[yStr][xStr]["text"] = std::string(1, text);
                if (smallRows_.count(y) > 0) {
                    j[yStr][xStr]["small"] = true;
                }
                if (reverseRows_.count(y) > 0) {
                    j[yStr][xStr]["reverse"] = true;
                }
                if (unusedRows_.count(y) > 0) {
                    j[yStr][xStr]["unused"] = true;
                }

                if (colorRows_.count(y) > 0) {
                    j[yStr][xStr]["color"] = colors_[colorRows_[y]];
                }
            }
        }
        f(j.dump().c_str());
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
