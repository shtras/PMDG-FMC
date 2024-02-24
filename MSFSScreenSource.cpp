#include "MSFSScreenSource.h"
#include "Finally.h"

#include <windows.h>
#include <spdlog/spdlog.h>
#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <array>
#include <map>
#include <thread>
#include <chrono>

#include <SimConnect.h>

#include "PMDG_NG3_SDK.h"

namespace
{
enum DATA_REQUEST_ID { CDU_DATA_REQUEST };
enum EVENT_ID {
    EVENT_CDU_L_NEXT_PAGE,
    EVENT_CDU_L_L1,
    EVENT_CDU_L_L2,
    EVENT_CDU_L_L3,
    EVENT_CDU_L_L4,
    EVENT_CDU_L_L5,
    EVENT_CDU_L_L6,
    EVENT_CDU_L_R1,
    EVENT_CDU_L_R2,
    EVENT_CDU_L_R3,
    EVENT_CDU_L_R4,
    EVENT_CDU_L_R5,
    EVENT_CDU_L_R6,
    EVENT_CDU_INIT_REF,
    EVENT_CDU_RTE,
    EVENT_CDU_CLB,
    EVENT_CDU_CRZ,
    EVENT_CDU_DES,
    EVENT_CDU_MENU,
    EVENT_CDU_LEGS,
    EVENT_CDU_DEP_ARR,
    EVENT_CDU_HOLD,
    EVENT_CDU_PROG,
    EVENT_CDU_EXEC,
    EVENT_CDU_N1_LIMIT,
    EVENT_CDU_FIX,
    EVENT_CDU_PREV_PAGE,
    EVENT_CDU_NEXT_PAGE,
    EVENT_CDU_1,
    EVENT_CDU_2,
    EVENT_CDU_3,
    EVENT_CDU_4,
    EVENT_CDU_5,
    EVENT_CDU_6,
    EVENT_CDU_7,
    EVENT_CDU_8,
    EVENT_CDU_9,
    EVENT_CDU_0,
    EVENT_CDU_DOT,
    EVENT_CDU_PLUS_MINUS,
    EVENT_CDU_A,
    EVENT_CDU_B,
    EVENT_CDU_C,
    EVENT_CDU_D,
    EVENT_CDU_E,
    EVENT_CDU_F,
    EVENT_CDU_G,
    EVENT_CDU_H,
    EVENT_CDU_I,
    EVENT_CDU_J,
    EVENT_CDU_K,
    EVENT_CDU_L,
    EVENT_CDU_M,
    EVENT_CDU_N,
    EVENT_CDU_O,
    EVENT_CDU_P,
    EVENT_CDU_Q,
    EVENT_CDU_R,
    EVENT_CDU_S,
    EVENT_CDU_T,
    EVENT_CDU_U,
    EVENT_CDU_V,
    EVENT_CDU_W,
    EVENT_CDU_X,
    EVENT_CDU_Y,
    EVENT_CDU_Z,
    EVENT_CDU_SP,
    EVENT_CDU_DEL,
    EVENT_CDU_SLASH,
    EVENT_CDU_CLR
};

std::map<EVENT_ID, int> EventsMap = {{EVENT_CDU_L_NEXT_PAGE, EVT_CDU_L_NEXT_PAGE}, {EVENT_CDU_L_L1, EVT_CDU_L_L1},
    {EVENT_CDU_L_L2, EVT_CDU_L_L2}, {EVENT_CDU_L_L3, EVT_CDU_L_L3}, {EVENT_CDU_L_L4, EVT_CDU_L_L4},
    {EVENT_CDU_L_L5, EVT_CDU_L_L5}, {EVENT_CDU_L_L6, EVT_CDU_L_L6}, {EVENT_CDU_L_R1, EVT_CDU_L_R1},
    {EVENT_CDU_L_R2, EVT_CDU_L_R2}, {EVENT_CDU_L_R3, EVT_CDU_L_R3}, {EVENT_CDU_L_R4, EVT_CDU_L_R4},
    {EVENT_CDU_L_R5, EVT_CDU_L_R5}, {EVENT_CDU_L_R6, EVT_CDU_L_R6}, {EVENT_CDU_INIT_REF, EVT_CDU_L_INIT_REF},
    {EVENT_CDU_RTE, EVT_CDU_L_RTE}, {EVENT_CDU_CLB, EVT_CDU_L_CLB}, {EVENT_CDU_CRZ, EVT_CDU_L_CRZ},
    {EVENT_CDU_DES, EVT_CDU_L_DES}, {EVENT_CDU_MENU, EVT_CDU_L_MENU}, {EVENT_CDU_LEGS, EVT_CDU_L_LEGS},
    {EVENT_CDU_DEP_ARR, EVT_CDU_L_DEP_ARR}, {EVENT_CDU_HOLD, EVT_CDU_L_HOLD}, {EVENT_CDU_PROG, EVT_CDU_L_PROG},
    {EVENT_CDU_EXEC, EVT_CDU_L_EXEC}, {EVENT_CDU_N1_LIMIT, EVT_CDU_L_N1_LIMIT}, {EVENT_CDU_FIX, EVT_CDU_L_FIX},
    {EVENT_CDU_PREV_PAGE, EVT_CDU_L_PREV_PAGE}, {EVENT_CDU_NEXT_PAGE, EVT_CDU_L_NEXT_PAGE}, {EVENT_CDU_1, EVT_CDU_L_1},
    {EVENT_CDU_2, EVT_CDU_L_2}, {EVENT_CDU_3, EVT_CDU_L_3}, {EVENT_CDU_4, EVT_CDU_L_4}, {EVENT_CDU_5, EVT_CDU_L_5},
    {EVENT_CDU_6, EVT_CDU_L_6}, {EVENT_CDU_7, EVT_CDU_L_7}, {EVENT_CDU_8, EVT_CDU_L_8}, {EVENT_CDU_9, EVT_CDU_L_9},
    {EVENT_CDU_0, EVT_CDU_L_0}, {EVENT_CDU_DOT, EVT_CDU_L_DOT}, {EVENT_CDU_PLUS_MINUS, EVT_CDU_L_PLUS_MINUS},
    {EVENT_CDU_A, EVT_CDU_L_A}, {EVENT_CDU_B, EVT_CDU_L_B}, {EVENT_CDU_C, EVT_CDU_L_C}, {EVENT_CDU_D, EVT_CDU_L_D},
    {EVENT_CDU_E, EVT_CDU_L_E}, {EVENT_CDU_F, EVT_CDU_L_F}, {EVENT_CDU_G, EVT_CDU_L_G}, {EVENT_CDU_H, EVT_CDU_L_H},
    {EVENT_CDU_I, EVT_CDU_L_I}, {EVENT_CDU_J, EVT_CDU_L_J}, {EVENT_CDU_K, EVT_CDU_L_K}, {EVENT_CDU_L, EVT_CDU_L_L},
    {EVENT_CDU_M, EVT_CDU_L_M}, {EVENT_CDU_N, EVT_CDU_L_N}, {EVENT_CDU_O, EVT_CDU_L_O}, {EVENT_CDU_P, EVT_CDU_L_P},
    {EVENT_CDU_Q, EVT_CDU_L_Q}, {EVENT_CDU_R, EVT_CDU_L_R}, {EVENT_CDU_S, EVT_CDU_L_S}, {EVENT_CDU_T, EVT_CDU_L_T},
    {EVENT_CDU_U, EVT_CDU_L_U}, {EVENT_CDU_V, EVT_CDU_L_V}, {EVENT_CDU_W, EVT_CDU_L_W}, {EVENT_CDU_X, EVT_CDU_L_X},
    {EVENT_CDU_Y, EVT_CDU_L_Y}, {EVENT_CDU_Z, EVT_CDU_L_Z}, {EVENT_CDU_SP, EVT_CDU_L_SPACE},
    {EVENT_CDU_DEL, EVT_CDU_L_DEL}, {EVENT_CDU_SLASH, EVT_CDU_L_SLASH}, {EVENT_CDU_CLR, EVT_CDU_L_CLR}

};

std::map<std::string, EVENT_ID> ButtonsMap = {{"L1", EVENT_CDU_L_L1}, {"L2", EVENT_CDU_L_L2}, {"L3", EVENT_CDU_L_L3},
    {"L4", EVENT_CDU_L_L4}, {"L5", EVENT_CDU_L_L5}, {"L6", EVENT_CDU_L_L6}, {"R1", EVENT_CDU_L_R1},
    {"R2", EVENT_CDU_L_R2}, {"R3", EVENT_CDU_L_R3}, {"R4", EVENT_CDU_L_R4}, {"R5", EVENT_CDU_L_R5},
    {"R6", EVENT_CDU_L_R6}, {"INIT,REF", EVENT_CDU_INIT_REF}, {"RTE", EVENT_CDU_RTE}, {"CLB", EVENT_CDU_CLB},
    {"CRZ", EVENT_CDU_CRZ}, {"DES", EVENT_CDU_DES}, {"MENU", EVENT_CDU_MENU}, {"LEGS", EVENT_CDU_LEGS},
    {"DEP,ARR", EVENT_CDU_DEP_ARR}, {"HOLD", EVENT_CDU_HOLD}, {"PROG", EVENT_CDU_PROG}, {"EXEC", EVENT_CDU_EXEC},
    {"N1,LIMIT", EVENT_CDU_N1_LIMIT}, {"FIX", EVENT_CDU_FIX}, {"PREV,PAGE", EVENT_CDU_PREV_PAGE},
    {"NEXT,PAGE", EVENT_CDU_NEXT_PAGE}, {"1", EVENT_CDU_1}, {"2", EVENT_CDU_2}, {"3", EVENT_CDU_3}, {"4", EVENT_CDU_4},
    {"5", EVENT_CDU_5}, {"6", EVENT_CDU_6}, {"7", EVENT_CDU_7}, {"8", EVENT_CDU_8}, {"9", EVENT_CDU_9},
    {"0", EVENT_CDU_0}, {".", EVENT_CDU_DOT}, {"+/-", EVENT_CDU_PLUS_MINUS}, {"A", EVENT_CDU_A}, {"B", EVENT_CDU_B},
    {"C", EVENT_CDU_C}, {"D", EVENT_CDU_D}, {"E", EVENT_CDU_E}, {"F", EVENT_CDU_F}, {"G", EVENT_CDU_G},
    {"H", EVENT_CDU_H}, {"I", EVENT_CDU_I}, {"J", EVENT_CDU_J}, {"K", EVENT_CDU_K}, {"L", EVENT_CDU_L},
    {"M", EVENT_CDU_M}, {"N", EVENT_CDU_N}, {"O", EVENT_CDU_O}, {"P", EVENT_CDU_P}, {"Q", EVENT_CDU_Q},
    {"R", EVENT_CDU_R}, {"S", EVENT_CDU_S}, {"T", EVENT_CDU_T}, {"U", EVENT_CDU_U}, {"V", EVENT_CDU_V},
    {"W", EVENT_CDU_W}, {"X", EVENT_CDU_X}, {"Y", EVENT_CDU_Y}, {"Z", EVENT_CDU_Z}, {"SP", EVENT_CDU_SP},
    {"DEL", EVENT_CDU_DEL}, {"/", EVENT_CDU_SLASH}, {"CLR", EVENT_CDU_CLR}};
} // namespace

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

        rapidjson::Value ToJson(rapidjson::Document::AllocatorType& a)
        {
            rapidjson::Value res(rapidjson::kObjectType);
            if (text_ <= 125) {
                rapidjson::Value value(std::string(1, text_), a);
                res.AddMember("text", value, a);
            } else {
                res.AddMember("text", static_cast<int>(text_), a);
            }
            if (small_) {
                res.AddMember("small", true, a);
            }
            if (reverse_) {
                res.AddMember("reverse", true, a);
            }
            if (unused_) {
                res.AddMember("unused", true, a);
            }
            if (color_ != Color::WHITE) {
                res.AddMember("color", colorName(color_), a);
            }

            return res;
        }

    private:
        std::string colorName(Color color)
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

    void Assign(const PMDG_NG3_CDU_Screen* pmdgScreen, rapidjson::Writer<rapidjson::StringBuffer>* writer = nullptr)
    {
        rapidjson::Document d;
        d.SetObject();
        auto& a = d.GetAllocator();

        for (int y = 0; y < CDU_ROWS; ++y) {
            rapidjson::Value row(rapidjson::kObjectType);
            for (int x = 0; x < CDU_COLUMNS; ++x) {
                if (pmdgScreen) {
                    const PMDG_NG3_CDU_Cell& pmdgCell = pmdgScreen->Cells[x][y];
                    Cell c(pmdgCell);
                    if (c == cells_[y][x]) {
                        continue;
                    }
                    cells_[y][x] = c;
                }
                rapidjson::Value key(std::to_string(x), a);
                row.AddMember(key, cells_[y][x].ToJson(a), a);
            }
            rapidjson::Value colKey(std::to_string(y), a);
            d.AddMember(colKey, row, a);
        }
        if (writer) {
            d.Accept(*writer);
        }
    }

private:
    std::array<std::array<Cell, CDU_COLUMNS>, CDU_ROWS> cells_;
};

MSFSScreenSource::MSFSScreenSource()
    : screen_(std::make_unique<Screen>())
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

    // Sign up for notification of data change.
    // SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED flag asks for the data to be sent only when some of the data is changed.
    hr = SimConnect_RequestClientData(hSimConnect_, PMDG_NG3_CDU_0_ID, CDU_DATA_REQUEST, PMDG_NG3_CDU_0_DEFINITION,
        SIMCONNECT_CLIENT_DATA_PERIOD_VISUAL_FRAME, SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED, 0, 0, 0);
    //// End init simconnect
    while (running_) {
        SIMCONNECT_RECV* pData = nullptr;
        DWORD cbData = 0;

        HRESULT hr = SimConnect_GetNextDispatch(hSimConnect_, &pData, &cbData);
        bool success = false;
        PMDG_NG3_CDU_Screen* screen = nullptr;
        if (SUCCEEDED(hr)) {
            switch (pData->dwID) {
                case SIMCONNECT_RECV_ID_CLIENT_DATA: // Receive and save the NG3 CDU screen data block
                {
                    SIMCONNECT_RECV_CLIENT_DATA* pObjData = (SIMCONNECT_RECV_CLIENT_DATA*)pData;

                    switch (pObjData->dwRequestID) {
                        case CDU_DATA_REQUEST: {
                            screen = (PMDG_NG3_CDU_Screen*)&pObjData->dwData;
                            success = true;
                            break;
                        }
                    }
                    break;
                }
            }
        }
        if (!success) {
            if (invalidated_) {
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                screen_->Assign(nullptr, &writer);
                f(buffer.GetString());
                invalidated_ = false;
            }
            Sleep(50);
            continue;
        }
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        screen_->Assign(screen, &writer);

        f(buffer.GetString());
        Sleep(50);
    }
}

void MSFSScreenSource::Test()
{
    int parameter = 1;
    SimConnect_TransmitClientEvent(hSimConnect_, 0, EVENT_CDU_L_NEXT_PAGE, parameter, SIMCONNECT_GROUP_PRIORITY_HIGHEST,
        SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
}

void MSFSScreenSource::ButtonClick(std::string btn)
{
    if (ButtonsMap.count(btn) == 0) {
        spdlog::error("Unknown button: {}", btn);
        return;
    }
    int parameter = 1;
    auto hr = SimConnect_TransmitClientEvent(hSimConnect_, 0, ButtonsMap.at(btn), parameter,
        SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
    if (FAILED(hr)) {
        spdlog::error("Result: {}", hr);
    }
}

void MSFSScreenSource::Invalidate()
{
    invalidated_ = true;
}
} // namespace FMC_Server