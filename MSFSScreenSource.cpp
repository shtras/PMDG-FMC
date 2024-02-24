#include "MSFSScreenSource.h"
#include "Finally.h"

#include <windows.h>
#include <spdlog/spdlog.h>
#include <array>
#include <map>
#include <thread>
#include <chrono>

#include <nlohmann/json.hpp>
#include <SimConnect.h>

#include "PMDG_NG3_SDK.h"

#include "event_definitions_inl.h"

namespace FMC_Server
{
class Screen
{
public:
    class Cell
    {
    public:
        enum class Color { WHITE, CYAN, GREEN, MAGENTA, AMBER, RED };
        Cell()
        {
        }
        explicit Cell(const PMDG_NG3_CDU_Cell& pmdgCell)
            : text_(pmdgCell.Symbol)
        {
            if (pmdgCell.Flags & PMDG_NG3_CDU_FLAG_SMALL_FONT) {
                small_ = true;
            }
            if (pmdgCell.Flags & PMDG_NG3_CDU_FLAG_REVERSE) {
                reverse_ = true;
            }
            if (pmdgCell.Flags & PMDG_NG3_CDU_FLAG_UNUSED) {
                unused_ = true;
            }
            switch (pmdgCell.Color) {
                case PMDG_NG3_CDU_COLOR_WHITE:
                    color_ = Color::WHITE;
                    break;
                case PMDG_NG3_CDU_COLOR_CYAN:
                    color_ = Color::CYAN;
                    break;
                case PMDG_NG3_CDU_COLOR_GREEN:
                    color_ = Color::GREEN;
                    break;
                case PMDG_NG3_CDU_COLOR_MAGENTA:
                    color_ = Color::MAGENTA;
                    break;
                case PMDG_NG3_CDU_COLOR_AMBER:
                    color_ = Color::AMBER;
                    break;
                case PMDG_NG3_CDU_COLOR_RED:
                    color_ = Color::RED;
                    break;
                default:
                    spdlog::error("Unknown color value: {}", pmdgCell.Color);
                    break;
            }
        }
        bool operator==(const Cell& other) const
        {
            return other.small_ == small_ && other.reverse_ == reverse_ && other.unused_ == unused_ &&
                   other.color_ == color_ && other.text_ == text_;
        }

        nlohmann::json ToJson() const
        {
            nlohmann::json j;
            if (text_ <= 125) {
                auto s = std::string(1, text_);
                j["text"] = std::string(1, text_);
            } else {
                j["text"] = static_cast<int>(text_);
            }
            if (small_) {
                j["small"] = true;
            }
            if (reverse_) {
                j["reverse"] = true;
            }
            if (unused_) {
                j["unused"] = true;
            }
            if (color_ != Color::WHITE) {
                j["color"] = colorName(color_);
            }
            return j;
        }

    private:
        std::string colorName(Color color) const
        {
            switch (color) {
                case Color::WHITE:
                    return "white";
                case Color::CYAN:
                    return "cyan";
                case Color::GREEN:
                    return "lawngreen";
                case Color::MAGENTA:
                    return "magenta";
                case Color::AMBER:
                    return "chocolate";
                case Color::RED:
                    return "red";
            }
            spdlog::error("Unknown color: {}", static_cast<int>(color));
            return "white";
        }
        bool small_ = false;
        bool reverse_ = false;
        bool unused_ = false;
        Color color_ = Color::WHITE;
        unsigned char text_ = '?';
    };

    Screen()
    {
    }

    explicit Screen(const PMDG_NG3_CDU_Screen* pmdgScreen)
    {
        Assign(pmdgScreen);
    }
    ~Screen() = default;

    nlohmann::json Assign(const PMDG_NG3_CDU_Screen* pmdgScreen)
    {
        nlohmann::json j;
        for (int y = 0; y < CDU_ROWS; ++y) {
            auto yStr = std::to_string(y);
            for (int x = 0; x < CDU_COLUMNS; ++x) {
                if (pmdgScreen) {
                    const PMDG_NG3_CDU_Cell& pmdgCell = pmdgScreen->Cells[x][y];
                    Cell c(pmdgCell);
                    if (c == cells_[y][x]) {
                        continue;
                    }
                    cells_[y][x] = c;
                }
                j[yStr][std::to_string(x)] = cells_[y][x].ToJson();
            }
        }
        return j;
    }

private:
    std::array<std::array<Cell, CDU_COLUMNS>, CDU_ROWS> cells_;
};

MSFSScreenSource::MSFSScreenSource()
    : screens_{std::make_unique<Screen>(), std::make_unique<Screen>()}
{
}

MSFSScreenSource::~MSFSScreenSource() = default;

void MSFSScreenSource::Start(std::function<void(const char*)> f)
{
    HRESULT hr;
    finally _([this] {
        if (hSimConnect_) {
            SimConnect_Close(hSimConnect_);
        }
        stoppingSem_.release();
    });
    spdlog::info("Starting MSFS screen source");
    running_ = true;
    for (;;) {
        if (!running_) {
            return;
        }
        hr = SimConnect_Open(&hSimConnect_, "PMDG NG3 CDU Test", NULL, 0, 0, 0);
        if (FAILED(hr)) {
            spdlog::error("Failed to open SimConnect. Will retry in 30 seconds...");
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(30s);
            continue;
        }
        break;
    }
    spdlog::info("SimConnect opened");
    // Associate an ID with the PMDG data area name
    hr = SimConnect_MapClientDataNameToID(hSimConnect_, PMDG_NG3_CDU_0_NAME, PMDG_NG3_CDU_0_ID);
    hr = SimConnect_MapClientDataNameToID(hSimConnect_, PMDG_NG3_CDU_1_NAME, PMDG_NG3_CDU_1_ID);

    for (const auto& eventMapping : EventsMap) {
        std::stringstream ss;
        ss << "#" << eventMapping.second;
        hr = SimConnect_MapClientEventToSimEvent(hSimConnect_, eventMapping.first, ss.str().c_str());
        if (FAILED(hr)) {
            spdlog::error("Event mapping failed");
            return;
        }
    }

    // Define the data area structure - this is a required step
    hr = SimConnect_AddToClientDataDefinition(
        hSimConnect_, PMDG_NG3_CDU_0_DEFINITION, 0, sizeof(PMDG_NG3_CDU_Screen), 0, 0);
    hr = SimConnect_AddToClientDataDefinition(
        hSimConnect_, PMDG_NG3_CDU_1_DEFINITION, 0, sizeof(PMDG_NG3_CDU_Screen), 0, 0);

    // Sign up for notification of data change.
    // SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED flag asks for the data to be sent only when some of the data is changed.
    hr = SimConnect_RequestClientData(hSimConnect_, PMDG_NG3_CDU_0_ID, CDU_L_DATA_REQUEST, PMDG_NG3_CDU_0_DEFINITION,
        SIMCONNECT_CLIENT_DATA_PERIOD_VISUAL_FRAME, SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED, 0, 0, 0);
    if (FAILED(hr)) {
        spdlog::error("Request CDU1 failed");
    }
    hr = SimConnect_RequestClientData(hSimConnect_, PMDG_NG3_CDU_1_ID, CDU_R_DATA_REQUEST, PMDG_NG3_CDU_1_DEFINITION,
        SIMCONNECT_CLIENT_DATA_PERIOD_VISUAL_FRAME, SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED, 0, 0, 0);
    if (FAILED(hr)) {
        spdlog::error("Request CDU2 failed");
    }
    //// End init simconnect
    while (running_) {
        SIMCONNECT_RECV* pData = nullptr;
        DWORD cbData = 0;

        HRESULT hr = SimConnect_GetNextDispatch(hSimConnect_, &pData, &cbData);
        bool success = false;
        PMDG_NG3_CDU_Screen* screen = nullptr;
        nlohmann::json j;

        if (SUCCEEDED(hr)) {
            switch (pData->dwID) {
                case SIMCONNECT_RECV_ID_CLIENT_DATA: // Receive and save the NG3 CDU screen data block
                {
                    SIMCONNECT_RECV_CLIENT_DATA* pObjData = (SIMCONNECT_RECV_CLIENT_DATA*)pData;

                    switch (pObjData->dwRequestID) {
                        case CDU_L_DATA_REQUEST: {
                            spdlog::info("CDU L");
                            screen = (PMDG_NG3_CDU_Screen*)&pObjData->dwData;
                            j["L"] = screens_.first->Assign(screen);
                            success = true;
                            break;
                        }
                        case CDU_R_DATA_REQUEST:
                            spdlog::info("CDU R");
                            screen = (PMDG_NG3_CDU_Screen*)&pObjData->dwData;
                            j["R"] = screens_.second->Assign(screen);
                            success = true;
                            break;
                    }
                    break;
                }
            }
        }
        if (!success) {
            if (invalidated_) {
                j["L"] = screens_.first->Assign(nullptr);
                j["R"] = screens_.second->Assign(nullptr);
                f(j.dump().c_str());
                invalidated_ = false;
            }
            Sleep(50);
            continue;
        }
        f(j.dump().c_str());
        Sleep(50);
    }
}

void MSFSScreenSource::ButtonClick(std::string btn)
{
    if (btn.size() < 2 || (btn[0] != '0' && btn[0] != '1' && btn[1] != ';')) {
        btn = "0;" + btn;
    }
    if (ButtonsMap.count(btn) == 0) {
        spdlog::error("Unknown button: {}", btn);
        return;
    }
    int parameter = 1;
    auto hr = SimConnect_TransmitClientEvent(hSimConnect_, 0, ButtonsMap.at(btn), MOUSE_FLAG_LEFTSINGLE,
        SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
    hr = SimConnect_TransmitClientEvent(hSimConnect_, 0, ButtonsMap.at(btn), MOUSE_FLAG_LEFTRELEASE,
        SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
    using namespace std::chrono_literals;
    //std::this_thread::sleep_for(10ms);
    if (FAILED(hr)) {
        spdlog::error("Result: {}", hr);
    }
}

void MSFSScreenSource::Invalidate()
{
    invalidated_ = true;
}
} // namespace FMC_Server