#pragma once

#include "ScreenSource.h"

#include <functional>
#include <string>
#include <set>
#include <map>
#include <array>

namespace FMC_Server
{
class DummyScreen;
class DummyScreenSource : public ScreenSource
{
public:
    DummyScreenSource();
    ~DummyScreenSource();
    void Start(std::function<void(const char*)> f) override;
    void Invalidate() override;
    void ButtonClick(std::string btn) override;

private:
    std::pair<std::unique_ptr<DummyScreen>, std::unique_ptr<DummyScreen>> screens_;

};
} // namespace MFC_Server
