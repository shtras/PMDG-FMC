#pragma once

#include <Finally.h>

#include <spdlog/spdlog.h>

#include <functional>
#include <string>
#include <atomic>
#include <semaphore>

namespace FMC_Server
{
class ScreenSource
{
public:
    virtual ~ScreenSource()
    {
    }

    virtual void Start(std::function<void(const char*)> f) = 0;
    void Stop()
    {
        spdlog::info("Stopping source...");
        finally f([] { spdlog::info("Stopped source"); });
        if (!running_) {
            return;
        }
        running_ = false;
        stoppingSem_.acquire();
    }

    virtual void Test()
    {
    }

    virtual void ButtonClick(std::string btn) = 0;

    virtual void Invalidate() = 0;

protected:
    std::atomic<bool> running_{false};
    std::binary_semaphore stoppingSem_{0};
};
} // namespace MFC_Server
