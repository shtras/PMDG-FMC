#include "FrontendHandler.h"

#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <map>

namespace
{
std::map<std::string, std::string> contentTypes = {{"html", "text/html"}, {"css", "text/css"},
    {"js", "text/javascript"}, {"png", "image/png"}, {"ico", "image/x-icon"}, {"json", "application/json"},
    {"wav", "audio/wav"}, {"txt", "text/plain"}, {"map", "text/plain"}};
}

namespace FMC_Server
{
FrontendHandler::FrontendHandler(const std::string& uri, const std::string& path)
    : path_(path)
{
}

bool FrontendHandler::handleGet(CivetServer* server, mg_connection* conn)
{
    namespace fs = std::filesystem;

    const mg_request_info* req_info = mg_get_request_info(conn);
    std::string uri = req_info->request_uri;

    spdlog::info("Frontend: {}", uri);

    if (uri == "/") {
        uri = "/index.html";
    }
    std::string filePath = path_ + uri;
    fs::path path{filePath};
    if (!std::filesystem::exists(path)) {
        spdlog::warn("Not found: {}", filePath);
        return false;
    }
    std::string extension = path.extension().string();
    if (extension.empty()) {
        spdlog::warn("Empty extension: {}", filePath);
        return false;
    }
    extension = extension.substr(1);
    auto contentType = contentTypes.find(extension);
    if (contentType == contentTypes.end()) {
        spdlog::warn("Unknown content type: {}", filePath);
        return false;
    }
    mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: ");
    mg_printf(conn, contentType->second.c_str());
    std::ios_base::openmode openMode = std::ios_base::in;
    if (extension == "ico" || extension == "png" || extension == "wav") {
        mg_printf(conn, "\r\nContent-Length: ");
        mg_printf(conn, std::to_string(fs::file_size(path)).c_str());

        openMode |= std::ios_base::binary;
    }
    mg_printf(conn, "\r\n\r\n");
    
    mg_send_file(conn, filePath.c_str());

    return true;
}

} // namespace FMC_Server
