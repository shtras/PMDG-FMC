#include "DummyScreenSource.h"

#include <Windows.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include <string>

namespace FMC_Server
{
class DummyScreen
{
public:
    DummyScreen()
    {
        for (int i = 0; i < ROWS; ++i) {
            rows_.emplace_back();
            for (int j = 0; j < COLUMNS; ++j) {
                rows_.back() += 'A' + (i + j) % 26;
            }
        }
    }

    ~DummyScreen() = default;

    nlohmann::json ToJson()
    {
        nlohmann::json j;
        for (int y = 0; y < ROWS; ++y) {
            auto yStr = std::to_string(y);
            for (int x = 0; x < COLUMNS; ++x) {
                auto xStr = std::to_string(x);
                if (x >= rows_[y].size()) {
                    j[yStr][xStr]["text"] = " ";
                } else {
                    j[yStr][xStr]["text"] = std::string(1, rows_[y][x]);
                }
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
        for (int x = 0; x < COLUMNS; ++x) {
            if (x >= scratchpad_.size()) {
                j["13"][std::to_string(x)]["text"] = " ";
            } else {
                j["13"][std::to_string(x)]["text"] = std::string(1, scratchpad_[x]);
            }
        }
        return j;
    }

    void ButtonClick(std::string btn)
    {
        if (btn.size() == 2 && (btn[0] == 'L' || btn[0] == 'R')) {
            int rowId = (btn[1] - '0') * 2;
            if (btn[0] == 'L') {
                if (!scratchpad_.empty()) {
                    rows_[rowId] = scratchpad_;
                    scratchpad_.clear();
                } else if (smallRows_.count(rowId) > 0) {
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
        } else if (btn.size() == 1) {
            scratchpad_ += btn;
        } else if (btn == "CLR" && scratchpad_.size() > 0) {
            scratchpad_.pop_back();
        }
    }

private:
    static constexpr int ROWS = 13;
    static constexpr int COLUMNS = 24;
    std::vector<std::string> rows_;
    std::set<int> smallRows_;
    std::set<int> reverseRows_;
    std::set<int> unusedRows_;
    std::map<int, int> colorRows_;
    std::array<std::string, 6> colors_ = {"white", "cyan", "lawngreen", "magenta", "chocolate", "red"};
    std::string scratchpad_;
};

DummyScreenSource::DummyScreenSource()
    : screens_{std::make_unique<DummyScreen>(), std::make_unique<DummyScreen>()}
{
}

DummyScreenSource::~DummyScreenSource() = default;

void DummyScreenSource::Start(std::function<void(const char*)> f)
{
    spdlog::info("Starting dummy screen source");
    running_ = true;
    while (running_) {
        nlohmann::json j;
        j["L"] = screens_.first->ToJson();
        j["R"] = screens_.second->ToJson();
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
    if (btn.empty()) {
        spdlog::error("Empty button code received");
        return;
    }
    if (btn.size() <= 2 || btn[1] != ';') {
        screens_.first->ButtonClick(btn);
    } else if (btn[0] == '0') {
        screens_.first->ButtonClick(btn.substr(2));
    } else if (btn[0] == '1') {
        screens_.second->ButtonClick(btn.substr(2));
    } else {
        spdlog::error("Wrong button code received: {}", btn);
    }
}
} // namespace FMC_Server
