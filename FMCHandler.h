#pragma once
#include "MSFSScreenSource.h"
#include "DummyScreenSource.h"

#include <CivetServer.h>

#include <string>
#include <set>
#include <array>
#include <functional>
#include <memory>
#include <mutex>

namespace FMC_Server
{
class FMCHandler : public CivetHandler
{
public:
    explicit FMCHandler(bool dummy, std::string uri);

    bool handleGet(CivetServer* server, mg_connection* conn) override;

    bool handlePost(CivetServer* server, struct mg_connection* conn) override;

    void Start();

private:
    void sendToSessions(const char* s);

    std::set<mg_connection*> activeSessions_;
    std::shared_ptr<ScreenSource> source_ = nullptr;
    bool dummy_ = false;
    std::string uri_;
    mutable std::mutex postMutex_;
    std::thread sourceThread_;
};
} // namespace FMC_Server