#pragma once

#include "ScreenSource.h"

#include <functional>
#include <string>
#include <set>
#include <map>
#include <array>

namespace FMC_Server
{
class DummyScreenSource : public ScreenSource
{
public:
    void Start(std::function<void(const char*)> f) override;
    void Invalidate() override;
    void ButtonClick(std::string btn) override;

private:
    std::array<std::string, 6> colors_ = {"white", "cyan", "lawngreen", "magenta", "chocolate", "red"}; 
    std::set<int> smallRows_;
    std::set<int> reverseRows_;
    std::set<int> unusedRows_;
    std::map<int, int> colorRows_;
};
} // namespace MFC_Server
