#pragma once

#include <winsock2.h>
#include <CivetServer.h>

#include <string>

namespace FMC_Server
{

class FrontendHandler : public CivetHandler
{
public:
    explicit FrontendHandler(const std::string& uri, const std::string& path);

    bool handleGet(CivetServer* server, mg_connection* conn) override;

private:
    std::string path_;
};
} // namespace FMC_Server
