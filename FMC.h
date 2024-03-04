#pragma once
#include "FMCHandler.h"

#include <thread>
#include <memory>

namespace FMC_Server
{
class FMC
{
public:
    void Start(const std::string& port);
    void Stop();
    void ToggleDummy();

private:
    void threadProc();
    std::thread runThread_;
    std::string port_;
    std::atomic<bool> running_{false};
    std::unique_ptr<FMCHandler> fmcHandler_ = nullptr;
};
} // namespace FMC_Server
