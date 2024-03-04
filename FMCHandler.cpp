#include "FMCHandler.h"
namespace FMC_Server
{
FMCHandler::FMCHandler(bool dummy, std::string uri)
    : dummy_(dummy)
    , uri_(uri)
{
    if (dummy) {
        source_ = std::make_shared<DummyScreenSource>();
    } else {
        source_ = std::make_shared<MSFSScreenSource>();
    }
}

FMCHandler::~FMCHandler()
{
    Stop();
}

bool FMCHandler::handleGet(CivetServer* server, mg_connection* conn)
{
    const mg_request_info* req_info = mg_get_request_info(conn);
    source_->Invalidate();
    spdlog::info("Received connection: {}", req_info->request_uri);
    std::string uri = req_info->request_uri;
    if (uri == uri_ + "/server_event") {
        mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: "
                        "text/event-stream\r\nAccess-Control-Allow-Origin: *\r\n\r\n");
        activeSessions_.insert(conn);
        int i = 0;
        while (running_) {
            if (activeSessions_.count(conn) == 0) {
                break;
            }
            Sleep(1000);
        }
        return true;
    }
    return false;
}

bool FMCHandler::handlePost(CivetServer* server, mg_connection* conn)
{
    std::lock_guard l(postMutex_);
    mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: "
                    "text/html\r\nAccess-Control-Allow-Origin: *\r\n\r\n");
    mg_printf(conn, "<html><body>");
    mg_printf(conn, "</body></html>\n");
    const mg_request_info* req_info = mg_get_request_info(conn);
    std::string uri = req_info->request_uri;
    if (uri == uri_ + "/button_click") {
        auto postData = CivetServer::getPostData(conn);
        spdlog::info("Post data: {}", postData);
        source_->ButtonClick(postData);
        return true;
    } else if (uri == uri_ + "/refresh") {
        source_->Invalidate();
        return true;
    } else if (uri == uri_ + "/toggle_dummy") {
        ToggleDummy();
        return true;
    }
    return false;
}

void FMCHandler::Start()
{
    sourceThread_ = std::thread([this]() { source_->Start([this](const char* s) { sendToSessions(s); }); });
}

void FMCHandler::Stop()
{
    if (!running_) {
        return;
    }
    running_ = false;
    source_->Stop();
    if (sourceThread_.joinable()) {
        sourceThread_.join();
    }
}

void FMCHandler::ToggleDummy()
{
    source_->Stop();
    if (sourceThread_.joinable()) {
        sourceThread_.join();
    }
    if (dummy_) {
        source_ = std::make_shared<FMC_Server::MSFSScreenSource>();
    } else {
        source_ = std::make_shared<FMC_Server::DummyScreenSource>();
    }
    dummy_ = !dummy_;
    Start();
}

void FMCHandler::sendToSessions(const char* s)
{
    std::set<mg_connection*> toDelete;
    for (const auto& session : activeSessions_) {
        std::stringstream ss;
        ss << "data: " << s << "\r\n\r\n";
        //spdlog::info("{}", ss.str());
        int rc = mg_printf(session, ss.str().c_str());
        if (rc < 0) {
            spdlog::info("Connection seems to have been closed");
            toDelete.insert(session);
        }
    }
    for (const auto& session : toDelete) {
        activeSessions_.erase(session);
    }
}
} // namespace FMC_Server
