#include "FMC.h"

#include "FMCHandler.h"
#include "FrontendHandler.h"

#include <windows.h>

#include <spdlog/spdlog.h>
#include <CivetServer.h>
#include "PMDG_NG3_SDK.h"
#include <SimConnect.h>



int main()
{
    spdlog::info("Hello CMake.");

    mg_init_library(0);

    const char* options[] = {"listening_ports", "3456", 0};

    std::vector<std::string> cpp_options;
    for (int i = 0; i < (sizeof(options) / sizeof(options[0]) - 1); i++) {
        cpp_options.push_back(options[i]);
    }
    CivetServer server(cpp_options);

    FMC_Server::FMCHandler h_ex(true, "/fmc");
    server.addHandler("/fmc", h_ex);

    FMC_Server::FrontendHandler frontendHandler("", "frontend");
    server.addHandler("", frontendHandler);
    h_ex.Start();

    for (;;) {
        Sleep(1000);
    }

    mg_exit_library();
    return 0;
}
