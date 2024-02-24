#pragma once

#include "ScreenSource.h"

#include <Windows.h>
#include <memory>
#include <utility>
#include <atomic>

namespace FMC_Server
{
class Screen;
class MSFSScreenSource : public ScreenSource
{
public:
    MSFSScreenSource();
    ~MSFSScreenSource() override;
    void Start(std::function<void(const char*)> f) override;
    void ButtonClick(std::string btn) override;
    void Invalidate() override;

private:
    HANDLE hSimConnect_ = nullptr;
    std::pair<std::unique_ptr<Screen>, std::unique_ptr<Screen>> screens_;
    bool invalidated_ = false;
};
} // namespace MFC_Server
