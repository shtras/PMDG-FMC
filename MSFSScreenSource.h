#pragma once

#include "ScreenSource.h"

#include <Windows.h>
#include <memory>
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
    void Test() override;
    void ButtonClick(std::string btn) override;
    void Invalidate() override;

private:
    HANDLE hSimConnect_ = nullptr;
    std::unique_ptr<Screen> screen_ = nullptr;
    bool invalidated_ = false;
};
} // namespace MFC_Server
