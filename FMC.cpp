#include "FMC.h"

#include "MSFSScreenSource.h"
#include "DummyScreenSource.h"

#define RAPIDJSON_HAS_STDSTRING 1

#include <windows.h>

#include <spdlog/spdlog.h>
#include <CivetServer.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include "PMDG_NG3_SDK.h"
#include <SimConnect.h>

#include <set>
#include <array>
#include <optional>
#include <functional>
#include <memory>
#include <mutex>

class ExampleHandler : public CivetHandler
{
public:
    explicit ExampleHandler(bool dummy)
        : dummy_(dummy)
    {
        if (dummy) {
            source_ = std::make_shared<FMC_Server::DummyScreenSource>();
        } else {
            source_ = std::make_shared<FMC_Server::MSFSScreenSource>();
        }
    }

    bool handleGet(CivetServer* server, mg_connection* conn)
    {
        const mg_request_info* req_info = mg_get_request_info(conn);
        source_->Invalidate();
        spdlog::info("Received connection: {}", req_info->request_uri);
        std::string uri = req_info->request_uri;
        if (uri == "/server_event") {
            mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: "
                            "text/event-stream\r\nAccess-Control-Allow-Origin: *\r\n\r\n");
            activeSessions_.insert(conn);
            int i = 0;
            for (;;) {
                if (activeSessions_.count(conn) == 0) {
                    break;
                }
                Sleep(1000);
            }

            return true;
        }
        return false;
    }

    bool handlePost(CivetServer* server, struct mg_connection* conn)
    {
        std::lock_guard l(postMutex_);
        mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: "
                        "text/html\r\nAccess-Control-Allow-Origin: *\r\n\r\n");
        mg_printf(conn, "<html><body>");
        mg_printf(conn, "</body></html>\n");
        const mg_request_info* req_info = mg_get_request_info(conn);
        std::string uri = req_info->request_uri;
        if (uri == "/button_click") {
            auto postData = CivetServer::getPostData(conn);
            spdlog::info("Post data: {}", postData);
            source_->ButtonClick(postData);
            return true;
        } else if (uri == "/refresh") {
            source_->Invalidate();
            return true;
        } else if (uri == "/toggle_dummy") {
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
        return false;
    }

    void sendToSessions(const char* s)
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

    void Start()
    {
        sourceThread_ = std::thread([this]() { source_->Start([this](const char* s) { sendToSessions(s); }); });
    }

private:
    std::set<mg_connection*> activeSessions_;
    std::shared_ptr<FMC_Server::ScreenSource> source_ = nullptr;
    bool dummy_ = false;
    mutable std::mutex postMutex_;
    std::thread sourceThread_;
};

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

    ExampleHandler h_ex(false);
    server.addHandler("/*", h_ex);
    h_ex.Start();

    for (;;) {
        Sleep(1000);
    }

    mg_exit_library();
    return 0;
}
