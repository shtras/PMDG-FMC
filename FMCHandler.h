#pragma once
#include "MSFSScreenSource.h"
#include "DummyScreenSource.h"

#include <winsock2.h>
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
    ~FMCHandler();

    bool handleGet(CivetServer* server, mg_connection* conn) override;

    bool handlePost(CivetServer* server, struct mg_connection* conn) override;

    void Start();
    void Stop();
    void ToggleDummy();

private:
    void sendToSessions(const char* s);

    std::set<mg_connection*> activeSessions_;
    std::shared_ptr<ScreenSource> source_ = nullptr;
    bool dummy_ = false;
    std::string uri_;
    mutable std::mutex postMutex_;
    std::thread sourceThread_;
    std::atomic<bool> running_{true};
};
} // namespace FMC_Server