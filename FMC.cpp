#include "FMC.h"

#include "FrontendHandler.h"

#include <winsock2.h>
#include <CivetServer.h>
#include "PMDG_NG3_SDK.h"
#include <SimConnect.h>

namespace FMC_Server
{
void FMC::Start(const std::string& port)
{
    port_ = port;
    running_ = true;
    runThread_ = std::thread(&FMC::threadProc, this);
    spdlog::info("start completed");
}

void FMC::Stop()
{
    running_ = false;
    if (runThread_.joinable()) {
        runThread_.join();
    }
}

void FMC::ToggleDummy()
{
    if (fmcHandler_) {
        fmcHandler_->ToggleDummy();
    }
}

void FMC::threadProc()
{
    mg_init_library(0);

    const char* options[] = {"listening_ports", port_.c_str(), 0};

    std::vector<std::string> cpp_options;
    for (int i = 0; i < (sizeof(options) / sizeof(options[0]) - 1); i++) {
        cpp_options.push_back(options[i]);
    }
    CivetServer server(cpp_options);

    fmcHandler_ = std::make_unique<FMCHandler>(true, "/fmc");
    server.addHandler("/fmc", *fmcHandler_);

    FrontendHandler frontendHandler("", "frontend");
    server.addHandler("", frontendHandler);

    fmcHandler_->Start();

    while (running_) {
        Sleep(1000);
    }

    fmcHandler_->Stop();
    server.close();
    mg_exit_library();
    fmcHandler_.reset();
    spdlog::info("Threadproc finished");
}
} // namespace FMC_Server
